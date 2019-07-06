#include "bios.h"

void BIOS_menu(MinesweeperOptions_t * MS_options, float * htz_adjustment, pvr_init_params_t * pvr_params, uint8_t region, uint8_t first_time){

	//For debug
	uint8_t original_htz = MS_options->htz;
	uint8_t original_os = MS_options->operating_system;

	//MS_options->htz is always set no matter what
		//Redream uses a vga cable, lxdream doesn't
	uint8_t vga_enabled = (vid_check_cable() == CT_VGA);
	if(vga_enabled){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
		MS_options->htz = 1;	//60Hz
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		if(first_time){
			if(region != FLASHROM_REGION_EUROPE){
				vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
				MS_options->htz = 1;
			}
			else{
				vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
				MS_options->htz = 0;
			}
		}
		else if(MS_options->htz){
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
		}
		else{
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
		}
	}


	//What needs to happen.

	//Display a screen with this kind of layout

	/*

				Operating System
	>	2000						XP

				Refresh Rate
		50Hz						60Hz


							Booting in 10s
	*/

	//The arrow shows which option you are modifying. The currently chosen one is highlighted (Inverted palette)
	//Press LEFT/RIGHT on d-pad to choose between the two modes per option
	//Press UP/DOWN to swicth between Refresh rate and OS

	//There's a counter on the bottom. Goes away if DPAD is pressed
	//Pressing Start just skips the counter to zero

	//If VGA is detected, don't show the refresh rate option




	pvr_init(pvr_params);

	crayon_palette_t BIOS_P, BIOS_invert_P;
	crayon_font_mono_t BIOS_font;
	BIOS_font.texture = NULL;

	#if CRAYON_BOOT_MODE == 1
		crayon_memory_mount_romdisk("/sd/BIOS.img", "/BIOS");
	#else
		crayon_memory_mount_romdisk("/cd/BIOS.img", "/BIOS");
	#endif

	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 0, "/BIOS/BIOS_font.dtex");

	fs_romdisk_unmount("/BIOS");

	crayon_memory_clone_palette(&BIOS_P, &BIOS_invert_P, 1);
	
	//Invert the palette...kinda
	crayon_memory_swap_colour(&BIOS_invert_P, 0xFF000000, 0xFFFFFFFF, 0);
	crayon_memory_swap_colour(&BIOS_invert_P, 0xFFAAAAAA, 0xFF000000, 0);
	crayon_memory_swap_colour(&BIOS_invert_P, 0xFFFFFFFF, 0xFFAAAAAA, 0);

	//Palettes don't change so set them up
	crayon_graphics_setup_palette(&BIOS_P);			//0
	crayon_graphics_setup_palette(&BIOS_invert_P);	//1

	uint16_t os_head_x = (640 - (2 * crayon_graphics_string_get_length_mono(&BIOS_font, "Operating System", 0))) / 2;
	uint16_t os_head_y = 67;
	uint16_t os_option_y = os_head_y + (BIOS_font.char_height * 2) + 67;

	uint16_t htz_head_x = (640 - (2 * crayon_graphics_string_get_length_mono(&BIOS_font, "Refresh Rate", 0))) / 2;
	uint16_t htz_head_y = os_option_y + (BIOS_font.char_height * 2) + 67;
	uint16_t htz_option_y = htz_head_y + (BIOS_font.char_height * 1.5) + 67;

	//x is text height, y is gap between texts
	//(4 * x) + (5 * y) + (1 * 16) = 480
	//(4 * 32) + (5 * y) = 464
	//5 * y = 464 - 128 = 336
	//y = 67.2 or 67


	//Set the palettes for each option
	int8_t * palette_2000,* palette_XP,* palette_50htz,* palette_60htz;
	if(MS_options->operating_system == 0){
		palette_2000 = &BIOS_invert_P.palette_id;
		palette_XP = &BIOS_P.palette_id;
	}
	else{
		palette_2000 = &BIOS_P.palette_id;
		palette_XP = &BIOS_invert_P.palette_id;
	}

	if(MS_options->htz == 0){
		palette_50htz = &BIOS_invert_P.palette_id;
		palette_60htz = &BIOS_P.palette_id;
	}
	else{
		palette_50htz = &BIOS_P.palette_id;
		palette_60htz = &BIOS_invert_P.palette_id;
	}

	if(vga_enabled){
		os_head_y = 133;
		os_option_y = os_head_y + (BIOS_font.char_height * 2) + 133;

		//(2 * x) + (3 * y) = 464
		//3 * y = 464 - 64
		//y = 400/3 = 133.3 or 133
	}
	uint16_t cursor_y = os_option_y;

	uint8_t current_option = 0;	//0 is OS, 1 is refresh
	uint8_t htz = (MS_options->htz ? 60 : 50);
	int16_t counter = (htz * 10) - 1;	//Set this to 10 secs (So current fps * 10)
	char countdown_buffer[16];
	while(counter > 0){
		counter--;

		pvr_wait_ready();	//Need vblank for inputs

		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
		if(st->buttons & CONT_DPAD_LEFT){
			if(current_option == 0){
				palette_2000 = &BIOS_invert_P.palette_id;	//0 is 2000
				palette_XP = &BIOS_P.palette_id;
			}
			else{
				palette_50htz = &BIOS_invert_P.palette_id;	//0 is 50 Htz
				palette_60htz = &BIOS_P.palette_id;
			}
		}
		else if(st->buttons & CONT_DPAD_RIGHT){
			if(current_option == 0){
				palette_2000 = &BIOS_P.palette_id;	//1 is XP
				palette_XP = &BIOS_invert_P.palette_id;
			}
			else{
				palette_50htz = &BIOS_P.palette_id;	//1 is 60 Htz
				palette_60htz = &BIOS_invert_P.palette_id;
			}
		}

		//Chose the option, but don't let this happen if we have a VGA cable
		if(!vga_enabled){
			if((st->buttons & CONT_DPAD_UP) && current_option != 0){
				current_option--;
				cursor_y = os_option_y;
			}
			else if((st->buttons & CONT_DPAD_DOWN) && current_option != 1){
				current_option++;
				cursor_y = htz_option_y;
			}
		}

		//Press A or Start to skip the countdown
		if((st->buttons & CONT_START) || (st->buttons & CONT_A)){
			counter = 0;
		}

		MAPLE_FOREACH_END()

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			//Scaled *2, each char becomes 16 wide and 32 tall
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, os_head_x, os_head_y, 50, 2, 2, BIOS_P.palette_id, "Operating System");
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 50, os_option_y, 50, 2, 2, *palette_2000, "Windows 2000");
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 450, os_option_y, 50, 2, 2, *palette_XP, "Windows XP");

			if(!vga_enabled){
				OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, htz_head_x, htz_head_y, 50, 2, 2, BIOS_P.palette_id, "Refresh Rate");
				OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 80, htz_option_y, 50, 2, 2, *palette_50htz, "50 Htz");
				OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 480, htz_option_y, 50, 2, 2, *palette_60htz, "60 Htz");
			}
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 50 - 36, cursor_y, 50, 2, 2, BIOS_P.palette_id, ">");

			sprintf(countdown_buffer, "Booting in %ds", counter/htz);
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 640 - crayon_graphics_string_get_length_mono(&BIOS_font, countdown_buffer, 0),
				480 - 24, 50, 1, 1, BIOS_P.palette_id, countdown_buffer);
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Free up PVR memory
	crayon_memory_free_mono_font_sheet(&BIOS_font);
	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_palette(&BIOS_invert_P);

	//If you're changing refresh rate
	if(original_htz == 1 && *palette_50htz == 0){	//Changing to 50Htz
		MS_options->htz = 0;
		//Shutdown pvr
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);	//Set vid mode
		//Re-init pvr
	}
	else if(original_htz == 0 && *palette_60htz == 1){	//Changing to 60Htz
		MS_options->htz = 1;
		//Shutdown pvr
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//Set vid mode
		//Re-init pvr
	}

	//Set the adjustment
	if(MS_options->htz == 0){
		*htz_adjustment = 1.2;
	}
	else{
		*htz_adjustment = 1;
	}

	//Set the OS
	if(*palette_2000){
		MS_options->operating_system = 0;
	}
	else{
		MS_options->operating_system = 1;
	}
}
