//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For region and htz stuff
#include <dc/flashrom.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include "audio_assist.h"

//Change this to only give room for PT list (Since other ones aren't used)
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

float width, height;
crayon_sprite_array_t fade_draw;

float g_deadspace;

uint8_t g_htz, g_htz_adjustment;
uint8_t vga_enabled;

void init_fade_struct(){
	crayon_memory_init_sprite_array(&fade_draw, NULL, 0, NULL, 1, 0, 0, PVR_FILTER_NONE, 0);
	fade_draw.coord[0].x = 0;
	fade_draw.coord[0].y = 0;
	fade_draw.scale[0].x = width;
	fade_draw.scale[0].y = height;
	fade_draw.fade[0] = 254;
	fade_draw.colour[0] = 0xFFFF0000;	//Full Black (Currently red for debugging)
	fade_draw.rotation[0] = 0;
	fade_draw.visible[0] = 1;
	fade_draw.layer[0] = 255;
}

void htz_select(){
	//If we have a VGA cable, then skip this screen
	if(vga_enabled){return;}

	crayon_memory_mount_romdisk("/cd/stuff.img", "/Setup");

	crayon_font_mono_t BIOS_font;
	crayon_palette_t BIOS_P;		//Entry 0
	crayon_palette_t BIOS_Red_P;	//Entry 1

	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 0, "/Setup/BIOS.dtex");

	fs_romdisk_unmount("/Setup");

	//Make the red font
	crayon_memory_clone_palette(&BIOS_P, &BIOS_Red_P, 1);
	crayon_memory_swap_colour(&BIOS_Red_P, 0xFFAAAAAA, 0xFFFF0000, 0);

	//Set the palettes in PVR memory
	crayon_graphics_setup_palette(&BIOS_P);	//0
	crayon_graphics_setup_palette(&BIOS_Red_P);	//1

	uint16_t htz_head_x = (width - (2 * crayon_graphics_string_get_length_mono(&BIOS_font, "Select Refresh Rate"))) / 2;
	uint16_t htz_head_y = 133;
	uint16_t htz_option_y = htz_head_y + (BIOS_font.char_height * 2) + 80;

	//Set the palettes for each option
		//Red is highlighted, white is normal
	int8_t * palette_50htz,* palette_60htz;

	if(g_htz == 50){	//For highlights
		palette_50htz = &BIOS_Red_P.palette_id;
		palette_60htz = &BIOS_P.palette_id;
	}
	else{
		palette_50htz = &BIOS_P.palette_id;
		palette_60htz = &BIOS_Red_P.palette_id;
	}
	
	int16_t counter = g_htz  * 11;	//Set this to 11 secs (So current fps * 11)
									//We choose 11 seconds because the fade-in takes 1 second

	uint8_t fade_frame_count = 0;
	float fade_cap = g_htz * 1.0;	//1 second, 60 or 50 frames

	//While counting || fade is not full alpha
	while(counter > 0 || crayon_assist_extract_bits(fade_draw.colour[0], 8, 24) != 0xFF){

		//Update the fade-in/out effects
		counter--;
		if(counter < 0){	//Fading out
			if(crayon_assist_extract_bits(fade_draw.colour[0], 8, 24) == 0){
				//Have another sound, not a blip, but some acceptance thing
			}
			// fade = 0xFF * (fade_frame_count / fade_cap);
			fade_draw.colour[0] = (uint32_t)(0xFF * (fade_frame_count / fade_cap)) << 24;
			fade_frame_count++;
		}
		else if(crayon_assist_extract_bits(fade_draw.colour[0], 8, 24) > 0){	//fading in
			// fade = 0xFF - (0xFF * (fade_frame_count / fade_cap));
			fade_draw.colour[0] = (uint32_t)(0xFF - (0xFF * (fade_frame_count / fade_cap))) << 24;
			fade_frame_count++;
		}
		else{	//10 seconds of choice
			fade_frame_count = 0;
		}

		pvr_wait_ready();	//Need vblank for inputs

		//Don't really want players doing stuff when it fades out
		if(counter >= 0){
			MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			if(st->buttons & CONT_DPAD_LEFT){
				palette_50htz = &BIOS_Red_P.palette_id;	//0 is 50 Htz
				palette_60htz = &BIOS_P.palette_id;
				//Make "blip" sound
			}
			else if(st->buttons & CONT_DPAD_RIGHT){
				palette_50htz = &BIOS_P.palette_id;	//1 is 60 Htz
				palette_60htz = &BIOS_Red_P.palette_id;
				//Make "blip" sound
			}

			//Press A or Start to skip the countdown
			if((st->buttons & CONT_START) || (st->buttons & CONT_A)){
				counter = 0;
			}

			MAPLE_FOREACH_END()
		}

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

			//The fading in/out effect
			if(crayon_assist_extract_bits(fade_draw.colour[0], 8, 24)){
				crayon_graphics_draw_sprites(&fade_draw, NULL, PVR_LIST_TR_POLY, CRAY_DRAW_SIMPLE);
			}

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			crayon_graphics_draw_text_mono("Select Refresh Rate", &BIOS_font, PVR_LIST_OP_POLY, htz_head_x, htz_head_y, 50, 2, 2,
				BIOS_P.palette_id);
			crayon_graphics_draw_text_mono("50Hz", &BIOS_font, PVR_LIST_OP_POLY, 40, htz_option_y, 50, 2, 2, *palette_50htz);
			crayon_graphics_draw_text_mono("60Hz", &BIOS_font, PVR_LIST_OP_POLY, 480, htz_option_y, 50, 2, 2, *palette_60htz);

		pvr_list_finish();

		pvr_scene_finish();

	}

	if(g_htz == 50 && *palette_60htz == BIOS_Red_P.palette_id){
		g_htz = 60;
		g_htz_adjustment = 1;
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}
	else if(g_htz == 60 && *palette_50htz == BIOS_Red_P.palette_id){
		g_htz = 50;
		g_htz_adjustment = 1.2;
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
	}

	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_palette(&BIOS_Red_P);

	crayon_memory_free_mono_font_sheet(&BIOS_font);

	return;
}

void crayon_graphics_init_display(){
	pvr_init(&pvr_params);

	width = crayon_graphics_get_window_width();
	height = crayon_graphics_get_window_height();

	vga_enabled = (vid_check_cable() == CT_VGA);
	if(vga_enabled){
		vid_set_mode(DM_640x480_VGA, PM_RGB565);	//60Hz
		g_htz = 60;
		g_htz_adjustment = 1;
	}
	else{
		if(flashrom_get_region() == FLASHROM_REGION_EUROPE){
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
			g_htz = 50;
			g_htz_adjustment = 1.2;
		}
		else{
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
			g_htz = 60;
			g_htz_adjustment = 1;
		}
	}

	return;
}

int main(){
	#if CRAYON_BOOT_MODE != 0
		#error SD/PC support not implemented
	#endif

	//Initialise audio (Has to be done early for some reason)
	ALCenum error;
	if(audio_init() != 0){return -1;}
	audio_info_t infoFX, infoMusic;
	audio_source_t sourceFX, sourceMusic;


	g_deadspace = 0.4;

	crayon_graphics_init_display();
	init_fade_struct();
	htz_select();


	crayon_font_mono_t BIOS_font;
	crayon_palette_t BIOS_P;		//Entry 0
	crayon_memory_mount_romdisk("/cd/stuff.img", "/Setup");
	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 0, "/Setup/BIOS.dtex");
	fs_romdisk_unmount("/Setup");
	crayon_graphics_setup_palette(&BIOS_P);


	//load in assets here
	//Setup chopper sound effect
	if(audio_load_WAV_file_info("/cd/wololo.wav", &infoFX, AUDIO_NOT_STREAMING) == AL_FALSE){return -1;}
	// if(audio_load_WAV_file_info("/cd/test.wav", &infoFX, AUDIO_NOT_STREAMING) == AL_FALSE){return -1;}
	if(audio_test_error(&error, "loading wav file") == AL_TRUE){return -1;}
	
	//Create the source for playback
	if(audio_create_source(&sourceFX, &infoFX, (vec2_f_t){0,0}, AL_FALSE, 0.5, 1, AUDIO_FREE_DATA) == AL_FALSE){return -1;}
	// if(audio_create_source(&sourceFX, &infoFX, (vec2_f_t){0,0}, AL_TRUE, 0.25, 1, AUDIO_FREE_DATA) == AL_FALSE){return -1;}

	//Setup music
	if(audio_load_WAV_file_info("/cd/The-Haunted-House.wav", &infoMusic, AUDIO_STREAMING) == AL_FALSE){return -1;}
	if(audio_test_error(&error, "loading wav file") == AL_TRUE){return -1;}

	//Note last param is ignored for streaming
	if(audio_create_source(&sourceMusic, &infoMusic, (vec2_f_t){0,0}, AL_FALSE, 0.5, 1, AUDIO_FREE_DATA) == AL_FALSE){return -1;}

	// audio_play_source(&sourceFX);
	// audio_play_source(&sourceMusic);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	#if CRAYON_BOOT_MODE == 1
		unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif


	char BUFFER[512];
	uint32_t previous_input[4] = {0};
	while(1){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

			if((st->buttons & CONT_A) && !(previous_input[__dev->port] & CONT_A)){
				audio_play_source(&sourceMusic);
			}

			if((st->buttons & CONT_B) && !(previous_input[__dev->port] & CONT_B)){
				audio_stop_source(&sourceMusic);
			}

			if((st->buttons & CONT_X) && !(previous_input[__dev->port] & CONT_X)){
				audio_pause_source(&sourceMusic);
			}

			if((st->buttons & CONT_Y) && !(previous_input[__dev->port] & CONT_Y)){
				audio_unpause_source(&sourceMusic);
			}

		previous_input[__dev->port] = st->buttons;
		MAPLE_FOREACH_END()

		pvr_wait_ready();

		pvr_scene_begin();


		pvr_list_begin(PVR_LIST_TR_POLY);
			// crayon_graphics_draw_sprites(&fade_draw, NULL, PVR_LIST_TR_POLY, CRAY_DRAW_SIMPLE);
		pvr_list_finish();


		pvr_list_begin(PVR_LIST_OP_POLY);
			// crayon_graphics_draw_text_mono("TEST", &BIOS_font, PVR_LIST_OP_POLY, 32, 32, 50, 2, 2, BIOS_P.palette_id);

			// sprintf(BUFFER, "%d, %d, %p, %.2f\n%.2f, %d, %.2f, %.2f\n%d", infoFX.streaming, infoFX.data_type, sourceFX.info,
				// sourceFX.position.x, sourceFX.position.y,
				// sourceFX.looping, sourceFX.volume, sourceFX.speed, sourceFX.num_buffers);
			audio_update_source_state(&sourceMusic);
			sprintf(BUFFER, "CONTROLS: \n-A starts the Streaming piano music\n-B stops it (bugged?)\n-X pauses it\n-Y unpauses it\nSTATE %d\n%d", sourceMusic.state, _audio_streamer_stopping);
			crayon_graphics_draw_text_mono(BUFFER, &BIOS_font, PVR_LIST_OP_POLY, 32, 100, 50, 2, 2, BIOS_P.palette_id);
			// crayon_graphics_draw_text_mono(BUFFER, &B2, PVR_LIST_OP_POLY, 32, 100, 50, 2, 2, B2_P.palette_id);

			//Chosing stop when stopped sets the state, but it never gets cleared. Maybe the thread is locked

		pvr_list_finish();


		pvr_list_begin(PVR_LIST_PT_POLY);
			;
		pvr_list_finish();


		pvr_scene_finish();
	}

	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_mono_font_sheet(&BIOS_font);

	crayon_memory_free_sprite_array(&fade_draw);

	audio_stop_source(&sourceFX);
	audio_stop_source(&sourceMusic);

	audio_free_source(&sourceMusic);
	audio_unload_info(&infoMusic);

	audio_free_source(&sourceFX);
	audio_unload_info(&infoFX);

	audio_shutdown();

	return 0;
}
