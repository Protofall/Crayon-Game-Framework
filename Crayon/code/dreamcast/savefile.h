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

//The struct that contains all of a save file info. This is useful for passing
//by reference into a function and if you want to modify the save file data easily
//or even use different save files for one game
typedef struct savefile_details_t{
	unsigned char * savefile_icon;		//uint8_t
	unsigned short * savefile_palette;	//uint16_t

	uint8_t * savefile_data;
	size_t savefile_size;
	uint8_t anim_count;		//Decided to not go with a uint16_t (Largest VMU supports) because
							//when would you ever want more than 255 frames of animation here?
	uint16_t anim_speed;

	char desc_long[32];
	char desc_short[16];
	char app_id[16];
	char save_name[26];		//Name is 32 chars long max I think and its prefixed with "/vmu/XX/"

	int8_t port;
	int8_t slot;

	uint8_t valid_vmus;			//VMUs with enough space for a save file
	uint8_t valid_vmu_screens;	//All VMU screens
	uint8_t valid_saves;		//All VMUs with a savefile
} savefile_details_t;


//-------------------------Internal use------------------------------


uint8_t crayon_savefile_check_for_save(savefile_details_t * savefile_details, int8_t port, int8_t slot);	//0 if save DNE. 1 if it does
uint8_t crayon_savefile_check_for_device(int8_t port, int8_t slot, uint32_t function);	//1 if device is valid
uint16_t crayon_savefile_get_save_block_count(savefile_details_t * savefile_details);	//Returns the number of blocks your save file will need (Uncompressed)


//-------------------------Both internal and external----------------


uint8_t crayon_savefile_get_vmu_bitmap(uint8_t vmu_bitmap, int8_t port, int8_t slot);	//Returns boolean
void crayon_savefile_set_vmu_bitmap(uint8_t * vmu_bitmap, int8_t port, int8_t slot);	//Updates vmu_bitmap


//-------------------------Called externally-------------------------


void crayon_savefile_load_icon(savefile_details_t * savefile_details, char * image, char * palette);
void crayon_savefile_free_icon(savefile_details_t * savefile_details);

//Set some defaults easily. Call this first
void crayon_savefile_init_savefile_details(savefile_details_t * savefile_details,  uint8_t * savefile_data, size_t savefile_size,
	uint8_t anim_count, uint16_t anim_speed, char * desc_long, char * desc_short, char * app_id, char * save_name);

//Returns an 8 bit var for each VMU (a1a2b1b2c1c2d1d2)
uint8_t crayon_savefile_get_valid_vmus(savefile_details_t * savefile_details);
uint8_t crayon_savefile_get_valid_saves(savefile_details_t * savefile_details);
uint8_t crayon_savefile_get_valid_screens();

uint8_t crayon_savefile_load_uncompressed_save(savefile_details_t * savefile_details);
int crayon_savefile_save_uncompressed_save(savefile_details_t * savefile_details);

//Add a savefile deletion function using this KOS function probably 
// int vmufs_delete(maple_device_t * dev, const char * fn)

#endif
