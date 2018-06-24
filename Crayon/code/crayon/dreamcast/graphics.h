#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and anim structs

//#include <dc/pvr.h> //Not sure if this is needed since texture_struct.h includes it

//Sets a palette for a spritesheet
extern int graphics_setup_palette(uint8_t palette_number, const struct spritesheet *ss);

//Give an animation struct, frame and coordinates and it will generate the coordinates
extern void graphics_frame_coordinates(const struct animation *anim, uint16_t *frame_x,
	uint16_t *frame_y, uint8_t frame);

//Que a texture to be rendered (If RGB565 or ARGB4444 then paletteNumber is never read)
extern uint8_t graphics_draw_sprite(const struct spritesheet *ss,
  const struct animation *anim, float draw_x, float draw_y, float draw_z,
  float scale_x, float scale_y, uint16_t frame_x, uint16_t frame_y,
  uint8_t paletteNumber);

//Testing drawing multiple of same thing (WIP)
extern uint8_t graphics_draw_sprites(const struct spritesheet *ss,
  const struct animation *anim, uint16_t *draw_coords, uint16_t *frame_data, uint16_t fd_size,
  uint16_t num_sprites, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber);

#endif
