#ifndef MS_EXTRA_STRUCTS_H
#define MS_EXTRA_STRUCTS_H

#include "../../Crayon/code/dreamcast/texture_structs.h"

//This file exist just to make the main file a little bit more clean

//Should add some code here to move the sd and region icons (Or store their x/y's here)
typedef struct MinesweeperOS{
	uint8_t *ids;	//Animation ids
	uint16_t *coords_pos;	//The x and y for placing it
	uint16_t *coords_frame;	//The frame coordinates for UV
	uint16_t *scale;	//The scale factor
	uint8_t sprite_count;	//Number of sprites to draw
	crayon_spritesheet_t *windows_ss;	//The spritesheet for the windows assets

	//These two are for stuff that appear in both OSes, but vary depending on which OS you use
	uint16_t *variant_pos;	//Records the x and y for all the OS dependent stuff not in OS spritesheet
	uint8_t clock_palette;	//The palette for the clock to use
} MinesweeperOS_t;

//Figure out what I need to do here if anything
typedef struct MinesweeperLang{
	;
} MinesweeperLang_t;

typedef struct MinesweeperSaveFile{
	uint8_t BS_Mode;	//Bulletsweeper mode. 0 for never won, 1 for Beat with 1 player, 2 for beat with 2 players, etc.
	uint8_t checkbox_options;	//Sound and Question marks
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[11][6];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint8_t pref_mines;
} MinesweeperSaveFile_t;

/*

OS dependent stuff

[Menu_X] [Menu_Y]	//Game, Options, About
[Clock_X] [Clock_Y]
[Icon_Eject_X] [Icon_Eject_Y]
[Icon_DC_X] [Icon_DC_Y]
[Icon_BS_X] [Icon_BS_Y]

*/

#endif
