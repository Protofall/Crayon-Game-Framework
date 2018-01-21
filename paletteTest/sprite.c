#include "sprite.h"

// Contains a lot of (modified) code from this tutorial:
//   http://dcemulation.org/?title=PVR_Spritesheets

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dc/pvr.h>

//These header types are kinda redundant
typedef struct dtex_header{
  uint8_t magic[4]; //magic number "DTEX"
  uint16_t   width; //texture width in pixels
  uint16_t  height; //texture height in pixels
  uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
  uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
  uint8_t     magic[4]; //magic number "DPAL"
  uint32_t color_count; //number of 24-bit ARGB palette entries
} dpal_header_t;

extern int sprite_load(struct sprite *sprite,
  const char *texture_filename,
  const char *palette_filename){

  int result = 0;
  pvr_ptr_t texture = NULL;
  uint32_t *palette = NULL;

  // Open all files
  //---------------------------------------------------------------------------

#define ERROR(n) {result = (n); goto cleanup;}

  FILE *texture_file = fopen(texture_filename, "rb");
  FILE *palette_file = fopen(palette_filename, "rb");

  if(!texture_file | !palette_file){ERROR(1);}

  // Load texture
  //---------------------------------------------------------------------------

  dtex_header_t dtex_header;
  if(fread(&dtex_header, sizeof(dtex_header), 1, texture_file) != 1){ERROR(2);}

  if(memcmp(dtex_header.magic, "DTEX", 4)){ERROR(3);}

  texture = pvr_mem_malloc(dtex_header.size);
  if(!texture){ERROR(4);}

  if(fread(texture, dtex_header.size, 1, texture_file) != 1){ERROR(5);}

  // Write all metadata except palette stuff
  //---------------------------------------------------------------------------

  sprite->width       = dtex_header.width;
  sprite->height      = dtex_header.height;
  //sprite->type        = dtex_header.type;
  sprite->texture     = texture;

  //This assumes no mip-mapping, no stride, twiddled on, uncompressed and no stride setting (I'm doing this to save on space)
  if(dtex_header.type == 0x08000000){ //RGB565
    sprite->format = 1;
  }
  else if(dtex_header.type == 0x10000000){ //ARGB4444
    sprite->format = 2;
  }
  else if(dtex_header.type == 0x28000000){ //PAL4BPP
    sprite->format = 5;
  }
  else if(dtex_header.type == 0x30000000){ //PAL8BPP
    sprite->format = 6;
  }
  else{ERROR(6);}

  // Load palette if needed
  //---------------------------------------------------------------------------

  if(sprite->format == 5 ||sprite->format == 6){
    dpal_header_t dpal_header;
    if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){ERROR(7);}

    if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){ERROR(8);}

    palette = malloc(dpal_header.color_count * sizeof(uint32_t));
    if(!palette){ERROR(9);}

    if(fread(palette, sizeof(uint32_t), dpal_header.color_count,
      palette_file) != dpal_header.color_count){ERROR(10);}

  // Write palette metadata
  //---------------------------------------------------------------------------

    sprite->palette     = palette;
    sprite->color_count = dpal_header.color_count;
  }
  else{
    sprite->palette = NULL; //color_count doesn't need to be defined...nor does palette really...
  }

#undef ERROR

  // Cleanup
  //---------------------------------------------------------------------------
  cleanup:

  if(texture_file){fclose(texture_file);}
  if(palette_file){fclose(palette_file);}

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  return result;
}

extern void sprite_free(struct sprite *sprite){
  if(sprite){
    pvr_mem_free(sprite->texture);
    free(sprite->palette);
  }
}

extern void draw_sprite(const struct sprite *sprite,
  float x, float y, uint8_t palette_number, uint8_t bpp_mode){

  const float x0 = x;
  const float y0 = y;
  const float x1 = x + sprite->width;
  const float y1 = y + sprite->height;
  const float z = 1;

  pvr_sprite_cxt_t context;
  if(bpp_mode == 5){  //PAL4BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY,
    PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(palette_number),
    sprite->width, sprite->height, sprite->texture, PVR_FILTER_NONE);
  }
  else{
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY,
    PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(palette_number),
    sprite->width, sprite->height, sprite->texture, PVR_FILTER_NONE);
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
