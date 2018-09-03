#ifndef MEMORY_CRAYON_H
#define MEMORY_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and animation structs
#include "render_structs.h"  //For the crayon_sprite_array struct

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kos/fs_romdisk.h> //For romdisk swapping
#include <zlib/zlib.h>
extern int zlib_getlength(char *filename);	//Because zlib.h won't declare it for us

//Load a sprite from a path to the texture. The path is used to generate a dtex and dtex.pal paths
extern int memory_load_dtex(struct spritesheet *ss, char *path);

//Loads the "crayon_packer_sheet" field directory content
extern int memory_load_crayon_packer_sheet(struct spritesheet *ss, char *path);

//If given a valid path it will insert the colours and amount of them into the first two arguments
extern int memory_load_palette(uint32_t **palette, uint16_t *colourCount, char *path);

//Free up all memory from a spritesheet struct
extern int memory_free_crayon_packer_sheet(struct spritesheet *ss);

//Mount a romdisk
extern int memory_mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk
extern int memory_mount_romdisk_gz(char *filename, char *mountpoint);

#endif
