// Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/debug.h>
#include <crayon/crayon.h>

// For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include "audio_assist.h"

float width, height;

int main(){
	// Note: First parameter is ignore for now since Crayon is only on Dreamcast ATM
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		error_freeze("Crayon failed to initialise");
	}

	// Initialise audio (Has to be done early for some reason)
	ALCenum error;
	if(audio_init() != 0){
		error_freeze("Failed to initialise audio");
	}

	audio_info_t infoFX, infoMusic;
	audio_source_t sourceFX, sourceMusic;

	crayon_font_mono_t BIOS_font;
	crayon_palette_t BIOS_P;		// Entry 0

	crayon_memory_mount_romdisk("stuff.img", "/Setup", CRAYON_ADD_BASE_PATH);
	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, "/Setup/BIOS.dtex",
		CRAYON_USE_EXACT_PATH, 0);

	fs_romdisk_unmount("/Setup");

	crayon_graphics_setup_palette(&BIOS_P);

	#if CRAYON_BOOT_MODE != CRAYON_BOOT_OPTICAL
		#error "This demo is hard coded to use the cd drive in places. Please fix"
	#endif

	// Load in assets here
	// Setup chopper sound effect
	if(audio_load_WAV_file_info("/cd/wololo.wav", &infoFX, AUDIO_NOT_STREAMING) == AL_FALSE){return -1;}
	// if(audio_load_WAV_file_info("/cd/test.wav", &infoFX, AUDIO_NOT_STREAMING) == AL_FALSE){return -1;}
	if(audio_test_error(&error, "loading wav file") == AL_TRUE){return -1;}
	
	// Create the source for playback
	if(audio_create_source(&sourceFX, &infoFX, (vec2_f_t){0,0}, AL_FALSE, 0.5, 1) == AL_FALSE){return -1;}
	// if(audio_create_source(&sourceFX, &infoFX, (vec2_f_t){0,0}, AL_TRUE, 0.25, 1) == AL_FALSE){return -1;}

	// Setup music
	if(audio_load_WAV_file_info("/cd/The-Haunted-House.wav", &infoMusic, AUDIO_STREAMING) == AL_FALSE){return -1;}
	if(audio_test_error(&error, "loading wav file") == AL_TRUE){return -1;}

	// Note last param is ignored for streaming
	if(audio_create_source(&sourceMusic, &infoMusic, (vec2_f_t){0,0}, AL_FALSE, 0.5, 1) == AL_FALSE){return -1;}

	// audio_play_source(&sourceFX);
	// audio_play_source(&sourceMusic);

	crayon_graphics_set_bg_colour(0.3, 0.3, 0.3); // Its useful-ish for debugging

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

		pvr_list_begin(PVR_LIST_OP_POLY);
			// crayon_graphics_draw_text_mono("TEST", &BIOS_font, PVR_LIST_OP_POLY, 32, 32, 50, 2, 2, BIOS_P.palette_id);

			// sprintf(BUFFER, "%d, %d, %p, %.2f\n%.2f, %d, %.2f, %.2f\n%d", infoFX.streaming, infoFX.data_type, sourceFX.info,
				// sourceFX.position.x, sourceFX.position.y,
				// sourceFX.looping, sourceFX.volume, sourceFX.speed, sourceFX.num_buffers);
			audio_update_source_state(&sourceMusic);
			sprintf(BUFFER, "CONTROLS: \n-A starts the Streaming piano music\n-B stops it (bugged?)\n-X pauses it\n-Y unpauses it\nSTATE %d\n%d", sourceMusic.state, __audio_streamer_stopping);
			crayon_graphics_draw_text_mono(BUFFER, &BIOS_font, PVR_LIST_OP_POLY, 32, 100, 50, 2, 2, BIOS_P.palette_id);
			// crayon_graphics_draw_text_mono(BUFFER, &B2, PVR_LIST_OP_POLY, 32, 100, 50, 2, 2, B2_P.palette_id);

			// Chosing stop when stopped sets the state, but it never gets cleared. Maybe the thread is locked

		pvr_list_finish();

		pvr_scene_finish();
	}

	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_mono_font_sheet(&BIOS_font);

	audio_stop_source(&sourceFX);
	audio_stop_source(&sourceMusic);

	audio_free_source(&sourceMusic);
	audio_free_info(&infoMusic);

	audio_free_source(&sourceFX);
	audio_free_info(&infoFX);

	audio_shutdown();
	crayon_shutdown();

	return 0;
}
