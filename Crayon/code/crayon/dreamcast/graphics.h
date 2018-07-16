#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehsheet and anim structs
#include "render_structs.h"  //For the crayon_sprite_array struct

//Sets a palette for a spritesheet
extern int graphics_setup_palette(uint8_t palette_number, const struct spritesheet *ss);

//Give an animation struct, frame, coordinates and frame and it will populate the coordinates
extern void graphics_frame_coordinates(const struct animation *anim, uint16_t *frame_x,
	uint16_t *frame_y, uint8_t frame);

//Queue a colour/alpha poly to be rendered (Incomplete)
extern void graphics_draw_colour_poly(pvr_ptr_t name, uint16_t draw_x, uint16_t draw_y,
	uint16_t draw_z, uint16_t dim_x, uint16_t dim_y, uint32_t colour);

//Queue a texture to be rendered (If RGB565 or ARGB4444 then paletteNumber is never read)
extern uint8_t graphics_draw_sprite(const struct spritesheet *ss,
  const struct animation *anim, float draw_x, float draw_y, float draw_z,
  float scale_x, float scale_y, uint16_t frame_x, uint16_t frame_y,
  uint8_t paletteNumber);

//Testing drawing multiple of same thing (WIP) CHANGE fd_size TO "BOOLEAN" (0/1)
extern uint8_t graphics_draw_sprites(const struct spritesheet *ss,
  const struct animation *anim, uint16_t *draw_coords, uint16_t *frame_data, uint16_t fd_size,
  uint16_t num_sprites, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber);

//Testing the new render struct (Delete other sprite draws after)
extern uint8_t graphics_draw_sprites_NEW(crayon_sprite_array_t *sprite_array);

#endif
