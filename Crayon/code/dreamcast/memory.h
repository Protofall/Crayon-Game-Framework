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
extern uint8_t memory_load_palette(crayon_palette_t *cp, char *path);

//------------------Allocating memory------------------//

//Loads a "crayon_packer_sheet" spritesheet into memory
extern uint8_t memory_load_crayon_packer_sheet(struct crayon_spritesheet *ss, char *path);

//Loads a proportionally-spaced fontsheet into memory
extern uint8_t memory_load_prop_font_sheet(struct crayon_font_prop *fp, char *path);	//(UNFINISHED)

//Loads a mono-spaced fontsheet into memory
extern uint8_t memory_load_mono_font_sheet(struct crayon_font_mono *fm, char *path);

//------------------Freeing memory------------------//

//Free up all memory from a spritesheet struct. if free_palette is true, it will also free the palette
extern uint8_t memory_free_crayon_packer_sheet(struct crayon_spritesheet *ss, uint8_t free_palette);

//Same as above, but for mono-spaced fontsheets
extern uint8_t memory_free_prop_font_sheet(struct crayon_font_prop *fp, uint8_t free_palette);	//(UNFINISHED)

//Same as above, but for proportionally-spaced fontsheets
extern uint8_t memory_free_mono_font_sheet(struct crayon_font_mono *fm, uint8_t free_palette);

//------------------Mounting romdisks------------------//

//Mount a regular img romdisk
extern uint8_t memory_mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk
extern uint8_t memory_mount_romdisk_gz(char *filename, char *mountpoint);

#endif
