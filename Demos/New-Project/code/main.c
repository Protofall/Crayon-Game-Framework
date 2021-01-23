// Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/input.h>
#include <crayon/crayon.h>

// For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

void init_fade_struct(crayon_sprite_array_t *fade_draw){
	crayon_memory_init_sprite_array(fade_draw, NULL, 0, NULL, 1, 0, 0, PVR_FILTER_NONE, 0);
	fade_draw->coord[0].x = 0;
	fade_draw->coord[0].y = 0;
	fade_draw->scale[0].x = crayon_graphics_get_window_width();
	fade_draw->scale[0].y = crayon_graphics_get_window_height();
	fade_draw->colour[0] = 0xFF000000;	// Full Black
	fade_draw->rotation[0] = 0;
	fade_draw->visible[0] = 1;
	fade_draw->layer[0] = 255;
}

void hz_select(crayon_sprite_array_t *fade_draw){
	// If we have a VGA cable, then skip this screen
	if(vid_check_cable() == CT_VGA){return;}

	crayon_font_mono_t BIOS_font;

	#if CRAYON_BOOT_MODE == CRAYON_BOOT_OPTICAL
		crayon_memory_mount_romdisk("/cd/stuff.img", "/Setup");
	#elif CRAYON_BOOT_MODE == CRAYON_BOOT_SD
		crayon_memory_mount_romdisk("/sd/stuff.img", "/Setup");
	#elif CRAYON_BOOT_MODE == CRAYON_BOOT_PC_LAN
		crayon_memory_mount_romdisk("/pc/stuff.img", "/Setup");
	#else
		#error "Invalid CRAYON_BOOT_MODE"
	#endif

	crayon_palette_t BIOS_P;		// Entry 0
	crayon_palette_t BIOS_Red_P;	// Entry 1

	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 0, "/Setup/BIOS.dtex");

	fs_romdisk_unmount("/Setup");

	// Make the red font
	crayon_memory_clone_palette(&BIOS_P, &BIOS_Red_P, 1);
	crayon_memory_swap_colour(&BIOS_Red_P, 0xFFAAAAAA, 0xFFFF0000, 0);

	// Set the palettes in graphics memory
	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&BIOS_Red_P);

	uint16_t hz_head_x = (crayon_graphics_get_window_width() -
		(2 * crayon_graphics_string_get_length_mono(&BIOS_font, "Select Refresh Rate"))) / 2;
	uint16_t hz_head_y = 133;
	uint16_t hz_option_y = hz_head_y + (BIOS_font.char_height * 2) + 80;

	uint16_t sixty_hz_width = 2 * crayon_graphics_string_get_length_mono(&BIOS_font, "60Hz");

	// Set the palettes for each option
		// Red is highlighted, white is normal
	int8_t *palette_50hz, *palette_60hz;

	if(__hz == 50){	// For highlights
		palette_50hz = &BIOS_Red_P.palette_id;
		palette_60hz = &BIOS_P.palette_id;
	}
	else{
		palette_50hz = &BIOS_P.palette_id;
		palette_60hz = &BIOS_Red_P.palette_id;
	}
	
	int16_t counter = __hz  * 11;	// Set this to 11 secs (So current fps * 11)
									// We choose 11 seconds because the fade-in takes 1 second

	uint8_t fade_frame_count = 0;
	float fade_cap = __hz * 1.0;	// 1 second, 60 or 50 frames


	// ---------------------------------------


	unsigned int i;
	uint32_t prev_buttons[4] = {0};
	uint32_t curr_buttons[4] = {0};

	// While counting || the faded effect isn't fully opaque
	while(counter > 0 || crayon_misc_extract_bits(fade_draw->colour[0], 8, 24) != 0xFF){
		// Update the fade-in/out effects
		counter--;
		if(counter < 0){	// Fading out
			if(crayon_misc_extract_bits(fade_draw->colour[0], 8, 24) == 0){
				// Have another sound, not a blip, but some acceptance thing
			}
			fade_draw->colour[0] = (uint32_t)(0xFF * (fade_frame_count / fade_cap)) << 24;
			fade_frame_count++;
		}
		else if(crayon_misc_extract_bits(fade_draw->colour[0], 8, 24) > 0){	// fading in
			fade_draw->colour[0] = (uint32_t)(0xFF - (0xFF * (fade_frame_count / fade_cap))) << 24;
			fade_frame_count++;
		}
		else{	// 10 seconds of choice
			fade_frame_count = 0;
		}


		// Don't really want players doing stuff when it fades out
		if(counter >= 0){
			// Poll for input
			MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
				prev_buttons[__dev->port] = curr_buttons[__dev->port];
				curr_buttons[__dev->port] = st->buttons;
			MAPLE_FOREACH_END()

			// Process input
			for(i = 0; i < 4; i++){
				if(crayon_input_button_pressed(curr_buttons[i], prev_buttons[i], CONT_DPAD_LEFT)){
					palette_50hz = &BIOS_Red_P.palette_id;
					palette_60hz = &BIOS_P.palette_id;
				}
				else if(crayon_input_button_pressed(curr_buttons[i], prev_buttons[i], CONT_DPAD_RIGHT)){
					palette_50hz = &BIOS_P.palette_id;
					palette_60hz = &BIOS_Red_P.palette_id;
				}

				// Press A or Start to skip the countdown
					// We can use held because if this enters, it will never come back here
				if(crayon_input_button_held(curr_buttons[i], CONT_A | CONT_START)){
					counter = 0;
					break;
				}
			}
		}

		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

			// The fading in/out effect
			if(crayon_misc_extract_bits(fade_draw->colour[0], 8, 24)){
				crayon_graphics_draw_sprites(fade_draw, NULL, PVR_LIST_TR_POLY, CRAYON_DRAW_SIMPLE);
			}

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			crayon_graphics_draw_text_mono("Select Refresh Rate", &BIOS_font, PVR_LIST_OP_POLY, hz_head_x, hz_head_y, 50, 2, 2,
				BIOS_P.palette_id);

			// Render these strings 60 pixels odd of each side of the screen
			crayon_graphics_draw_text_mono("50Hz", &BIOS_font, PVR_LIST_OP_POLY, 60, hz_option_y, 50, 2, 2, *palette_50hz);
			crayon_graphics_draw_text_mono("60Hz", &BIOS_font, PVR_LIST_OP_POLY, crayon_graphics_get_window_width() - 60 - sixty_hz_width,
				hz_option_y, 50, 2, 2, *palette_60hz);

		pvr_list_finish();

		pvr_scene_finish();

	}

	if(__hz == 50 && *palette_60hz == BIOS_Red_P.palette_id){
		__hz = 60;
		__hz_adjustment = 1;
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}
	else if(__hz == 60 && *palette_50hz == BIOS_Red_P.palette_id){
		__hz = 50;
		__hz_adjustment = 1.2;
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
	}

	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_palette(&BIOS_Red_P);

	crayon_memory_free_mono_font_sheet(&BIOS_font);

	return;
}

int main(){
	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		return 1;
	}

	crayon_sprite_array_t fade_draw;
	init_fade_struct(&fade_draw);

	// If not on VGA output, select a refresh rate
	hz_select(&fade_draw);

	crayon_graphics_set_bg_colour(0.3, 0.3, 0.3); // Its useful-ish for debugging

	while(1){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			// Poll for input here
		MAPLE_FOREACH_END()


		pvr_scene_begin();


		pvr_list_begin(PVR_LIST_TR_POLY);
			;
		pvr_list_finish();


		pvr_list_begin(PVR_LIST_OP_POLY);
			;
		pvr_list_finish();


		pvr_list_begin(PVR_LIST_PT_POLY);
			;
		pvr_list_finish();


		pvr_scene_finish();
	}

	crayon_memory_free_sprite_array(&fade_draw);

	return 0;
}
