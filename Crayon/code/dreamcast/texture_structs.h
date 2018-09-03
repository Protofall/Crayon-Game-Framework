#ifndef TEXTURE_STRUCTS_CRAYON_H
#define TEXTURE_STRUCTS_CRAYON_H

#include <dc/pvr.h>
#include <stdint.h> //For the uintX_t types

typedef struct animation{
  char *animation_name; //Fix this later so its not hard coded and instead is a dynamic pointer
  uint16_t animation_x;	//Since the animations are designed to be in tiles
  uint16_t animation_y;	//We can select an frame based off of the first frame's coords
  uint16_t animation_sheet_width;	//Width of the animation sheet
  uint16_t animation_sheet_height;
  uint16_t animation_frame_width;	//Width of each frame
  uint16_t animation_frame_height;
  uint8_t animation_frames;  //How many sprites make up the animation (Dunno if this should be a short or int yet)
  //With these widths and heights, it might be possible to deduce some of them from other info, but idk
} animation_t;

typedef struct spritesheet{
  pvr_ptr_t *spritesheet_texture;
  char *spritesheet_name;	//Might be useful for when it comes time to un-mount a romdisk, otherwise I don't think its needed
  uint16_t spritesheet_dims;	//Since a pvr texture must be a square, we don't need height/width
  uint8_t spritesheet_format; //1 for 4BPP, 2 for 8BPP, 3 for RGB565 and 4 for ARGB4444 (0 for unknown)
  uint32_t *spritesheet_palette; //Pointer to heap allocated palette (Its treated like an array of size spritesheet_colour_count)
  uint16_t spritesheet_color_count; //Number of colours in the palette

  animation_t *spritesheet_animation_array; //Allows me to make an array of animation_t pointers
  uint8_t spritesheet_animation_count; //The number of animations per spritesheet
} spritesheet_t;

#endif
