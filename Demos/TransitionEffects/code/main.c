//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For region and htz stuff
#include <dc/flashrom.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

#if CRAYON_BOOT_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <fat/fs_fat.h>
#endif


#if CRAYON_BOOT_MODE == 1
	#define MNT_MODE FS_FAT_MOUNT_READONLY

	static void unmount_fat_sd(){
		fs_fat_unmount("/sd");
		fs_fat_shutdown();
		sd_shutdown();
	}

	static int mount_fat_sd(){
		kos_blockdev_t sd_dev;
		uint8 partition_type;

		// Initialize the sd card if its present
		if(sd_init()){
			return 1;
		}

		// Grab the block device for the first partition on the SD card. Note that
		// you must have the SD card formatted with an MBR partitioning scheme
		if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
			return 2;
		}

		// Check to see if the MBR says that we have a valid partition
		// if(partition_type != 0x83){
			//I don't know what value I should be comparing against, hence this check is disabled for now
			// This: https://en.wikipedia.org/wiki/Partition_type
				//Suggests there's multiple types for FAT...not sure how to handle this
			// return 3;
		// }

		// Initialize fs_fat and attempt to mount the device
		if(fs_fat_init()){
			return 4;
		}

		//Mount the SD card to the sd dir in the VFS
		if(fs_fat_mount("/sd", &sd_dev, MNT_MODE)){
			return 5;
		}
		return 0;
	}

#endif

//Change this to only give room for PT list (Since other ones aren't used)
pvr_init_params_t pvr_params = {
		// Enable opaque, translucent and punch through polygons with size 16
			//To better explain, the opb_sizes or Object Pointer Buffer sizes
			//Work like this: Set to zero to disable. Otherwise the higher the
			//number the more space used (And more efficient under higher loads)
			//The lower the number, the less space used and less efficient under
			//high loads. You can choose 0, 8, 16 or 32
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },

		// Vertex buffer size 512K
		512 * 1024,

		// No DMA
		0,

		// No FSAA
		0,

		// Translucent Autosort enabled
		0
};

float width, height;

float g_deadspace;

uint8_t g_htz, g_htz_adjustment;
uint8_t vga_enabled;

void crayon_graphics_init_display(){
	pvr_init(&pvr_params);

	width = crayon_graphics_get_window_width();
	height = crayon_graphics_get_window_height();

	vga_enabled = (vid_check_cable() == CT_VGA);
	if(vga_enabled){
		vid_set_mode(DM_640x480_VGA, PM_RGB565);	//60Hz
		g_htz = 60;
		g_htz_adjustment = 1;
	}
	else{
		if(flashrom_get_region() == FLASHROM_REGION_EUROPE){
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
			g_htz = 50;
			g_htz_adjustment = 1.2;
		}
		else{
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
			g_htz = 60;
			g_htz_adjustment = 1;
		}
	}

	return;
}

void set_indexes(uint16_t * indexes, uint16_t size){
	#if CRAYON_BOOT_MODE == 0
		FILE * fp = fopen("cd/boxy_indexes.bin", "rb");
	#elif CRAYON_BOOT_MODE == 1
		FILE * fp = fopen("sd/boxy_indexes.bin", "rb");
	#elif CRAYON_BOOT_MODE == 2
		FILE * fp = fopen("pc/boxy_indexes.bin", "rb");
	#else
		#error "Invalid CRAYON_BOOT_MODE"
	#endif

	fread(indexes, sizeof(uint16_t), size, fp);
	fclose(fp);
	return;
}

void modify_fade_effect(crayon_transition_t * effect, void * params){
	if(params != NULL){return;}	//This only exists to make the compiler shut up
	effect->draw->colour[0] = crayon_assist_extract_bits(effect->draw->colour[0], 24, 0) +
		((uint8_t)(crayon_graphics_transition_get_curr_percentage(effect) * 255) << 24);
	return;
}

//To do the effect I'll just modify how many boxes are visible
void modify_boxy_effect(crayon_transition_t * effect, void * params){
	if(params != NULL){return;}	//This only exists to make the compiler shut up
	uint16_t num_visible = crayon_graphics_transition_get_curr_percentage(effect) * effect->draw->list_size;
	uint16_t i;
	for(i = 0; i < effect->draw->list_size; i++){
		effect->draw->visible[i] = (i < num_visible) ? 1 : 0;
	}
	return;
}

//These two get modified together
void modify_flash_effect(crayon_transition_t * effect, void * params){
	if(params != NULL || effect == NULL){return;}	//This only exists to make the compiler shut up
	return;
}

void modify_flower_effect(crayon_transition_t * effect, void * params){
	if(params != NULL || effect == NULL){return;}	//This only exists to make the compiler shut up
	return;
}

int main(){
	#if CRAYON_BOOT_MODE == 1
		int sdRes = mount_fat_sd();	//This function should be able to mount a FAT32 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	g_deadspace = 0.4;

	crayon_graphics_init_display();
	// htz_select();

	uint16_t i, j;

	crayon_sprite_array_t scene_draw;
	crayon_memory_init_sprite_array(&scene_draw, NULL, 0, NULL, 4, 0,
		CRAY_MULTI_DIM | CRAY_MULTI_ROTATE | CRAY_MULTI_COLOUR, PVR_FILTER_NONE, 0);
	for(i = 0; i < 20; i++){
		scene_draw.visible[i] = 1;
	}

	//Roof
	scene_draw.scale[0].x = 200;
	scene_draw.scale[0].y = 200;
	scene_draw.coord[0].x = (width - scene_draw.scale[0].x) / 2;
	scene_draw.coord[0].y = ((height - scene_draw.scale[0].y) / 2) - 50;
	scene_draw.layer[0] = 100;
	scene_draw.colour[0] = 0xFF344151;
	scene_draw.rotation[0] = 45;

	//House body
	scene_draw.scale[1].x = 270;
	scene_draw.scale[1].y = 210;
	scene_draw.coord[1].x = (width - scene_draw.scale[1].x) / 2;
	scene_draw.coord[1].y = scene_draw.coord[0].y + (scene_draw.scale[0].y / 2) + 5;
	scene_draw.layer[1] = 101;
	scene_draw.colour[1] = 0xFFFFF8F4;
	scene_draw.rotation[1] = 0;

	//Door
	scene_draw.scale[2].x = 50;
	scene_draw.scale[2].y = 80;
	scene_draw.coord[2].x = (width - scene_draw.scale[2].x) / 2;
	scene_draw.coord[2].y = scene_draw.coord[1].y + scene_draw.scale[1].y - scene_draw.scale[2].y;
	scene_draw.layer[2] = 102;
	scene_draw.colour[2] = 0xFF7F3300;
	scene_draw.rotation[2] = 0;

	//Door knob
	scene_draw.scale[3].x = 4;
	scene_draw.scale[3].y = 4;
	scene_draw.coord[3].x = scene_draw.coord[2].x + scene_draw.scale[2].x - scene_draw.scale[3].x - 5;
	scene_draw.coord[3].y = scene_draw.coord[2].y + ((scene_draw.scale[2].y - scene_draw.scale[3].y) / 2);
	scene_draw.layer[3] = 103;
	scene_draw.colour[3] = 0xFFFFFF00;
	scene_draw.rotation[3] = 0;


	//---------------------------------------------------


	crayon_sprite_array_t fade_draw;
	crayon_sprite_array_t boxy_draw;
	crayon_sprite_array_t multi_draw[3];

	crayon_transition_t effect[3];
	uint8_t curr_effect = 1;

	crayon_memory_init_sprite_array(&fade_draw, NULL, 0, NULL, 1, 0, 0, PVR_FILTER_NONE, 0);
	fade_draw.coord[0].x = 0;
	fade_draw.coord[0].y = 0;
	fade_draw.scale[0].x = width;
	fade_draw.scale[0].y = height;
	fade_draw.fade[0] = 250;
	fade_draw.colour[0] = 0xFFFF0000;
	fade_draw.rotation[0] = 0;
	fade_draw.visible[0] = 1;
	fade_draw.layer[0] = 250;

	vec2_u16_t num_boxes_dims = (vec2_u16_t){width / 32, height / 32};	//20 and 15
	uint16_t num_boxes = num_boxes_dims.x * num_boxes_dims.y;
	uint16_t * indexes = malloc(sizeof(uint16_t) * num_boxes);
	set_indexes(indexes, num_boxes);
	crayon_memory_init_sprite_array(&boxy_draw, NULL, 0, NULL, num_boxes, 0, 0, PVR_FILTER_NONE, 0);
	for(j = 0; j < height / 32; j++){
		for(i = 0; i < width / 32; i++){
			boxy_draw.coord[indexes[i + (j * num_boxes_dims.x)]].x = (i * 32);
			boxy_draw.coord[indexes[i + (j * num_boxes_dims.x)]].y = (j * 32);
			boxy_draw.visible[indexes[i + (j * num_boxes_dims.x)]] = 1;
			boxy_draw.layer[indexes[i + (j * num_boxes_dims.x)]] = 250;
		}
	}
	free(indexes);
	boxy_draw.scale[0].x = 32;
	boxy_draw.scale[0].y = 32;
	boxy_draw.fade[0] = 250;
	boxy_draw.colour[0] = 0xFF000000;
	boxy_draw.rotation[0] = 0;

	crayon_graphics_transistion_init(&effect[0], &fade_draw, modify_fade_effect, 5 * g_htz, 1 * g_htz);	//5 seconds to fade in, 1 to fade out
	crayon_graphics_transistion_init(&effect[1], &boxy_draw, modify_boxy_effect, 2 * g_htz, 2 * g_htz);

	//Fade out and fade in are reversed

	//We want them both to start off as faded out (This is redundant due to below, but anyways)
	crayon_graphics_transistion_skip_to_state(&effect[0], NULL, CRAY_FADE_STATE_OUT);
	crayon_graphics_transistion_skip_to_state(&effect[1], NULL, CRAY_FADE_STATE_OUT);

	//Start the transition
	crayon_graphics_transistion_change_state(&effect[curr_effect], CRAY_FADE_STATE_IN);

	//For the 3rd effect
	crayon_sprite_array_t letter_box_draw;
	uint16_t widescreen_height = (2 * 180);
	crayon_memory_init_sprite_array(&letter_box_draw, NULL, 0, NULL, 2, 0, 0, PVR_FILTER_NONE, 0);
	letter_box_draw.scale[0].x = width;
	letter_box_draw.scale[0].y = (height - widescreen_height) / 2;
	letter_box_draw.colour[0] = 0xFF000000;
	letter_box_draw.rotation[0] = 0;

	letter_box_draw.coord[0].x = 0;
	letter_box_draw.coord[0].y = 0;
	letter_box_draw.visible[0] = 1;
	letter_box_draw.layer[0] = 254;
	letter_box_draw.coord[1].x = 0;
	letter_box_draw.coord[1].y = letter_box_draw.scale[0].y + widescreen_height;
	letter_box_draw.visible[1] = 1;
	letter_box_draw.layer[1] = 254;

	//The camera
	crayon_viewport_t Camera;
	crayon_memory_init_camera(&Camera, (vec2_f_t){0, 0}, (vec2_u16_t){width, height},
		(vec2_u16_t){0, 0}, (vec2_u16_t){width, height}, 1);

	//load in assets here
	crayon_font_mono_t BIOS_font;
	crayon_palette_t BIOS_P;		//Entry 0
	
	#if CRAYON_BOOT_MODE == 0
		crayon_memory_mount_romdisk("/cd/stuff.img", "/Setup");
	#elif CRAYON_BOOT_MODE == 1
		crayon_memory_mount_romdisk("/sd/stuff.img", "/Setup");
	#elif CRAYON_BOOT_MODE == 2
		crayon_memory_mount_romdisk("/pc/stuff.img", "/Setup");
	#else
		#error "Invalid CRAYON_BOOT_MODE"
	#endif


	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 0, "/Setup/BIOS.dtex");

	fs_romdisk_unmount("/Setup");

	char debug[100];

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging
	crayon_graphics_setup_palette(&BIOS_P);

	#if CRAYON_BOOT_MODE == 1
		unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	uint32_t curr_btns[4] = {0};
	uint32_t prev_btns[4] = {0};
	while(1){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			prev_btns[__dev->port] = curr_btns[__dev->port];
			curr_btns[__dev->port] = st->buttons;
		MAPLE_FOREACH_END()

		for(i = 0; i < 4; i++){
			//Press A to fade in
			if((curr_btns[i] & CONT_A) && !(prev_btns[i] & CONT_A)){
				if(effect[curr_effect].resting_state == CRAY_FADE_STATE_RESTING_OUT){
					crayon_graphics_transistion_change_state(&effect[curr_effect], CRAY_FADE_STATE_IN);
				}
			}

			//Press B to fade out
			if((curr_btns[i] & CONT_B) && !(prev_btns[i] & CONT_B)){
				if(effect[curr_effect].resting_state == CRAY_FADE_STATE_RESTING_IN){
					crayon_graphics_transistion_change_state(&effect[curr_effect], CRAY_FADE_STATE_OUT);
				}
			}

			if((curr_btns[i] & CONT_X) && !(prev_btns[i] & CONT_X)){
				curr_effect++;
				if(curr_effect >= 2){
					curr_effect = 0;
				}
			}
		}

		//Apply the effect
		crayon_graphics_transistion_apply(&effect[curr_effect], NULL);

		if(curr_effect == 2){
			Camera.window_y = (height - widescreen_height) / 2;
			Camera.window_height = widescreen_height;
		}
		else{
			Camera.window_y = 0;
			Camera.window_height = height;
		}

		pvr_wait_ready();

		pvr_scene_begin();


		pvr_list_begin(PVR_LIST_TR_POLY);
			if(curr_effect == 0){
				crayon_graphics_draw_sprites(&fade_draw, NULL, PVR_LIST_TR_POLY, CRAY_DRAW_SIMPLE);
			}
		pvr_list_finish();


		pvr_list_begin(PVR_LIST_OP_POLY);
			if(curr_effect == 1){
				crayon_graphics_draw_sprites(&boxy_draw, NULL, PVR_LIST_OP_POLY, CRAY_DRAW_SIMPLE);
			}
			else if(curr_effect == 2){
				crayon_graphics_draw_sprites(&letter_box_draw, NULL, PVR_LIST_OP_POLY, CRAY_DRAW_SIMPLE);
			}

			crayon_graphics_draw_sprites(&scene_draw, &Camera, PVR_LIST_OP_POLY, CRAY_DRAW_ENHANCED);

			sprintf(debug, "State: %d %d. Progress %d, %d %d", effect[curr_effect].curr_state, effect[curr_effect].resting_state,
				effect[curr_effect].curr_duration, effect[curr_effect].duration_fade_in, effect[curr_effect].duration_fade_out);
			// sprintf(debug, "Colour: 0x%x", fade_draw.colour[0]);
			// sprintf(debug, "Press A to fade in, B to fade out and X to swap effects");
			crayon_graphics_draw_text_mono(debug, &BIOS_font, PVR_LIST_OP_POLY, 10, 10, 255, 1, 1, BIOS_P.palette_id);

		pvr_list_finish();


		pvr_list_begin(PVR_LIST_PT_POLY);
			if(curr_effect == 2){
				;
			}
		pvr_list_finish();


		pvr_scene_finish();
	}

	crayon_memory_free_sprite_array(&scene_draw);
	crayon_memory_free_sprite_array(&fade_draw);
	crayon_memory_free_sprite_array(&boxy_draw);

	// crayon_memory_free_sprite_array(&multi_draw[0]);
	// crayon_memory_free_sprite_array(&multi_draw[1]);
	// crayon_memory_free_sprite_array(&multi_draw[2]);

	crayon_memory_free_sprite_array(&letter_box_draw);

	return 0;
}
