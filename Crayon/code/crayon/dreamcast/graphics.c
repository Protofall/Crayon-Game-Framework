#include "graphics.h"

extern uint8_t old_graphics_draw_paletted_sprite(const struct spritesheet *ss,
  float x, float y, uint8_t palette_number){

  const float x0 = x;
  const float y0 = y;
  const float x1 = x + ss->spritesheet_width;
  const float y1 = y + ss->spritesheet_height;
  const float z = 1;

  pvr_sprite_cxt_t context;
  if(ss->spritesheet_format == 3){  //PAL4BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(palette_number),
    ss->spritesheet_width, ss->spritesheet_height, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else if(ss->spritesheet_format == 4){ //PAL8BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(palette_number),
    ss->spritesheet_width, ss->spritesheet_height, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else{
    return 1;
    //error_freeze("%d isn't a paletted format\n", ss->spritesheet_format); //Probs best to do a return here and let the main or something else call error_freeze
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
  const float x1 = x + ss->spritesheet_width;
  const float y1 = y + ss->spritesheet_height;
  const float z = 1;

  pvr_sprite_cxt_t context;
  pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (ss->spritesheet_format) << 27,
    ss->spritesheet_width, ss->spritesheet_height, ss->spritesheet_texture, PVR_FILTER_NONE);

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
    //error_freeze("Wrong palette format! It was set to %d\n", ss->spritesheet_format);
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