#ifndef MS_EXTRA_STRUCTS_H
#define MS_EXTRA_STRUCTS_H

#include "../../Crayon/code/crayon/dreamcast/texture_structs.h"

//This file exist just to make the main file a little bit more clean

typedef struct MinesweeperOS{
	uint8_t * ids;	//Animation ids
	uint16_t * coords_pos;	//The x and y for placing it
	uint16_t * coords_frame;	//The frame coordinates for UV
	uint16_t * scale;	//The scale factor
	uint8_t sprite_count;	//Number of sprites to draw
	spritesheet_t * windows_ss;	//The spritesheet for the windows assets
} MinesweeperOS_t;

//Figure out what I need to do here if anything
typedef struct MinesweeperLang{
	;
} MinesweeperLang_t;

#endif
