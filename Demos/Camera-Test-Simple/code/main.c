//Crayon libraries
#include <crayon/memory.h>
#include <crayon/debug.h>
#include <crayon/graphics.h>

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

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
	pvr_init(&pvr_params);
	
	#if CRAYON_BOOT_MODE == 1
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

	crayon_spritesheet_t Dwarf, Opaque, Man;
	crayon_sprite_array_t Dwarf_Draw, Rainbow_Draw, Frames_Draw, Red_Man_Draw, Green_Man_Draw, Man_BG;
	crayon_sprite_array_t Cam_BGs[4];
	crayon_sprite_array_t Rainbow_Draw2;
	crayon_font_prop_t Tahoma;
	crayon_font_mono_t BIOS;
	crayon_palette_t Tahoma_P, BIOS_P, Red_Man_P, Green_Man_P;

	#if CRAYON_BOOT_MODE == 1
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

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	//Draws 4 faces and rotates between all 12 faces
	crayon_memory_init_sprite_array(&Frames_Draw, &Opaque, 0, NULL, 4, 16, CRAY_MULTI_FRAME, PVR_FILTER_NONE, 0);
	Frames_Draw.scale[0].x = 3;
	Frames_Draw.scale[0].y = 3;
	Frames_Draw.coord[0].x = 380;
	Frames_Draw.coord[0].y = 50;
	Frames_Draw.coord[1].x = Frames_Draw.coord[0].x + (Frames_Draw.scale[0].x * Opaque.animation[0].frame_width);
	Frames_Draw.coord[1].y = Frames_Draw.coord[0].y;
	Frames_Draw.coord[2].x = Frames_Draw.coord[0].x;
	Frames_Draw.coord[2].y = Frames_Draw.coord[0].y + (Frames_Draw.scale[0].y * Opaque.animation[0].frame_height);
	Frames_Draw.coord[3].x = Frames_Draw.coord[1].x;
	Frames_Draw.coord[3].y = Frames_Draw.coord[2].y;
	Frames_Draw.layer[0] = 18;
	Frames_Draw.flip[0] = 0;
	Frames_Draw.rotation[0] = 0;
	Frames_Draw.colour[0] = 0;
	Frames_Draw.frame_id[0] = 0;
	Frames_Draw.frame_id[1] = 1;
	Frames_Draw.frame_id[2] = 2;
	Frames_Draw.frame_id[3] = 3;
	uint8_t i;
	for(i = 0; i < 12; i++){
		crayon_memory_set_frame_uv(&Frames_Draw, i, i);
	}

	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Dwarf_Draw, &Dwarf, 0, NULL, 3, 1, CRAY_MULTI_SCALE, PVR_FILTER_NONE, 0);
	Dwarf_Draw.coord[0].x = 50;
	Dwarf_Draw.coord[0].y = 20;
	Dwarf_Draw.coord[1].x = Dwarf_Draw.coord[0].x;
	Dwarf_Draw.coord[1].y = Dwarf_Draw.coord[0].y + (Dwarf_Draw.animation[0].frame_height / 2);
	Dwarf_Draw.coord[2].x = Dwarf_Draw.coord[0].x;
	Dwarf_Draw.coord[2].y = Dwarf_Draw.coord[1].y + Dwarf_Draw.animation[0].frame_height;
	Dwarf_Draw.layer[0] = 18;
	Dwarf_Draw.scale[0].x = 0.5;
	Dwarf_Draw.scale[0].y = 0.5;
	Dwarf_Draw.scale[1].x = 1;
	Dwarf_Draw.scale[1].y = 1;
	Dwarf_Draw.scale[2].x = 2;
	Dwarf_Draw.scale[2].y = 2;
	Dwarf_Draw.flip[0] = 0;
	Dwarf_Draw.rotation[0] = 0;
	Dwarf_Draw.colour[0] = 0;
	Dwarf_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw, 0, 0);

	//Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotation with sprites where height != width
	crayon_memory_init_sprite_array(&Red_Man_Draw, &Man, 0, &Red_Man_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Red_Man_Draw.coord[0].x = 50;
	Red_Man_Draw.coord[0].y = 280;
	Red_Man_Draw.layer[0] = 18;
	Red_Man_Draw.scale[0].x = 6;
	Red_Man_Draw.scale[0].y = 6;
	Red_Man_Draw.flip[0] = 0;
	Red_Man_Draw.rotation[0] = 450;
	Red_Man_Draw.colour[0] = 0;
	Red_Man_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Red_Man_Draw, 0, 0);

	//Copy the red palette over and modify red with green
	crayon_memory_clone_palette(&Red_Man_P, &Green_Man_P, 3);
	crayon_memory_swap_colour(&Green_Man_P, 0xFFFF0000, 0xFF00D200, 0);

	//Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotation with sprites where height != width
	crayon_memory_init_sprite_array(&Green_Man_Draw, &Man, 0, &Green_Man_P, 8, 1, CRAY_MULTI_FLIP | CRAY_MULTI_ROTATE, PVR_FILTER_NONE, 0);
	Green_Man_Draw.coord[0].x = 300;
	Green_Man_Draw.coord[0].y = 360;
	Green_Man_Draw.coord[1].x = Green_Man_Draw.coord[0].x + 60;
	Green_Man_Draw.coord[1].y = Green_Man_Draw.coord[0].y;
	Green_Man_Draw.coord[2].x = Green_Man_Draw.coord[0].x;
	Green_Man_Draw.coord[2].y = Green_Man_Draw.coord[0].y + 60;
	Green_Man_Draw.coord[3].x = Green_Man_Draw.coord[1].x;
	Green_Man_Draw.coord[3].y = Green_Man_Draw.coord[2].y;
	Green_Man_Draw.coord[4].x = Green_Man_Draw.coord[0].x;
	Green_Man_Draw.coord[4].y = Green_Man_Draw.coord[3].y + 60;
	Green_Man_Draw.coord[5].x = Green_Man_Draw.coord[1].x;
	Green_Man_Draw.coord[5].y = Green_Man_Draw.coord[4].y;
	Green_Man_Draw.coord[6].x = Green_Man_Draw.coord[0].x;
	Green_Man_Draw.coord[6].y = Green_Man_Draw.coord[4].y + 60;
	Green_Man_Draw.coord[7].x = Green_Man_Draw.coord[1].x;
	Green_Man_Draw.coord[7].y = Green_Man_Draw.coord[6].y;
	Green_Man_Draw.layer[0] = 50;
	Green_Man_Draw.scale[0].x = 3;
	Green_Man_Draw.scale[0].y = 3;
	Green_Man_Draw.flip[0] = 0;
	Green_Man_Draw.flip[1] = 1;
	Green_Man_Draw.flip[2] = 0;
	Green_Man_Draw.flip[3] = 1;
	Green_Man_Draw.flip[4] = 0;
	Green_Man_Draw.flip[5] = 1;
	Green_Man_Draw.flip[6] = 0;
	Green_Man_Draw.flip[7] = 1;
	Green_Man_Draw.rotation[0] = 0;
	Green_Man_Draw.rotation[1] = 0;
	Green_Man_Draw.rotation[2] = 90;
	Green_Man_Draw.rotation[3] = 90;
	Green_Man_Draw.rotation[4] = 180;
	Green_Man_Draw.rotation[5] = 180;
	Green_Man_Draw.rotation[6] = 270;
	Green_Man_Draw.rotation[7] = 270;
	Green_Man_Draw.colour[0] = 0;
	Green_Man_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Green_Man_Draw, 0, 0);

	//8 sprites, 1 frame, multi rotation and flip
	crayon_memory_init_sprite_array(&Rainbow_Draw, &Opaque, 1, NULL, 8, 1, CRAY_MULTI_FLIP | CRAY_MULTI_ROTATE, PVR_FILTER_NONE, 0);
	Rainbow_Draw.coord[0].x = Dwarf_Draw.coord[0].x + (2 * Dwarf_Draw.animation[0].frame_width) + 20;
	Rainbow_Draw.coord[0].y = 20;
	Rainbow_Draw.coord[1].x = Rainbow_Draw.coord[0].x;
	Rainbow_Draw.coord[1].y = Rainbow_Draw.coord[0].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.coord[2].x = Rainbow_Draw.coord[0].x;
	Rainbow_Draw.coord[2].y = Rainbow_Draw.coord[1].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.coord[3].x = Rainbow_Draw.coord[0].x;
	Rainbow_Draw.coord[3].y = Rainbow_Draw.coord[2].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.coord[4].x = Rainbow_Draw.coord[0].x + (8 * Rainbow_Draw.animation[0].frame_width) + 10;
	Rainbow_Draw.coord[4].y = Rainbow_Draw.coord[0].y;
	Rainbow_Draw.coord[5].x = Rainbow_Draw.coord[4].x;
	Rainbow_Draw.coord[5].y = Rainbow_Draw.coord[0].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.coord[6].x = Rainbow_Draw.coord[4].x;
	Rainbow_Draw.coord[6].y = Rainbow_Draw.coord[5].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.coord[7].x = Rainbow_Draw.coord[4].x;
	Rainbow_Draw.coord[7].y = Rainbow_Draw.coord[6].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10;
	Rainbow_Draw.layer[0] = 17;
	Rainbow_Draw.scale[0].x = 8;
	Rainbow_Draw.scale[0].y = 8;
	Rainbow_Draw.flip[0] = 0;
	Rainbow_Draw.flip[1] = 0;
	Rainbow_Draw.flip[2] = 0;
	Rainbow_Draw.flip[3] = 0;
	Rainbow_Draw.flip[4] = 1;
	Rainbow_Draw.flip[5] = 1;
	Rainbow_Draw.flip[6] = 1;
	Rainbow_Draw.flip[7] = 1;
	Rainbow_Draw.rotation[0] = 0;
	Rainbow_Draw.rotation[1] = 90;
	Rainbow_Draw.rotation[2] = 180;
	Rainbow_Draw.rotation[3] = 270;
	Rainbow_Draw.rotation[4] = 0;
	Rainbow_Draw.rotation[5] = 90;
	Rainbow_Draw.rotation[6] = 180;
	Rainbow_Draw.rotation[7] = 270;
	Rainbow_Draw.colour[0] = 0;
	Rainbow_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Rainbow_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Rainbow_Draw2, &Opaque, 1, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Rainbow_Draw2.coord[0].x = Rainbow_Draw.coord[0].x - 60;
	Rainbow_Draw2.coord[0].y = Rainbow_Draw.coord[0].y;
	Rainbow_Draw2.layer[0] = 17;
	Rainbow_Draw2.scale[0].x = 1;
	Rainbow_Draw2.scale[0].y = 1;
	Rainbow_Draw2.flip[0] = 0;
	Rainbow_Draw2.rotation[0] = 180;
	Rainbow_Draw2.colour[0] = 0;
	Rainbow_Draw2.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Rainbow_Draw2, 0, 0);

	crayon_memory_init_sprite_array(&Man_BG, NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Man_BG.coord[0].x = Red_Man_Draw.coord[0].x;
	Man_BG.coord[0].y = Red_Man_Draw.coord[0].y;
	Man_BG.layer[0] = Red_Man_Draw.layer[0] - 1;
	Man_BG.scale[0].x = Red_Man_Draw.animation->frame_width * Red_Man_Draw.scale[0].x;
	Man_BG.scale[0].y = Red_Man_Draw.animation->frame_height * Red_Man_Draw.scale[0].y;
	Man_BG.rotation[0] = 0;
	Man_BG.colour[0] = 0xFF000000;

	crayon_memory_init_sprite_array(&Cam_BGs[0], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[0].coord[0].x = 0;
	Cam_BGs[0].coord[0].y = 0;
	Cam_BGs[0].layer[0] = 1;
	Cam_BGs[0].scale[0].x = 640;
	Cam_BGs[0].scale[0].y = 480;
	Cam_BGs[0].rotation[0] = 0;
	Cam_BGs[0].colour[0] = 0xFF888888;

	crayon_memory_init_sprite_array(&Cam_BGs[1], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[1].coord[0].x = 32;
	Cam_BGs[1].coord[0].y = 32;
	Cam_BGs[1].layer[0] = 1;
	Cam_BGs[1].scale[0].x = 400;
	Cam_BGs[1].scale[0].y = 300;
	Cam_BGs[1].rotation[0] = 0;
	Cam_BGs[1].colour[0] = 0xFF888888;

	crayon_memory_init_sprite_array(&Cam_BGs[2], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[2].coord[0].x = 160;
	Cam_BGs[2].coord[0].y = 120;
	Cam_BGs[2].layer[0] = 1;
	Cam_BGs[2].scale[0].x = 320;
	Cam_BGs[2].scale[0].y = 240;
	Cam_BGs[2].rotation[0] = 0;
	Cam_BGs[2].colour[0] = 0xFF888888;

	crayon_memory_init_sprite_array(&Cam_BGs[3], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[3].coord[0].x = 0;
	Cam_BGs[3].coord[0].y = 0;
	Cam_BGs[3].layer[0] = 1;
	Cam_BGs[3].scale[0].x = 640;
	Cam_BGs[3].scale[0].y = 480;
	Cam_BGs[3].rotation[0] = 0;
	Cam_BGs[3].colour[0] = 0xFF888888;

	uint8_t current_camera_id = 0;
	crayon_viewport_t cameras[4];
	crayon_viewport_t * current_camera = &cameras[current_camera_id];

	//This is the same as no camera
	crayon_memory_init_camera(&cameras[0], (vec2_f_t){Cam_BGs[0].coord[0].x,Cam_BGs[0].coord[0].y},
		(vec2_u16_t){Cam_BGs[0].scale[0].x,Cam_BGs[0].scale[0].y},
		(vec2_u16_t){Cam_BGs[0].coord[0].x,Cam_BGs[0].coord[0].y},
		(vec2_u16_t){Cam_BGs[0].scale[0].x,Cam_BGs[0].scale[0].y}, 0);

	//This is the basic view, no scaling, but we crop everything outside the inner 300/200 box
	crayon_memory_init_camera(&cameras[1], (vec2_f_t){Cam_BGs[1].coord[0].x,Cam_BGs[1].coord[0].y},
		(vec2_u16_t){Cam_BGs[1].scale[0].x,Cam_BGs[1].scale[0].y},
		(vec2_u16_t){Cam_BGs[1].coord[0].x,Cam_BGs[1].coord[0].y},
		(vec2_u16_t){Cam_BGs[1].scale[0].x,Cam_BGs[1].scale[0].y}, 0);

	//Magnify the selectioned zone (160,120 to 480, 360) The world is half the size of the window
	crayon_memory_init_camera(&cameras[2], (vec2_f_t){0,0},
		(vec2_u16_t){640,480},
		(vec2_u16_t){Cam_BGs[2].coord[0].x,Cam_BGs[2].coord[0].y},
		(vec2_u16_t){Cam_BGs[2].scale[0].x,Cam_BGs[2].scale[0].y}, 0);

	crayon_memory_init_camera(&cameras[3], (vec2_f_t){0,0},
		(vec2_u16_t){320,240},
		(vec2_u16_t){Cam_BGs[3].coord[0].x,Cam_BGs[3].coord[0].y},
		(vec2_u16_t){Cam_BGs[3].scale[0].x,Cam_BGs[3].scale[0].y}, 0);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Tahoma_P);
	crayon_graphics_setup_palette(&Red_Man_P);
	crayon_graphics_setup_palette(&Green_Man_P);

	char snum1[20];
	char snum2[20];
	char snum3[20];
	for(i = 0; i < 8; i++){
		__GRAPHICS_DEBUG_VARIABLES[i] = 0;
	}

	pvr_stats_t stats;
	uint32_t previous_buttons[4] = {0};
	while(1){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		if(st->buttons & CONT_DPAD_LEFT){
			current_camera->world_x += 1;
		}
		else if(st->buttons & CONT_DPAD_RIGHT){
			current_camera->world_x -= 1;
		}

		if(st->buttons & CONT_DPAD_UP){
			current_camera->world_y += 1;
		}
		else if(st->buttons & CONT_DPAD_DOWN){
			current_camera->world_y -= 1;
		}

		if((st->buttons & CONT_A) && !(previous_buttons[__dev->port] & CONT_A)){
			current_camera_id += 1;
			current_camera_id %= 4;
			current_camera = &cameras[current_camera_id];
		}

		// if((st->buttons & CONT_B) && !(previous_buttons[__dev->port] & CONT_B)){
		// 	;
		// }

		// if((st->buttons & CONT_X) && !(previous_buttons[__dev->port] & CONT_X)){
		// 	;
		// }

		previous_buttons[__dev->port] = st->buttons;	//Store the previous button presses
		MAPLE_FOREACH_END()

		pvr_scene_begin();

		pvr_get_stats(&stats);

		//Animation of red man falling and faces rotating
		if(stats.frame_count % 120 <= 30){
			Red_Man_Draw.rotation[0] = 0;
		}
		else if(stats.frame_count % 120 <= 60){
			Red_Man_Draw.rotation[0] = 90;
		}
		else if(stats.frame_count % 120 <= 90){
			Red_Man_Draw.rotation[0] = 180;
		}
		else{
			Red_Man_Draw.rotation[0] = 270;
		}

		if(stats.frame_count % 60 == 0){
			Frames_Draw.frame_id[0] = (Frames_Draw.frame_id[0] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[1] = (Frames_Draw.frame_id[1] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[2] = (Frames_Draw.frame_id[2] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[3] = (Frames_Draw.frame_id[3] + 1) % Frames_Draw.animation->frame_count;
		}

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw_sprites(&Red_Man_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw_sprites(&Green_Man_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);

			//Fonts aren't supported by cameras yet
			// crayon_graphics_draw_text_prop("Tahoma\0", &Tahoma, PVR_LIST_PT_POLY, 120, 20, 30, 1, 1, Tahoma_P.palette_id);
			// crayon_graphics_draw_text_mono("BIOS\0", &BIOS, PVR_LIST_PT_POLY, 120, 40, 30, 1, 1, BIOS_P.palette_id);

			// crayon_graphics_draw_text_mono("Rotation: 0 Degrees\0", &BIOS, PVR_LIST_PT_POLY,
			// 	Rainbow_Draw.coord[4].x + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
			// 	Rainbow_Draw.coord[0].y + 24, 30, 1, 1, BIOS_P.palette_id);

			// crayon_graphics_draw_text_mono("Rotation: 90 Degrees\0", &BIOS, PVR_LIST_PT_POLY,
			// 	Rainbow_Draw.coord[4].x + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
			// 	Rainbow_Draw.coord[1].y + 24, 30, 1, 1, BIOS_P.palette_id);

			// crayon_graphics_draw_text_mono("Rotation: 180 Degrees\0", &BIOS, PVR_LIST_PT_POLY,
			// 	Rainbow_Draw.coord[4].x + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
			// 	Rainbow_Draw.coord[2].y + 24, 30, 1, 1, BIOS_P.palette_id);

			// crayon_graphics_draw_text_mono("Rotation: 270 Degrees\0", &BIOS, PVR_LIST_PT_POLY,
			// 	Rainbow_Draw.coord[4].x + (8 * Rainbow_Draw.animation[0].frame_width) + 10,
			// 	Rainbow_Draw.coord[3].y + 24, 30, 1, 1, BIOS_P.palette_id);


			// crayon_graphics_draw_text_mono("Normal\0", &BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.coord[0].x,
			// 	Rainbow_Draw.coord[3].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10, 30, 1, 1, BIOS_P.palette_id);
			// crayon_graphics_draw_text_mono("Flipped\0", &BIOS, PVR_LIST_PT_POLY, Rainbow_Draw.coord[4].x,
			// 	Rainbow_Draw.coord[3].y + (8 * Rainbow_Draw.animation[0].frame_height) + 10, 30, 1, 1, BIOS_P.palette_id);

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			crayon_graphics_draw_sprites(&Frames_Draw, current_camera, PVR_LIST_OP_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw_sprites(&Rainbow_Draw2, current_camera, PVR_LIST_OP_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw_sprites(&Rainbow_Draw, current_camera, PVR_LIST_OP_POLY, CRAY_DRAW_SIMPLE);

			//Represents the boundry box for the red man when not rotated
			// crayon_graphics_draw_sprites(&Man_BG, current_camera, PVR_LIST_OP_POLY, 0);

			//This represents camera 2's space
			crayon_graphics_draw_sprites(&Cam_BGs[current_camera_id], NULL, PVR_LIST_OP_POLY, 0);

			// sprintf(snum1, "Left %.9f", __GRAPHICS_DEBUG_VARIABLES[0]);
			// sprintf(snum2, "Top %.9f", __GRAPHICS_DEBUG_VARIABLES[1]);
			// crayon_graphics_draw_text_mono(snum1, &BIOS, PVR_LIST_PT_POLY, 0, 400, 30, 1, 1, BIOS_P.palette_id);
			// crayon_graphics_draw_text_mono(snum2, &BIOS, PVR_LIST_PT_POLY, 0, 400 + BIOS.char_height, 30, 1, 1, BIOS_P.palette_id);

			sprintf(snum1, "X: %.2f", current_camera->world_x);
			sprintf(snum2, "Y: %.2f", current_camera->world_y);
			sprintf(snum3, "Camera ID: %d", current_camera_id);
			crayon_graphics_draw_text_mono("World Coords:", &BIOS, PVR_LIST_PT_POLY, 0, 420 - BIOS.char_height, 30, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(snum1, &BIOS, PVR_LIST_PT_POLY, 0, 420, 30, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(snum2, &BIOS, PVR_LIST_PT_POLY, 0, 420 + BIOS.char_height, 30, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(snum3, &BIOS, PVR_LIST_PT_POLY, 0, 420 + (2 * BIOS.char_height), 30, 1, 1, BIOS_P.palette_id);

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
