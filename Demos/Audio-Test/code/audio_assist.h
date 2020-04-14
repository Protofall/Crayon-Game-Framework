#ifndef AUDIO_ASSIST_H
#define AUDIO_ASSIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

//Crayon implements this
// #if defined(_arch_dreamcast)
	#include <crayon/vector_structs.h>
// #else
	// typedef struct vec2_f{
		// float x, y;
	// } vec2_f_t;
// #endif

#if defined(__APPLE__) || defined(__linux__) || defined(_arch_dreamcast)
	#include <sched.h>
	#include <pthread.h>
	#include <time.h>
#elif defined(_WIN32)
	#include <windows.h>
	#error "Windows not supported due to pthreads"
#endif

#include <AL/al.h>
#include <AL/alc.h>

#define AUDIO_NOT_STREAMING 0
#define AUDIO_STREAMING 1

#define AUDIO_DATA_TYPE_WAV 0
#define AUDIO_DATA_TYPE_CDDA 1
#define AUDIO_DATA_TYPE_OGG 2

typedef struct audio_info{
	uint8_t streaming;	//In RAM or streaming. We can only have one streaming at a time
	uint8_t data_type;	//WAV, CDDA, OGG

	ALsizei size, freq;
	ALenum format;

	//Move the buffer info to here
	uint8_t buff_cnt;	//Number of buffers
	ALuint * buff_id;
	uint8_t srcs_attached;	//Keeps a record of how many sources are using this info and buffer
} audio_info_t;

typedef struct audio_source{
	audio_info_t * info;
	vec2_f_t position;	//Since we are doing 2D we only need two
						//velocity is always zero

	ALboolean looping;
	float current_volume;	//Current gain
	float target_volume;	//Target gain
	float speed;	//Pitch

	ALuint src_id;	//The source it uses
	ALint state;
} audio_source_t;

//We only need one of each for all audio
ALCcontext * _al_context;
ALCdevice * _al_device;

#define AUDIO_COMMAND_NONE 0
#define AUDIO_COMMAND_PLAY 1
#define AUDIO_COMMAND_PAUSE 2
#define AUDIO_COMMAND_UNPAUSE 3
#define AUDIO_COMMAND_STOP 4
#define AUDIO_COMMAND_END 5	//This will terminate the streamer thread

//Since it only makes sense to stream one audio source (The music). I've hard coded it to only use one
volatile uint8_t _audio_streamer_command;	//Should only be accessed with a mutex
volatile uint8_t _audio_streamer_thd_active;	//Says if the streamer thread is currently active or not
volatile uint8_t _audio_streamer_stopping;	//Only used for non-looping
pthread_t        _audio_streamer_thd_id;
pthread_mutex_t  _audio_streamer_lock;	//We lock the streamer command and thd_active vars

FILE*            _audio_streamer_fp;	//If a pointer to the file/data on disc
audio_source_t*  _audio_streamer_source;	//Is null if none are streaming, otherwise points to the streaming struct
										//And this contains a pointer to the info struct
audio_info_t*    _audio_streamer_info;

#define AUDIO_STREAMING_NUM_BUFFERS 16	//4 is the bare minimum, but its safer to have more so you don't run out
#define AUDIO_STREAMING_DATA_CHUNK_SIZE (1024 * 64)
#define WAV_HDR_SIZE 44


//----------------------MISC---------------------------//


ALboolean audio_test_error(ALCenum * error, char * msg);


//----------------------SETUP---------------------------//


uint8_t audio_init();	//Returns 1 if an error occured, 0 otherwise
void audio_shutdown();

//These load functions will instanly fail if you want to stream and there's another streamer present
ALboolean audio_load_WAV_file_info(const char * path, audio_info_t * info, uint8_t mode);	//Mode is stream/local
ALboolean audio_load_CDDA_track_info(uint8_t drive, uint8_t track, audio_info_t * info, uint8_t mode);	//Data is never stored if in stream mode
ALboolean audio_load_OGG_file_info(const char * path, audio_info_t * info, uint8_t mode);

ALboolean audio_free_info(audio_info_t * info);	//This will free path and data if they are set
ALboolean audio_free_source(audio_source_t * source);

ALboolean audio_create_source(audio_source_t * source, audio_info_t * info, vec2_f_t position, ALboolean looping,
	float volume, float speed);

ALboolean audio_update_source_state(audio_source_t * source);

ALboolean audio_play_source(audio_source_t * source);	//When called on a source which is already playing, the source will restart at the beginning
ALboolean audio_pause_source(audio_source_t * source);
ALboolean audio_unpause_source(audio_source_t * source);	//If pause it will resume from the point it stoped. If not paused this will return false
ALboolean audio_stop_source(audio_source_t * source);	//Next time we run "play" it will resume from the beginning of the song/sfx
															//Note: Stopping a streaming source will terminate the `audio_stream_player()` call its in

ALboolean audio_streamer_buffer_fill(ALuint id);
ALboolean audio_prep_stream_buffers();
void * audio_stream_player(void * args);	//This function is called by a pthread

void audio_WAV_buffer_fill(ALvoid * data);
void audio_CDDA_buffer_fill(ALvoid * data);
void audio_OGG_buffer_fill(ALvoid * data);


//----------------------ADJUSTMENT---------------------//


uint8_t audio_adjust_master_volume(float vol);	//adjust's listener's gain
uint8_t audio_adjust_source_volume(audio_source_t * source, float vol);	//adjust's source's gain
uint8_t audio_adjust_source_speed(audio_source_t * source, float speed);
uint8_t audio_set_source_looping(audio_source_t * source, ALboolean looping);


#endif