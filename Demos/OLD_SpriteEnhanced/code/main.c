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
	#define MNT_MODE FS_EXT2_MOUNT_READWRITE	//Might manually change it so its not a define anymore

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

int main(){
	#if CRAYON_BOOT_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	crayon_graphics_init(CRAYON_ENABLE_OP | CRAYON_ENABLE_TR | CRAYON_ENABLE_PT);

	crayon_spritesheet_t Man_SS, Opaque_SS;
	crayon_palette_t Man_P;
	crayon_sprite_array_t Man_Draw, Opaque_Blend_Draw, Opaque_Add_Draw;

	crayon_font_mono_t BIOS_Font;
	crayon_palette_t BIOS_P;

	crayon_memory_mount_romdisk("/pc/stuff.img", "/files");

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
	crayon_memory_load_spritesheet(&Man_SS, &Man_P, 0, "/files/Man.dtex");
	crayon_memory_load_spritesheet(&Opaque_SS, NULL, -1, "/files/Opaque.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS_Font, &BIOS_P, 6, "/files/Fonts/BIOS_font.dtex");	//REMOVE LATER

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_sprite_array(&Man_Draw, &Man_SS, 0, &Man_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Man_Draw.layer[0] = 2;
	Man_Draw.scale[0].x = 7;
	Man_Draw.scale[0].y = 7;
	Man_Draw.coord[0].x = (640 - crayon_graphics_get_draw_element_width(&Man_Draw, 0)) / 2.0f;
	Man_Draw.coord[0].y = (480 - crayon_graphics_get_draw_element_height(&Man_Draw, 0)) / 2.0f;;
	Man_Draw.flip[0] = 1;
	Man_Draw.rotation[0] = 0;
	Man_Draw.colour[0] = 0xFF0000FF;
	Man_Draw.fade[0] = 255;
	Man_Draw.frame_id[0] = 0;
	Man_Draw.visible[0] = 1;
	crayon_memory_set_frame_uv(&Man_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Opaque_Blend_Draw, &Opaque_SS, 0, NULL, 2, 1, CRAY_MULTI_COLOUR + CRAY_COLOUR_BLEND, PVR_FILTER_NONE, 0);
	Opaque_Blend_Draw.scale[0].x = 12;
	Opaque_Blend_Draw.scale[0].y = 12;
	Opaque_Blend_Draw.coord[0].x = 0;
	Opaque_Blend_Draw.coord[0].y = 0;
	Opaque_Blend_Draw.coord[1].x = 4 + crayon_graphics_get_draw_element_width(&Opaque_Blend_Draw, 0);
	Opaque_Blend_Draw.coord[1].y = Opaque_Blend_Draw.coord[0].y;
	Opaque_Blend_Draw.layer[0] = 1;
	Opaque_Blend_Draw.layer[1] = 1;
	Opaque_Blend_Draw.flip[0] = 0;
	Opaque_Blend_Draw.rotation[0] = 0;
	Opaque_Blend_Draw.colour[0] = 0xFF00FF00;
	Opaque_Blend_Draw.colour[1] = 0xFFFF0000;
	Opaque_Blend_Draw.fade[0] = 255;
	Opaque_Blend_Draw.fade[1] = 255;
	Opaque_Blend_Draw.frame_id[0] = 0;
	uint8_t i;
	for(i = 0; i < Opaque_Blend_Draw.size; i++){
		Opaque_Blend_Draw.visible[i] = 1;
	}
	crayon_memory_set_frame_uv(&Opaque_Blend_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Opaque_Add_Draw, &Opaque_SS, 0, NULL, 2, 1, CRAY_MULTI_COLOUR + CRAY_COLOUR_ADD, PVR_FILTER_NONE, 0);
	Opaque_Add_Draw.scale[0].x = 12;
	Opaque_Add_Draw.scale[0].y = 12;
	Opaque_Add_Draw.coord[0].x = 0;
	Opaque_Add_Draw.coord[0].y = 4 + crayon_graphics_get_draw_element_height(&Opaque_Blend_Draw, 0);
	Opaque_Add_Draw.coord[1].x = 4 + crayon_graphics_get_draw_element_width(&Opaque_Add_Draw, 0);
	Opaque_Add_Draw.coord[1].y = Opaque_Add_Draw.coord[0].y;
	Opaque_Add_Draw.layer[0] = 1;
	Opaque_Add_Draw.layer[1] = 1;
	Opaque_Add_Draw.flip[0] = 0;
	Opaque_Add_Draw.rotation[0] = 0;
	Opaque_Add_Draw.colour[0] = 0xFF00FF00;
	Opaque_Add_Draw.colour[1] = 0xFFFF0000;
	Opaque_Add_Draw.fade[0] = 255;
	Opaque_Add_Draw.fade[1] = 255;
	Opaque_Add_Draw.frame_id[0] = 0;
	for(i = 0; i < Opaque_Add_Draw.size; i++){
		Opaque_Add_Draw.visible[i] = 1;
	}
	crayon_memory_set_frame_uv(&Opaque_Add_Draw, 0, 0);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging
	char buffer[15];

	while(1){
		pvr_wait_ready();
		// MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			//If any button is pressed, start the game (Doesn't check thumbstick)
			// if(st->buttons & (CONT_DPAD_RIGHT)){
			// 	Man_Draw.rotation[0]++;
			// }
			// if(st->buttons & (CONT_DPAD_LEFT)){
			// 	Man_Draw.rotation[0]--;
			// }
		// MAPLE_FOREACH_END()

		crayon_graphics_setup_palette(&Man_P);
		crayon_graphics_setup_palette(&BIOS_P);

		sprintf(buffer, "Angle: %d", (int)Man_Draw.rotation[0]);

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&Opaque_Blend_Draw, NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Opaque_Add_Draw, NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_text_mono(buffer, &BIOS_Font, PVR_LIST_OP_POLY, 280, 360, 30, 1, 1, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Man_Draw, NULL, PVR_LIST_PT_POLY, CRAYON_DRAW_ENHANCED);
		pvr_list_finish();

		pvr_scene_finish();

		//Rotate the man and keep it within the 0 - 360 range
		Man_Draw.rotation[0]++;
		if(Man_Draw.rotation[0] > 360){
			Man_Draw.rotation[0] -= 360;
		}
	}

	//Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Man_Draw);
	crayon_memory_free_sprite_array(&Opaque_Blend_Draw);
	crayon_memory_free_sprite_array(&Opaque_Add_Draw);

	crayon_memory_free_mono_font_sheet(&BIOS_Font);

	crayon_memory_free_spritesheet(&Man_SS);
	crayon_memory_free_spritesheet(&Opaque_SS);

	crayon_memory_free_palette(&Man_P);
	crayon_memory_free_palette(&BIOS_P);

	return 0;
}
