#include "graphics.h"

//There are 4 palettes for 8BPP and 64 palettes for 4BPP. palette_number is the id
extern int graphics_setup_palette(uint8_t palette_number, const struct spritesheet *ss){
  int entries;
  if(ss->spritesheet_format == 5){
    entries = 16;
  }
  else if(ss->spritesheet_format == 6){
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

extern void graphics_draw_colour_poly(uint16_t draw_x, uint16_t draw_y, uint16_t draw_z, uint16_t dim_x,
  uint16_t dim_y, uint32_t colour){
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = colour;
    vert.oargb = 0;	//Not sure what this does
    vert.flags = PVR_CMD_VERTEX;    //I think this is used to define the start of a new polygon

    //These define the verticies of the triangles "strips" (One triangle uses verticies of other triangle)
    vert.x = draw_x;
    vert.y = draw_y;
    vert.z = draw_z;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = draw_x + dim_x;
    vert.y = draw_y;
    vert.z = draw_z;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = draw_x;
    vert.y = draw_y + dim_y;
    vert.z = draw_z;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = draw_x + dim_x;
    vert.y = draw_y + dim_y;
    vert.z = draw_z;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
    return;
}

extern uint8_t graphics_draw_sprite(const struct spritesheet *ss,
	const struct animation *anim, float draw_x, float draw_y, float draw_z,
	float scale_x, float scale_y, uint16_t frame_x, uint16_t frame_y,
	uint8_t paletteNumber){

	//Screen coords. letter0 is top left coord, letter1 is bottom right coord. Z is depth (Layer)
	const float x0 = draw_x;  //Do these really need to be floats?
	const float y0 = draw_y;
	const float x1 = draw_x + (anim->animation_frame_width) * scale_x;
	const float y1 = draw_y + (anim->animation_frame_height) * scale_y;
	const float z = draw_z;

	//Texture coords. letter0 and letter1 have same logic as before (CHECK)
	const float u0 = frame_x / (float)ss->spritesheet_dims;
	const float v0 = frame_y / (float)ss->spritesheet_dims;
	const float u1 = (frame_x + anim->animation_frame_width) / (float)ss->spritesheet_dims;
	const float v1 = (frame_y + anim->animation_frame_height) / (float)ss->spritesheet_dims;

	pvr_sprite_cxt_t context;
	if(ss->spritesheet_format == 6){  //PAL8BPP format
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(paletteNumber),
		ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
	}
	else if(ss->spritesheet_format == 5){ //PAL4BPP format
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(paletteNumber),
		ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
	}
	else if(ss->spritesheet_format == 0 || ss->spritesheet_format == 1 || ss->spritesheet_format == 2){  //ARGB1555, RGB565 and RGB4444
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (ss->spritesheet_format) << 27,
		ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
	}
	else{ //Unknown format
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

//Note this always draws transparent textures sortal (look at pvr_sprite_cxt_txr() calls)
extern uint8_t graphics_draw_sprites_OLD(const struct spritesheet *ss,
	const struct animation *anim, uint16_t *draw_coords, uint16_t *frame_data, uint16_t fd_size,
	uint16_t num_sprites, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber){

	//Texture coords. u0,v0 is the bottom left texel and u1,v1 is the top right
	float u0 = frame_data[0] / (float)ss->spritesheet_dims;
	float v0 = frame_data[1] / (float)ss->spritesheet_dims;
	float u1 = (frame_data[0] + anim->animation_frame_width) / (float)ss->spritesheet_dims;
	float v1 = (frame_data[1] + anim->animation_frame_height) / (float)ss->spritesheet_dims;

	uint8_t extras_mode = 0;
	if(fd_size != 2){
		extras_mode = 1;
	}

	pvr_sprite_cxt_t context;
	if(ss->spritesheet_format == 6){  //PAL8BPP format
    // pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(paletteNumber),
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | ((paletteNumber) << 25),
		ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
	}
	else if(ss->spritesheet_format == 5){ //PAL4BPP format
    // pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(paletteNumber),
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | ((paletteNumber) << 21),
		ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
	}
	else if(ss->spritesheet_format == 0 || ss->spritesheet_format == 1 || ss->spritesheet_format == 2){  //ARGB1555, RGB565 and RGB4444
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (ss->spritesheet_format) << 27,
		ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
	}
	else{ //Unknown format
	return 1;
	}

	//Set shared vert data (Should z be like frame, different per sprite?)
	pvr_sprite_txr_t vert = {
		.flags = PVR_CMD_VERTEX_EOL,
		.az = draw_z, .auv = PVR_PACK_16BIT_UV(u0, v0),
		.bz = draw_z, .buv = PVR_PACK_16BIT_UV(u1, v0),
		.cz = draw_z, .cuv = PVR_PACK_16BIT_UV(u1, v1),
	};

	//draws all sprites in the array at their coords as listed in the array
	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));	//Doesn't seem to need to be in the loop, not too sure why though
	int i;
	for(i = 0; i < num_sprites * 2; i = i + 2){
		vert.ax = draw_coords[i];
		vert.ay = draw_coords[i + 1];
		vert.bx = draw_coords[i] + anim->animation_frame_width;
		vert.by = draw_coords[i + 1];
		vert.cx = draw_coords[i] + anim->animation_frame_width;
		vert.cy = draw_coords[i + 1] + anim->animation_frame_height;
		vert.dx = draw_coords[i];
		vert.dy = draw_coords[i + 1] + anim->animation_frame_height;
		if(extras_mode == 1){	//When each sprite has its own frame data
			u0 = frame_data[i] / (float)ss->spritesheet_dims;
			v0 = frame_data[i + 1] / (float)ss->spritesheet_dims;
			u1 = (frame_data[i] + anim->animation_frame_width) / (float)ss->spritesheet_dims;
			v1 = (frame_data[i + 1] + anim->animation_frame_height) / (float)ss->spritesheet_dims;
			vert.auv = PVR_PACK_16BIT_UV(u0, v0);
			vert.buv = PVR_PACK_16BIT_UV(u1, v0);
			vert.cuv = PVR_PACK_16BIT_UV(u1, v1);
		}
		pvr_prim(&vert, sizeof(vert));
	}

	return 0;
}

// draw "num_sprites" amount of times using all "draw_pos" and the relevant frames/scales/rotations (Read options)
// options also includes filter mode and format is taken from spritesheet struct
// draw_z, colour and palette_num are obvious
// poly_list_mode is used to draw opaque, punchthrough or transparent stuff depending on current list
extern uint8_t graphics_draw_sprites(CrayonSpriteArray_t *sprite_array, uint8_t poly_list_mode){
	return 0;
}

/*

typedef struct crayon_sprite_array{
  uint16_t * draw_pos;  //Width then Height extracted from anim/frame data, Each group of 2 is for one sub-texture
  uint16_t * frame_coords;  //Each group of 4 elements is one sub-texture to draw
  uint8_t * scales; //I think 8 bits is good enough for most cases
  float rotations;  //Poly uses angles to rotate on Z axis, sprite uses booleans/flip bits. Decide what type this should be...
  uint16_t num_sprites; //This tells the draw function how many sprites/polys to draw.

  uint8_t options;  //Format FSRX XFFF, Basically 3 booleans options relating to frameCoords, scales and rotations
            //if that bit is set to true, then we use the first element of F/S/R array for all sub-textures
            //Else we assume each sub-texture has its own unique F/S/R value
            //The 3 F's at the end are for the filtering mode. Can easily access it with a modulo 8 operation
            //0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2

  uint8_t draw_z; //The layer to help deal with overlapping sprites/polys
  uint8_t palette_num;  //Also ask if palettes can start at not multiples of 16 or 256
  uint32_t colour;  //For poly mode this dictates the rgb and alpha of a polygon
  spritesheet_t * ss;
  animation_t * anim;
} crayon_sprite_array_t;

*/
