//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){
	printf("INITIAL\n");

	float g_deadspace = 0.4;

	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_OPTICAL)){
		return 1;
	}

	// Screen width/height
	float width, height;
	width = crayon_graphics_get_window_width();
	height = crayon_graphics_get_window_height();

	// Load the text
	;

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

	crayon_sprite_array_t Poly_Draw;	// Contains 3 polys, 1st (0 index) one controlled by the player
	crayon_memory_init_sprite_array(&Poly_Draw, NULL, 0, NULL, 3, 0, CRAY_MULTI_DIM | CRAY_MULTI_COLOUR | CRAY_MULTI_ROTATE,
		CRAYON_FILTER_NEAREST, 0);
	int i;
	for(i = 0; i < Poly_Draw.size; i++){
		Poly_Draw.visible[i] = 1;
		Poly_Draw.layer[i] = 2 + i;
	}

	Poly_Draw.scale[0].x = width * 0.20;
	Poly_Draw.scale[0].y = height * 0.20;
	Poly_Draw.coord[0].x = 20;
	Poly_Draw.coord[0].y = 20;
	Poly_Draw.colour[0] = 0xFFFF0000;
	Poly_Draw.rotation[0] = 0;

	Poly_Draw.scale[1].x = width * 0.5;
	Poly_Draw.scale[1].y = height * 0.1;
	Poly_Draw.coord[1].x = -60;
	Poly_Draw.coord[1].y = 150;
	Poly_Draw.colour[1] = 0xFF00FF00;
	Poly_Draw.rotation[1] = 0;

	Poly_Draw.scale[2].x = width * 0.2;
	Poly_Draw.scale[2].y = height * 0.2;
	Poly_Draw.coord[2].x = 200;
	Poly_Draw.coord[2].y = 0;
	Poly_Draw.colour[2] = 0xFF0000FF;
	Poly_Draw.rotation[2] = 225;

	uint8_t cropping = CRAYON_DRAW_CROP;
	uint8_t oob_culling = CRAYON_DRAW_OOB_SKIP;

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	uint32_t curr_btns[4] = {0};
	uint32_t prev_btns[4] = {0};
	int loop = 1;
	while(loop){
		// Graphics
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&Poly_Draw, &Camera, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED | cropping | oob_culling);
			crayon_graphics_draw_sprites(&Cam_BG, NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED);
		pvr_list_finish();

		pvr_scene_finish();

		// Input handling
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			prev_btns[__dev->port] = curr_btns[__dev->port];
			curr_btns[__dev->port] = st->buttons;
		MAPLE_FOREACH_END()

		// Buggy code
			// 0: 435.313721 175.195953
			// 1: 344.804047 84.686295
			// 2: 412.686279 16.804039
			// 3: 503.195953 107.313705
		// Solid code
			// 0: 435.313721 175.195953
			// 1: 344.804047 84.686295
			// 2: 503.195953 107.313705
			// 3: 412.686279 16.804039

		for(i = 0; i < 4; i++){
			// Start to terminate program
			if((curr_btns[i] & CONT_START) && !(prev_btns[i] & CONT_START)){
				loop = 0;
				break;
			}

			// A press to toggle OOB culling
			if((curr_btns[i] & CONT_A) && !(prev_btns[i] & CONT_A)){
				if(oob_culling){
					oob_culling = 0;
				}
				else{
					oob_culling = CRAYON_DRAW_OOB_SKIP;
				}
			}

			// B press to toggle Cropping mode
			if((curr_btns[i] & CONT_A) && !(prev_btns[i] & CONT_A)){
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

			// DPAD movement
			if((curr_btns[i] & CONT_DPAD_UP) && !(prev_btns[i] & CONT_DPAD_UP)){
				;
			}
			else if((curr_btns[i] & CONT_DPAD_DOWN) && !(prev_btns[i] & CONT_DPAD_DOWN)){
				;
			}
			if((curr_btns[i] & CONT_DPAD_LEFT) && !(prev_btns[i] & CONT_DPAD_LEFT)){
				;
			}
			else if((curr_btns[i] & CONT_DPAD_RIGHT) && !(prev_btns[i] & CONT_DPAD_RIGHT)){
				;
			}

			// if((curr_btns[i] & CONT_X) && !(prev_btns[i] & CONT_X)){
			// 	;
			// }
		}
	}

	crayon_memory_free_sprite_array(&Poly_Draw);

	return 0;
}
