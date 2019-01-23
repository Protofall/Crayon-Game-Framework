#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//For the sound effects
#include <dc/sound/stream.h>
#include <dc/sound/sfxmgr.h>
 
//Controller stuff
#include <dc/maple.h>
#include <dc/maple/controller.h>

// Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

//Check if check_button was just released (Unused)
uint8_t button_released(cont_state_t *st, uint32_t check_button, uint32_t changed_buttons){
	if(!(st->buttons & check_button) && (changed_buttons & check_button)){
		return 1;
	}
	return 0;
}

//Check if check_button is currently held (Unused)
uint8_t button_held(cont_state_t *st, uint32_t check_button){
	if(st->buttons & check_button){
		return 1;
	}
	return 0;
}

//Checks to see if check_button was just pressed
uint8_t button_pressed(cont_state_t *st, uint32_t check_button, uint32_t changed_buttons){
	if((st->buttons & check_button) && (changed_buttons & check_button)){
		return 1;
	}
	return 0;
}

//When running the program, wait a few seconds before pressing buttons since it takes a second or two to load
int main(){
	snd_stream_init();

	sfxhnd_t Sound_T = snd_sfx_load("/rd/Sound_009.wav");

	//Note. Generally all the bits we care about are in the first 16 bits, so we could store our button inputs in unit16_t's if we wanted to
	uint32_t previous_buttons = 0;	//Records the previous buttons polled
	uint32_t changed_buttons = 0;	//Records all buttons that have changed state since they were last pressed

	maple_device_t *controller;
	cont_state_t *current_state = NULL;
 
	while(1){
		controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);	//Reads the first plugged in controller
		current_state = (cont_state_t *)maple_dev_status(controller);	//State of the controller (What buttons are currently pressed)

		changed_buttons = current_state->buttons ^ previous_buttons;

		if(current_state != NULL && button_pressed(current_state, CONT_A, changed_buttons)){	//When the "A" button is pressed, play the sound
			snd_sfx_play(Sound_T, 127, 128);    // ~50% volume, pan centred
		}

		previous_buttons = current_state->buttons;
	}

	snd_sfx_unload(Sound_T);    //Unloads the sound file from the sound system
	return 0;
}
