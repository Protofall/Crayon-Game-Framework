#ifndef MEMORY_H
#define MEMORY_H

#include "texture_structs.h"  //For the spritehseet and animation structs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdint.h> //Not sure if this is needed since texture_struct.h includes it
//#include <dc/pvr.h> //Not sure if this is needed since texture_struct.h includes it
#include <kos/fs_romdisk.h> //For romdisk swapping
#include <zlib/zlib.h>  //Not too sure if I need this for the gz romdisk...

//Load a sprite from a path to the texture. The path is used to generate a dtex and dtex.pal paths
extern int memory_load_dtex(struct spritesheet *ss, char *path);

//Loads the "crayon_packer_sheet" field directory content
extern int memory_load_crayon_packer_sheet(struct spritesheet *ss, char *path);

//If given a valid path it will insert the colours and amount of them into the first two arguments
extern int memory_load_palette(uint32_t **palette, uint16_t *colourCount, char *path);

//Free any resources used by a sprite
extern int memory_spritesheet_free(struct spritesheet *ss);

//Mount a romdisk
extern int mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk
extern int mount_romdisk_gz(char *filename, char *mountpoint);

#endif
