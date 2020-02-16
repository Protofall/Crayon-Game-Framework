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

#include <crayon/vector_structs.h>

#if defined(_arch_unix) || defined(_arch_dreamcast)
	#include <sched.h>
	#include <pthread.h>
#elif defined(_arch_win)
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
	uint8_t data_type;

	ALvoid * data;	//Is NULL when in streaming mode or when the user decides to free it up
	ALsizei size, freq;
	ALenum format;
} audio_info_t;

typedef struct audio_source{
	audio_info_t * info;
	vec2_f_t position;	//Since we are doing 2D we only need two
						//velocity is always zero

	ALboolean looping;
	float volume;	//Gain
	float speed;	//Pitch

	uint8_t num_buffers;
	ALuint * buffer_id;	//Each source can use 1 or more buffers (Hard-code streaming to use 4 buffers, else only 1?)
	ALuint source_id;	//The source it uses
	ALint source_state;
} audio_source_t;

ALCcontext * _al_context;	//We only need one for all audio
ALCdevice * _al_device;

#define AUDIO_COMMAND_NONE 0
#define AUDIO_COMMAND_PLAY 1
#define AUDIO_COMMAND_PAUSE 2
#define AUDIO_COMMAND_UNPAUSE 3
#define AUDIO_COMMAND_STOP 4
#define AUDIO_COMMAND_END 5	//This will terminate the streamer thread

//Since it only makes sense to stream one audio source (The music). I've hard coded it to only use one
uint8_t         _audio_streamer_command;	//Should only be accessed with a mutex
uint8_t         _audio_streamer_thd_active;	//Says if the streamer thread is currently active or not
pthread_t       _audio_streamer_thd_id;	//Currently unused
pthread_mutex_t _audio_streamer_lock;	//We lock the streamer command and thd_active vars

FILE *          _audio_streamer_fp;	//If a pointer to the file/data on disc
audio_source_t* _audio_streamer_source;	//Is null if none are streaming, otherwise points to the streaming struct
										//And this contains a pointer to the info struct
audio_info_t*   _audio_streamer_info;

#define AUDIO_STREAMING_NUM_BUFFERS 4
#define AUDIO_STREAMING_DATA_CHUNK_SIZE (1024 * 64)
#define WAV_HDR_SIZE 44

uint8_t audio_init();	//Returns 1 if an error occured, 0 otherwise
void audio_shutdown();

//NOTE: I want an option to load a CDDA song into RAM instead of streaming if thats what the user wants

//These load functions will instanly fail if you want to stream and there's another streamer present
ALboolean audio_load_WAV_file_info(const char * path, audio_info_t * info, uint8_t mode);	//Mode is stream/local
ALboolean audio_load_CDDA_track_info(uint8_t track, audio_info_t * info, uint8_t mode);	//Data is never stored if in stream mode

ALboolean audio_unload_info(audio_info_t * info);	//This will free path and data if they are set
void audio_free_info_data(audio_info_t * info);
ALboolean audio_free_source(audio_source_t * source);

//Note: Despite what option you give the loader, it will never store the data in stream mode
#define AUDIO_KEEP_DATA 0
#define AUDIO_FREE_DATA 1

// `delete_data` means delete the original data buffer once used. You might not want to do this if you have multiple sources
// using the same sound file
ALboolean audio_create_source(audio_source_t * source, audio_info_t * info, vec2_f_t position, ALboolean looping,
	float volume, float speed, uint8_t delete_data);

ALboolean audio_update_source_state(audio_source_t * source);

ALboolean audio_play_source(audio_source_t * source);	//When called on a source which is already playing, the source will restart at the beginning
ALboolean audio_pause_source(audio_source_t * source);
ALboolean audio_unpause_source(audio_source_t * source);	//If pause it will resume from the point it stoped. If not paused this will return false
ALboolean audio_stop_source(audio_source_t * source);	//Next time we run "play" it will resume from the beginning of the song/sfx
															//Note: Stopping a streaming source will terminate the `audio_stream_player()` call its in

ALboolean audio_prep_stream_buffers();
void * audio_stream_player(void * args);	//This function is called by a pthread

void audio_WAVE_buffer_fill(ALvoid * data);
// void audio_CDDA_buffer_fill(ALvoid * data);

//----------------------ADJUSTMENT---------------------//

uint8_t audio_adjust_master_volume(float vol);	//adjust's listener's gain
uint8_t audio_adjust_source_volume(audio_source_t * source, float vol);	//adjust's source's gain
uint8_t audio_adjust_source_speed(audio_source_t * source, float speed);
uint8_t audio_set_source_looping(audio_source_t * source, ALboolean looping);


//----------------------MISC---------------------------//
//MAYBE MAKE THESE STATIC?

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

ALboolean audio_test_error(ALCenum * error, char * msg);

void al_list_audio_devices(const ALCchar *devices);

inline ALenum to_al_format(short channels, short samples);	//Unused

bool is_big_endian();

int convert_to_int(char * buffer, int len);


#endif