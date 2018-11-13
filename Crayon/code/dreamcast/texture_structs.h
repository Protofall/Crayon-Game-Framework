#ifndef TEXTURE_STRUCTS_CRAYON_H
#define TEXTURE_STRUCTS_CRAYON_H

#include <dc/pvr.h>
#include <stdint.h> //For the uintX_t types

//The palette has its own struct since users might want to share one palette across multiple textures
typedef struct crayon_palette{
	uint32_t *palette;		//Pointer to heap allocated palette (Its treated like an array of size "colour_count")
	uint16_t colour_count;	//Number of colours in the palette
	int8_t bpp;
	int8_t palette_id;		//Used for graphics_setup_palette. If its -1 then its unset
} crayon_palette_t;

typedef struct crayon_animation{
	char *animation_name;
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
	pvr_ptr_t spritesheet_texture;
	char *spritesheet_name;	//Might be useful for when it comes time to un-mount a romdisk, otherwise I don't think its needed
	uint16_t spritesheet_dims;	//Since a pvr texture must be a square, we don't need height/width
	uint8_t spritesheet_format;	//1 for 4BPP, 2 for 8BPP, 3 for RGB565 and 4 for ARGB4444 (0 for unknown)
								//CHANGE TO UINT32 AND JUST SAVE WHOLE FORMAT

	crayon_animation_t *spritesheet_animation_array;	//Allows me to make an array of animation_t pointers
	uint8_t spritesheet_animation_count;	//The number of animations per spritesheet
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
	pvr_ptr_t fontsheet_texture;
	uint16_t fontsheet_dim;
	uint8_t texture_format;	//Contains the texel format
	uint8_t *char_width;	//Num elements equal to num_chars
	uint8_t char_height;
	uint8_t *chars_per_row;	//Total of array is num_rows
	uint8_t *char_x_coord;	//If you want to use fontsheets with fontsheet_dim > 256
							//then bump this up to a uint16_t and modify the load
							//code in memory_free_prop_font_sheet()
	uint8_t num_rows;
	uint8_t num_chars;
} crayon_font_prop_t;

//All chars have the same width and height
//Info file format: "char_width char_height num_columns num_rows"
typedef struct crayon_font_mono{
	pvr_ptr_t fontsheet_texture;
	uint16_t fontsheet_dim;
	uint8_t texture_format;	//Contains the texel format
	uint8_t char_width;
	uint8_t char_height;
	uint8_t num_columns;
	uint8_t num_rows;
} crayon_font_mono_t;

#endif
