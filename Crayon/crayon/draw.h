#ifndef DRAW_H
#define DRAW_H

//Que a paletted texture to be rendered
extern void draw_paletted_sprite(const struct spritesheet *ss,
  float x, float y, uint8_t palette_number);

//Que non-paletted texture to be rendered
extern void draw_non_paletted_sprite(const struct spritesheet *ss,
  float x, float y);

#endif
