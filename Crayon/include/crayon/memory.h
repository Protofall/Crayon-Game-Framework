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
extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, uint16_t *dims, uint32_t *format, char *texture_path);

//Loads a "crayon_spritesheet" spritesheet into memory
extern uint8_t crayon_memory_load_spritesheet(crayon_spritesheet_t *ss, crayon_palette_t *cp, int8_t palette_id, char *path);

//Loads a proportionally-spaced fontsheet into memory
extern uint8_t crayon_memory_load_prop_font_sheet(crayon_font_prop_t *fp, crayon_palette_t *cp, int8_t palette_id, char *path);

//Loads a mono-spaced fontsheet into memory
extern uint8_t crayon_memory_load_mono_font_sheet(crayon_font_mono_t *fm, crayon_palette_t *cp, int8_t palette_id, char *path);

//If given a valid path and a crayon_palette object, it will populate the palette object with the correct data
extern uint8_t crayon_memory_load_palette(crayon_palette_t *cp, int8_t bpp, char *path);

//This will make a new palette struct thats a copy of another one.
extern void crayon_memory_clone_palette(crayon_palette_t *original, crayon_palette_t *copy, int8_t palette_id);

//Set initial array sizes and options for your sprite_array
extern void crayon_memory_init_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t num_sprites,
	uint8_t unique_frames, uint8_t multi_draw_z, uint8_t multi_frames, uint8_t multi_scales, uint8_t multi_flips,
	uint8_t multi_rotations, uint8_t multi_colours, uint8_t filter, crayon_spritesheet_t *ss, crayon_animation_t *anim,
	crayon_palette_t *pal);

//UNTESTED (MS manually does the same things this function does)
extern void crayon_memory_init_untextered_array(crayon_untextured_array_t *poly_array, uint16_t num_polys, uint8_t multi_draw_z,
	uint8_t multi_colours, uint8_t multi_draw_dims, uint8_t multi_rotations);


//------------------Modifying memory----------------//


//This function attempts to search for colour1 in the palette and swaps it for colour2
//If _continue is false then it only replaces the first instance of colour1, else it replaces all instances
//It returns 0 if colour1 wasn't found, otherwise it returns the number of "colour1"s that were swapped
extern uint16_t crayon_memory_swap_colour(crayon_palette_t *cp, uint32_t colour1, uint32_t colour2, uint8_t _continue);


//------------------Freeing memory------------------//


//Free up all memory from a spritesheet struct. if free_palette is true, it will also free the palette
extern uint8_t crayon_memory_free_spritesheet(crayon_spritesheet_t *ss);

//Same as above, but for mono-spaced fontsheets
extern uint8_t crayon_memory_free_prop_font_sheet(crayon_font_prop_t *fp);

//Same as above, but for proportionally-spaced fontsheets
extern uint8_t crayon_memory_free_mono_font_sheet(crayon_font_mono_t *fm);

//Frees a palette
extern uint8_t crayon_memory_free_palette(crayon_palette_t *cp);

//Frees a sprite array
extern uint8_t crayon_memory_free_sprite_array(crayon_sprite_array_t *sprite_array, uint8_t free_ss, uint8_t free_pal);

//Free an untextured poly array
extern uint8_t crayon_memory_free_untextured_array(crayon_untextured_array_t *untextured_array);


//------------------Mounting romdisks------------------//


//Mount a regular img romdisk
extern uint8_t crayon_memory_mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk (Apparently its kinda dodgy)
extern uint8_t crayon_memory_mount_romdisk_gz(char *filename, char *mountpoint);

#endif
