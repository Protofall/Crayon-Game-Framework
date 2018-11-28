#ifndef MEMORY_CRAYON_H
#define MEMORY_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and animation structs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <kos/fs_romdisk.h> //For romdisk swapping

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
extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, uint16_t *dims, int *format, char *texture_path);

//Loads a mono-spaced fontsheet into memory
extern uint8_t crayon_memory_load_mono_font_sheet(crayon_font_mono_t *fm, crayon_palette_t *cp, int8_t palette_id, char *path);

//If given a valid path and a crayon_palette object, it will populate the palette object with the correct data
extern uint8_t crayon_memory_load_palette(crayon_palette_t *cp, int8_t bpp, char *path);

//------------------Freeing memory------------------//

//Same as above, but for mono-spaced fontsheets
extern uint8_t crayon_memory_free_mono_font_sheet(struct crayon_font_mono *fm);

//Frees a palette
extern uint8_t crayon_memory_free_palette(crayon_palette_t *cp);

#endif
