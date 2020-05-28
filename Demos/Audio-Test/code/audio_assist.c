#include "audio_assist.h"


//----------------------MISC---------------------------//


ALboolean audio_test_error(ALCenum * error, char * msg){
	*error = alGetError();
	if(*error != AL_NO_ERROR){
		fprintf(stderr, "ERROR: %s\n", msg);
		return AL_TRUE;
	}
	return AL_FALSE;
}

static void al_list_audio_devices(const ALCchar *devices){
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	fprintf(stdout, "Devices list:\n");
	fprintf(stdout, "----------\n");
	while(device && *device != '\0' && next && *next != '\0'){
		fprintf(stdout, "%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	fprintf(stdout, "----------\n");
}

static bool is_big_endian(){
	int a = 1;
	return !((char*)&a)[0];
}

static int convert_to_int(char * buffer, int len){
	int i = 0;
	int a = 0;
	if(!is_big_endian()){
		for(; i<len; i++){
			((char*)&a)[i] = buffer[i];
		}
	}
	else{
		for(; i<len; i++){
			((char*)&a)[3 - i] = buffer[i];
		}
	}
	return a;
}

static void sleep_ms(int milliseconds){
#ifdef _WIN32
	Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
#else
	usleep(milliseconds * 1000);
#endif
	return;
}


//----------------------SETUP---------------------------//


uint8_t audio_init(){
	_audio_streamer_source = NULL;
	_audio_streamer_fp = NULL;
	_audio_streamer_stopping = 0;
	_audio_streamer_command = AUDIO_COMMAND_NONE;
	_audio_streamer_thd_active = 0;

	ALboolean enumeration;
	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };	//Double check what these vars mean
	ALCenum error;

	enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if(enumeration == AL_FALSE){
		fprintf(stderr, "enumeration extension not available\n");
	}

	al_list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	//Chooses the preferred/default device
	const ALCchar *defaultDeviceName;
	defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	_al_device = alcOpenDevice(defaultDeviceName);
	if(!_al_device){
		fprintf(stderr, "unable to open default device\n");
		return 1;
	}

	alGetError();	//This resets the error state

	_al_context = alcCreateContext(_al_device, NULL);
	if(!alcMakeContextCurrent(_al_context)){
		fprintf(stderr, "failed to make default context\n");
		goto error1;
	}
	if(audio_test_error(&error, "make default context") == AL_TRUE){goto error1;}

	// set orientation
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	if(audio_test_error(&error, "listener position") == AL_TRUE){goto error2;}

	alListener3f(AL_VELOCITY, 0, 0, 0);
	if(audio_test_error(&error, "listener velocity") == AL_TRUE){goto error2;}

	alListenerfv(AL_ORIENTATION, listenerOri);
	if(audio_test_error(&error, "listener orientation") == AL_TRUE){goto error2;}

	// Init the mutex for the streamer thread
	if(pthread_mutex_init(&_audio_streamer_lock, NULL) != 0){
		printf("Mutex init failed\n");
		goto error2;
	}

	return 0;

	error2:
	alcMakeContextCurrent(NULL);
	alcDestroyContext(_al_context);

	error1:
	alcCloseDevice(_al_device);

	return 1;
}

void audio_shutdown(){
	//Just incase it hasn't finished shutting down yet
	uint8_t status = 0;
	pthread_mutex_lock(&_audio_streamer_lock);
	if(_audio_streamer_thd_active == 1){
		status = 1;
		_audio_streamer_command = AUDIO_COMMAND_END;
	}
	pthread_mutex_unlock(&_audio_streamer_lock);
	if(status){	//Note, if it has already terminated, then this does nothing
		pthread_join(_audio_streamer_thd_id, NULL);
	}
	pthread_mutex_destroy(&_audio_streamer_lock);

	// al_context = alcGetCurrentContext();	//With only one device/context, this line might not be required
	// _al_device = alcGetContextsDevice(al_context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(_al_context);
	alcCloseDevice(_al_device);

	return;
}

ALboolean audio_load_WAV_file_info(const char * filename, audio_info_t * info, uint8_t mode){
	if((mode & AUDIO_STREAMING) && (_audio_streamer_info != NULL || _audio_streamer_fp != NULL)){
		return AL_FALSE;
	}

	info->streaming = !!(mode & AUDIO_STREAMING);
	info->data_type = AUDIO_DATA_TYPE_WAV;

	char buffer[4];

	FILE* in = fopen(filename, "rb");
	if(!in){
		fprintf(stderr, "Couldn't open file\n");
		return AL_FALSE;
	}

	fread(buffer, 4, sizeof(char), in);

	if(strncmp(buffer, "RIFF", 4) != 0){
		fprintf(stderr, "Not a valid wave file\n");
		goto error1;
	}

	fread(buffer, 4, sizeof(char), in);
	fread(buffer, 4, sizeof(char), in);		//WAVE
	fread(buffer, 4, sizeof(char), in);		//fmt
	fread(buffer, 4, sizeof(char), in);		//16
	fread(buffer, 2, sizeof(char), in);		//1
	fread(buffer, 2, sizeof(char), in);

	int chan = convert_to_int(buffer, 2);
	fread(buffer, 4, sizeof(char), in);
	info->freq = convert_to_int(buffer, 4);
	fread(buffer, 4, sizeof(char), in);
	fread(buffer, 2, sizeof(char), in);
	fread(buffer, 2, sizeof(char), in);
	int bps = convert_to_int(buffer, 2);
	fread(buffer, 4, sizeof(char), in);		//data
	fread(buffer, 4, sizeof(char), in);
	info->size = (ALsizei) convert_to_int(buffer, 4);	//This isn't the true size

	ALvoid * data = NULL;;
	if(info->streaming == AUDIO_NOT_STREAMING){
		data = (ALvoid*) malloc(info->size * sizeof(char));
		if(data == NULL){goto error1;}
		fread(data, info->size, sizeof(char), in);
		fclose(in);
	}
	else{
		_audio_streamer_fp = in;
	}

	if(chan == 1){
		info->format = (bps == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	}
	else{
		info->format = (bps == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}

	//Generate the buffers
	ALCenum error;
	info->srcs_attached = 0;
	info->buff_cnt = (info->streaming == AUDIO_STREAMING) ? AUDIO_STREAMING_NUM_BUFFERS : 1;
	info->buff_id = malloc(sizeof(ALuint) * info->buff_cnt);
	if(info->buff_id == NULL){if(data){free(data)}; return AL_FALSE;}
	alGenBuffers(info->buff_cnt, info->buff_id);	//Generating "info->buff_cnt" buffers. 2nd param is a pointer to an array
													//of ALuint values which will store the names of the new buffers
													//Seems "buff_id" doesn't actually contain the data?
	if(audio_test_error(&error, "buffer generation") == AL_TRUE){goto error2;}

	//Filling the buffer for non-streamers
	if(info->streaming == AUDIO_NOT_STREAMING){
		alBufferData(info->buff_id[0], info->format, data, info->size, info->freq);	//Fill the buffer with PCM data
		if(audio_test_error(&error, "buffer copy") == AL_TRUE){goto error3;}

		free(data);
	}
	else{
		_audio_streamer_info = info;
	}

	printf("Loaded WAV file!\n");

	return AL_TRUE;


	//--------------------


	error3:
	alDeleteBuffers(info->buff_cnt, info->buff_id);

	error2:
	free(info->buff_id);
	if(data){free(data);}
	return AL_FALSE;

	error1:
	fclose(in);
	return AL_FALSE;
}

//Currently unimplemented
ALboolean audio_load_CDDA_track_info(uint8_t drive, uint8_t track, audio_info_t * info, uint8_t mode){
	if(mode & AUDIO_STREAMING && (_audio_streamer_source != NULL || _audio_streamer_fp != NULL)){
		return AL_FALSE;
	}

	//Delete these later
	(void)drive;
	(void)track;
	(void)info;

	//Fix later
	return AL_FALSE;
}

ALboolean audio_load_OGG_file_info(const char * path, audio_info_t * info, uint8_t mode){
	if(mode & AUDIO_STREAMING && (_audio_streamer_source != NULL || _audio_streamer_fp != NULL)){
		return AL_FALSE;
	}

	//Delete these later
	(void)path;
	(void)info;

	//Fix later
	return AL_FALSE;
}

ALboolean audio_free_info(audio_info_t * info){
	if(info == NULL){
		return AL_FALSE;
	}

	if(info->streaming == AUDIO_STREAMING){
		//We haven't tried to free the source yet
		if(_audio_streamer_thd_active && _audio_streamer_command != AUDIO_COMMAND_END){
			return AL_FALSE;
		}
		//The sleep time might need adjusting
		//Loop purpose is to make sure streamer has stopped working before we continue
		while(_audio_streamer_thd_active && _audio_streamer_command == AUDIO_COMMAND_END){
			sleep_ms(10);
		}

		fclose(_audio_streamer_fp);
		_audio_streamer_fp = NULL;
		_audio_streamer_info = NULL;
	}

	alDeleteBuffers(info->buff_cnt, info->buff_id);	//1st param is number of buffers
	free(info->buff_id);

	return AL_TRUE;
}

ALboolean audio_free_source(audio_source_t * source){
	if(source == NULL){
		return AL_FALSE;
	}

	//So we can later know there isn't a streamer presents
	if(source == _audio_streamer_source){
		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1){
			_audio_streamer_command = AUDIO_COMMAND_END;	//Doesn't matter what we said before, we want it to terminate now
		}
		pthread_mutex_unlock(&_audio_streamer_lock);
	}
	else{	//Streamer thread will handle this itself
		alDeleteSources(1, &source->src_id);
	}

	source->info->srcs_attached--;

	return AL_TRUE;
}

ALboolean audio_create_source(audio_source_t * source, audio_info_t * info, vec2_f_t position, ALboolean looping,
	float volume, float speed){

	if(source == NULL || info == NULL || volume < 0 || speed < 0){
		return AL_FALSE;
	}

	if(info->streaming == AUDIO_STREAMING){
		if(_audio_streamer_source != NULL){return AL_FALSE;}

		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1){	//There's already a streamer thread
			pthread_mutex_unlock(&_audio_streamer_lock);
			return AL_FALSE;
		}
		pthread_mutex_unlock(&_audio_streamer_lock);
		
		_audio_streamer_source = source;
	}

	source->info = info;
	source->position = position;
	source->looping = looping;
	source->current_volume = volume;
	source->target_volume = volume;
	source->speed = speed;

	ALCenum error;

	alGenSources((ALuint)1, &source->src_id);	//Generate one source
	if(audio_test_error(&error, "source generation") == AL_TRUE){return AL_FALSE;}

	alSourcef(source->src_id, AL_PITCH, speed);
	if(audio_test_error(&error, "source pitch") == AL_TRUE){return AL_FALSE;}

	alSourcef(source->src_id, AL_GAIN, volume);
	if(audio_test_error(&error, "source gain") == AL_TRUE){return AL_FALSE;}

	/*
		Other alSourcef options:

		AL_MIN_GAIN
		AL_MAX_GAIN
		AL_MAX_DISTANCE
		AL_ROLLOFF_FACTOR
		AL_CONE_OUTER_GAIN
		AL_CONE_INNER_ANGLE
		AL_CONE_OUTER_ANGLE
		AL_REFERENCE_DISTANCE 
	*/

	alSource3f(source->src_id, AL_POSITION, position.x, position.y, 0);	//Since we're 2D, Z is always zero
	if(audio_test_error(&error, "source position") == AL_TRUE){return AL_FALSE;}

	alSource3f(source->src_id, AL_VELOCITY, 0, 0, 0);
	if(audio_test_error(&error, "source velocity") == AL_TRUE){return AL_FALSE;}

	//When streaming we handle looping manually, so we tell OpenAL not to bother with looping
	alSourcei(source->src_id, AL_LOOPING, (info->streaming == AUDIO_STREAMING) ? AL_FALSE : looping);
	if(audio_test_error(&error, "source looping") == AL_TRUE){return AL_FALSE;}

	/*
		Other alSourcei options:

		AL_SOURCE_RELATIVE
		AL_CONE_INNER_ANGLE
		AL_CONE_OUTER_ANGLE
		AL_BUFFER
		AL_SOURCE_STATE
	*/
	if(info->streaming == AUDIO_NOT_STREAMING){
		alSourcei(source->src_id, AL_BUFFER, info->buff_id[0]);
		if(audio_test_error(&error, "buffer binding") == AL_TRUE){return AL_FALSE;}
	}
	else{	//We start the streamer thread
		if(pthread_create(&_audio_streamer_thd_id, NULL, audio_stream_player, NULL)){
			printf("Failed to create streamer thread\n");
			return AL_FALSE;
		}
		else{	//The threaded function already does this, but incase you try to change the state right after doing this and
				//somehow the threaded function hasn't run yet, this will allow it to do so
			pthread_mutex_lock(&_audio_streamer_lock);
			_audio_streamer_thd_active = 1;
			pthread_mutex_unlock(&_audio_streamer_lock);
		}
	}

	info->srcs_attached++;

	return AL_TRUE;
}

ALboolean audio_update_source_state(audio_source_t * source){
	if(source == NULL){
		return AL_FALSE;
	}

	ALCenum error;
	alGetSourcei(source->src_id, AL_SOURCE_STATE, &source->state);
	if(audio_test_error(&error, "source state get") == AL_TRUE){return AL_FALSE;}

	return AL_TRUE;
}

ALboolean audio_play_source(audio_source_t * source){
	if(source == NULL){return AL_FALSE;}

	ALCenum error;
	ALboolean ret_val;
	if(source == _audio_streamer_source){
		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1){
			if(_audio_streamer_command == AUDIO_COMMAND_NONE){
				_audio_streamer_command = AUDIO_COMMAND_PLAY;
				ret_val = AL_TRUE;
			}
			else{ret_val = AL_FALSE;}
		}
		else{ret_val = AL_FALSE;}	//This should never occur
		pthread_mutex_unlock(&_audio_streamer_lock);
		return ret_val;
	}
	alSourcePlay(source->src_id);	//If called on a source that is already playing, it will restart from the beginning
	if(audio_test_error(&error, "source playing") == AL_TRUE){return AL_FALSE;}
	return AL_TRUE;
}

ALboolean audio_pause_source(audio_source_t * source){
	if(source == NULL){return AL_FALSE;}

	ALCenum error;
	ALboolean ret_val;
	if(source == _audio_streamer_source){
		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1 && _audio_streamer_command == AUDIO_COMMAND_NONE){
			ret_val = AL_TRUE;
		}
		else{ret_val = AL_FALSE;}

		//Should checking the source state be out of the mutex lock?
		audio_update_source_state(source);
		if(source->state != AL_PLAYING){
			ret_val = AL_FALSE;
		}

		if(ret_val == AL_TRUE){
			_audio_streamer_command = AUDIO_COMMAND_PAUSE;
		}

		pthread_mutex_unlock(&_audio_streamer_lock);
		return ret_val;
	}
	alSourcePause(source->src_id);
	if(audio_test_error(&error, "source pausing") == AL_TRUE){return AL_FALSE;}
	return AL_TRUE;
}

ALboolean audio_unpause_source(audio_source_t * source){
	if(source == NULL){return AL_FALSE;}

	ALCenum error;
	ALboolean ret_val;
	if(source == _audio_streamer_source){
		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1 && _audio_streamer_command == AUDIO_COMMAND_NONE){
			ret_val = AL_TRUE;
		}
		else{ret_val = AL_FALSE;}

		//Should checking the source state be out of the mutex lock?
		audio_update_source_state(source);
		if(source->state != AL_PAUSED && source->state != AL_STOPPED){
			ret_val = AL_FALSE;
		}

		if(ret_val == AL_TRUE){
			_audio_streamer_command = AUDIO_COMMAND_UNPAUSE;
		}

		pthread_mutex_unlock(&_audio_streamer_lock);
		return ret_val;
	}
	alSourcePlay(source->src_id);
	if(audio_test_error(&error, "source playing") == AL_TRUE){return AL_FALSE;}
	return AL_TRUE;
}

ALboolean audio_stop_source(audio_source_t * source){
	if(source == NULL){return AL_FALSE;}

	ALCenum error;
	ALboolean ret_val;
	if(source == _audio_streamer_source){
		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1){
			if(_audio_streamer_command == AUDIO_COMMAND_NONE){
				_audio_streamer_command = AUDIO_COMMAND_STOP;
				ret_val = AL_TRUE;
			}
			else{ret_val = AL_FALSE;}
		}
		else{ret_val = AL_FALSE;}	//This should never occur
		pthread_mutex_unlock(&_audio_streamer_lock);
		return ret_val;
	}
	alSourceStop(source->src_id);
	if(audio_test_error(&error, "source stopping") == AL_TRUE){return AL_FALSE;}
	return AL_TRUE;
}

ALboolean audio_streamer_buffer_fill(ALuint id){
	ALvoid * data;
	ALCenum error;
	data = malloc(AUDIO_STREAMING_DATA_CHUNK_SIZE);

	//data array is filled with song info
	switch(_audio_streamer_info->data_type){
	case AUDIO_DATA_TYPE_WAV:
		audio_WAV_buffer_fill(data);
		break;
	// case AUDIO_DATA_TYPE_CDDA:
	// 	audio_CDDA_buffer_fill(data);
	// 	break;
	// case AUDIO_DATA_TYPE_OGG:
	// 	audio_OGG_buffer_fill(data);
	// 	break;
	default:
		free(data);
		return AL_FALSE;
	}

	alBufferData(id, _audio_streamer_info->format, data, AUDIO_STREAMING_DATA_CHUNK_SIZE, _audio_streamer_info->freq);
	free(data);

	if(audio_test_error(&error, "loading wav file") == AL_TRUE){return AL_FALSE;}
	alSourceQueueBuffers(_audio_streamer_source->src_id, 1, &id);	//last parameter is a pointer to an array
	if(audio_test_error(&error, "queue-ing buffer") == AL_TRUE){return AL_FALSE;}

	return AL_TRUE;
}

ALboolean audio_prep_stream_buffers(){
	// Fill all the buffers with audio data from the wave file
	uint8_t i;
	ALboolean res;
	for(i = 0; i < _audio_streamer_info->buff_cnt; i++){
		res = audio_streamer_buffer_fill(_audio_streamer_info->buff_id[i]);
		if(res == AL_FALSE){return AL_FALSE;}
	}

	return AL_TRUE;
}

//Need to fix returns to unalloc memory if need be
//I won't be checking the return type so it doesn't matter
void * audio_stream_player(void * args){
	(void)args;	//This only exists to make the compiler shut up

	pthread_mutex_lock(&_audio_streamer_lock);
	_audio_streamer_thd_active = 1;
	pthread_mutex_unlock(&_audio_streamer_lock);

	//Queue up all buffers for the source with the beginning of the song
	if(audio_prep_stream_buffers() == AL_FALSE){_audio_streamer_thd_active = 0; return NULL;}

	uint8_t command;

	ALint iBuffersProcessed = 0;
	ALuint uiBuffer;

	uint8_t starting = 0;
	uint8_t refresh_buffers = 0;

	// ALfloat speed, new_speed;
	// alGetSourcef(_audio_streamer_source->src_id, AL_PITCH, &speed);
	// new_speed = speed;

	//NOTE: This doesn't really account for playback speed very well
	// int sleep_time = (_audio_streamer_info->freq / AUDIO_STREAMING_DATA_CHUNK_SIZE) * 1000 / speed;

	//Different play states
	// AL_STOPPED
	// AL_PLAYING
	// AL_PAUSED
	// AL_INITIAL (Set by rewind)
	// AL_UNDETERMINED (When a source is initially stated)
	// AL_STREAMING (after successful alSourceQueueBuffers)

	// - The difference between STOP and PAUSE is that calling alSourcePlay after pausing
	// will resume from the position the source was when it was paused and
	// calling alSourcePlay after stopping will resume from the beginning of
	// the buffer(s).

	while(1){
		pthread_mutex_lock(&_audio_streamer_lock);
		command = _audio_streamer_command;
		_audio_streamer_command = AUDIO_COMMAND_NONE;
		pthread_mutex_unlock(&_audio_streamer_lock);

		//If source update is false then that means the source has been removed before this thread has finished
		//Its not harmful though since if its false the statement is never entered and we exit when we check the command
			//Next two conditions basically detect when it naturally stops playing (Non-looping) and it will reset the buffers
		if(audio_update_source_state(_audio_streamer_source) == AL_TRUE &&
			_audio_streamer_source->state == AL_STOPPED && _audio_streamer_stopping == 1){

			fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);
			refresh_buffers = 1;
			_audio_streamer_stopping = 0;
		}

		//If the user changed the playback speed, we'll update our sleep time
		// alGetSourcef(_audio_streamer_source->src_id, AL_PITCH, &new_speed);
		// if(new_speed != speed){
		// 	sleep_time = (_audio_streamer_info->freq / AUDIO_STREAMING_DATA_CHUNK_SIZE) * 1000 / speed;
		// 	speed = new_speed;
		// }

		if(command > AUDIO_COMMAND_END){command = AUDIO_COMMAND_NONE;}
		if(command == AUDIO_COMMAND_PLAY || command == AUDIO_COMMAND_UNPAUSE){
			if(_audio_streamer_source->state == AL_PLAYING){	//If we play during playing then we reset
				alSourceStop(_audio_streamer_source->src_id);
				fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);
				refresh_buffers = 1;
				starting = 1;
			}
			else{
				alSourcePlay(_audio_streamer_source->src_id);
				//Don't need to refresh buffers since they should already be prep-d
			}
			
			// printf("STARTING\n");
		}
		else if(command == AUDIO_COMMAND_PAUSE){
			alSourcePause(_audio_streamer_source->src_id);
		}
		else if(command == AUDIO_COMMAND_STOP){
			alSourceStop(_audio_streamer_source->src_id);	//All buffers should now be unqueued unless your Nvidia driver sucks
			fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);	//Reset to beginning
			refresh_buffers = 1;
			// printf("STOPPING\n");
		}
		else if(command == AUDIO_COMMAND_END){break;}

		// if(_audio_streamer_source->state == AL_PLAYING){printf("STATE: PLAYING\n");}
		// else if(_audio_streamer_source->state == AL_STOPPED){printf("STATE: STOPPED\n");}
		// else if(_audio_streamer_source->state == AL_PAUSED){printf("STATE: PAUSED\n");}
		// else if(_audio_streamer_source->state == AL_STREAMING){printf("STATE: STREAMING\n");}	//Never seem to see these
		// else if(_audio_streamer_source->state == AL_INITIAL){printf("STATE: INITIAL\n");}
		// else if(_audio_streamer_source->state == AL_UNDETERMINED){printf("STATE: UNDETERMINED\n");}	//Never seem to see these
		// else{printf("STATE: Unknown\n");}	//Never seem to see these

		//So we don't waste time doing stuff when stopped
			//I've never seen the second condition before
		if(_audio_streamer_source->state == AL_PLAYING || _audio_streamer_source->state == AL_STREAMING ||
			refresh_buffers){
			// printf("I AM CHECKING THE PROCESSED BUFFERS\n");

			// Buffer queuing loop must operate in a new thread
			iBuffersProcessed = 0;
			alGetSourcei(_audio_streamer_source->src_id, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

			// if(iBuffersProcessed > 0){
			// 	printf("I AM ACTUALLY PROCESSING %d BUFFERS\n", iBuffersProcessed);
			// }

			// For each processed buffer, remove it from the source queue, read the next chunk of
			// audio data from the file, fill the buffer with new data, and add it to the source queue
			// But we don't read if we're currently playing, but the audio is ending
			while(iBuffersProcessed && !_audio_streamer_stopping){
				// Remove the buffer from the queue (uiBuffer contains the buffer ID for the dequeued buffer)
				//The unqueue operation will only take place if all n (1) buffers can be removed from the queue.
				uiBuffer = 0;
				alSourceUnqueueBuffers(_audio_streamer_source->src_id, 1, &uiBuffer);

				audio_streamer_buffer_fill(uiBuffer);

				iBuffersProcessed--;
			}
		}
		refresh_buffers = 0;

		if(starting){
			alSourcePlay(_audio_streamer_source->src_id);
			starting = 0;
		}

		//All of these will basically tell the thread manager that this thread is done and if any other threads are waiting then
		//we should process them
		#if defined(__APPLE__) || defined(__linux__) || defined(_arch_dreamcast)
			sched_yield();
		#endif
		#ifdef _WIN32
			Sleep(0);	// https://stackoverflow.com/questions/3727420/significance-of-sleep0
						//Might want to replace this with something else since the CPU will be at 100% if this is the only active thread
		#endif

		//Might be an issue for higher frequency audio, but right now this works
		// sleep_ms(sleep_time);
	}

	//Shutdown the system
	audio_stop_source(_audio_streamer_source);	//This will de-queue all of the queue-d buffers

	//Free the source
	alDeleteSources(1, &_audio_streamer_source->src_id);

	//Tell the world we're done
	pthread_mutex_lock(&_audio_streamer_lock);
	_audio_streamer_thd_active = 0;
	_audio_streamer_source = NULL;
	_audio_streamer_command = AUDIO_COMMAND_NONE;
	pthread_mutex_unlock(&_audio_streamer_lock);

	return 0;
}

void audio_WAV_buffer_fill(ALvoid * data){
	int spare = (_audio_streamer_source->info->size + WAV_HDR_SIZE) - ftell(_audio_streamer_fp);	//This is how much data in the entire file
	int read = fread(data, 1, (AUDIO_STREAMING_DATA_CHUNK_SIZE < spare) ? AUDIO_STREAMING_DATA_CHUNK_SIZE : spare, _audio_streamer_fp);
	if(read < AUDIO_STREAMING_DATA_CHUNK_SIZE){
		fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);	//Skips the header, beginning of body
		if(_audio_streamer_source->looping){	//Fill from beginning
			fread(&((char*)data)[read], AUDIO_STREAMING_DATA_CHUNK_SIZE - read, 1, _audio_streamer_fp);
		}
		else{	//Fill with zeroes/silence
			memset(&((char *)data)[read], 0, AUDIO_STREAMING_DATA_CHUNK_SIZE - read);
			_audio_streamer_stopping = 1;	//It will take a second before the source state goes to stopped
			//Can't reset the file pointer yet since its kinda used above
		}
	}
}

void audio_CDDA_buffer_fill(ALvoid * data){
	(ALvoid)data;	//This only exists to make the compiler shut up
	;
}

void audio_OGG_buffer_fill(ALvoid * data){
	(ALvoid)data;	//This only exists to make the compiler shut up
	;
}


//----------------------ADJUSTMENT---------------------//


uint8_t audio_adjust_master_volume(float vol){
	if(vol < 0){return 1;}

	ALCenum error;
	alListenerf(AL_GAIN, vol);
	if(audio_test_error(&error, "listener gain") == AL_TRUE){return 1;}
	return 0;
}

uint8_t audio_adjust_source_volume(audio_source_t * source, float vol){
	if(vol < 0 || source == NULL){return 1;}

	ALCenum error;
	alSourcef(source->src_id, AL_GAIN, vol);
	if(audio_test_error(&error, "source gain") == AL_TRUE){return 1;}
	return 0;
}

uint8_t audio_adjust_source_speed(audio_source_t * source, float speed){
	if(speed < 0 || source == NULL){return 1;}

	ALCenum error;
	alSourcef(source->src_id, AL_PITCH, speed);
	if(audio_test_error(&error, "source pitch") == AL_TRUE){return 1;}
	return 0;
}

uint8_t audio_set_source_looping(audio_source_t * source, ALboolean looping){
	if(source == NULL){return 1;}

	ALCenum error;
	alSourcei(source->src_id, AL_LOOPING, looping);
	if(audio_test_error(&error, "source looping") == AL_TRUE){return AL_FALSE;}
	return 0;
}
