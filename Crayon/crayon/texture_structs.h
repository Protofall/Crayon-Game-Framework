#ifndef TEXTURE_STRUCTS_H
#define TEXTURE_STRUCTS_H

#include <dc/pvr.h>
#include <stdint.h> //For the uintX_t types

//Spritesheet stuff
//-----------------------------------------------------------------------------

typedef struct anim{
  char *anim_name;
  uint8_t anim_frames;	//How many sprites make up the animation (Dunno if this should be a short or int yet)
  uint16_t anim_top_left_x_coord;	//Since the anims are designed to be in tiles
  uint16_t anim_top_left_y_coord;	//We can select an frame based off of the first frame's coords
  uint16_t anim_sheet_width;	//Width of the anim sheet
  uint16_t anim_sheet_height;
  uint16_t anim_frame_width;	//Width of each frame
  uint16_t anim_frame_height;
  //With these widths and heights, it might be possible to deduce some of them from other info, but idk
} anim_t;

typedef struct spritesheet{
  pvr_ptr_t *spritesheet_texture;
  char *spritesheet_name;	//Might be useful for when it comes time to un-mount a romdisk?
  anim_t **spritesheet_anim_array; //Allows me to make an array of anim_t pointers hopefully
  //uint16_t spritesheet_dims;	//Since a pvr texture must be a square, we don't need height/width
  uint16_t spritesheet_width;	//Doubling up just incase I'm wrong
  uint16_t spritesheet_height;
  uint8_t spritesheet_format; //1 for 4BPP, 2 for 8BPP, 3 for RGB565 and 4 for ARGB4444 (0 for unknown)
  uint32_t *spritesheet_palette; //Pointer to heap allocated palette
  uint16_t spritesheet_color_count; //Number of colours in the palette
} spritesheet_t;

#endif

//If you want to get the name of the i-th anim object you'd used spritesheet->spritesheet_anim_array[i-1]->anim_name
