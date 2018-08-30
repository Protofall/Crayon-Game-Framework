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

//When running the program, wait a few seconds before pressing buttons since it takes a second or two to load
int main(){
    snd_stream_init();

    sfxhnd_t Sound_T = snd_sfx_load("/rd/Sound_009.wav");

    uint32_t previous_buttons = 0;	//Records the previous buttons polled
    maple_device_t  * controller;
    cont_state_t * st;
 
    while(1){
	controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);	//Reads the first plugged in controller
        st = (cont_state_t *)maple_dev_status(controller);	//State of controller

        if(st->buttons & (1 << 2) && (!(previous_buttons & (1 << 2)))){	//When the "A" button is pressed, play the sound
            snd_sfx_play(Sound_T, 127, 128);    // ~50% volume, pan centred
        }

        previous_buttons = st->buttons;
    }
}
