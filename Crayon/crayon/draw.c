#include <dc/pvr.h>

extern void draw_paletted_sprite(const struct spritesheet *ss,
  float x, float y, uint8_t palette_number){

  const float x0 = x;
  const float y0 = y;
  const float x1 = x + ss->width;
  const float y1 = y + ss->height;
  const float z = 1;

  pvr_sprite_cxt_t context;
  if(ss->spritesheet_format == 3){  //PAL4BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(palette_number),
    ss->spritesheet_width, ss->spritesheet_height, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else if(ss->spritesheet_format == 4){
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(palette_number),
    ss->spritesheet_width, ss->spritesheet_height, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else{
    error_freeze("%d isn't a paletted format\n", ss->spritesheet_format);
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
}

extern void draw_non_paletted_sprite(const struct spritesheet *ss, float x, float y){

  const float x0 = x;
  const float y0 = y;
  const float x1 = x + ss->width;
  const float y1 = y + ss->height;
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
}