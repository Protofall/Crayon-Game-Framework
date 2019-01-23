//Crayon libraries
#include <crayon/memory.h>
#include <crayon/debug.h>
#include <crayon/graphics.h>

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

#if CRAYON_SD_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <ext2/fs_ext2.h>
#endif

#if CRAYON_SD_MODE == 1
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

int main(){
	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB and we default to NTSC interlace
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}

	pvr_init(&pvr_params);

	crayon_spritesheet_t Dwarf, Opaque, Man;
	crayon_sprite_array_t Dwarf_Draw, Rainbow_Draw, Frames_Draw, Red_Man_Draw, Green_Man_Draw;
	crayon_font_prop_t Tahoma;
	crayon_font_mono_t BIOS;
	crayon_palette_t Tahoma_P, BIOS_P, Red_Man_P, Green_Man_P;

	#if CRAYON_SD_MODE == 1
		crayon_memory_mount_romdisk("/sd/colourMod.img", "/files");
	#else
		crayon_memory_mount_romdisk("/cd/colourMod.img", "/files");
	#endif

	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, 0, "/files/Fonts/Tahoma_font.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/files/Fonts/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Dwarf, NULL, -1, "/files/Dwarf.dtex");
	crayon_memory_load_spritesheet(&Opaque, NULL, -1, "/files/Opaque.dtex");
	crayon_memory_load_spritesheet(&Man, &Red_Man_P, 2, "/files/Man.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	//Basic test to show valid_string checking. Top fails, bottom works
	// const char * string = " Weird littole 347 test strin\ng \thellow";
	const char * string = " Weird littole 347 test strin\ng hellow~3";
	if(crayon_graphics_valid_string(string, Tahoma.num_chars)){
		return 1;
	}

	//Draws 4 faces and rotates between all 12 faces
	crayon_memory_init_sprite_array(&Frames_Draw, 4, 16, 0, 1, 0, 0, 0, 0, 0, &Opaque, &Opaque.animation_array[0], NULL);
	Frames_Draw.positions[0] = 540;
	Frames_Draw.positions[1] = 20;
	Frames_Draw.positions[2] = 540 + 32;
	Frames_Draw.positions[3] = 20;
	Frames_Draw.positions[4] = 540;
	Frames_Draw.positions[5] = 20 + 32;
	Frames_Draw.positions[6] = 540 + 32;
	Frames_Draw.positions[7] = 20 + 32;
	Frames_Draw.draw_z[0] = 18;
	Frames_Draw.scales[0] = 2;
	Frames_Draw.scales[1] = 2;
	Frames_Draw.flips[0] = 0;
	Frames_Draw.rotations[0] = 0;
	Frames_Draw.colours[0] = 0;
	Frames_Draw.frame_coord_keys[0] = 0;
	Frames_Draw.frame_coord_keys[1] = 1;
	Frames_Draw.frame_coord_keys[2] = 2;
	Frames_Draw.frame_coord_keys[3] = 3;
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 0, Frames_Draw.frame_coord_map + 1, 0);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 2, Frames_Draw.frame_coord_map + 3, 1);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 4, Frames_Draw.frame_coord_map + 5, 2);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 6, Frames_Draw.frame_coord_map + 7, 3);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 8, Frames_Draw.frame_coord_map + 9, 4);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 10, Frames_Draw.frame_coord_map + 11, 5);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 12, Frames_Draw.frame_coord_map + 13, 6);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 14, Frames_Draw.frame_coord_map + 15, 7);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 16, Frames_Draw.frame_coord_map + 17, 8);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 18, Frames_Draw.frame_coord_map + 19, 9);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 20, Frames_Draw.frame_coord_map + 21, 10);
	crayon_graphics_frame_coordinates(Frames_Draw.animation, Frames_Draw.frame_coord_map + 22, Frames_Draw.frame_coord_map + 23, 11);

	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Dwarf_Draw, 3, 1, 0, 0, 1, 0, 0, 0, 0, &Dwarf, &Dwarf.animation_array[0], NULL);
	Dwarf_Draw.positions[0] = 50;
	Dwarf_Draw.positions[1] = 20;
	Dwarf_Draw.positions[2] = Dwarf_Draw.positions[0];
	Dwarf_Draw.positions[3] = Dwarf_Draw.positions[1] + (Dwarf_Draw.animation[0].frame_height / 2);
	Dwarf_Draw.positions[4] = Dwarf_Draw.positions[0];
	Dwarf_Draw.positions[5] = Dwarf_Draw.positions[3] + Dwarf_Draw.animation[0].frame_height;
	Dwarf_Draw.draw_z[0] = 18;
	Dwarf_Draw.scales[0] = 0.5;
	Dwarf_Draw.scales[1] = 0.5;
	Dwarf_Draw.scales[2] = 1;
	Dwarf_Draw.scales[3] = 1;
	Dwarf_Draw.scales[4] = 2;
	Dwarf_Draw.scales[5] = 2;
	Dwarf_Draw.flips[0] = 0;
	Dwarf_Draw.rotations[0] = 0;
	Dwarf_Draw.colours[0] = 0;
	Dwarf_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Dwarf_Draw.animation, Dwarf_Draw.frame_coord_map + 0, Dwarf_Draw.frame_coord_map + 1, 0);

	//Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotations with sprites where height != width
	crayon_memory_init_sprite_array(&Red_Man_Draw, 1, 1, 0, 0, 0, 0, 0, 0, 0, &Man, &Man.animation_array[0], &Red_Man_P);
	Red_Man_Draw.positions[0] = 50;
	Red_Man_Draw.positions[1] = 280;
	Red_Man_Draw.draw_z[0] = 18;
	Red_Man_Draw.scales[0] = 6;
	Red_Man_Draw.scales[1] = 6;
	Red_Man_Draw.flips[0] = 0;
	Red_Man_Draw.rotations[0] = 450;
	Red_Man_Draw.colours[0] = 0;
	Red_Man_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Red_Man_Draw.animation, Red_Man_Draw.frame_coord_map + 0, Red_Man_Draw.frame_coord_map + 1, 0);

	//Copy the red palette over and modify red with green
	crayon_memory_clone_palette(&Red_Man_P, &Green_Man_P, 3);
	crayon_memory_swap_colour(&Green_Man_P, 0xFFFF0000, 0xFF00D200, 0);

	//Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotations with sprites where height != width
	crayon_memory_init_sprite_array(&Green_Man_Draw, 1, 1, 0, 0, 0, 0, 0, 0, 0, &Man, &Man.animation_array[0], &Green_Man_P);
	Green_Man_Draw.positions[0] = 299;
	Green_Man_Draw.positions[1] = 380;
	Green_Man_Draw.draw_z[0] = 50;
	Green_Man_Draw.scales[0] = 6;
	Green_Man_Draw.scales[1] = 6;
	Green_Man_Draw.flips[0] = 0;
	Green_Man_Draw.rotations[0] = 0;
	Green_Man_Draw.colours[0] = 0;
	Green_Man_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Green_Man_Draw.animation, Green_Man_Draw.frame_coord_map + 0, Green_Man_Draw.frame_coord_map + 1, 0);

	//8 sprites, 1 frame, multi rotations and flips
	crayon_memory_init_sprite_array(&Rainbow_Draw, 8, 1, 0, 0, 0, 1, 1, 0, 0, &Opaque, &Opaque.animation_array[1], NULL);
	Rainbow_Draw.positions[0] = Dwarf_Draw.positions[0] + (2 * Dwarf_Draw.animation[0].frame_width) + 20;
	Rainbow_Draw.positions[1] = 20;
	Rainbow_Draw.positions[2] = Rainbow_Draw.positions[0];
	Rainbow_Draw.positions[3] = Rainbow_Draw.positions[1] + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.positions[4] = Rainbow_Draw.positions[0];
	Rainbow_Draw.positions[5] = Rainbow_Draw.positions[3] + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.positions[6] = Rainbow_Draw.positions[0];
	Rainbow_Draw.positions[7] = Rainbow_Draw.positions[5] + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.positions[8] = Rainbow_Draw.positions[0] + (8 * Rainbow_Draw.animation[0].frame_width) + 10;
	Rainbow_Draw.positions[9] = Rainbow_Draw.positions[1];
	Rainbow_Draw.positions[10] = Rainbow_Draw.positions[8];
	Rainbow_Draw.positions[11] = Rainbow_Draw.positions[9] + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.positions[12] = Rainbow_Draw.positions[8];
	Rainbow_Draw.positions[13] = Rainbow_Draw.positions[11] + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.positions[14] = Rainbow_Draw.positions[8];
	Rainbow_Draw.positions[15] = Rainbow_Draw.positions[13] + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.draw_z[0] = 17;
	Rainbow_Draw.scales[0] = 8;
	Rainbow_Draw.scales[1] = 8;
	Rainbow_Draw.flips[0] = 0;
	Rainbow_Draw.flips[1] = 0;
	Rainbow_Draw.flips[2] = 0;
	Rainbow_Draw.flips[3] = 0;
	Rainbow_Draw.flips[4] = 1;
	Rainbow_Draw.flips[5] = 1;
	Rainbow_Draw.flips[6] = 1;
	Rainbow_Draw.flips[7] = 1;
	Rainbow_Draw.rotations[0] = 0;
	Rainbow_Draw.rotations[1] = 90;
	Rainbow_Draw.rotations[2] = 180;
	Rainbow_Draw.rotations[3] = 270;
	Rainbow_Draw.rotations[4] = 0;
	Rainbow_Draw.rotations[5] = 90;
	Rainbow_Draw.rotations[6] = 180;
	Rainbow_Draw.rotations[7] = 270;
	Rainbow_Draw.colours[0] = 0;
	Rainbow_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Rainbow_Draw.animation, Rainbow_Draw.frame_coord_map + 0, Rainbow_Draw.frame_coord_map + 1, 0);
	//ADD "Rotation from float to flip array converter call" HERE

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	pvr_stats_t stats;
	// uint32_t previous_buttons[4] = {0};

	while(1){

		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		//Change the direction the guy is facing
		if(st->buttons & CONT_DPAD_LEFT){
			Green_Man_Draw.flips[0] = 1;
		}
		else if(st->buttons & CONT_DPAD_RIGHT){
			Green_Man_Draw.flips[0] = 0;
		}

		// previous_buttons[__dev->port] = st->buttons;	//Store the previous button presses
		MAPLE_FOREACH_END()

		pvr_wait_ready();
		pvr_scene_begin();

		crayon_graphics_setup_palette(&BIOS_P);
		crayon_graphics_setup_palette(&Tahoma_P);
		crayon_graphics_setup_palette(&Red_Man_P);
		crayon_graphics_setup_palette(&Green_Man_P);

		pvr_get_stats(&stats);

		//Animation of red man falling and faces rotating
		if(stats.frame_count % 60 <= 30){
			Red_Man_Draw.rotations[0] = 0;
		}
		else{
			Red_Man_Draw.rotations[0] = 90;
		}

		if(stats.frame_count % 60 == 0){
			Frames_Draw.frame_coord_keys[0] = (Frames_Draw.frame_coord_keys[0] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_coord_keys[1] = (Frames_Draw.frame_coord_keys[1] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_coord_keys[2] = (Frames_Draw.frame_coord_keys[2] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_coord_keys[3] = (Frames_Draw.frame_coord_keys[3] + 1) % Frames_Draw.animation->frame_count;
		}

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Red_Man_Draw, PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Green_Man_Draw, PVR_LIST_PT_POLY);

			crayon_graphics_draw_text_prop(&Tahoma, PVR_LIST_PT_POLY, 120, 20, 30, 1, 1, Tahoma_P.palette_id, "Tahoma\0");
			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, 120, 40, 30, 1, 1, BIOS_P.palette_id, "BIOS\0");

			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.positions[8] + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
				Rainbow_Draw.positions[1] + 24, 30, 1, 1, BIOS_P.palette_id, "Rotation: 0 Degrees\0");

			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.positions[8] + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
				Rainbow_Draw.positions[3] + 24, 30, 1, 1, BIOS_P.palette_id, "Rotation: 90 Degrees\0");

			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.positions[8] + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
				Rainbow_Draw.positions[5] + 24, 30, 1, 1, BIOS_P.palette_id, "Rotation: 180 Degrees\0");

			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.positions[8] + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
				Rainbow_Draw.positions[7] + 24, 30, 1, 1, BIOS_P.palette_id, "Rotation: 270 Degrees\0");


			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.positions[0],
				Rainbow_Draw.positions[7] + (8 * Rainbow_Draw.animation[0].frame_height) + 10, 30, 1, 1, BIOS_P.palette_id, "Normal\0");
			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.positions[8],
				Rainbow_Draw.positions[7] + (8 * Rainbow_Draw.animation[0].frame_height) + 10, 30, 1, 1, BIOS_P.palette_id, "Flipped\0");

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			crayon_graphics_draw_sprites(&Rainbow_Draw, PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&Frames_Draw, PVR_LIST_OP_POLY);

			//Represents the boundry box for the red man when not rotated
			crayon_graphics_draw_untextured_poly(Red_Man_Draw.positions[0], Red_Man_Draw.positions[1], Red_Man_Draw.draw_z[0] - 1,
				Red_Man_Draw.animation->frame_width * Red_Man_Draw.scales[0],
				Red_Man_Draw.animation->frame_height * Red_Man_Draw.scales[1], 0xFF000000, PVR_LIST_OP_POLY);

		pvr_list_finish();

		pvr_scene_finish();

	}

	//Confirm everything was unloaded successfully (Should equal zero)
	// int retVal = 0;
	// retVal += memory_free_crayon_spritesheet(&Fade, 1);
	// retVal += memory_free_crayon_spritesheet(&Enlarge, 1);
	// retVal += memory_free_crayon_spritesheet(&Frames, 1);
	// error_freeze("Free-ing result %d!\n", retVal);

	return 0;
}