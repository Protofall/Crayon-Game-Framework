#include "bios.h"

void BIOS_menu(MinesweeperOptions_t * MS_options, float * htz_adjustment, pvr_init_params_t * pvr_params, uint8_t region, uint8_t first_time){
	//MS_options->htz is always set no matter what
		//Redream uses a vga cable
	uint8_t vga_enabled = (vid_check_cable() == CT_VGA);
	if(vga_enabled){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
		MS_options->htz = 1;	//60Hz
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		if(first_time){
			if(region != FLASHROM_REGION_EUROPE){
				vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
			}
			else{
				vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
				MS_options->htz = 0;	//50Hz
			}
		}
		else{
			if(MS_options->htz){
				vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
			}
			else{
				vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
			}
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

	uint16_t os_head_x = (640-(1.5*crayon_graphics_string_get_length_mono(&BIOS_font, "Operating System", 0)))/2;
	uint16_t os_head_y = 70;
	uint16_t os_option_y = os_head_y + (BIOS_font.char_height * 1.5) + 40;

	uint16_t htz_head_x = (640-(1.5*crayon_graphics_string_get_length_mono(&BIOS_font, "Refresh Rate", 0)))/2;
	uint16_t htz_head_y = 30 + os_option_y + (BIOS_font.char_height * 1.5) + 70;
	uint16_t htz_option_y = htz_head_y + (BIOS_font.char_height * 1.5) + 40;

	uint16_t cursor_y = os_option_y;

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

	//464 height, 4 evenly spaced rows. Each thing is 24 tall.
	//24 * 4 = 96. 464 - 96 = 368
	//Giving gaps of 73.6


	//Currently disabled cause redream
	if(vga_enabled){
		//os_head_y = 320 - (1.5 * BIOS_font.char_height/2);	//Fix
		//os_option_y = os_head_y + (BIOS_font.char_height * 1.5) + 30;
	}

	uint8_t current_option = 0;	//0 is OS, 1 is refresh
	uint8_t htz = (MS_options->htz ? 60 : 50);
	int16_t counter = (htz * 10) - 1;	//Set this to 10 secs (So current fps * 10)
	char countdown_buffer[16];
	while(counter >= 0){
		counter--;

		pvr_wait_ready();	//Need vblank for inputs

		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
		if(st->buttons & CONT_DPAD_LEFT){
			if(current_option == 0){
				// MS_options->operating_system = 0;	//0 is 2000
				palette_2000 = &BIOS_invert_P.palette_id;
				palette_XP = &BIOS_P.palette_id;
			}
			else{
				// MS_options->htz = 0;	//0 is 50 Htz
				palette_50htz = &BIOS_invert_P.palette_id;
				palette_60htz = &BIOS_P.palette_id;
			}
		}
		else if(st->buttons & CONT_DPAD_RIGHT){
			if(current_option == 0){
				// MS_options->operating_system = 1;	//1 is XP
				palette_2000 = &BIOS_P.palette_id;
				palette_XP = &BIOS_invert_P.palette_id;
			}
			else{
				// MS_options->htz = 1;	//1 is 60 Htz
				palette_50htz = &BIOS_P.palette_id;
				palette_60htz = &BIOS_invert_P.palette_id;
			}
		}

		//Chose the option, but don't let this happen if we have a VGA cable
		if(!vga_enabled || 1){	//COME BACK HERE LATER
			if((st->buttons & CONT_DPAD_UP) && current_option != 0){
				current_option--;
				cursor_y = os_option_y;
			}
			else if((st->buttons & CONT_DPAD_DOWN) && current_option != 1){
				current_option++;
				cursor_y = htz_option_y;
			}
		}

		if(st->buttons & CONT_START){
			counter = 0;
		}

		MAPLE_FOREACH_END()

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			//Scaled *1.5, each char becomes 12 wide and 24 tall
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, os_head_x, os_head_y, 50, 1.5, 1.5, BIOS_P.palette_id, "Operating System");
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 70, os_option_y, 50, 1.5, 1.5, *palette_2000, "Windows 2000");
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 500, os_option_y, 50, 1.5, 1.5, *palette_XP, "Windows XP");

			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, htz_head_x, htz_head_y, 50, 1.5, 1.5, BIOS_P.palette_id, "Refresh Rate");
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 70, htz_option_y, 50, 1.5, 1.5, *palette_50htz, "50 Htz");
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 500, htz_option_y, 50, 1.5, 1.5, *palette_60htz, "60 Htz");

			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 70 - 36, cursor_y, 50, 1.5, 1.5, BIOS_P.palette_id, ">");

			sprintf(countdown_buffer, "Booting in %ds", counter/htz);
			OLD_crayon_graphics_draw_text_mono(&BIOS_font, PVR_LIST_OP_POLY, 640 - crayon_graphics_string_get_length_mono(&BIOS_font, countdown_buffer, 0),
				480 - 24, 50, 1, 1, BIOS_P.palette_id, countdown_buffer);
		pvr_list_finish();

		pvr_scene_finish();
	}

	if(*palette_2000){
		//Set OS to 2000
		MS_options->operating_system = 0;
	}
	else{
		//set OS to XP
		MS_options->operating_system = 1;
	}

	if(*palette_50htz){
		if(!vga_enabled){	//REMOVE THIS IF CHECK LATER ONCE VGA MODE IS PROPERLY HANDLED
			//Set refresh to 50
			*htz_adjustment = 1.2;
			MS_options->htz = 0;
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
		}
	}
	else{
		//set refresh to 60
		*htz_adjustment = 1;
		MS_options->htz = 1;
		if(!vga_enabled){	//VGA is 60Htz, but not interlaced
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
		}
		else{	//If we have a VGA cable, use VGA
			vid_set_mode(DM_640x480_VGA, PM_RGB565);	//MIGHT BE REDUNDANT
		}
	}

	crayon_memory_free_mono_font_sheet(&BIOS_font);
	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_palette(&BIOS_invert_P);
}
