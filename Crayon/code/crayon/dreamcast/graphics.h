#ifndef DRAW_H
#define DRAW_H

#include "texture_structs.h"  //For the spritehseet and anim structs

//#include <dc/pvr.h> //Not sure if this is needed since texture_struct.h includes it

//Que a paletted texture to be rendered (OLD)
extern uint8_t old_modded_graphics_draw_sprite(const struct spritesheet *ss,
  float x, float y, uint8_t palette_number);

//Sets a palette for a spritesheet
extern int graphics_setup_palette(uint8_t palette_number, const struct spritesheet *ss);

//Give an animation struct, frame and coordinates and it will generate the coordinates
extern void graphics_frame_coordinates(const struct animation *anim, uint16_t *frame_x,
	uint16_t *frame_y, uint8_t frame);

//Que a texture to be rendered (If RGB565 or ARGB4444 then paletteNumber is never read)
extern uint8_t graphics_draw_sprite(const struct spritesheet *ss,
  const struct animation *anim, float draw_x, float draw_y, float draw_z,
  uint8_t paletteNumber, uint16_t frame_x, uint16_t frame_y);

#endif
