#ifndef DRAW_H
#define DRAW_H

#include "texture_structs.h"  //For the spritehseet and anim structs

//#include <dc/pvr.h> //Not sure if this is needed since texture_struct.h includes it

//Que a paletted texture to be rendered
extern uint8_t render_paletted_sprite(const struct spritesheet *ss,
  float x, float y, uint8_t palette_number);

//Que non-paletted texture to be rendered
extern uint8_t render_non_paletted_sprite(const struct spritesheet *ss,
  float x, float y);

//Sets a palette for a spritesheet
extern int render_setup_palette(uint8_t palette_number, const struct spritesheet *ss);

#endif
