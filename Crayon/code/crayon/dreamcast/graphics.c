#include "graphics.h"

extern uint8_t old_graphics_draw_paletted_sprite(const struct spritesheet *ss,
  float x, float y, uint8_t palette_number){

  const float x0 = x;
  const float y0 = y;
  const float x1 = x + ss->spritesheet_dims;
  const float y1 = y + ss->spritesheet_dims;
  const float z = 1;

  pvr_sprite_cxt_t context;
  if(ss->spritesheet_format == 3){  //PAL4BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(palette_number),
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else if(ss->spritesheet_format == 4){ //PAL8BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(palette_number),
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else{
    return 1;
  }

  pvr_sprite_hdr_t header;
  pvr_sprite_compile(&header, &context);
  pvr_prim(&header, sizeof(header));

  pvr_sprite_txr_t vert = {
    .flags = PVR_CMD_VERTEX_EOL,
    .ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(0.0, 0.0),
    .bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(1.0, 0.0),
    .cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(1.0, 1.0),
    .dx = x0, .dy = y1
  };
  pvr_prim(&vert, sizeof(vert));

  return 0;
}

extern uint8_t old_graphics_draw_non_paletted_sprite(const struct spritesheet *ss, float x, float y){
  const float x0 = x;
  const float y0 = y;
  const float x1 = x + ss->spritesheet_dims;
  const float y1 = y + ss->spritesheet_dims;
  const float z = 1;

  pvr_sprite_cxt_t context;
  pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (ss->spritesheet_format) << 27,
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);

  pvr_sprite_hdr_t header;
  pvr_sprite_compile(&header, &context);
  pvr_prim(&header, sizeof(header));

  pvr_sprite_txr_t vert = {
    .flags = PVR_CMD_VERTEX_EOL,
    .ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(0.0, 0.0),
    .bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(1.0, 0.0),
    .cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(1.0, 1.0),
    .dx = x0, .dy = y1
  };
  pvr_prim(&vert, sizeof(vert));

  return 0;
}

//There are 4 palettes for 8BPP and 64 palettes for 4BPP. palette_number is the id
extern int graphics_setup_palette(uint8_t palette_number, const struct spritesheet *ss){
  int entries;
  if(ss->spritesheet_format == 3){
    entries = 16;
  }
  else if(ss->spritesheet_format == 4){
    entries = 256;
  }
  else{
    return 1;
  }

  pvr_set_pal_format(PVR_PAL_ARGB8888);
  uint16_t i; //Can't this be a uint8_t instead? 0 to 255 and max 256 entries per palette
  //...but then again how would the loop be able to break? since it would overflow back to 0
  for(i = 0; i < ss->spritesheet_color_count; ++i){
    pvr_set_pal_entry(i + entries * palette_number, ss->spritesheet_palette[i]);
  }
  return 0;
}

extern void graphics_frame_coordinates(const struct animation *anim, uint16_t *frame_x, uint16_t *frame_y, uint8_t frame){
  int framesPerRow = anim->animation_sheet_width/anim->animation_frame_width;
  int colNum = frame%framesPerRow; //Gets the column (Zero indexed)
  int rowNum = frame/framesPerRow;  //Gets the row (Zero indexed)

  *frame_x = anim->animation_x + (colNum) * anim->animation_frame_width;
  *frame_y = anim->animation_y + (rowNum) * anim->animation_frame_height;

  return;
}

extern uint8_t graphics_draw_paletted_sprite(const struct spritesheet *ss,
  const struct animation *anim, float draw_x, float draw_y, float draw_z, uint8_t paletteNumber,
  uint16_t frame_x, uint16_t frame_y){

  const float x0 = draw_x;
  const float y0 = draw_y;
  const float x1 = draw_x + anim->animation_frame_width;
  const float y1 = draw_y + anim->animation_frame_height;
  const float z = draw_z;

  const float u0 = frame_x / (float)ss->spritesheet_dims;
  const float v0 = frame_y / (float)ss->spritesheet_dims;
  const float u1 = (frame_x + anim->animation_frame_width) / (float)ss->spritesheet_dims;
  const float v1 = (frame_y + anim->animation_frame_height) / (float)ss->spritesheet_dims;

  pvr_sprite_cxt_t context;
  if(ss->spritesheet_format == 3){  //PAL4BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(paletteNumber),
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else if(ss->spritesheet_format == 4){ //PAL8BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(paletteNumber),
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else{
    return 1;
  }

  pvr_sprite_hdr_t header;
  pvr_sprite_compile(&header, &context);
  pvr_prim(&header, sizeof(header));

  pvr_sprite_txr_t vert = {
    .flags = PVR_CMD_VERTEX_EOL,
    .ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(u0, v0),
    .bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(u1, v0),
    .cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(u1, v1),
    .dx = x0, .dy = y1
  };
  pvr_prim(&vert, sizeof(vert));

  return 0;
}