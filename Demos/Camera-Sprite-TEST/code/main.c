//Crayon libraries
#include <crayon/memory.h>
#include <crayon/debug.h>
#include <crayon/graphics.h>
#include <crayon/input.h>

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

#include <math.h>

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

	crayon_spritesheet_t Dwarf;
	crayon_sprite_array_t Dwarf_Draw;
	crayon_sprite_array_t Cam_BGs[4];
	crayon_font_prop_t Tahoma;
	crayon_font_mono_t BIOS;
	crayon_palette_t Tahoma_P, BIOS_P;

	crayon_memory_mount_romdisk("/cd/colourMod.img", "/files");

	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, 0, "/files/Fonts/Tahoma_font.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/files/Fonts/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Dwarf, NULL, -1, "/files/Dwarf.dtex");

	fs_romdisk_unmount("/files");









	//WITH THE CAMERA THAT SHOWS THE BUG, DWARFY CAN BE FOUND AROUND X = 81, Y = 62











	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Dwarf_Draw, &Dwarf, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
	Dwarf_Draw.coord[0].x = 50;
	Dwarf_Draw.coord[0].y = 20;
	Dwarf_Draw.layer[0] = 18;
	Dwarf_Draw.scale[0].x = 2;
	Dwarf_Draw.scale[0].y = 2;
	Dwarf_Draw.flip[0] = 0;
	Dwarf_Draw.rotation[0] = 0;
	Dwarf_Draw.colour[0] = 0xFF000000;	// Need full alpha for Opaque render list
	Dwarf_Draw.fade[0] = 0;
	Dwarf_Draw.frame_id[0] = 0;
	Dwarf_Draw.visible[0] = 1;
	crayon_memory_set_frame_uv(&Dwarf_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Cam_BGs[0], NULL, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
	Cam_BGs[0].coord[0].x = 0;
	Cam_BGs[0].coord[0].y = 0;
	Cam_BGs[0].layer[0] = 1;
	Cam_BGs[0].scale[0].x = 640;
	Cam_BGs[0].scale[0].y = 480;
	Cam_BGs[0].rotation[0] = 0;
	Cam_BGs[0].colour[0] = 0xFF888888;
	Cam_BGs[0].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[1], NULL, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
	Cam_BGs[1].coord[0].x = 140;
	Cam_BGs[1].coord[0].y = 32;
	Cam_BGs[1].layer[0] = 1;
	Cam_BGs[1].scale[0].x = 400;
	Cam_BGs[1].scale[0].y = 300;
	Cam_BGs[1].rotation[0] = 0;
	Cam_BGs[1].colour[0] = 0xFF888888;
	Cam_BGs[1].visible[0] = 1;

	crayon_memory_init_sprite_array(&Cam_BGs[2], NULL, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
	Cam_BGs[2].coord[0].x = 160;
	Cam_BGs[2].coord[0].y = 120;
	Cam_BGs[2].layer[0] = 1;
	Cam_BGs[2].scale[0].x = 320;
	Cam_BGs[2].scale[0].y = 240;
	Cam_BGs[2].rotation[0] = 0;
	Cam_BGs[2].colour[0] = 0xFF888888;
	Cam_BGs[2].visible[0] = 1;

	#define MODE 1

	if(MODE != 2){
		crayon_memory_init_sprite_array(&Cam_BGs[3], NULL, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
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
		crayon_memory_init_sprite_array(&Cam_BGs[3], NULL, 0, NULL, 1, 1, CRAY_NO_MULTIS, PVR_FILTER_NONE, 0);
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

	// Unused buffer for rendering the graphics vars
	char g_buffer[300];

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Tahoma_P);

	char snum1[100];

	char instructions[320];	//I'm only using 298 chars, but I gave more space just to be safe
	set_msg(instructions);

	//World_movement_factor_adjustment
	int8_t scale_adjustment[4] = {0};

	//LTRB
	uint8_t last_dir = 0;
	uint8_t info_disp = 1;

	uint8_t RENDERER = CRAYON_DRAW_SIMPLE;

	unsigned int i;

	uint32_t prev_btns[4] = {0};
	vec2_u8_t prev_trigs[4] = {(vec2_u8_t){0,0}};
	uint32_t curr_thumb = 0;

	vec2_f_t moved_on_frame;	//This is the distance to move the camera per frame. It makes moving James easier

	uint8_t escape = 0;
	while(!escape){
		moved_on_frame = (vec2_f_t){0,0};
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		//Use this instead of the dpad
		curr_thumb = crayon_input_thumbstick_to_dpad(st->joyx, st->joyy, 0.4);

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
			if(RENDERER == CRAYON_DRAW_SIMPLE){
				RENDERER = CRAYON_DRAW_ENHANCED;
			}
			else{
				RENDERER = CRAYON_DRAW_SIMPLE;
			}
		}

		if((st->buttons & CONT_X) && !(prev_btns[__dev->port] & CONT_X)){
			current_camera_id += 1;
			current_camera_id %= NUM_CAMERAS;
			current_camera = &cameras[current_camera_id];
		}

		if((st->buttons & CONT_B) && !(prev_btns[__dev->port] & CONT_B)){
			// __CRAYON_GRAPHICS_DEBUG_VARS[8]++;
			// if(__CRAYON_GRAPHICS_DEBUG_VARS[8] > 7){__CRAYON_GRAPHICS_DEBUG_VARS[8] = 0;}
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

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, current_camera, PVR_LIST_PT_POLY,
				RENDERER | CRAYON_DRAW_CHECK_OOB | CRAYON_DRAW_SOFTWARE_CROP);

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			// __CRAYON_GRAPHICS_DEBUG_VARS[0] = 1;
			// __CRAYON_GRAPHICS_DEBUG_VARS[0] = 0;

			// sprintf(g_buffer, "%.4f\n%.4f\n%.4f\n%.4f\n%.4f\n%.4f\n",
			// 		__CRAYON_GRAPHICS_DEBUG_VARS[1], __CRAYON_GRAPHICS_DEBUG_VARS[2], __CRAYON_GRAPHICS_DEBUG_VARS[3],
			// 		__CRAYON_GRAPHICS_DEBUG_VARS[4], __CRAYON_GRAPHICS_DEBUG_VARS[5], __CRAYON_GRAPHICS_DEBUG_VARS[6]);

			// crayon_graphics_draw_text_mono(g_buffer, &BIOS, PVR_LIST_PT_POLY, 32, 280, 254, 1, 1, BIOS_P.palette_id);

			//This represents the camera's space
			crayon_graphics_draw_sprites(&Cam_BGs[current_camera_id], NULL, PVR_LIST_OP_POLY, 0);

			if(info_disp){
				sprintf(snum1, "Camera ID: %d. Renderer: %d\nX: %.2f\nY: %.2f\nWorld_Movement_Factor: %.2f",
					current_camera_id, RENDERER, current_camera->world_x, current_camera->world_y, current_camera->world_movement_factor);

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

	retVal += crayon_memory_free_mono_font_sheet(&BIOS);
	retVal += crayon_memory_free_prop_font_sheet(&Tahoma);

	retVal += crayon_memory_free_palette(&BIOS_P);
	retVal += crayon_memory_free_palette(&Tahoma_P);

	retVal += crayon_memory_free_sprite_array(&Dwarf_Draw);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[0]);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[1]);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[2]);
	retVal += crayon_memory_free_sprite_array(&Cam_BGs[3]);

	pvr_shutdown();

	return retVal;
}
