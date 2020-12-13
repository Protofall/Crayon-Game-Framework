// Crayon libraries
#include <crayon/memory.h>
#include <crayon/debug.h>
#include <crayon/graphics.h>
#include <crayon/input.h>
#include <crayon/misc.h>

#include <dc/maple.h>
#include <dc/maple/controller.h> // For the "Press start to exit" thing

#include <math.h>

// Char count below is incorrect

//Char count:
//63
//28
//27
//39
//68
//72
//27
//29
//Total: 353
void set_msg(char *buffer){
	strcpy(buffer,
		"A: Cycles through cameras\n"
		"B: Toggles cropping and OOB checks\n"
		"X: Switch between simple and enhanced renderers\n"
		"Thumbstick: Moves the current camera around the world\n"
		"L/R-Triggers: Move one unit in the last direction moved\n"
		"D-PAD Up/Down: Inc/Decrease the WMF (Powers of 2)\n"
		"Start: Terminate program\n"
		"Y: Hides these instructions"
	);

	return;
}

uint8_t james_direction = 0;	// 0 = South, 1 = North, 2 = West, 3 = East
uint32_t james_start_frame = 0;	// Frame he starts moving in a direction

// 0 for changing direction. 1 for same direction. 2 for standing still
int8_t check_james_dir(vec2_f_t distance){
	if(distance.x == 0 && distance.y == 0){	// If not moving, return same way of facing
		return james_direction;
	}

	uint8_t x_dir = 4, y_dir = 4;	// Defaults only stay if x or y is exactly 0
	if(distance.y > 0){	// South
		y_dir = 0;
	}
	else if(distance.y < 0){	// North
		y_dir = 1;
	}

	if(distance.x < 0){	// Right
		x_dir = 2;
	}
	else if(distance.x > 0){	// Left
		x_dir = 3;
	}

	// Note, this has biased towards top and bottom against left and right
		// x_dir and y_dir are uninitialised if x or y is zero
		// If player stands still. It really messes up
	if(james_direction == 0 && james_direction == y_dir){return james_direction;}
	if(james_direction == 1 && james_direction == y_dir){return james_direction;}
	if(james_direction == 2 && james_direction == x_dir){return james_direction;}
	if(james_direction == 3 && james_direction == x_dir){return james_direction;}

	// This should never trigger
	if(x_dir > 3 || y_dir > 3){
		// error_freeze("Fail");
	}

	// In an effort to forcce facing the direction your pointing in
	if(y_dir < 4){
		return y_dir;
	}
	if(x_dir < 4){
		return x_dir;
	}
	return 0;	// Should never get here
}

void move_james(crayon_sprite_array_t * James, vec2_f_t distance, uint32_t current_frame){
	if(distance.x == 0 && distance.y == 0){	// Stationary
		James->frame_id[0] = (james_direction == 3) ? (3 * 2) : (3 * james_direction);
		James->flip[0] = (james_direction == 3) ? 1 : 0;
		return;
	}

	uint8_t direction = check_james_dir(distance);

	if(james_direction != direction){	// Changed direction
		// if(direction > 3){error_freeze("Fail2");}
		James->frame_id[0] = (direction == 3) ? (3 * 2) + 1 : (3 * direction) + 1;
		James->flip[0] = (direction == 3) ? 1 : 0;
		james_direction = direction;
		james_start_frame = current_frame;
	}
	else{	// Still walking in the same direction
		uint8_t time = (james_start_frame - current_frame) % 60;
		uint8_t frame_offset;
		if(time < 15){	// Right foot out
			frame_offset = 1;
		}
		else if(time < 30){	// Both legs same pos
			frame_offset = 0;
		}
		else if(time < 45){	// Left foot out
			frame_offset = 2;
		}
		else{	// Both legs same pos
			frame_offset = 0;
		}

		James->frame_id[0] = (direction == 3) ? (3 * 2) + frame_offset : (3 * direction) + frame_offset;
		James->flip[0] = (direction == 3) ? 1 : 0;
	}

	// Movement code too
	James->coord[0].x += distance.x;
	James->coord[0].y += distance.y;

	return;
}

char *renderer_func(uint8_t renderer){
	char *msgs[10] = {
		"Simple",
		"Enhanced",
		"ERROR"
	};

	if(renderer >= 2){
		return msgs[2];
	}

	return msgs[renderer];
}

char *crop_mode(uint8_t crop_oob_mode){
	char *msgs[26] = {
		"No Crop/OOB",
		"No Crop, using\n  OOB",
		"Hardware Crop,\n  no OOB",
		"Hardware Crop\n  and OOB",
		"Software Crop,\n  no OOB",
		"Software Crop\n  and OOB",
		"Full crop, no\n  OOB",
		"Full crop and\n  OOB",
		"ERROR"
	};

	// Get rid of the renderer part
	crop_oob_mode = crop_oob_mode >> 1;

	if(crop_oob_mode >= 8){
		return msgs[8];
	}

	return msgs[crop_oob_mode];
}

int main(){
	crayon_graphics_init(CRAYON_ENABLE_OP | CRAYON_ENABLE_TR | CRAYON_ENABLE_PT);

	crayon_spritesheet_t Dwarf, Opaque, Man, Characters;
	crayon_sprite_array_t Dwarf_Draw, Rainbow_Draw, Frames_Draw, Red_Man_Draw, Green_Man_Draw, Man_BG, James_Draw;
	#define NUM_CAMERAS 5
	crayon_sprite_array_t Cam_BGs[NUM_CAMERAS];
	crayon_font_prop_t Tahoma;
	crayon_font_mono_t BIOS;
	crayon_palette_t Tahoma_P, BIOS_P, Red_Man_P, Green_Man_P;

	crayon_memory_mount_romdisk("/cd/colourMod.img", "/files");

	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, 0, "/files/Fonts/Tahoma_font.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/files/Fonts/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Dwarf, NULL, -1, "/files/Dwarf.dtex");
	crayon_memory_load_spritesheet(&Opaque, NULL, -1, "/files/Opaque.dtex");
	crayon_memory_load_spritesheet(&Man, &Red_Man_P, 2, "/files/Man.dtex");	// Palette 3 will be reserved for Green Man
	crayon_memory_load_spritesheet(&Characters, NULL, 4, "/files/Characters.dtex");	// Since it has 23 colours, we'll just use ARGB1555

	fs_romdisk_unmount("/files");

	crayon_memory_init_sprite_array(&James_Draw, &Characters, 0, NULL, 1, 9, 0, PVR_FILTER_NONE, 0);
	James_Draw.scale[0].x = 2;
	James_Draw.scale[0].y = 2;
	James_Draw.coord[0].x = 307;	// These are about the mid-point given sprite sizes and scale
	James_Draw.coord[0].y = 220;
	James_Draw.layer[0] = 160;
	James_Draw.flip[0] = (james_direction == 3) ? 1 : 0;	// If facing East, use West sprite but flipped
	James_Draw.rotation[0] = 0;
	James_Draw.colour[0] = 0xFF000000;
	James_Draw.fade[0] = 0;
	James_Draw.frame_id[0] = (james_direction == 3) ? 3 * 2 : 3 * james_direction;	// If facing East, use west sprite but flipped
	James_Draw.visible[0] = 1;
	unsigned int i;
	for(i = 0; i < James_Draw.frames_used; i++){
		crayon_memory_set_frame_uv(&James_Draw, i, i);
	}

	// Draws 4 faces and rotates between all 12 faces
	crayon_memory_init_sprite_array(&Frames_Draw, &Opaque, 0, NULL, 4, 12, CRAY_MULTI_FRAME, PVR_FILTER_NONE, 0);
	Frames_Draw.scale[0].x = 3;
	Frames_Draw.scale[0].y = 3;
	Frames_Draw.coord[0].x = 120;
	Frames_Draw.coord[0].y = 270;
	Frames_Draw.coord[1].x = Frames_Draw.coord[0].x + (Frames_Draw.scale[0].x * Opaque.animation[0].frame_width);
	Frames_Draw.coord[1].y = Frames_Draw.coord[0].y;
	Frames_Draw.coord[2].x = Frames_Draw.coord[0].x;
	Frames_Draw.coord[2].y = Frames_Draw.coord[0].y + (Frames_Draw.scale[0].y * Opaque.animation[0].frame_height);
	Frames_Draw.coord[3].x = Frames_Draw.coord[1].x;
	Frames_Draw.coord[3].y = Frames_Draw.coord[2].y;
	Frames_Draw.layer[0] = 14;
	Frames_Draw.layer[1] = 14;
	Frames_Draw.layer[2] = 14;
	Frames_Draw.layer[3] = 14;
	Frames_Draw.flip[0] = 0;
	Frames_Draw.rotation[0] = 0;
	Frames_Draw.colour[0] = 0;
	Frames_Draw.fade[0] = 0;
	Frames_Draw.frame_id[0] = 0;
	Frames_Draw.frame_id[1] = 1;
	Frames_Draw.frame_id[2] = 2;
	Frames_Draw.frame_id[3] = 3;
	for(i = 0; i < Frames_Draw.frames_used; i++){
		crayon_memory_set_frame_uv(&Frames_Draw, i, i);
	}
	for(i = 0; i < Frames_Draw.size; i++){
		Frames_Draw.visible[i] = 1;
	}
	Frames_Draw.visible[0] = 0;

	// 3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Dwarf_Draw, &Dwarf, 0, NULL, 3, 4, CRAY_MULTI_SCALE, PVR_FILTER_NONE, 0);
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
	Dwarf_Draw.colour[0] = 0xFF000000;	// Needs full alpha apparently
	Dwarf_Draw.fade[0] = 0;
	Dwarf_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw, 0, 0);
	crayon_memory_set_frame_uv(&Dwarf_Draw, 1, 1);
	crayon_memory_set_frame_uv(&Dwarf_Draw, 2, 2);
	crayon_memory_set_frame_uv(&Dwarf_Draw, 3, 3);
	for(i = 0; i < Dwarf_Draw.size; i++){
		Dwarf_Draw.visible[i] = 1;
	}

	// Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotation with sprites where height != width
	crayon_memory_init_sprite_array(&Red_Man_Draw, &Man, 0, &Red_Man_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Red_Man_Draw.coord[0].x = 70;
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

	// Copy the red palette over and modify red with green
	crayon_memory_clone_palette(&Red_Man_P, &Green_Man_P, 3);
	crayon_memory_swap_colour(&Green_Man_P, 0xFFFF0000, 0xFF00D200, 0);

	// Sprite is 7 high by 14 wide. Showcases 90/270 degree angle rotation with sprites where height != width
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
	for(i = 0; i < Green_Man_Draw.size; i++){
		Green_Man_Draw.visible[i] = 1;
	}

	// 8 sprites, 1 frame, multi rotation and flip
	crayon_memory_init_sprite_array(&Rainbow_Draw, &Opaque, 1, NULL, 8, 1,
		CRAY_MULTI_FLIP | CRAY_MULTI_ROTATE | CRAY_COLOUR_ADD, PVR_FILTER_NONE, 0);
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
	Rainbow_Draw.layer[0] = 18;
	Rainbow_Draw.layer[1] = 18;
	Rainbow_Draw.layer[2] = 18;
	Rainbow_Draw.layer[3] = 18;
	Rainbow_Draw.layer[4] = 18;
	Rainbow_Draw.layer[5] = 18;
	Rainbow_Draw.layer[6] = 18;
	Rainbow_Draw.layer[7] = 18;
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
	Rainbow_Draw.colour[0] = 0x88000088;	// Half alpha and half blue
	Rainbow_Draw.fade[0] = 0xFF;
	Rainbow_Draw.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Rainbow_Draw, 0, 0);
	for(i = 0; i < Rainbow_Draw.size; i++){
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
	Man_BG.visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[0], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[0].coord[0].x = 0;
	Cam_BGs[0].coord[0].y = 0;
	Cam_BGs[0].layer[0] = 1;
	Cam_BGs[0].scale[0].x = 640;
	Cam_BGs[0].scale[0].y = 480;
	Cam_BGs[0].rotation[0] = 0;
	Cam_BGs[0].colour[0] = 0xFF888888;
	Cam_BGs[0].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[1], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[1].coord[0].x = 140;
	Cam_BGs[1].coord[0].y = 32;
	Cam_BGs[1].layer[0] = 1;
	Cam_BGs[1].scale[0].x = 400;
	Cam_BGs[1].scale[0].y = 300;
	Cam_BGs[1].rotation[0] = 0;
	Cam_BGs[1].colour[0] = 0xFF888888;
	Cam_BGs[1].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[2], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[2].coord[0].x = 160;
	Cam_BGs[2].coord[0].y = 80;
	Cam_BGs[2].layer[0] = 1;
	Cam_BGs[2].scale[0].x = 320;
	Cam_BGs[2].scale[0].y = 240;
	Cam_BGs[2].rotation[0] = 0;
	Cam_BGs[2].colour[0] = 0xFF888888;
	Cam_BGs[2].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[3], NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Cam_BGs[3].coord[0].x = 20;
	Cam_BGs[3].coord[0].y = 20;
	Cam_BGs[3].layer[0] = 1;
	Cam_BGs[3].scale[0].x = 600;
	Cam_BGs[3].scale[0].y = 400;
	Cam_BGs[3].rotation[0] = 0;
	Cam_BGs[3].colour[0] = 0xFF888888;
	Cam_BGs[3].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[4], NULL, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
	Cam_BGs[4].coord[0].x = 160;
	Cam_BGs[4].coord[0].y = 80;
	Cam_BGs[4].layer[0] = 1;
	Cam_BGs[4].scale[0].x = 320;
	Cam_BGs[4].scale[0].y = 240;
	Cam_BGs[4].rotation[0] = 0;
	Cam_BGs[4].colour[0] = 0xFF888888;
	Cam_BGs[4].visible[0] = 1;

	uint8_t current_camera_id = 0;
	crayon_viewport_t cameras[NUM_CAMERAS];
	crayon_viewport_t * current_camera = &cameras[current_camera_id];

	// This is the same as no camera
	crayon_memory_init_camera(&cameras[0],
		(vec2_f_t){Cam_BGs[0].coord[0].x, Cam_BGs[0].coord[0].y},
		(vec2_u16_t){Cam_BGs[0].scale[0].x, Cam_BGs[0].scale[0].y},
		(vec2_u16_t){Cam_BGs[0].coord[0].x, Cam_BGs[0].coord[0].y},
		(vec2_u16_t){Cam_BGs[0].scale[0].x, Cam_BGs[0].scale[0].y},
		1
	);

	// This is the basic view, no scaling, but we crop everything outside the inner 300/200 box
	crayon_memory_init_camera(&cameras[1],
		(vec2_f_t){Cam_BGs[1].coord[0].x, Cam_BGs[1].coord[0].y},
		(vec2_u16_t){Cam_BGs[1].scale[0].x, Cam_BGs[1].scale[0].y},
		(vec2_u16_t){Cam_BGs[1].coord[0].x, Cam_BGs[1].coord[0].y},
		(vec2_u16_t){Cam_BGs[1].scale[0].x, Cam_BGs[1].scale[0].y},
		1
	);

	// Magnify the selectioned zone (160, 120 to 480, 360) The world is half the size of the window
	crayon_memory_init_camera(&cameras[2],
		(vec2_f_t){0, 0},
		(vec2_u16_t){640, 480},
		(vec2_u16_t){Cam_BGs[2].coord[0].x, Cam_BGs[2].coord[0].y},
		(vec2_u16_t){Cam_BGs[2].scale[0].x, Cam_BGs[2].scale[0].y},
		1
	);

	// Zoomed in
	crayon_memory_init_camera(&cameras[3],
		(vec2_f_t){160, 100},
		(vec2_u16_t){300, 200},
		(vec2_u16_t){Cam_BGs[3].coord[0].x, Cam_BGs[3].coord[0].y},
		(vec2_u16_t){Cam_BGs[3].scale[0].x, Cam_BGs[3].scale[0].y},
		1
	);

	// Super zoomed in with small window
	crayon_memory_init_camera(&cameras[4],
		(vec2_f_t){0, 0},
		(vec2_u16_t){80, 60},
		(vec2_u16_t){Cam_BGs[4].coord[0].x, Cam_BGs[4].coord[0].y},
		(vec2_u16_t){Cam_BGs[4].scale[0].x, Cam_BGs[4].scale[0].y},
		1
	);

	crayon_graphics_set_bg_colour(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Tahoma_P);
	crayon_graphics_setup_palette(&Red_Man_P);
	crayon_graphics_setup_palette(&Green_Man_P);

	char snum1[200];

	char instructions[400];	// I'm only using ??? chars, but I gave more space just to be safe
	set_msg(instructions);

	// World_movement_factor_adjustment
	int8_t scale_adjustment[4] = {0};

	// LTRB
	uint8_t last_dir = 0;
	uint8_t info_disp = 1;

	uint8_t crop_oob_mode = CRAYON_DRAW_FULL_CROP;
	uint8_t renderer = CRAYON_DRAW_SIMPLE;

	pvr_stats_t stats;
	pvr_get_stats(&stats);

	// Getting inputs
	uint32_t curr_btns[4] = {0};
	uint32_t prev_btns[4] = {0};
	vec2_u8_t curr_trigs[4] = {{0,0}};
	vec2_u8_t prev_trigs[4] = {{0,0}};

	uint32_t curr_thumb[4] = {0};
	uint32_t prev_thumb[4] = {0};
	vec2_f_t moved_on_frame;	// This is the distance to move the camera per frame. It makes moving James easier

	uint8_t escape = 0;
	while(!escape){
		moved_on_frame = (vec2_f_t){0,0};
		pvr_wait_ready();

		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			prev_btns[__dev->port] = curr_btns[__dev->port];
			prev_trigs[__dev->port].x = curr_trigs[__dev->port].x;
			prev_trigs[__dev->port].y = curr_trigs[__dev->port].y;

			curr_btns[__dev->port] = st->buttons;
			curr_trigs[__dev->port].x = st->ltrig;
			curr_trigs[__dev->port].y = st->rtrig;

			// Use this instead of the dpad
			prev_thumb[__dev->port] = curr_thumb[__dev->port];
			curr_thumb[__dev->port] = crayon_input_thumbstick_to_dpad(st->joyx, st->joyy, 0.4);
		MAPLE_FOREACH_END()

		for(i = 0; i < 4; i++){
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_START)){
				escape = 1;
				break;
			}

			// Movement
			if(crayon_input_button_held(curr_thumb[i], CONT_DPAD_LEFT)){
				moved_on_frame.x = -1;
				last_dir = 0;
			}
			else if(crayon_input_button_held(curr_thumb[i], CONT_DPAD_RIGHT)){
				moved_on_frame.x = 1;
				last_dir = 2;
			}

			if(crayon_input_button_held(curr_thumb[i], CONT_DPAD_UP)){
				moved_on_frame.y = -1;
				last_dir = 1;
			}
			else if(crayon_input_button_held(curr_thumb[i], CONT_DPAD_DOWN)){
				moved_on_frame.y = 1;
				last_dir = 3;
			}

			// Adjust the current world movement factor
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_DPAD_UP)){
				scale_adjustment[current_camera_id]++;
				current_camera->world_movement_factor = pow(2, scale_adjustment[current_camera_id]);
			}
			else if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_DPAD_DOWN)){
				scale_adjustment[current_camera_id]--;
				current_camera->world_movement_factor = pow(2, scale_adjustment[current_camera_id]);
			}

			// Swap cameras
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_A)){
				current_camera_id += 1;
				current_camera_id %= NUM_CAMERAS;
				current_camera = &cameras[current_camera_id];
			}

			// Toggle oob/crop mode
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_B)){
				if(crop_oob_mode >= CRAYON_DRAW_FULL_CROP){
					crop_oob_mode = 0;
				}
				else{
					crop_oob_mode += 2;	// The first bit is responsible for enhanced/simple
				}
			}

			// Swap between enhanced and simple renderers
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_X)){
				if(renderer == CRAYON_DRAW_ENHANCED){
					renderer = CRAYON_DRAW_SIMPLE;
				}
				else{
					renderer = CRAYON_DRAW_ENHANCED;
				}
			}

			// Hide the info message
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_Y)){
				info_disp = !info_disp;
			}

			// Move by one unit
			if(crayon_input_trigger_pressed(curr_trigs[i].x, prev_trigs[i].x)){
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

			if(crayon_input_trigger_pressed(curr_trigs[i].y, prev_trigs[i].y)){
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
		}
		if(escape){break;}

		for(i = 0; i < NUM_CAMERAS; i++){
			crayon_memory_move_camera_x(&cameras[i], moved_on_frame.x);
			crayon_memory_move_camera_y(&cameras[i], moved_on_frame.y);
		}

		move_james(&James_Draw, moved_on_frame, stats.frame_count);

		pvr_scene_begin();

		pvr_get_stats(&stats);

		Red_Man_Draw.rotation[0]++;
		if(Red_Man_Draw.rotation[0] > 360){
			Red_Man_Draw.rotation[0] -= 360;
		}

		Man_BG.rotation[0] = Red_Man_Draw.rotation[0];
		if(renderer == CRAYON_DRAW_SIMPLE){
			float holder = fmod(Man_BG.rotation[0], 360.0);	// If angle is more than 360 degrees, this fixes that
			if(holder < 0){	// fmod has range of about -359 to +359, this changes it to 0 to +359
				holder += 360.0;
			}

			// For sprite mode don't simply "rotate" the verts because we want to avoid sin/cos, instead we need to change the uv
			if(crayon_misc_almost_equals(holder, 90.0, 45.0)){
				Man_BG.rotation[0] = 90;
			}
			else if(crayon_misc_almost_equals(holder, 180.0, 45.0)){
				Man_BG.rotation[0] = 180;
			}
			else if(crayon_misc_almost_equals(holder, 270.0, 45.0)){
				Man_BG.rotation[0] = 270;
			}
			else{
				Man_BG.rotation[0] = 0;
			}
		}

		// Animation of red man falling and faces rotating
		// if(stats.frame_count % 180 <= 45){
		// 	Red_Man_Draw.rotation[0] = 0;
		// }
		// else if(stats.frame_count % 180 <= 90){
		// 	Red_Man_Draw.rotation[0] = 90;
		// }
		// else if(stats.frame_count % 180 <= 135){
		// 	Red_Man_Draw.rotation[0] = 180;
		// }
		// else{
		// 	Red_Man_Draw.rotation[0] = 270;
		// }
		// Man_BG.rotation[0] = Red_Man_Draw.rotation[0];

		if(stats.frame_count % 60 == 0){
			Frames_Draw.frame_id[0] = (Frames_Draw.frame_id[0] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[1] = (Frames_Draw.frame_id[1] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[2] = (Frames_Draw.frame_id[2] + 1) % Frames_Draw.animation->frame_count;
			Frames_Draw.frame_id[3] = (Frames_Draw.frame_id[3] + 1) % Frames_Draw.animation->frame_count;
		}

		if(stats.frame_count % 10 == 0){
			Dwarf_Draw.frame_id[0] = (Dwarf_Draw.frame_id[0] + 1) % Dwarf_Draw.animation->frame_count;
		}

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, current_camera, PVR_LIST_PT_POLY, renderer | crop_oob_mode);
			crayon_graphics_draw_sprites(&Red_Man_Draw, current_camera, PVR_LIST_PT_POLY, renderer | crop_oob_mode);
			crayon_graphics_draw_sprites(&Green_Man_Draw, current_camera, PVR_LIST_PT_POLY, renderer | crop_oob_mode);

			crayon_graphics_draw_sprites(&James_Draw, current_camera, PVR_LIST_PT_POLY, renderer | crop_oob_mode);

			// // Fonts aren't supported by cameras yet
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

			crayon_graphics_draw_sprites(&Frames_Draw, current_camera, PVR_LIST_OP_POLY, renderer | crop_oob_mode);

			// Represents the boundry box for the red man when not rotated
			crayon_graphics_draw_sprites(&Man_BG, current_camera, PVR_LIST_OP_POLY, renderer | crop_oob_mode);

			// This represents the camera's space
			crayon_graphics_draw_sprites(&Cam_BGs[current_camera_id], NULL, PVR_LIST_OP_POLY, 0);

			if(info_disp){
				sprintf(snum1, "X: %.2f\nY: %.2f\nWMF: %.2f\nCamera ID: %d\nRenderer: %s\n%s",
					current_camera->world_x, current_camera->world_y, current_camera->world_movement_factor,
					current_camera_id, renderer_func(renderer), crop_mode(crop_oob_mode)
				);

				crayon_graphics_draw_text_mono(snum1, &BIOS, PVR_LIST_PT_POLY, 480, 340, 254, 1, 1, BIOS_P.palette_id);

				crayon_graphics_draw_text_mono(instructions, &BIOS, PVR_LIST_PT_POLY, 32, 340, 254, 1, 1, BIOS_P.palette_id);
			}

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_TR_POLY);

			crayon_graphics_draw_sprites(&Rainbow_Draw, current_camera, PVR_LIST_TR_POLY, renderer | crop_oob_mode);

		pvr_list_finish();

		pvr_scene_finish();

	}

	// Confirm everything was unloaded successfully (Should equal zero)
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
