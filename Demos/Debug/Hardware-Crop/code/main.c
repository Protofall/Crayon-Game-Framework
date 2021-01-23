//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>
#include <crayon/input.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){
	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		return 1;
	}

	// Screen width/height
	float width, height;
	width = crayon_graphics_get_window_width();
	height = crayon_graphics_get_window_height();

	// Load the text
	crayon_font_mono_t BIOS;
	crayon_palette_t BIOS_P;

	#if CRAYON_BOOT_MODE == CRAYON_BOOT_OPTICAL
		uint8_t ret = crayon_memory_mount_romdisk("/cd/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == CRAYON_BOOT_SD
		uint8_t ret = crayon_memory_mount_romdisk("/sd/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == CRAYON_BOOT_PC_LAN
		uint8_t ret = crayon_memory_mount_romdisk("/pc/stuff.img", "/files");
	#else
		#error "UNSUPPORTED BOOT MODE"
	#endif

	if(ret){
		error_freeze("Failed to load. %d, %s", __sd_present, __game_base_path);
	}

	int val = crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 0, "/files/BIOS.dtex");
	if(val){error_freeze("Issue loading BIOS font. %d", val);}

	fs_romdisk_unmount("/files");

	// What do I want to do
		// See if TA command persists to next frame
		// See if TA command from one list affects another list
	// Have 3 polys with max x and y's of
		// - 0 - 160
		// - 160 - 320
		// - 320 - 480
	;

	crayon_viewport_t Camera;
	crayon_memory_init_camera(&Camera,
		(vec2_f_t){0, 0},
		(vec2_u16_t){640, 480},
		(vec2_u16_t){0, 0},
		(vec2_u16_t){640, 480},
		1
	);

	crayon_sprite_array_t polys[3];
	crayon_memory_init_sprite_array(&polys[0], NULL, 0, NULL, 1, 0, 0, CRAYON_FILTER_NEAREST, 0);
	polys[0].scale[0].x = 640;
	polys[0].scale[0].y = 160;

	polys[0].coord[0].x = 0;
	polys[0].coord[0].y = 0;

	polys[0].colour[0] = 0xFFFF0000;
	polys[0].rotation[0] = 0;
	polys[0].visible[0] = 1;
	polys[0].layer[0] = 1;

	crayon_memory_init_sprite_array(&polys[1], NULL, 0, NULL, 1, 0, 0, CRAYON_FILTER_NEAREST, 0);
	polys[1].scale[0].x = 640;
	polys[1].scale[0].y = 160;

	polys[1].coord[0].x = 0;
	polys[1].coord[0].y = 160;

	polys[1].colour[0] = 0xFF00FF00;
	polys[1].rotation[0] = 0;
	polys[1].visible[0] = 1;
	polys[1].layer[0] = 1;

	crayon_memory_init_sprite_array(&polys[2], NULL, 0, NULL, 1, 0, 0, CRAYON_FILTER_NEAREST, 0);
	polys[2].scale[0].x = 640;
	polys[2].scale[0].y = 160;

	polys[2].coord[0].x = 0;
	polys[2].coord[0].y = 320;

	polys[2].colour[0] = 0xFF0000FF;
	polys[2].rotation[0] = 0;
	polys[2].visible[0] = 1;
	polys[2].layer[0] = 1;
	
	uint8_t cropping = CRAYON_DRAW_HARDWARE_CROP;
	uint8_t oob_culling = CRAYON_DRAW_CHECK_OOB;

	// The selected poly/list we want to modify
	uint8_t selected_poly = 0;
	uint8_t modes[3] = {0};
	uint8_t on[3] = {0};

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&BIOS_P);

	uint32_t curr_btns[4] = {0};
	uint32_t prev_btns[4] = {0};

	char options_msg[200];

	char msg[] = "Controls:\n"
		"D-PAD UP/DOWN to select a poly/list (OP, TR, PT)\n"
		"\"A\" to submit the cmd for this frame\n"
		"\"B\" to switch our cmd (full screen or windowed)\n"
		"\"X\" to make the next A press freeze the main loop\n"
		"START to terminate program\n";
	uint8_t lines = 6;

	vec2_u16_t region[2] = {{161,80},{479,400}};
	crayon_clipping_cmd_t Region_CMD = crayon_graphics_clamp_hardware_clip(region);

	vec2_u16_t region2[2] = {{0,0},{640,480}};
	region2[0].x = 0;
	region2[0].y = 0;
	region2[1].x = 640;
	region2[1].y = 480;
	crayon_clipping_cmd_t Region_CMD2 = crayon_graphics_clamp_hardware_clip(region2);

	uint8_t will_freeze = 0;
	unsigned int i;
	unsigned int loop = 1;
	while(loop){
		sprintf(options_msg, "OOB: %d, Crop: %d, Poly/list: %d\n ON: %d, %d, %d. modes: %d, %d, %d\nWill Freeze? %d",
			oob_culling, cropping, selected_poly, on[0], on[1], on[2], modes[0], modes[1], modes[2], will_freeze);

		// Graphics
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			if(on[0]){
				crayon_graphics_set_hardware_clip(modes[0] == 0 ? &Region_CMD : &Region_CMD2);
			}

			crayon_graphics_draw_sprites(&polys[0], NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED | cropping | oob_culling);

			crayon_graphics_draw_text_mono(options_msg, &BIOS, PVR_LIST_OP_POLY, 32, height - 24 - ((lines + 3.5) * BIOS.char_height), 254, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(msg, &BIOS, PVR_LIST_OP_POLY, 32, height - 24 - (lines * BIOS.char_height), 254, 1, 1, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_TR_POLY);
			if(on[1]){
				crayon_graphics_set_hardware_clip(modes[1] == 0 ? &Region_CMD : &Region_CMD2);
			}

			crayon_graphics_draw_sprites(&polys[1], NULL, PVR_LIST_TR_POLY, CRAYON_DRAW_ENHANCED | cropping | oob_culling);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);
			if(on[2]){
				crayon_graphics_set_hardware_clip(modes[2] == 0 ? &Region_CMD : &Region_CMD2);
			}

			crayon_graphics_draw_sprites(&polys[2], NULL, PVR_LIST_PT_POLY, CRAYON_DRAW_ENHANCED | cropping | oob_culling);
		pvr_list_finish();

		pvr_scene_finish();

		// So turns out the command persists through frames and its applied in logical order
			// If we only submit to TR then for one frame only the TR and PT stuff would be rendered because
			// OP was before TR. DO NOTE if say PT appeared first and we did PT then everything renders since
			// that was first
		if((on[0] || on[1] || on[2]) && will_freeze){
			while(1){;}
		}

		// Input handling
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			prev_btns[__dev->port] = curr_btns[__dev->port];

			curr_btns[__dev->port] = st->buttons;
		MAPLE_FOREACH_END()

		on[0] = 0;
		on[1] = 0;
		on[2] = 0;

		for(i = 0; i < 4; i++){
			// Start to terminate program
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_START)){
				loop = 0;
				break;
			}

			// A press submit the command for just this frame
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_A)){
				on[selected_poly] = 1;
			}

			// B press to change between full screen and boxed
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_B)){
				modes[selected_poly] = !modes[selected_poly];
			}

			// X press enables a freeze after the next A press
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_X)){
				will_freeze = !will_freeze;
			}

			// If its selected, we don't want to change it now
			if(on[selected_poly]){
				continue;
			}

			// Choose which poly poly/list to select
			if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_DPAD_UP)){
				if(selected_poly > 0){
					selected_poly--;
				}
			}
			else if(crayon_input_button_pressed(curr_btns[i], prev_btns[i], CONT_DPAD_DOWN)){
				if(selected_poly < 2){
					selected_poly++;
				}
			}
		}
	}

	// Confirm everything was unloaded successfully (Should equal zero)
	uint32_t return_val = 0;

	return_val += crayon_memory_free_mono_font_sheet(&BIOS);
	return_val += crayon_memory_free_palette(&BIOS_P);

	return_val += crayon_memory_free_sprite_array(&polys[0]);
	return_val += crayon_memory_free_sprite_array(&polys[1]);
	return_val += crayon_memory_free_sprite_array(&polys[2]);

	if(return_val){
		error_freeze("An error occured in shutdown");
	}

	crayon_shutdown();

	return 0;
}
