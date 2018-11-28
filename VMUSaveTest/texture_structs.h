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

/*
On fontsheets. FOR NOW EACH CHAR IN THE FONTSHEET CONTAINS BLANK LINES OF PIXELS SEPERATING STUFF. EG (- is blank pixel row/column)

A-B-C-
------
D-E-F-
------

*/

//All chars have the same width and height
//Info file format: "char_width char_height num_columns num_rows"
typedef struct crayon_font_mono{
	pvr_ptr_t fontsheet_texture;
	uint16_t fontsheet_dim;
	int texture_format;		//The raw dtex type value
	uint8_t char_width;
	uint8_t char_height;
	uint8_t num_columns;
	uint8_t num_rows;
} crayon_font_mono_t;

#endif
