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

void set_screen(float * htz_adjustment){
	*htz_adjustment = 1.0;
	int8_t region = flashrom_get_region();
	if(region < 0){	//If error we just default to green swirl. Apparently its possible for some DCs to return -1
		region = 0;	//Invalid region
	}

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		if(region == FLASHROM_REGION_EUROPE){
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
			*htz_adjustment = 1.2;	//60/50Hz
		}
		else{
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
		}
	}

	return;
}

int main(){
	pvr_init(&pvr_params);	//Init the pvr system

	#if CRAYON_BOOT_MODE == 1
		int sdRes = mount_fat_sd();	//This function should be able to mount a FAT32 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	float htz_adjustment;
	set_screen(&htz_adjustment);

	srand(time(0));	//Set the seed for rand()

	crayon_spritesheet_t Dwarf_SS;
	crayon_sprite_array_t Dwarf_Draw_Flip, Dwarf_Draw_Rotate, Dwarf_Draw_Scale, Dwarf_Draw_Frame,
		Dwarf_Draw_Colour_Blend, Dwarf_Draw_Colour_Add, Dwarf_Draw_Mash;
	crayon_font_mono_t BIOS;
	crayon_palette_t BIOS_P;

	//Load the logo
	#if CRAYON_BOOT_MODE == 2
		crayon_memory_mount_romdisk("/pc/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == 1
		crayon_memory_mount_romdisk("/sd/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == 0
		crayon_memory_mount_romdisk("/cd/stuff.img", "/files");
	#else
		#error Invalid CRAYON_BOOT_MODE value
	#endif

	//Load the asset
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 0, "/files/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Dwarf_SS, NULL, -1, "/files/sprite.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_sprite_array(&Dwarf_Draw_Flip, &Dwarf_SS, 0, NULL, 2, 1, CRAYON_MULTI_FLIP, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Flip.scale[0].x = 1;
	Dwarf_Draw_Flip.scale[0].y = 1;

	Dwarf_Draw_Flip.coord[0].x = 32;
	Dwarf_Draw_Flip.coord[0].y = 32;
	Dwarf_Draw_Flip.coord[1].x = Dwarf_Draw_Flip.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Flip, 0) + 32;
	Dwarf_Draw_Flip.coord[1].y = Dwarf_Draw_Flip.coord[0].y;

	Dwarf_Draw_Flip.layer[0] = 2;
	Dwarf_Draw_Flip.layer[1] = 2;
	Dwarf_Draw_Flip.flip[0] = 0;
	Dwarf_Draw_Flip.flip[1] = 1;
	Dwarf_Draw_Flip.rotation[0] = 0;
	Dwarf_Draw_Flip.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Flip.fade[0] = 0;
	Dwarf_Draw_Flip.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Flip, 0, 0);
	uint8_t i;
	for(i = 0; i < Dwarf_Draw_Flip.size; i++){
		Dwarf_Draw_Flip.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Rotate, &Dwarf_SS, 0, NULL, 4, 1, CRAYON_MULTI_ROTATE, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Rotate.scale[0].x = 1;
	Dwarf_Draw_Rotate.scale[0].y = 1;

	Dwarf_Draw_Rotate.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Rotate.coord[0].y = Dwarf_Draw_Flip.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Flip, 0) + 32;
	Dwarf_Draw_Rotate.coord[1].x = Dwarf_Draw_Rotate.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Rotate, 0) + 32;
	Dwarf_Draw_Rotate.coord[1].y = Dwarf_Draw_Rotate.coord[0].y;
	Dwarf_Draw_Rotate.coord[2].x = Dwarf_Draw_Rotate.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Rotate, 0) + 32;
	Dwarf_Draw_Rotate.coord[2].y = Dwarf_Draw_Rotate.coord[1].y;
	Dwarf_Draw_Rotate.coord[3].x = Dwarf_Draw_Rotate.coord[2].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Rotate, 0) + 32;
	Dwarf_Draw_Rotate.coord[3].y = Dwarf_Draw_Rotate.coord[2].y;

	Dwarf_Draw_Rotate.layer[0] = 2;
	Dwarf_Draw_Rotate.layer[1] = 2;
	Dwarf_Draw_Rotate.layer[2] = 2;
	Dwarf_Draw_Rotate.layer[3] = 2;
	Dwarf_Draw_Rotate.flip[0] = 0;
	Dwarf_Draw_Rotate.rotation[0] = 0;
	Dwarf_Draw_Rotate.rotation[1] = 81;
	Dwarf_Draw_Rotate.rotation[2] = 199;
	Dwarf_Draw_Rotate.rotation[3] = 226;
	Dwarf_Draw_Rotate.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Rotate.fade[0] = 0;
	Dwarf_Draw_Rotate.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Rotate, 0, 0);
	for(i = 0; i < Dwarf_Draw_Rotate.size; i++){
		Dwarf_Draw_Rotate.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Scale, &Dwarf_SS, 0, NULL, 3, 1, CRAYON_MULTI_SCALE, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Scale.scale[0].x = 1;
	Dwarf_Draw_Scale.scale[0].y = 1;
	Dwarf_Draw_Scale.scale[1].x = 0.5;
	Dwarf_Draw_Scale.scale[1].y = 0.5;
	Dwarf_Draw_Scale.scale[2].x = 2;
	Dwarf_Draw_Scale.scale[2].y = 2;

	Dwarf_Draw_Scale.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Scale.coord[0].y = Dwarf_Draw_Rotate.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Scale, 0) + 32;
	Dwarf_Draw_Scale.coord[1].x = Dwarf_Draw_Scale.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Scale, 0) + 32;
	Dwarf_Draw_Scale.coord[1].y = Dwarf_Draw_Scale.coord[0].y;
	Dwarf_Draw_Scale.coord[2].x = Dwarf_Draw_Scale.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Scale, 0);
	Dwarf_Draw_Scale.coord[2].y = Dwarf_Draw_Scale.coord[0].y;

	Dwarf_Draw_Scale.layer[0] = 2;
	Dwarf_Draw_Scale.layer[1] = 2;
	Dwarf_Draw_Scale.layer[2] = 2;
	Dwarf_Draw_Scale.flip[0] = 0;
	Dwarf_Draw_Scale.rotation[0] = 0;
	Dwarf_Draw_Scale.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Scale.fade[0] = 0;
	Dwarf_Draw_Scale.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Scale, 0, 0);
	for(i = 0; i < Dwarf_Draw_Scale.size; i++){
		Dwarf_Draw_Scale.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Frame, &Dwarf_SS, 0, NULL, 3, 2, CRAYON_MULTI_FRAME, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Frame.scale[0].x = 1;
	Dwarf_Draw_Frame.scale[0].y = 1;

	Dwarf_Draw_Frame.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Frame.coord[0].y = Dwarf_Draw_Scale.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Frame, 0) + 32 + 6;	//6 because of scale over-stretching
	Dwarf_Draw_Frame.coord[1].x = Dwarf_Draw_Frame.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Frame, 0) + 32;
	Dwarf_Draw_Frame.coord[1].y = Dwarf_Draw_Frame.coord[0].y;
	Dwarf_Draw_Frame.coord[2].x = Dwarf_Draw_Frame.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Frame, 0) + 32;
	Dwarf_Draw_Frame.coord[2].y = Dwarf_Draw_Frame.coord[0].y;

	Dwarf_Draw_Frame.layer[0] = 2;
	Dwarf_Draw_Frame.layer[1] = 2;
	Dwarf_Draw_Frame.layer[2] = 2;
	Dwarf_Draw_Frame.flip[0] = 0;
	Dwarf_Draw_Frame.rotation[0] = 0;
	Dwarf_Draw_Frame.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Frame.fade[0] = 0;
	Dwarf_Draw_Frame.frame_id[0] = 0;
	Dwarf_Draw_Frame.frame_id[1] = 1;
	Dwarf_Draw_Frame.frame_id[2] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Frame, 0, 0);
	crayon_memory_set_frame_uv(&Dwarf_Draw_Frame, 1, 1);
	for(i = 0; i < Dwarf_Draw_Frame.size; i++){
		Dwarf_Draw_Frame.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Colour_Blend, &Dwarf_SS, 0, NULL, 3, 1, CRAYON_MULTI_COLOUR_BLEND, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Colour_Blend.scale[0].x = 1;
	Dwarf_Draw_Colour_Blend.scale[0].y = 1;

	Dwarf_Draw_Colour_Blend.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Colour_Blend.coord[0].y = Dwarf_Draw_Frame.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Colour_Blend, 0) + 32;
	Dwarf_Draw_Colour_Blend.coord[1].x = Dwarf_Draw_Colour_Blend.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Blend, 0) + 32;
	Dwarf_Draw_Colour_Blend.coord[1].y = Dwarf_Draw_Colour_Blend.coord[0].y;
	Dwarf_Draw_Colour_Blend.coord[2].x = Dwarf_Draw_Colour_Blend.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Blend, 0) + 32;
	Dwarf_Draw_Colour_Blend.coord[2].y = Dwarf_Draw_Colour_Blend.coord[0].y;

	Dwarf_Draw_Colour_Blend.layer[0] = 2;
	Dwarf_Draw_Colour_Blend.layer[1] = 2;
	Dwarf_Draw_Colour_Blend.layer[2] = 2;

	Dwarf_Draw_Colour_Blend.colour[0] = 0xFF00FF00;	//Green
	Dwarf_Draw_Colour_Blend.fade[0] = 0;
	Dwarf_Draw_Colour_Blend.colour[1] = 0xFF00FF00;
	Dwarf_Draw_Colour_Blend.fade[1] = 128;
	Dwarf_Draw_Colour_Blend.colour[2] = 0xFF00FF00;
	Dwarf_Draw_Colour_Blend.fade[2] = 255;

	Dwarf_Draw_Colour_Blend.flip[0] = 0;
	Dwarf_Draw_Colour_Blend.rotation[0] = 0;
	Dwarf_Draw_Colour_Blend.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Colour_Blend, 0, 0);
	for(i = 0; i < Dwarf_Draw_Colour_Blend.size; i++){
		Dwarf_Draw_Colour_Blend.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Colour_Add, &Dwarf_SS, 0, NULL, 3, 1, CRAYON_MULTI_COLOUR_ADD, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Colour_Add.scale[0].x = 1;
	Dwarf_Draw_Colour_Add.scale[0].y = 1;

	Dwarf_Draw_Colour_Add.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Colour_Add.coord[0].y = Dwarf_Draw_Colour_Blend.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Colour_Add, 0) + 32;
	Dwarf_Draw_Colour_Add.coord[1].x = Dwarf_Draw_Colour_Add.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Add, 0) + 32;
	Dwarf_Draw_Colour_Add.coord[1].y = Dwarf_Draw_Colour_Add.coord[0].y;
	Dwarf_Draw_Colour_Add.coord[2].x = Dwarf_Draw_Colour_Add.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Add, 0) + 32;
	Dwarf_Draw_Colour_Add.coord[2].y = Dwarf_Draw_Colour_Add.coord[0].y;

	Dwarf_Draw_Colour_Add.layer[0] = 2;
	Dwarf_Draw_Colour_Add.layer[1] = 2;
	Dwarf_Draw_Colour_Add.layer[2] = 2;

	Dwarf_Draw_Colour_Add.colour[0] = 0xFF0000FF;	//Blue
	Dwarf_Draw_Colour_Add.fade[0] = 0;
	Dwarf_Draw_Colour_Add.colour[1] = 0xFF0000FF;
	Dwarf_Draw_Colour_Add.fade[1] = 128;
	Dwarf_Draw_Colour_Add.colour[2] = 0xFF0000FF;
	Dwarf_Draw_Colour_Add.fade[2] = 255;

	Dwarf_Draw_Colour_Add.flip[0] = 0;
	Dwarf_Draw_Colour_Add.rotation[0] = 0;
	Dwarf_Draw_Colour_Add.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Colour_Add, 0, 0);
	for(i = 0; i < Dwarf_Draw_Colour_Add.size; i++){
		Dwarf_Draw_Colour_Add.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Mash, &Dwarf_SS, 0, NULL, 6, 2,
		CRAYON_MULTI_FRAME | CRAYON_MULTI_FLIP | CRAYON_MULTI_ROTATE, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Mash.scale[0].x = 1;
	Dwarf_Draw_Mash.scale[0].y = 1;

	Dwarf_Draw_Mash.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Mash.coord[0].y = Dwarf_Draw_Colour_Add.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[1].x = Dwarf_Draw_Mash.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[1].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[2].x = Dwarf_Draw_Mash.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[2].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[3].x = Dwarf_Draw_Mash.coord[2].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[3].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[4].x = Dwarf_Draw_Mash.coord[3].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[4].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[5].x = Dwarf_Draw_Mash.coord[4].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[5].y = Dwarf_Draw_Mash.coord[0].y;

	Dwarf_Draw_Mash.layer[0] = 2;
	Dwarf_Draw_Mash.layer[1] = 2;
	Dwarf_Draw_Mash.layer[2] = 2;
	Dwarf_Draw_Mash.layer[3] = 2;
	Dwarf_Draw_Mash.layer[4] = 2;
	Dwarf_Draw_Mash.layer[5] = 2;

	Dwarf_Draw_Mash.flip[0] = 0;
	Dwarf_Draw_Mash.flip[1] = 1;
	Dwarf_Draw_Mash.flip[2] = 0;
	Dwarf_Draw_Mash.flip[3] = 1;
	Dwarf_Draw_Mash.flip[4] = 1;
	Dwarf_Draw_Mash.flip[5] = 0;

	Dwarf_Draw_Mash.rotation[0] = 0;
	Dwarf_Draw_Mash.rotation[1] = 167;
	Dwarf_Draw_Mash.rotation[2] = 51;
	Dwarf_Draw_Mash.rotation[3] = -113;
	Dwarf_Draw_Mash.rotation[4] = 218;
	Dwarf_Draw_Mash.rotation[5] = 299;

	Dwarf_Draw_Mash.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Mash.fade[0] = 0;

	Dwarf_Draw_Mash.frame_id[0] = 0;
	Dwarf_Draw_Mash.frame_id[1] = 1;
	Dwarf_Draw_Mash.frame_id[2] = 1;
	Dwarf_Draw_Mash.frame_id[3] = 1;
	Dwarf_Draw_Mash.frame_id[4] = 0;
	Dwarf_Draw_Mash.frame_id[5] = 1;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Mash, 0, 0);
	crayon_memory_set_frame_uv(&Dwarf_Draw_Mash, 1, 1);
	for(i = 0; i < Dwarf_Draw_Mash.size; i++){
		Dwarf_Draw_Mash.visible[i] = 1;
	}

	crayon_graphics_setup_palette(&BIOS_P);
	while(1){
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(CRAYON_PT_LIST);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Flip, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Rotate, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Scale, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Frame, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Colour_Blend, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Colour_Add, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Mash, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
		pvr_list_finish();

		pvr_list_begin(CRAYON_OP_LIST);
			crayon_graphics_draw_text_mono("Controls:", &BIOS, CRAYON_OP_LIST, 450, 32, 3, 2, 2, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Dwarf_Draw_Flip);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Rotate);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Scale);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Frame);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Colour_Blend);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Colour_Add);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Mash);

	crayon_memory_free_spritesheet(&Dwarf_SS);
	crayon_memory_free_mono_font_sheet(&BIOS);

	crayon_memory_free_palette(&BIOS_P);

	return 0;
}
