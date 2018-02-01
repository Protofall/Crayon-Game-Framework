#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <dc/pvr.h>

//Contains a lot of (modified) code from this tutorial:
//  http://dcemulation.org/?title=PVR_Spritesheets

//Sprite stuff
//-----------------------------------------------------------------------------

typedef struct sprite {
  uint16_t    width; //Texture width in pixels
  uint16_t   height; //Texture height in pixels
  uint8_t    format; //Format (see https://github.com/tvspelsfreak/texconv)
  pvr_ptr_t texture; //Pointer to texture in video memory
  uint32_t *palette; //Pointer to heap allocated palette
  uint16_t color_count; //Number of colours in the palette
} sprite_t;

//Load a sprite from a path to the texture. The path is used to generate a dtex and dtex.pal paths
extern int sprite_load(struct sprite *sprite, const char *path);

//Free any resources used by a sprite
extern void sprite_free(struct sprite *sprite);

//Draw a sprite with the PVR
extern void draw_sprite(const struct sprite *sheet,
  float x, float y, uint8_t palette_number, uint8_t bpp_mode);

#endif
