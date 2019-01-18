#ifndef TEXTURE_STRUCTS_CRAYON_H
#define TEXTURE_STRUCTS_CRAYON_H

#include <dc/pvr.h>
#include <stdint.h> //For the uintX_t types

//The palette has its own struct since users might want to share one palette across multiple textures
typedef struct crayon_palette{
	uint32_t *palette;		//Pointer to heap allocated palette (Its treated like an array of size "colour_count")
	uint16_t colour_count;	//Number of colours in the palette
	int8_t bpp;				//Bits per pixel. Either 4 or 8 modes for DC
	int8_t palette_id;		//Used for graphics_setup_palette. If its -1 then its unset
} crayon_palette_t;

typedef struct crayon_animation{
	char *name;
	uint16_t x;	//Since the animations are designed to be in tiles
	uint16_t y;	//We can select an frame based off of the first frame's coords
	uint16_t sheet_width;	//Width of the animation sheet
	uint16_t sheet_height;
	uint16_t frame_width;	//Width of each frame
	uint16_t frame_height;
	uint8_t frame_count;	//How many sprites make up the animation (Dunno if this should be a short or int yet)
	//With these widths and heights, it might be possible to deduce some of them from other info, but idk
} crayon_animation_t;

//For pre-processing. An anim file is in the format "width, height, frame count"
typedef struct crayon_spritesheet{
	pvr_ptr_t texture;
	char *name;	//UNUSED. Might be useful for when it comes time to un-mount a romdisk, otherwise I don't think its needed
	uint16_t dimensions;	//Since a pvr texture must be a square, we don't need height/width
	uint32_t texture_format;	//The raw dtex type value

	crayon_animation_t *animation_array;	//Allows me to make an array of animation_t pointers
	uint8_t animation_count;	//The number of animations per spritesheet
} crayon_spritesheet_t;

/*
On fontsheets. FOR NOW EACH CHAR IN THE FONTSHEET CONTAINS BLANK LINES OF PIXELS SEPERATING STUFF. EG (- is blank pixel row/column)

A-B-C-
------
D-E-F-
------

Each char has a varying width, but all have the same height
Info file format:

char_height
num_rows num_chars_row_1 num_chars_row_2 (etc)
1st_char_width 2nd_char_width (etc)
*/

typedef struct crayon_font_prop{
	pvr_ptr_t texture;
	uint16_t dimensions;
	uint32_t texture_format;		//The raw dtex type value
	uint8_t *char_width;	//Num elements equal to num_chars
	uint8_t char_height;
	uint8_t *chars_per_row;	//Total of array is num_rows
	uint8_t *char_x_coord;	//If you want to use fontsheets with dimensions > 256
							//then bump this up to a uint16_t and modify the load
							//code in memory_free_prop_font_sheet()
	uint8_t num_rows;
	uint16_t num_chars;
} crayon_font_prop_t;

//All chars have the same width and height
//Info file format: "char_width char_height num_columns num_rows"
typedef struct crayon_font_mono{
	pvr_ptr_t texture;
	uint16_t dimensions;
	uint32_t texture_format;		//The raw dtex type value
	uint8_t char_width;
	uint8_t char_height;

	uint8_t num_columns;
	uint8_t num_rows;
	uint16_t num_chars;	//Equal to num_columns * num_rows since without more infor, we can't guess
} crayon_font_mono_t;

#endif
