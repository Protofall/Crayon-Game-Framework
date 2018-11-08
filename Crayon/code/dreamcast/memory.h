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

typedef struct dtex_header{
	uint8_t magic[4]; //magic number "DTEX"
	uint16_t   width; //texture width in pixels
	uint16_t  height; //texture height in pixels
	uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
	uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
	uint8_t     magic[4]; //magic number "DPAL"
	uint32_t color_count; //number of 32-bit ARGB palette entries
} dpal_header_t;

//------------------Allocating memory------------------//

//Reads a file, loads it into a dtex
extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, dtex_header_t *dtex_header, char *texture_path);

//Loads a "crayon_spritesheet" spritesheet into memory
extern uint8_t crayon_memory_load_spritesheet(struct crayon_spritesheet *ss, char *path);

//Loads a proportionally-spaced fontsheet into memory
extern uint8_t crayon_memory_load_prop_font_sheet(struct crayon_font_prop *fp, char *path);

//Loads a mono-spaced fontsheet into memory
extern uint8_t crayon_memory_load_mono_font_sheet(struct crayon_font_mono *fm, char *path);

//If given a valid path and a crayon_palette object, it will populate the palette object with the correct data
extern uint8_t crayon_memory_load_palette(crayon_palette_t *cp, char *path);

//This will make a new palette struct thats a copy of another one.
extern crayon_palette_t * crayon_memory_clone_palette(crayon_palette_t *original);


//------------------Modifying memory----------------//


//This function attempts to search for colour1 in the palette and swapps it for colour2
//If _continue is false then it only replaces the first instance of colour1, else it replaces all instances
//It returns 0 if colour1 wasn't found, otherwise it returns the number of "colour1"s that were swapped
extern uint16_t crayon_memory_swap_colour(crayon_palette_t *cp, uint32_t colour1, uint32_t colour2, uint8_t _continue);


//------------------Freeing memory------------------//


//Free up all memory from a spritesheet struct. if free_palette is true, it will also free the palette
extern uint8_t crayon_memory_free_spritesheet(struct crayon_spritesheet *ss, uint8_t free_palette);

//Same as above, but for mono-spaced fontsheets
extern uint8_t crayon_memory_free_prop_font_sheet(struct crayon_font_prop *fp, uint8_t free_palette);

//Same as above, but for proportionally-spaced fontsheets
extern uint8_t crayon_memory_free_mono_font_sheet(struct crayon_font_mono *fm, uint8_t free_palette);

//Frees a palette
extern uint8_t crayon_memory_free_palette(crayon_palette_t *cp);


//------------------Mounting romdisks------------------//


//Mount a regular img romdisk
extern uint8_t crayon_memory_mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk (Apparently its kinda dodgy)
extern uint8_t crayon_memory_mount_romdisk_gz(char *filename, char *mountpoint);

#endif
