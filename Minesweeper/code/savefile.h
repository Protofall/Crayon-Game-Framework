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
	char record_names[6][16];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint8_t pref_mines;
} MinesweeperSaveFile_t;

//The struct that contains all of a save file info. This is useful for passing
//by reference into a function and if you want to modify the save file data easily
//or even use different save files for one game
typedef struct SaveFileDetails{
	MinesweeperSaveFile_t save_file;	//Currently not malloc-ed

	unsigned char * save_file_icon;		//uint8_t
	unsigned short * save_file_palette;	//uint16_t

	char * long_description;
	char * short_description;
	char * save_name;
	char * save_id;

	uint8_t port;
	uint8_t slot;

	uint8_t valid_vmus;			//VMUs with enough space for a save file
	uint8_t valid_vmu_screens;

} SaveFileDetails_t;

int vmu_load_icon(SaveFileDetails_t * save);
int vmu_free_icon(SaveFileDetails_t * save);
int vmu_save_uncompressed(SaveFileDetails_t * save);
int vmu_load_uncompressed(SaveFileDetails_t * save);

#endif
