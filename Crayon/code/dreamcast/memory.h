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

//If given a valid path and a crayon_palette object, it will populate the palette object with the correct data
extern int memory_load_palette(crayon_palette_t *cp, char *path);

//Loads the "crayon_packer_sheet" field directory content
extern int memory_load_crayon_packer_sheet(struct crayon_spritesheet *ss, char *path);

//Free up all memory from a spritesheet struct. if free_palette is true, it will also free the palette
extern int memory_free_crayon_packer_sheet(struct crayon_spritesheet *ss, uint8_t free_palette);

//Mount a romdisk
extern int memory_mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk
extern int memory_mount_romdisk_gz(char *filename, char *mountpoint);

#endif
