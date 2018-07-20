#ifndef EXTRA_STRUCTS_H
#define EXTRA_STRUCTS_H

#include "../../Crayon/code/crayon/dreamcast/texture_structs.h"

//This file exist just to make the main file a little bit more clean

//As far as I know, the only thing that I'd need to watch out for with different OSes is the fact that the task bar is skinnier
typedef struct minesweeper_OS{
	uint16_t taskBarStart;
} minesweeper_OS;

#endif
