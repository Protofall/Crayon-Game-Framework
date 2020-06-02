#ifndef MS_EXTRA_STRUCTS_H
#define MS_EXTRA_STRUCTS_H

#include <crayon/texture_structs.h>
#include <crayon/savefile.h>

//This file exist just to make the main file a little bit more clean
typedef struct minesweeper_savefile{
	uint8_t options;	//XXXH LOSQ (Refresh rate (Hz), Language, OS, Sound, Questions)
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[6][16];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint16_t var_16;

	uint8_t last_var;
} minesweeper_savefile_t;

#endif
