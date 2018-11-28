#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehsheet and anim structs

#include <stdlib.h>
#include <math.h>

//Sets a palette for a spritesheet
extern int graphics_setup_palette(const crayon_palette_t *cp);

//------------------Drawing Fonts------------------//

//Draw string using mono font (string must be null-terminated)
extern uint8_t graphics_draw_text_mono(const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber, char * string);

#endif
