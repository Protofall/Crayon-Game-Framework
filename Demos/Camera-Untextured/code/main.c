//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>
#include <crayon/input.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){
	// float g_deadspace = 0.4;

	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_OPTICAL)){
		return 1;
	}

	// Screen width/height
	float width, height;
	width = crayon_graphics_get_window_width();
	height = crayon_graphics_get_window_height();

	// Load the text
	crayon_font_mono_t BIOS;
	crayon_palette_t BIOS_P;

	// CHANGE THIS SO ITS USING CD, SD OR PC
	crayon_memory_mount_romdisk("/cd/stuff.img", "/files");

	int val = crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 0, "/files/BIOS.dtex");
	if(val){error_freeze("Issue loading BIOS font. %d", val);}

	fs_romdisk_unmount("/files");

	// Setup the camera and untextured polys
	crayon_sprite_array_t Cam_BG;
	crayon_memory_init_sprite_array(&Cam_BG, NULL, 0, NULL, 1, 0, 0, CRAYON_FILTER_NEAREST, 0);
	Cam_BG.scale[0].x = width * 0.5;
	Cam_BG.scale[0].y = height * 0.5;

	Cam_BG.coord[0].x = width * 0.25;
	Cam_BG.coord[0].y = height * 0.10;
	Cam_BG.colour[0] = 0xFF888888;
	Cam_BG.rotation[0] = 0;
	Cam_BG.visible[0] = 1;
	Cam_BG.layer[0] = 1;

	crayon_viewport_t Camera;
	crayon_memory_init_camera(&Camera,
		(vec2_f_t){0, 0},
		(vec2_u16_t){Cam_BG.scale[0].x, Cam_BG.scale[0].y},
		(vec2_u16_t){Cam_BG.coord[0].x, Cam_BG.coord[0].y},
		(vec2_u16_t){Cam_BG.scale[0].x, Cam_BG.scale[0].y},
		1
	);

	crayon_sprite_array_t Poly_Draw;	// Contains 4 polys, 1st (0 index) one controlled by the player
	crayon_memory_init_sprite_array(&Poly_Draw, NULL, 0, NULL, 4, 0,
		CRAYON_MULTI_DIM | CRAYON_MULTI_COLOUR | CRAYON_MULTI_ROTATE, CRAYON_FILTER_NEAREST, 0);
	unsigned int i;
	for(i = 0; i < Poly_Draw.size; i++){
		Poly_Draw.visible[i] = 1;
		Poly_Draw.layer[i] = (Poly_Draw.size - i + 1);	// 5, 4, 3, 2
	}

	// Player white poly
	Poly_Draw.scale[0].x = width * 0.20;
	Poly_Draw.scale[0].y = height * 0.20;
	Poly_Draw.coord[0].x = 20;
	Poly_Draw.coord[0].y = 20;
	Poly_Draw.colour[0] = 0xFFFFFFFF;
	Poly_Draw.rotation[0] = 0;

	// Green poly
	Poly_Draw.scale[1].x = width * 0.35;
	Poly_Draw.scale[1].y = height * 0.1;
	Poly_Draw.coord[1].x = 10;
	Poly_Draw.coord[1].y = 150;
	Poly_Draw.colour[1] = 0xFF00FF00;
	Poly_Draw.rotation[1] = 0;

	// Blue poly
	Poly_Draw.scale[2].x = width * 0.2;
	Poly_Draw.scale[2].y = height * 0.2;
	Poly_Draw.coord[2].x = 200;
	Poly_Draw.coord[2].y = 0;
	Poly_Draw.colour[2] = 0xFF0000FF;
	Poly_Draw.rotation[2] = 225;

	// Red poly
	Poly_Draw.scale[3].x = width * 0.15;
	Poly_Draw.scale[3].y = height * 0.5;
	Poly_Draw.coord[3].x = Cam_BG.scale[0].x + 20;
	Poly_Draw.coord[3].y = Cam_BG.scale[0].y - 40;
	Poly_Draw.colour[3] = 0xFFFF0000;
	Poly_Draw.rotation[3] = 1337;
	
	uint8_t cropping = CRAYON_DRAW_CROP;
	uint8_t oob_culling = CRAYON_DRAW_CHECK_OOB;

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&BIOS_P);

	uint32_t curr_btns[4] = {0};
	uint32_t prev_btns[4] = {0};
	vec2_u8_t curr_trigs[4] = {{0,0}};
	// vec2_u8_t prev_trigs[4] = {{0,0}};

	uint8_t move_mode = 1;

	char options_msg[80];

	char msg[] = "Controls\n"
		"D-PAD to move white poly around or change its dimensions\n"
		"\"A\" toggls OOB culling\n"
		"\"B\" toggles the cropping modes\n"
		"\"X\" moves the white poly to where it started\n"
		"\"Y\" toggles which D-PAD mode its in\n"
		"Left Trigger to rotate anti-clockwise and Right trigger for clockwise\n"
		"START to terminate program\n";
	uint8_t lines = 8;

	int loop = 1;
	while(loop){
		sprintf(options_msg, "Angle: %g, Coords: {%.1f, %.1f}, OOB: %d, Crop: %d, Move: %d",
			Poly_Draw.rotation[0], Poly_Draw.coord[0].x, Poly_Draw.coord[0].y, oob_culling, cropping, move_mode);

		// Graphics
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&Cam_BG, NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Poly_Draw, &Camera, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED | cropping | oob_culling);

			crayon_graphics_draw_text_mono(options_msg, &BIOS, PVR_LIST_OP_POLY, 32, height - 24 - ((lines + 1.5) * BIOS.char_height), 254, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(msg, &BIOS, PVR_LIST_OP_POLY, 32, height - 24 - (lines * BIOS.char_height), 254, 1, 1, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();

		// Input handling
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			prev_btns[__dev->port] = curr_btns[__dev->port];
			// prev_trigs[__dev->port].x = curr_trigs[__dev->port].x;
			// prev_trigs[__dev->port].y = curr_trigs[__dev->port].y;

			curr_btns[__dev->port] = st->buttons;
			curr_trigs[__dev->port].x = st->ltrig;
			curr_trigs[__dev->port].y = st->rtrig;
		MAPLE_FOREACH_END()

		for(i = 0; i < 4; i++){
			// Start to terminate program
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_START)){
				loop = 0;
				break;
			}

			// A press to toggle OOB culling
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_A)){
				if(oob_culling){
					oob_culling = 0;
				}
				else{
					oob_culling = CRAYON_DRAW_CHECK_OOB;
				}
			}

			// B press to toggle Cropping mode
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_B)){
				if(cropping == 0){
					cropping = CRAYON_DRAW_HARDWARE_CROP;
				}
				else if(cropping == CRAYON_DRAW_HARDWARE_CROP){
					cropping = CRAYON_DRAW_CROP;
				}
				else{
					cropping = 0;
				}
			}

			// X press Move poly to where it started
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_X)){
				Poly_Draw.coord[0].x = 20;
				Poly_Draw.coord[0].y = 20;
			}

			// Y press to toggle movement/size mode
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_Y)){
				move_mode = !move_mode;
			}

			// DPAD movement
			if(move_mode){
				if(crayon_input_button_held(curr_btns[i], CONT_DPAD_UP)){
					Poly_Draw.coord[0].y -= 1;
				}
				else if(crayon_input_button_held(curr_btns[i], CONT_DPAD_DOWN)){
					Poly_Draw.coord[0].y += 1;
				}
				if(crayon_input_button_held(curr_btns[i], CONT_DPAD_LEFT)){
					Poly_Draw.coord[0].x -= 1;
				}
				else if(crayon_input_button_held(curr_btns[i], CONT_DPAD_RIGHT)){
					Poly_Draw.coord[0].x += 1;
				}
			}
			else{
				if(crayon_input_button_held(curr_btns[i], CONT_DPAD_UP)){
					Poly_Draw.coord[0].y += 1;
					Poly_Draw.scale[0].y -= 2;
				}
				else if(crayon_input_button_held(curr_btns[i], CONT_DPAD_DOWN)){
					Poly_Draw.coord[0].y -= 1;
					Poly_Draw.scale[0].y += 2;
				}
				if(crayon_input_button_held(curr_btns[i], CONT_DPAD_LEFT)){
					Poly_Draw.coord[0].x += 1;
					Poly_Draw.scale[0].x -= 2;
				}
				else if(crayon_input_button_held(curr_btns[i], CONT_DPAD_RIGHT)){
					Poly_Draw.coord[0].x -= 1;
					Poly_Draw.scale[0].x += 2;
				}
			}

			// Rotation
			if(crayon_input_trigger_held(curr_trigs[i].x)){	// Left trigger
				Poly_Draw.rotation[0] -= 1;
			}
			if(crayon_input_trigger_held(curr_trigs[i].y)){	// Right trigger
				Poly_Draw.rotation[0] += 1;
			}
		}
	}

	// Confirm everything was unloaded successfully (Should equal zero)
	uint32_t return_val = 0;

	return_val += crayon_memory_free_mono_font_sheet(&BIOS);
	return_val += crayon_memory_free_palette(&BIOS_P);

	return_val += crayon_memory_free_sprite_array(&Poly_Draw);

	if(return_val){
		error_freeze("An error occured in shutdown");
	}

	crayon_shutdown();

	return 0;
}
