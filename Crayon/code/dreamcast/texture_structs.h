#ifndef TEXTURE_STRUCTS_CRAYON_H
#define TEXTURE_STRUCTS_CRAYON_H

#include <dc/pvr.h>
#include <stdint.h> //For the uintX_t types

//Currently unused. The fonts and ss will need to transition to use this
//The reason it is it's own struct is so multiple textures can easily share the same palette
	//Note this may become an issue when deleting a font/ss that shares a palette. Maybe have a "delete palette" toggle
typedef struct crayon_palette{
	uint32_t *palette;		//Pointer to heap allocated palette (Its treated like an array of size "colour_count")
	uint16_t colour_count;	//Number of colours in the palette
} crayon_palette_t;

typedef struct crayon_animation{
	char *animation_name;	//Fix this later so its not hard coded and instead is a dynamic pointer
	uint16_t animation_x;	//Since the animations are designed to be in tiles
	uint16_t animation_y;	//We can select an frame based off of the first frame's coords
	uint16_t animation_sheet_width;	//Width of the animation sheet
	uint16_t animation_sheet_height;
	uint16_t animation_frame_width;	//Width of each frame
	uint16_t animation_frame_height;
	uint8_t animation_frames;	//How many sprites make up the animation (Dunno if this should be a short or int yet)
	//With these widths and heights, it might be possible to deduce some of them from other info, but idk
} crayon_animation_t;

typedef struct crayon_spritesheet{
	pvr_ptr_t *spritesheet_texture;
	char *spritesheet_name;	//Might be useful for when it comes time to un-mount a romdisk, otherwise I don't think its needed
	uint16_t spritesheet_dims;	//Since a pvr texture must be a square, we don't need height/width
	uint8_t spritesheet_format;	//1 for 4BPP, 2 for 8BPP, 3 for RGB565 and 4 for ARGB4444 (0 for unknown)
	crayon_palette_t *palette_data;	//Right now we still malloc this regardless if we use it or not. Change that

	crayon_animation_t *spritesheet_animation_array;	//Allows me to make an array of animation_t pointers
	uint8_t spritesheet_animation_count;	//The number of animations per spritesheet
} crayon_spritesheet_t;

//On fontshetes. FOR NOW EACH CHAR IN THE FONTSHEET CONTAINS BLANK PIXELS SEPERATING STUFF. EG (- is blank pixel row/column)

/*
A-B-C
-----
D-E-F
-----
G-H-I
*/

//I might change it to compact it during pre-processing and hence be more efficient data wise (Only slightly), but for now its fine

//Each char has a varying width, but all have the same height
//Info file format:
/*
texture_dim char_height
num_rows num_chars_row_1 num_chars_row_2 (etc)
1st_char_width 2nd_char_width (etc)
*/
typedef struct crayon_font_prop{
	pvr_ptr_t *font_texture;
	uint16_t texture_dim;
	uint8_t texture_format;	//Contains the texel format
	uint8_t *char_width;	//Num elements equal to total of "chars_per_row" array
	uint8_t char_height;
	uint8_t *chars_per_row;	//Total of array is total of num_chars
	uint8_t chars_per_row_size;
	uint8_t num_rows;
	crayon_palette_t *palette_data;
} crayon_font_prop_t;

//All chars have the same width and height
//Info file format:
/*
texture_dim char_width char_height num_columns num_rows
*/
typedef struct crayon_font_mono{
	pvr_ptr_t *font_texture;
	uint16_t texture_dim;
	uint8_t texture_format;	//Contains the texel format
	uint8_t char_width;
	uint8_t char_height;
	uint8_t num_columns;
	uint8_t num_rows;
	crayon_palette_t *palette_data;
} crayon_font_mono_t;

#endif
