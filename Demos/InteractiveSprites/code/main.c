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
	#include <ext2/fs_ext2.h>
#endif

#if CRAYON_BOOT_MODE == 1
	#define MNT_MODE FS_EXT2_MOUNT_READONLY

	static void unmount_ext2_sd(){
		fs_ext2_unmount("/sd");
		fs_ext2_shutdown();
		sd_shutdown();
	}

	static int mount_ext2_sd(){
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

		// Check to see if the MBR says that we have a Linux partition
		if(partition_type != 0x83){
			return 3;
		}

		// Initialize fs_ext2 and attempt to mount the device
		if(fs_ext2_init()){
			return 4;
		}

		//Mount the SD card to the sd dir in the VFS
		if(fs_ext2_mount("/sd", &sd_dev, MNT_MODE)){
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

void set_msg(char * buffer, uint8_t code){
	switch(code){
		case 0:
			strcpy(buffer, "Hii!");
			break;
		default:
			strcpy(buffer, "Unknown code given");
	}
	return;
}

void set_msg_option(char * buffer, uint8_t code){
	sprintf(buffer, "Current Option: %d", code);
	return;
}
void set_msg_sprite(char * buffer, uint8_t code){
	sprintf(buffer, "Current Sprite: %d", code);
	return;
}

void set_screen(float * htz_adjustment){
	*htz_adjustment = 1.0;
	uint8_t region = flashrom_get_region();
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
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	float htz_adjustment;
	set_screen(&htz_adjustment);

	//load in assets here
	crayon_spritesheet_t Faces_SS;
	crayon_sprite_array_t Faces_Draw[3];
	crayon_font_mono_t BIOS;
	crayon_palette_t Faces_P, BIOS_P;

	#if CRAYON_BOOT_MODE == 1
		crayon_memory_mount_romdisk("/sd/stuff.img", "/stuff");
	#else
		crayon_memory_mount_romdisk("/cd/stuff.img", "/stuff");
	#endif

	crayon_memory_load_spritesheet(&Faces_SS, &Faces_P, 0, "/stuff/opaque.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/stuff/BIOS.dtex");

	fs_romdisk_unmount("/stuff");

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_sprite_array(&Faces_Draw[0], &Faces_SS, 0, &Faces_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Faces_Draw[0].coord[0].x = 0;
	Faces_Draw[0].coord[0].y = 0;
	Faces_Draw[0].layer[0] = 50;
	Faces_Draw[0].scale[0].x = 4;
	Faces_Draw[0].scale[0].y = 4;
	Faces_Draw[0].flip[0] = 0;
	Faces_Draw[0].rotation[0] = 0;
	Faces_Draw[0].colour[0] = 0xFFFFFFFF;
	Faces_Draw[0].fade[0] = 0;
	Faces_Draw[0].frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Faces_Draw[0], 0, 0);

	crayon_memory_init_sprite_array(&Faces_Draw[1], &Faces_SS, 0, &Faces_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Faces_Draw[1].coord[0].x = 0;
	Faces_Draw[1].coord[0].y = 0;
	Faces_Draw[1].layer[0] = 50;
	Faces_Draw[1].scale[0].x = 4;
	Faces_Draw[1].scale[0].y = 4;
	Faces_Draw[1].flip[0] = 0;
	Faces_Draw[1].rotation[0] = 0;
	Faces_Draw[1].colour[0] = 0xFFFFFFFF;
	Faces_Draw[1].fade[0] = 0;
	Faces_Draw[1].frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Faces_Draw[1], 0, 0);

	crayon_memory_init_sprite_array(&Faces_Draw[2], &Faces_SS, 0, &Faces_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Faces_Draw[2].coord[0].x = 0;
	Faces_Draw[2].coord[0].y = 0;
	Faces_Draw[2].layer[0] = 50;
	Faces_Draw[2].scale[0].x = 4;
	Faces_Draw[2].scale[0].y = 4;
	Faces_Draw[2].flip[0] = 0;
	Faces_Draw[2].rotation[0] = 0;
	Faces_Draw[2].colour[0] = 0xFFFFFFFF;
	Faces_Draw[2].fade[0] = 0;
	Faces_Draw[2].frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Faces_Draw[2], 0, 0);

	uint8_t current_option = 0;
	uint8_t max_options = 10;
	uint8_t current_sprite = 0;

	char msg[200];
	char msg_option[30];
	char msg_sprite[30];
	set_msg(msg, 0);
	set_msg_option(msg_option, current_option);
	set_msg_sprite(msg_sprite, current_sprite);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&Faces_P);
	crayon_graphics_setup_palette(&BIOS_P);

	uint32_t prev_btns[4] = {0};
	vec2_u8_t prev_trigs[4] = {(vec2_u8_t){0,0}};
	while(1){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

			//Choose option
			if((st->ltrig > 0xFF * 0.1) && (prev_trigs[__dev->port].x <= 0xFF * 0.1)){
				if(current_option == 0){current_option = max_options - 1;}
				else{current_option--;}
				set_msg_option(msg_option, current_option);
			}
			if((st->rtrig > 0xFF * 0.1) && (prev_trigs[__dev->port].y <= 0xFF * 0.1)){
				if(current_option == max_options - 1){current_option = 0;}
				else{current_option++;}
				set_msg_option(msg_option, current_option);
			}

			//Choose sprite
			if((st->buttons & CONT_A) && !(prev_btns[__dev->port] & CONT_A)){
				current_sprite++;
				current_sprite %= 3;
				set_msg_sprite(msg_sprite, current_sprite);
			}

			//Store the buttons and triggers for next loop
			prev_btns[__dev->port] = st->buttons;
			prev_trigs[__dev->port].x = st->ltrig;
			prev_trigs[__dev->port].y = st->rtrig;
		MAPLE_FOREACH_END()


		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&Faces_Draw[0], PVR_LIST_OP_POLY, CRAY_SCREEN_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Faces_Draw[1], PVR_LIST_OP_POLY, CRAY_SCREEN_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Faces_Draw[2], PVR_LIST_OP_POLY, CRAY_SCREEN_DRAW_ENHANCED);

			crayon_graphics_draw_text_mono(msg, &BIOS, PVR_LIST_OP_POLY, 32, 480 - (BIOS.char_height * 4), 255, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(msg_option, &BIOS, PVR_LIST_OP_POLY, 32, 480 - (BIOS.char_height * 3), 255, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(msg_sprite, &BIOS, PVR_LIST_OP_POLY, 32, 480 - (BIOS.char_height * 2), 255, 1, 1, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();
	}

	return 0;
}
