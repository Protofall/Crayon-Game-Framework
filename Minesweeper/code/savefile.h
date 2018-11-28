#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> //For the uintX_t types

#include <dc/maple/vmu.h>
#include <dc/vmu_pkg.h>
#include <dc/vmufs.h>
#include <kos/fs.h>

//+ save icon should be 2 blocks long
typedef struct MinesweeperSaveFile{
	// uint8_t BS_Mode;	//Bulletsweeper mode. 0 for never won, 1 for Beat with 1 player, 2 for beat with 2 players, etc.
	uint8_t options;	//XXXX LOSQ (Language, OS, Sound, Questions)
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[6][11];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint8_t pref_mines;
} MinesweeperSaveFile_t;

unsigned char * save_file_icon;		//uint8_t
unsigned short * save_file_palette;	//uint16_t

int save_uncompressed(uint8_t port, uint8_t unit, MinesweeperSaveFile_t * save);
int load_uncompressed(uint8_t port, uint8_t unit, MinesweeperSaveFile_t * save);

#endif
