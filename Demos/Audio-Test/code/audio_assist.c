#include "audio_assist.h"

uint8_t audio_init(){
	_audio_streamer_source = NULL;
	_audio_streamer_fp = NULL;
	_audio_streamer_stopping = 0;

	ALboolean enumeration;
	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };	//Double check what these vars mean
	ALCenum error;

	enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if(enumeration == AL_FALSE){
		fprintf(stderr, "enumeration extension not available\n");
	}

	al_list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	// const ALCchar *defaultDeviceName;
	// if(!defaultDeviceName){
	// 	defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	// }
	// _al_device = alcOpenDevice(defaultDeviceName);
	_al_device = alcOpenDevice(NULL);	//Chooses the preferred/default device
	if(!_al_device){
		fprintf(stderr, "unable to open default device\n");
		return 1;
	}

    // fprintf(stdout, "Device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));

	alGetError();	//This resets the error state

	_al_context = alcCreateContext(_al_device, NULL);
	if(!alcMakeContextCurrent(_al_context)){
		fprintf(stderr, "failed to make default context\n");
		return 1;
	}
	if(audio_test_error(&error, "make default context") == AL_TRUE){return 1;}

	// set orientation
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	if(audio_test_error(&error, "listener position") == AL_TRUE){return 1;}

	alListener3f(AL_VELOCITY, 0, 0, 0);
	if(audio_test_error(&error, "listener velocity") == AL_TRUE){return 1;}

	alListenerfv(AL_ORIENTATION, listenerOri);
	if(audio_test_error(&error, "listener orientation") == AL_TRUE){return 1;}

	// Init the mutex for the streamer thread
	if(pthread_mutex_init(&_audio_streamer_lock, NULL) != 0){
		printf("Mutex init failed\n");
		return 1;
	}

	_audio_streamer_command = AUDIO_COMMAND_NONE;
	_audio_streamer_thd_active = 0;

	return 0;
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
	if((mode & AUDIO_STREAMING) && (_audio_streamer_source != NULL || _audio_streamer_fp != NULL)){
		return AL_FALSE;
	}

	info->streaming = !!(mode & AUDIO_STREAMING);
	switch(info->streaming){
		case AUDIO_NOT_STREAMING:
			info->data_type = AUDIO_DATA_TYPE_WAV;
		break;
		case AUDIO_STREAMING:
			info->data_type = AUDIO_DATA_TYPE_WAV;
		break;
		default:
			fprintf(stderr, "Invalid wav load mode given\n");
			return AL_FALSE;
		break;
	}	

	char buffer[4];

	FILE* in = fopen(filename, "rb");
	if(!in){	//Triggers?
		fprintf(stderr, "Couldn't open file\n");
		return AL_FALSE;
	}

	fread(buffer, 4, sizeof(char), in);

	if(strncmp(buffer, "RIFF", 4) != 0){
		fprintf(stderr, "Not a valid wave file\n");
		return AL_FALSE;
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

	if(info->streaming != AUDIO_STREAMING){
		info->data = (ALvoid*) malloc(info->size * sizeof(char));
		fread(info->data, info->size, sizeof(char), in);
	}
	else{
		info->data = NULL;
	}

	if(chan == 1){
		info->format = (bps == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	}
	else{
		info->format = (bps == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}

	_audio_streamer_info = info;
	if(info->streaming != AUDIO_STREAMING){
		fclose(in);
	}
	else{
		_audio_streamer_fp = in;
	}

	printf("Loaded WAV file!\n");

	return AL_TRUE;
}

//Currently unimplemented
ALboolean audio_load_CDDA_track_info(uint8_t track, audio_info_t * info, uint8_t mode){
	if(mode & AUDIO_STREAMING && (_audio_streamer_source != NULL || _audio_streamer_fp != NULL)){
		return AL_FALSE;
	}

	return AL_FALSE;
}

ALboolean audio_unload_info(audio_info_t * info){
	if(info == NULL){
		return AL_FALSE;
	}

	audio_free_info_data(info);	//Note: If there's no data this doesn nothing

	if(info->streaming == AUDIO_STREAMING){
		fclose(_audio_streamer_fp);
		_audio_streamer_fp = NULL;
		_audio_streamer_source->info = NULL;
		audio_free_source(_audio_streamer_source);
		_audio_streamer_info = NULL;
	}

	return AL_TRUE;
}

void audio_free_info_data(audio_info_t * info){
	if(info != NULL && info->data){
		free(info->data);
		info->data = NULL;
	}
	return;
}

ALboolean audio_free_source(audio_source_t * source){
	if(source == NULL){
		return AL_FALSE;
	}

	alDeleteSources(1, &source->src_id);
	alDeleteBuffers(source->buff_cnt, source->buff_id);	//1st param is number of buffers

	//So we can later know there isn't a streamer presents
	if(source == _audio_streamer_source){
		pthread_mutex_lock(&_audio_streamer_lock);
		if(_audio_streamer_thd_active == 1){
			_audio_streamer_command = AUDIO_COMMAND_END;	//Doesn't matter what we said before, we want it to terminate now
		}
		pthread_mutex_unlock(&_audio_streamer_lock);

		//NOTE: I could add code to wait for the thread to end, but I think its fine to assume it will end on its own

		_audio_streamer_source = NULL;
	}

	return AL_TRUE;
}

ALboolean audio_create_source(audio_source_t * source, audio_info_t * info, vec2_f_t position, ALboolean looping,
	float volume, float speed, uint8_t delete_data){

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
	source->volume = volume;
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

	//1 buffer normally, but "AUDIO_STREAMING_NUM_BUFFERS" for streaming
	source->buff_cnt = (info->streaming == AUDIO_STREAMING) ? AUDIO_STREAMING_NUM_BUFFERS : 1;

	//Generate the buffers
	source->buff_id = malloc(sizeof(ALuint) * source->buff_cnt);
	alGenBuffers(source->buff_cnt, source->buff_id);	//Generating "source->buff_cnt" buffers. 2nd param is a pointer to an array
															//of ALuint values which will store the names of the new buffers
															//Seems "buffer" is just an ID and doesn't actually contain the data?
	if(audio_test_error(&error, "buffer generation") == AL_TRUE){return AL_FALSE;}

	if(info->streaming == AUDIO_NOT_STREAMING){
		alBufferData(source->buff_id[0], info->format, info->data, info->size, info->freq);	//Fill the buffer with PCM data
		if(audio_test_error(&error, "buffer copy") == AL_TRUE){return AL_FALSE;}

		alSourcei(source->src_id, AL_BUFFER, source->buff_id[0]);
		if(audio_test_error(&error, "buffer binding") == AL_TRUE){return AL_FALSE;}

		if(delete_data){
			audio_free_info_data(info);
		}
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

ALboolean audio_prep_stream_buffers(){
	ALvoid * data;

	// Fill all the buffers with audio data from the wave file
	uint8_t i;
	for(i = 0; i < _audio_streamer_source->buff_cnt; i++){
		data = malloc(AUDIO_STREAMING_DATA_CHUNK_SIZE);
		audio_WAVE_buffer_fill(data);	//data array is filled with song info
		alBufferData(_audio_streamer_source->buff_id[i], _audio_streamer_source->info->format, data, AUDIO_STREAMING_DATA_CHUNK_SIZE, _audio_streamer_source->info->freq);
		free(data);
		alSourceQueueBuffers(_audio_streamer_source->src_id, 1, &_audio_streamer_source->buff_id[i]);
	}

	ALCenum error;
	if(audio_test_error(&error, "loading wav file") == AL_TRUE){return AL_FALSE;}
	return AL_TRUE;
}

//Need to fix returns to unalloc memory if need be
//I won't be checking the return type so it doesn't matter
void * audio_stream_player(void * args){
	pthread_mutex_lock(&_audio_streamer_lock);
	_audio_streamer_thd_active = 1;
	pthread_mutex_unlock(&_audio_streamer_lock);

	//Queue up all buffers for the source with the beginning of the song
	if(audio_prep_stream_buffers() == AL_FALSE){return NULL;}	//Currently only works for WAV files

	uint8_t command;
 	ALvoid *data;

	ALint iBuffersProcessed;
	ALuint uiBuffer;
	// Buffer queuing loop must operate in a new thread
	iBuffersProcessed = 0;

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

	//Normally when the attached buffers are done playing, the source will progress to the stopped state

	while(1){
		pthread_mutex_lock(&_audio_streamer_lock);
		command = _audio_streamer_command;
		_audio_streamer_command = AUDIO_COMMAND_NONE;
		pthread_mutex_unlock(&_audio_streamer_lock);

		audio_update_source_state(_audio_streamer_source);

		if(command > AUDIO_COMMAND_END){command = AUDIO_COMMAND_NONE;}	//Invalid command given
		else if(command == AUDIO_COMMAND_PLAY || command == AUDIO_COMMAND_UNPAUSE){
			if(_audio_streamer_source->state == AL_PLAYING){	//If we play during playing then we reset
				alSourceStop(_audio_streamer_source->src_id);
				fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);
			}
			alSourcePlay(_audio_streamer_source->src_id);
			_audio_streamer_stopping = 0;
		}
		else if(command == AUDIO_COMMAND_PAUSE){
			alSourcePause(_audio_streamer_source->src_id);
		}
		else if(command == AUDIO_COMMAND_STOP){	//I feel like this is done poorly, but I can't tell
			alSourceStop(_audio_streamer_source->src_id);	//All buffers should now be unqueued unless your Nvidia driver sucks
			fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);	//Reset to beginning
			// ALint lol;
			// alGetSourcei(_audio_streamer_source->src_id, AL_BUFFERS_PROCESSED, &lol);
			// if(lol != 4){error_freeze("");}	//Its equal to 4 as expected
		}
		else if(command == AUDIO_COMMAND_END){break;}

		// Buffer queuing loop must operate in a new thread
		iBuffersProcessed = 0;
		alGetSourcei(_audio_streamer_source->src_id, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

		// For each processed buffer, remove it from the source queue, read the next chunk of
		// audio data from the file, fill the buffer with new data, and add it to the source queue
		while(iBuffersProcessed && !_audio_streamer_stopping){
			// Remove the buffer from the queue (uiBuffer contains the buffer ID for the dequeued buffer)
			//The unqueue operation will only take place if all n (1) buffers can be removed from the queue.
			//Thats why we do it one at a time
			uiBuffer = 0;
			alSourceUnqueueBuffers(_audio_streamer_source->src_id, 1, &uiBuffer);

			// Read more pData audio data (if there is any)
			data = malloc(AUDIO_STREAMING_DATA_CHUNK_SIZE);
			audio_WAVE_buffer_fill(data);
			// Copy audio data to buffer
			alBufferData(uiBuffer, _audio_streamer_source->info->format, data, AUDIO_STREAMING_DATA_CHUNK_SIZE, _audio_streamer_source->info->freq);
			free(data);
			// Insert the audio buffer to the source queue
			alSourceQueueBuffers(_audio_streamer_source->src_id, 1, &uiBuffer);

			iBuffersProcessed--;
		}

		//All of these will basically tell the thread manager that this thread is done and if any other threads are waiting then
		//we should process them
		#if defined(_arch_unix) || defined(_arch_dreamcast)
			sched_yield();
		#endif
		#ifdef _arch_windows
			sleep(0);	// https://stackoverflow.com/questions/3727420/significance-of-sleep0
						//Might want to replace this with something else since the CPU will be at 100% if this is the only active thread
		#endif
	}

	//Shutdown the system
	audio_stop_source(_audio_streamer_source);	//This will de-queue all of the queue-d buffers
	fclose(_audio_streamer_fp);

	//Tell the world we're done
	pthread_mutex_lock(&_audio_streamer_lock);
	_audio_streamer_thd_active = 0;
	pthread_mutex_unlock(&_audio_streamer_lock);

	return 0;
}

void audio_WAVE_buffer_fill(ALvoid * data){
	int spare = (_audio_streamer_source->info->size + WAV_HDR_SIZE) - ftell(_audio_streamer_fp);	//This is how much data in the entire file
	int read = fread(data, 1, MIN(AUDIO_STREAMING_DATA_CHUNK_SIZE, spare), _audio_streamer_fp);
	if(read < AUDIO_STREAMING_DATA_CHUNK_SIZE){
		fseek(_audio_streamer_fp, WAV_HDR_SIZE, SEEK_SET);	//Skips the header, beginning of body
		if(_audio_streamer_source->looping){	//Fill from beginning
			fread(&((char*)data)[read], AUDIO_STREAMING_DATA_CHUNK_SIZE - read, 1, _audio_streamer_fp);
		}
		else{	//Fill with zeroes/silence
			memset(&((char *)data)[read], 0, AUDIO_STREAMING_DATA_CHUNK_SIZE - read);
			_audio_streamer_stopping = 1;	//It will take a second before the source state goes to stopped
		}
	}
}

void audio_CDDA_buffer_fill(ALvoid * data){
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


//----------------------MISC---------------------------//


ALboolean audio_test_error(ALCenum * error, char * msg){
	*error = alGetError();
	if(*error != AL_NO_ERROR){
        fprintf(stderr, "ERROR: ");
		fprintf(stderr, msg);
		fprintf(stderr, "\n");
		return AL_TRUE;
	}
	return AL_FALSE;
}

void al_list_audio_devices(const ALCchar *devices){
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

bool is_big_endian(){
	int a = 1;
	return !((char*)&a)[0];
}

int convert_to_int(char * buffer, int len){
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
