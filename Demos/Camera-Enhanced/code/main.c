//Crayon libraries
#include <crayon/memory.h>
#include <crayon/debug.h>
#include <crayon/graphics.h>

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

#include <math.h>

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

float thumbstick_int_to_float(int joy){
	float ret_val;	//Range from -1 to 1

	if(joy > 0){	//Converting from -128, 127 to -1, 1
		ret_val = joy / 127.0;
	}
	else{
		ret_val = joy / 128.0;
	}

	return ret_val;
}

uint32_t thumbstick_to_dpad(int joyx, int joyy, float deadspace){
	float thumb_x = thumbstick_int_to_float(joyx);
	float thumb_y = thumbstick_int_to_float(joyy);

	//If the thumbstick is inside the deadspace then we don't check angle
	if((thumb_x * thumb_x) + (thumb_y * thumb_y) < deadspace * deadspace){
		return 0;
	}

	//8 options. N, NE, E, SE, S, SW, W, NW.

	//Rotate the thumbstick coordinate 22.5 degrees (Or 22.5 * (PI/180) ~= 0.3927 radians) clockwise
		//22.5 degrees is 1/16th of 360 degrees, this makes it easier to check which region the coords are in
	float angle = 22.5 * M_PI / 180.0;	//In radians

	vec2_f_t point = crayon_graphics_rotate_point((vec2_f_t){0, 0}, (vec2_f_t){thumb_x, thumb_y}, angle);
	thumb_x = point.x;
	thumb_y = point.y;

	float abs_x = fabs(thumb_x);
	float abs_y = fabs(thumb_y);

	uint32_t bitmap;
	if(thumb_y < 0){
		if(thumb_x > 0){
			if(abs_y > abs_x){	//N
				bitmap = CONT_DPAD_UP;
			}
			else{	//NE
				bitmap = CONT_DPAD_UP + CONT_DPAD_RIGHT;
			}
		}
		else{
			if(abs_y < abs_x){	//W
				bitmap = CONT_DPAD_LEFT;
			}
			else{	//NW
				bitmap = CONT_DPAD_UP + CONT_DPAD_LEFT;
			}
		}
	}
	else{
		if(thumb_x > 0){
			if(abs_y < abs_x){	//E
				bitmap = CONT_DPAD_RIGHT;
			}
			else{	//SE
				bitmap = CONT_DPAD_DOWN + CONT_DPAD_RIGHT;
			}
		}
		else{
			if(abs_y > abs_x){	//S
				bitmap = CONT_DPAD_DOWN;
			}
			else{	//SW
				bitmap = CONT_DPAD_DOWN + CONT_DPAD_LEFT;
			}
		}
	}

	return bitmap;
}

//Char count:
//63
//39
//68
//72
//27
//29
//Total: 298
void set_msg(char * buffer){
	strcpy(buffer, "Use the thumbstick to move the current camera around the world\n\
Press A to cycle between the 4 cameras\n\
Press L/R-Triggers move one unit in the direction you last moved in\n\
D-PAD Up/Down increase/decrease the world_movement_factor (Powers of 2)\n\
Start to terminate program\n\
Y to hide these instructions");
	return;
}

uint8_t james_direction = 0;	//0 = South, 1 = North, 2 = West, 3 = East
uint32_t james_start_frame = 0;	//Frame he starts moving in a direction

//0 for changing direction. 1 for same direction. 2 for standing still
int8_t check_james_dir(vec2_f_t distance){
	if(distance.x == 0 && distance.y == 0){	//If not moving, return same way of facing
		return james_direction;
	}

	uint8_t x_dir = 4, y_dir = 4;	//Defaults only stay if x or y is exactly 0
	if(distance.y > 0){	//South
		y_dir = 0;
	}
	else if(distance.y < 0){	//North
		y_dir = 1;
	}

	if(distance.x < 0){	//Right
		x_dir = 2;
	}
	else if(distance.x > 0){	//Left
		x_dir = 3;
	}

	//Note, this has biased towards top and bottom against left and right
		//x_dir and y_dir are uninitialised if x or y is zero
		//If player stands still. It really messes up
	if(james_direction == 0 && james_direction == y_dir){return james_direction;}
	if(james_direction == 1 && james_direction == y_dir){return james_direction;}
	if(james_direction == 2 && james_direction == x_dir){return james_direction;}
	if(james_direction == 3 && james_direction == x_dir){return james_direction;}

	//This should never trigger
	if(x_dir > 3 || y_dir > 3){
		//error_freeze("Fail");
	}

	//In an effort to forcce facing the direction your pointing in
	if(y_dir < 4){
		return y_dir;
	}
	if(x_dir < 4){
		return x_dir;
	}
	return 0;	//Should never get here
}

void move_james(crayon_sprite_array_t * James, vec2_f_t distance, uint32_t current_frame){

	if(distance.x == 0 && distance.y == 0){	//Stationary
		James->frame_id[0] = (james_direction == 3) ? (3 * 2) : (3 * james_direction);
		James->flip[0] = (james_direction == 3) ? 1 : 0;
		return;
	}

	uint8_t direction = check_james_dir(distance);

	if(james_direction != direction){	//Changed direction
		//if(direction > 3){error_freeze("Fail2");}
		James->frame_id[0] = (direction == 3) ? (3 * 2) + 1 : (3 * direction) + 1;
		James->flip[0] = (direction == 3) ? 1 : 0;
		james_direction = direction;
		james_start_frame = current_frame;
	}
	else{	//Still walking in the same direction
		uint8_t time = (james_start_frame - current_frame) % 60;
		uint8_t frame_offset;
		if(time < 15){	//Right foot out
			frame_offset = 1;
		}
		else if(time < 30){	//Both legs same pos
			frame_offset = 0;
		}
		else if(time < 45){	//Left foot out
			frame_offset = 2;
		}
		else{	//Both legs same pos
			frame_offset = 0;
		}

		James->frame_id[0] = (direction == 3) ? (3 * 2) + frame_offset : (3 * direction) + frame_offset;
		James->flip[0] = (direction == 3) ? 1 : 0;
	}

	//Movement code too
	James->coord[0].x += distance.x;
	James->coord[0].y += distance.y;

	return;
}

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
		int sdRes = mount_fat_sd();	//This function should be able to mount a FAT32 formatted sd card to the /sd dir	
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

	crayon_spritesheet_t Dwarf, Opaque, Man, Characters;
	crayon_sprite_array_t Dwarf_Draw, Rainbow_Draw, Frames_Draw, Red_Man_Draw, Green_Man_Draw, Man_BG, James_Draw;
	crayon_sprite_array_t Cam_BGs[4];
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
	crayon_memory_load_spritesheet(&Man, &Red_Man_P, 2, "/files/Man.dtex");	//Palette 3 will be reserved for Green Man
	crayon_memory_load_spritesheet(&Characters, NULL, 4, "/files/Characters.dtex");	//Since it has 23 colours, we'll just use ARGB1555

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_sprite_array(&James_Draw, &Characters, 0, NULL, 1, 9, 0, PVR_FILTER_NONE, 0);
	James_Draw.scale[0].x = 2;
	James_Draw.scale[0].y = 2;
	James_Draw.coord[0].x = 307;	//These are about the mid-point given sprite sizes and scale
	James_Draw.coord[0].y = 220;
	James_Draw.layer[0] = 16;
	James_Draw.flip[0] = (james_direction == 3) ? 1 : 0;	//If facing East, use West sprite but flipped
	James_Draw.rotation[0] = 0;
	James_Draw.colour[0] = 0xFF000000;
	James_Draw.fade[0] = 0;
	James_Draw.frame_id[0] = (james_direction == 3) ? 3 * 2 : 3 * james_direction;	//If facing East, use west sprite but flipped
	James_Draw.visible[0] = 1;
	uint8_t i;
	for(i = 0; i < James_Draw.frames_used; i++){
		crayon_memory_set_frame_uv(&James_Draw, i, i);
	}

	//Draws 4 faces and rotates between all 12 faces
	crayon_memory_init_sprite_array(&Frames_Draw, &Opaque, 0, NULL, 4, 12, CRAY_MULTI_FRAME, PVR_FILTER_NONE, 0);
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
	Frames_Draw.layer[1] = 18;
	Frames_Draw.layer[2] = 18;
	Frames_Draw.layer[3] = 18;
	Frames_Draw.flip[0] = 0;
	Frames_Draw.rotation[0] = 0;
	Frames_Draw.colour[0] = 0xFF000000;
	Frames_Draw.fade[0] = 0;
	Frames_Draw.frame_id[0] = 0;
	Frames_Draw.frame_id[1] = 1;
	Frames_Draw.frame_id[2] = 2;
	Frames_Draw.frame_id[3] = 3;
	for(i = 0; i < Frames_Draw.frames_used; i++){
		crayon_memory_set_frame_uv(&Frames_Draw, i, i);
	}
	for(i = 0; i < Frames_Draw.list_size; i++){
		Frames_Draw.visible[i] = 1;
	}
	Frames_Draw.visible[0] = 0;
	Frames_Draw.visible[1] = 1;
	Frames_Draw.visible[2] = 1;
	Frames_Draw.visible[3] = 1;

	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Dwarf_Draw, &Dwarf, 0, NULL, 3, 1, CRAY_MULTI_SCALE, PVR_FILTER_NONE, 0);
	Dwarf_Draw.coord[0].x = 50;
	Dwarf_Draw.coord[0].y = 20;
	Dwarf_Draw.coord[1].x = Dwarf_Draw.coord[0].x;
	Dwarf_Draw.coord[1].y = Dwarf_Draw.coord[0].y + (Dwarf_Draw.animation[0].frame_height / 2);
	Dwarf_Draw.coord[2].x = Dwarf_Draw.coord[0].x;
	Dwarf_Draw.coord[2].y = Dwarf_Draw.coord[1].y + Dwarf_Draw.animation[0].frame_height;
	Dwarf_Draw.layer[0] = 18;
	Dwarf_Draw.layer[1] = 18;
	Dwarf_Draw.layer[2] = 18;
	Dwarf_Draw.scale[0].x = 0.5;
	Dwarf_Draw.scale[0].y = 0.5;
	Dwarf_Draw.scale[1].x = 1;
	Dwarf_Draw.scale[1].y = 1;
	Dwarf_Draw.scale[2].x = 2;
	Dwarf_Draw.scale[2].y = 2;
	Dwarf_Draw.flip[0] = 0;
	Dwarf_Draw.rotation[0] = 0;
	Dwarf_Draw.colour[0] = 0xFF000000;
	Dwarf_Draw.fade[0] = 0;
	Dwarf_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw, 0, 0);
	for(i = 0; i < Dwarf_Draw.list_size; i++){
		Dwarf_Draw.visible[i] = 1;
	}

	//Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotation with sprites where height != width
	crayon_memory_init_sprite_array(&Red_Man_Draw, &Man, 0, &Red_Man_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Red_Man_Draw.coord[0].x = 50;
	Red_Man_Draw.coord[0].y = 280;
	Red_Man_Draw.layer[0] = 18;
	Red_Man_Draw.scale[0].x = 6;
	Red_Man_Draw.scale[0].y = 6;
	Red_Man_Draw.flip[0] = 0;
	Red_Man_Draw.rotation[0] = 450;
	Red_Man_Draw.colour[0] = 0xFF000000;
	Red_Man_Draw.fade[0] = 0;
	Red_Man_Draw.frame_id[0] = 0;
	Red_Man_Draw.visible[0] = 1;
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
	Green_Man_Draw.layer[1] = 50;
	Green_Man_Draw.layer[2] = 50;
	Green_Man_Draw.layer[3] = 50;
	Green_Man_Draw.layer[4] = 50;
	Green_Man_Draw.layer[5] = 50;
	Green_Man_Draw.layer[6] = 50;
	Green_Man_Draw.layer[7] = 50;
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
	Green_Man_Draw.colour[0] = 0xFF000000;
	Green_Man_Draw.fade[0] = 0;
	Green_Man_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Green_Man_Draw, 0, 0);
	for(i = 0; i < Green_Man_Draw.list_size; i++){
		Green_Man_Draw.visible[i] = 1;
	}

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
	Rainbow_Draw.layer[0] = 14;
	Rainbow_Draw.layer[1] = 14;
	Rainbow_Draw.layer[2] = 14;
	Rainbow_Draw.layer[3] = 14;
	Rainbow_Draw.layer[4] = 14;
	Rainbow_Draw.layer[5] = 14;
	Rainbow_Draw.layer[6] = 14;
	Rainbow_Draw.layer[7] = 14;
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
	Rainbow_Draw.colour[0] = 0xFF000000;
	Rainbow_Draw.fade[0] = 0;
	Rainbow_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Rainbow_Draw, 0, 0);
	for(i = 0; i < Rainbow_Draw.list_size; i++){
		Rainbow_Draw.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Man_BG, NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Man_BG.coord[0].x = Red_Man_Draw.coord[0].x;
	Man_BG.coord[0].y = Red_Man_Draw.coord[0].y;
	Man_BG.layer[0] = Red_Man_Draw.layer[0] - 1;
	Man_BG.scale[0].x = Red_Man_Draw.animation->frame_width * Red_Man_Draw.scale[0].x;
	Man_BG.scale[0].y = Red_Man_Draw.animation->frame_height * Red_Man_Draw.scale[0].y;
	Man_BG.rotation[0] = 0;
	Man_BG.colour[0] = 0xFF000000;
	Man_BG.fade[0] = 0;	//Probably not needed
	Man_BG.visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[0], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[0].coord[0].x = 0;
	Cam_BGs[0].coord[0].y = 0;
	Cam_BGs[0].layer[0] = 1;
	Cam_BGs[0].scale[0].x = 640;
	Cam_BGs[0].scale[0].y = 480;
	Cam_BGs[0].rotation[0] = 0;
	Cam_BGs[0].colour[0] = 0xFF888888;
	Cam_BGs[0].fade[0] = 0;
	Cam_BGs[0].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[1], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[1].coord[0].x = 140;
	Cam_BGs[1].coord[0].y = 32;
	Cam_BGs[1].layer[0] = 1;
	Cam_BGs[1].scale[0].x = 400;
	Cam_BGs[1].scale[0].y = 300;
	Cam_BGs[1].rotation[0] = 0;
	Cam_BGs[1].colour[0] = 0xFF888888;
	Cam_BGs[1].fade[0] = 0;
	Cam_BGs[1].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[2], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[2].coord[0].x = 160;
	Cam_BGs[2].coord[0].y = 120;
	Cam_BGs[2].layer[0] = 1;
	Cam_BGs[2].scale[0].x = 320;
	Cam_BGs[2].scale[0].y = 240;
	Cam_BGs[2].rotation[0] = 0;
	Cam_BGs[2].colour[0] = 0xFF888888;
	Cam_BGs[2].fade[0] = 0;
	Cam_BGs[2].visible[0] = 1;

	#define MODE 1

	if(MODE != 2){
		crayon_memory_init_sprite_array(&Cam_BGs[3], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
		Cam_BGs[3].coord[0].x = 160;
		Cam_BGs[3].coord[0].y = 120;
		Cam_BGs[3].layer[0] = 1;
		Cam_BGs[3].scale[0].x = 320;
		Cam_BGs[3].scale[0].y = 240;
		Cam_BGs[3].rotation[0] = 0;
		Cam_BGs[3].colour[0] = 0xFF888888;
		Cam_BGs[3].visible[0] = 1;
	}
	else{
		crayon_memory_init_sprite_array(&Cam_BGs[3], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
		Cam_BGs[3].coord[0].x = 20;
		Cam_BGs[3].coord[0].y = 40;
		Cam_BGs[3].layer[0] = 1;
		Cam_BGs[3].scale[0].x = 600;
		Cam_BGs[3].scale[0].y = 400;
		Cam_BGs[3].rotation[0] = 0;
		Cam_BGs[3].colour[0] = 0xFF888888;
		Cam_BGs[3].visible[0] = 1;
	}

	uint8_t current_camera_id = 0;
	#define NUM_CAMERAS 4
	crayon_viewport_t cameras[NUM_CAMERAS];
	crayon_viewport_t * current_camera = &cameras[current_camera_id];

	//This is the same as no camera
	crayon_memory_init_camera(&cameras[0], (vec2_f_t){Cam_BGs[0].coord[0].x,Cam_BGs[0].coord[0].y},
		(vec2_u16_t){Cam_BGs[0].scale[0].x,Cam_BGs[0].scale[0].y},
		(vec2_u16_t){Cam_BGs[0].coord[0].x,Cam_BGs[0].coord[0].y},
		(vec2_u16_t){Cam_BGs[0].scale[0].x,Cam_BGs[0].scale[0].y}, 1);

	//This is the basic view, no scaling, but we crop everything outside the inner 300/200 box
	crayon_memory_init_camera(&cameras[1], (vec2_f_t){Cam_BGs[1].coord[0].x,Cam_BGs[1].coord[0].y},
		(vec2_u16_t){Cam_BGs[1].scale[0].x,Cam_BGs[1].scale[0].y},
		(vec2_u16_t){Cam_BGs[1].coord[0].x,Cam_BGs[1].coord[0].y},
		(vec2_u16_t){Cam_BGs[1].scale[0].x,Cam_BGs[1].scale[0].y}, 1);

	//Magnify the selectioned zone (160,120 to 480, 360) The world is half the size of the window
	crayon_memory_init_camera(&cameras[2], (vec2_f_t){0,0},
		(vec2_u16_t){640,480},
		(vec2_u16_t){Cam_BGs[2].coord[0].x,Cam_BGs[2].coord[0].y},
		(vec2_u16_t){Cam_BGs[2].scale[0].x,Cam_BGs[2].scale[0].y}, 1);

	if(MODE == 0){
		crayon_memory_init_camera(&cameras[3], (vec2_f_t){0,0},
			(vec2_u16_t){320,240},
			(vec2_u16_t){Cam_BGs[3].coord[0].x,Cam_BGs[3].coord[0].y},
			(vec2_u16_t){Cam_BGs[3].scale[0].x,Cam_BGs[3].scale[0].y}, 1);
	}
	else if(MODE == 1){
		crayon_memory_init_camera(&cameras[3], (vec2_f_t){0,0},
			(vec2_u16_t){80,60},
			(vec2_u16_t){Cam_BGs[3].coord[0].x,Cam_BGs[3].coord[0].y},
			(vec2_u16_t){Cam_BGs[3].scale[0].x,Cam_BGs[3].scale[0].y}, 1);
	}
	else if(MODE == 2){
		crayon_memory_init_camera(&cameras[3], (vec2_f_t){0,0},
			(vec2_u16_t){150,100},
			(vec2_u16_t){Cam_BGs[3].coord[0].x,Cam_BGs[3].coord[0].y},
			(vec2_u16_t){Cam_BGs[3].scale[0].x,Cam_BGs[3].scale[0].y}, 1);
	}

	// char g_buffer[300];
	// for(i = 0; i < 7; i++){
	// 	__GRAPHICS_DEBUG_VARIABLES[i] = 0;
	// }

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Tahoma_P);
	crayon_graphics_setup_palette(&Red_Man_P);
	crayon_graphics_setup_palette(&Green_Man_P);

	char snum1[80];

	char instructions[320];	//I'm only using 298 chars, but I gave more space just to be safe
	set_msg(instructions);

	//World_movement_factor_adjustment
	int8_t scale_adjustment[4] = {0};

	//LTRB
	uint8_t last_dir = 0;
	uint8_t info_disp = 1;

	pvr_stats_t stats;
	pvr_get_stats(&stats);
	uint8_t escape = 0;
	uint32_t prev_btns[4] = {0};
	vec2_u8_t prev_trigs[4] = {(vec2_u8_t){0,0}};
	uint32_t curr_thumb = 0;
	// uint32_t prev_thumb = 0;
	vec2_f_t moved_on_frame;	//This is the distance to move the camera per frame. It makes moving James easier
	while(!escape){
		moved_on_frame = (vec2_f_t){0,0};
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		//Use this instead of the dpad
		curr_thumb = thumbstick_to_dpad(st->joyx, st->joyy, 0.4);

		if(curr_thumb & CONT_DPAD_LEFT){
			moved_on_frame.x = -1;
			last_dir = 0;
		}
		else if(curr_thumb & CONT_DPAD_RIGHT){
			moved_on_frame.x = 1;
			last_dir = 2;
		}

		if(curr_thumb & CONT_DPAD_UP){
			moved_on_frame.y = -1;
			last_dir = 1;
		}
		else if(curr_thumb & CONT_DPAD_DOWN){
			moved_on_frame.y = 1;
			last_dir = 3;
		}

		//Adjust the current world movement factor
		if((st->buttons & CONT_DPAD_UP) && !(prev_btns[__dev->port] & CONT_DPAD_UP)){
			scale_adjustment[current_camera_id]++;
			current_camera->world_movement_factor = pow(2, scale_adjustment[current_camera_id]);
		}
		else if((st->buttons & CONT_DPAD_DOWN) && !(prev_btns[__dev->port] & CONT_DPAD_DOWN)){
			scale_adjustment[current_camera_id]--;
			current_camera->world_movement_factor = pow(2, scale_adjustment[current_camera_id]);
		}

		if((st->buttons & CONT_A) && !(prev_btns[__dev->port] & CONT_A)){
			current_camera_id += 1;
			current_camera_id %= NUM_CAMERAS;
			current_camera = &cameras[current_camera_id];
		}

		if((st->buttons & CONT_B) && !(prev_btns[__dev->port] & CONT_B)){
			__GRAPHICS_DEBUG_VARIABLES[8]++;
			if(__GRAPHICS_DEBUG_VARIABLES[8] > 7){__GRAPHICS_DEBUG_VARIABLES[8] = 0;}
		}

		//Need to add the free-ing functions first
		if((st->buttons & CONT_START) && !(prev_btns[__dev->port] & CONT_START)){
			escape = 1;
		}

		if((st->rtrig > 0xFF * 0.1) && (prev_trigs[__dev->port].y <= 0xFF * 0.1)){
			switch(last_dir){
				case 0:
				moved_on_frame.x += -1;
				break;
				case 1:
				moved_on_frame.y += -1;
				break;
				case 2:
				moved_on_frame.x += 1;
				break;
				case 3:
				moved_on_frame.y += 1;
				break;
			}
		}

		if((st->ltrig > 0xFF * 0.1) && (prev_trigs[__dev->port].x <= 0xFF * 0.1)){
			switch(last_dir){
				case 0:
				moved_on_frame.x += 1;
				break;
				case 1:
				moved_on_frame.y += 1;
				break;
				case 2:
				moved_on_frame.x += -1;
				break;
				case 3:
				moved_on_frame.y += -1;
				break;
			}
		}

		if((st->buttons & CONT_Y) && !(prev_btns[__dev->port] & CONT_Y)){
			info_disp = !info_disp;
		}

		// prev_thumb = curr_thumb;
		prev_btns[__dev->port] = st->buttons;
		prev_trigs[__dev->port].x = st->ltrig;
		prev_trigs[__dev->port].y = st->rtrig;
		MAPLE_FOREACH_END()

		if(escape){break;}

		for(i = 0; i < NUM_CAMERAS; i++){
			crayon_memory_move_camera_x(&cameras[i], moved_on_frame.x);
			crayon_memory_move_camera_y(&cameras[i], moved_on_frame.y);
		}

		//Currently doesn't actually move you. Just updates the way you face
		move_james(&James_Draw, moved_on_frame, stats.frame_count);

		pvr_scene_begin();

		pvr_get_stats(&stats);

		Red_Man_Draw.rotation[0]++;
		if(Red_Man_Draw.rotation[0] > 360){
			Red_Man_Draw.rotation[0] -= 360;
		}

		if(stats.frame_count % 60 == 0){
			Frames_Draw.frame_id[0] = (Frames_Draw.frame_id[0] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[1] = (Frames_Draw.frame_id[1] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[2] = (Frames_Draw.frame_id[2] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[3] = (Frames_Draw.frame_id[3] + 1) % Frames_Draw.animation->frame_count;
		}

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Red_Man_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Green_Man_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_ENHANCED);

			//THe player sprite
			crayon_graphics_draw_sprites(&James_Draw, current_camera, PVR_LIST_PT_POLY, CRAY_DRAW_ENHANCED);

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			crayon_graphics_draw_sprites(&Frames_Draw, current_camera, PVR_LIST_OP_POLY, CRAY_DRAW_ENHANCED);
			// __GRAPHICS_DEBUG_VARIABLES[0] = 1;
			crayon_graphics_draw_sprites(&Rainbow_Draw, current_camera, PVR_LIST_OP_POLY, CRAY_DRAW_ENHANCED);
			// __GRAPHICS_DEBUG_VARIABLES[0] = 0;

			// sprintf(g_buffer, "%.4f\n%.4f\n%.4f\n%.4f\n%.4f\n%.4f\n",
			// 		__GRAPHICS_DEBUG_VARIABLES[1], __GRAPHICS_DEBUG_VARIABLES[2], __GRAPHICS_DEBUG_VARIABLES[3],
			// 		__GRAPHICS_DEBUG_VARIABLES[4], __GRAPHICS_DEBUG_VARIABLES[5], __GRAPHICS_DEBUG_VARIABLES[6]);

			// crayon_graphics_draw_text_mono(g_buffer, &BIOS, PVR_LIST_PT_POLY, 32, 280, 254, 1, 1, BIOS_P.palette_id);

			//Represents the boundry box for the red man when not rotated
			crayon_graphics_draw_sprites(&Man_BG, current_camera, PVR_LIST_OP_POLY, CRAY_DRAW_ENHANCED);

			//This represents the camera's space
			crayon_graphics_draw_sprites(&Cam_BGs[current_camera_id], NULL, PVR_LIST_OP_POLY, 0);

			if(info_disp){
				sprintf(snum1, "Camera ID: %d\nX: %.2f\nY: %.2f\nWorld_Movement_Factor: %.2f",
					current_camera_id, current_camera->world_x, current_camera->world_y, current_camera->world_movement_factor);

				crayon_graphics_draw_text_mono("World Coords:", &BIOS, PVR_LIST_PT_POLY, 32, 280, 254, 1, 1, BIOS_P.palette_id);
				crayon_graphics_draw_text_mono(snum1, &BIOS, PVR_LIST_PT_POLY, 32, 280 + (BIOS.char_height), 254, 1, 1, BIOS_P.palette_id);

				crayon_graphics_draw_text_mono(instructions, &BIOS, PVR_LIST_PT_POLY, 32, 368, 254, 1, 1, BIOS_P.palette_id);
			}

		pvr_list_finish();

		pvr_scene_finish();

	}

	//Confirm everything was unloaded successfully (Should equal zero)
	uint32_t retVal = 0;

	retVal += crayon_memory_free_spritesheet(&Dwarf);
	retVal += crayon_memory_free_spritesheet(&Opaque);
	retVal += crayon_memory_free_spritesheet(&Man);
	retVal += crayon_memory_free_spritesheet(&Characters);

	retVal += crayon_memory_free_mono_font_sheet(&BIOS);
	retVal += crayon_memory_free_prop_font_sheet(&Tahoma);

	retVal += crayon_memory_free_palette(&BIOS_P);
	retVal += crayon_memory_free_palette(&Tahoma_P);
	retVal += crayon_memory_free_palette(&Red_Man_P);
	retVal += crayon_memory_free_palette(&Green_Man_P);

	retVal += crayon_memory_free_sprite_array(&James_Draw);
	retVal += crayon_memory_free_sprite_array(&Dwarf_Draw);
	retVal += crayon_memory_free_sprite_array(&Rainbow_Draw);
	retVal += crayon_memory_free_sprite_array(&Frames_Draw);
	retVal += crayon_memory_free_sprite_array(&Red_Man_Draw);
	retVal += crayon_memory_free_sprite_array(&Green_Man_Draw);
	retVal += crayon_memory_free_sprite_array(&Man_BG);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[0]);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[1]);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[2]);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[3]);

	pvr_shutdown();

	return retVal;
}
