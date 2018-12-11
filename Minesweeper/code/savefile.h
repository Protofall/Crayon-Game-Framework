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
	MinesweeperSaveFile_t save_file;

	unsigned char * save_file_icon;		//uint8_t
	unsigned short * save_file_palette;	//uint16_t

	char long_description[36];
	char short_description[20];
	char save_name[26];		//Name is 32 chars long max I think and its prefixed with "/vmu/XX/"
	char save_id[20];

	int8_t port;
	int8_t slot;

	uint8_t valid_vmus;			//VMUs with enough space for a save file
	uint8_t valid_vmu_screens;

	size_t save_file_size;

} SaveFileDetails_t;

void vmu_load_icon(SaveFileDetails_t * save);
void vmu_free_icon(SaveFileDetails_t * save);

uint8_t vmu_get_bit(uint8_t valid_vmus, int8_t port, int8_t slot);	//returns boolean
void vmu_set_bit(uint8_t * valid_vmus, int8_t port, int8_t slot);
uint8_t vmu_check_for_savefile(SaveFileDetails_t * save, int8_t port, int8_t slot);	//0 if save DNE. 1 if it does
uint16_t vmu_get_save_block_count(size_t savefile_size);

uint8_t vmu_valid(SaveFileDetails_t * save);
uint8_t vmu_valid_screens();

uint8_t vmu_get_savefiles(SaveFileDetails_t * save);	//Returns an 8 bit var for each VMU

int vmu_save_uncompressed(SaveFileDetails_t * save);
uint8_t vmu_load_uncompressed(SaveFileDetails_t * save);

#endif
