#include "graphics.h"

//There are 4 palettes for 8BPP and 64 palettes for 4BPP. palette_number is the id
extern int graphics_setup_palette(const struct crayon_palette *cp, uint8_t format, uint8_t palette_number){
	int entries;
	if(format == 5){
		entries = 16;
	}
	else if(format == 6){
		entries = 256;
	}
	else{
		return 1;
	}

	pvr_set_pal_format(PVR_PAL_ARGB8888);
	uint16_t i; //Can't this be a uint8_t instead? 0 to 255 and max 256 entries per palette
	//...but then again how would the loop be able to break? since it would overflow back to 0
	for(i = 0; i < cp->colour_count; ++i){
		pvr_set_pal_entry(i + entries * palette_number, cp->palette[i]);
	}
	return 0;
}

extern void graphics_frame_coordinates(const struct crayon_animation *anim, uint16_t *frame_x, uint16_t *frame_y, uint8_t frame){
	int framesPerRow = anim->animation_sheet_width/anim->animation_frame_width;
	int colNum = frame%framesPerRow; //Gets the column (Zero indexed)
	int rowNum = frame/framesPerRow;  //Gets the row (Zero indexed)

	*frame_x = anim->animation_x + (colNum) * anim->animation_frame_width;
	*frame_y = anim->animation_y + (rowNum) * anim->animation_frame_height;

	return;
}

extern void graphics_draw_untextured_poly(uint16_t draw_x, uint16_t draw_y, uint16_t draw_z, uint16_t dim_x,
  uint16_t dim_y, uint32_t colour, uint8_t opaque){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;

	if(opaque){
		pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	}
	else{
		pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	}
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	vert.argb = colour;
	vert.oargb = 0;	//Not sure what this does
	vert.flags = PVR_CMD_VERTEX;    //I think this is used to define the start of a new polygon

	//These define the verticies of the triangles "strips" (One triangle uses verticies of other triangle)
	vert.x = draw_x;
	vert.y = draw_y;
	vert.z = draw_z;
	pvr_prim(&vert, sizeof(vert));

	vert.x = draw_x + dim_x;
	vert.y = draw_y;
	vert.z = draw_z;
	pvr_prim(&vert, sizeof(vert));

	vert.x = draw_x;
	vert.y = draw_y + dim_y;
	vert.z = draw_z;
	pvr_prim(&vert, sizeof(vert));

	vert.x = draw_x + dim_x;
	vert.y = draw_y + dim_y;
	vert.z = draw_z;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
	return;
}

//Not currently handling rotations, I'll do that at a later date
//NOTE: order might be different for different emulators :(
extern void graphics_draw_untextured_array(crayon_untextured_array_t *poly_array){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;

	if(poly_array->options & (1 << 4)){	//If its opaque
		pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	}
	else{
		pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	}
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	// uint8_t multiple_rotations = !!(poly_array->options & (1 << 0));	//Unused
	uint8_t multiple_dims = !!(poly_array->options & (1 << 1));
	uint8_t multiple_colours = !!(poly_array->options & (1 << 2));
	uint8_t multiple_z = !!(poly_array->options & (1 << 3));

	int i;
	for(i = 0; i < poly_array->num_polys; i++){
		vert.argb = poly_array->colours[multiple_colours * i];	//If only one colour, this is forced to colour zero
		vert.oargb = 0;	//Not sure what this does
		vert.flags = PVR_CMD_VERTEX;

		vert.x = poly_array->draw_pos[2 * i];
		vert.y = poly_array->draw_pos[(2 * i) + 1];
		vert.z = poly_array->draw_z[multiple_z * i];
		pvr_prim(&vert, sizeof(vert));

		vert.x = poly_array->draw_pos[2 * i] + poly_array->draw_dims[multiple_dims * 2 * i];	//If using one dim, multiple dims reduces it to the first value
		// vert.y = poly_array->draw_pos[(2 * i) + 1];
		// vert.z = poly_array->draw_z[multiple_z * i];
		pvr_prim(&vert, sizeof(vert));

		vert.x = poly_array->draw_pos[2 * i];
		vert.y = poly_array->draw_pos[(2 * i) + 1] + poly_array->draw_dims[(multiple_dims * 2 * i) + 1];
		// vert.z = poly_array->draw_z[multiple_z * i];
		pvr_prim(&vert, sizeof(vert));

		vert.x = poly_array->draw_pos[2 * i] + poly_array->draw_dims[multiple_dims * 2 * i];
		// vert.y = poly_array->draw_pos[(2 * i) + 1] + poly_array->draw_dims[(multiple_dims * 2 * i) + 1];
		// vert.z = poly_array->draw_z[multiple_z * i];
		vert.flags = PVR_CMD_VERTEX_EOL;
		pvr_prim(&vert, sizeof(vert));
	}
	return;
}


extern uint8_t graphics_draw_sprite(const struct crayon_spritesheet *ss,
	const struct crayon_animation *anim, float draw_x, float draw_y, float draw_z,
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
extern uint8_t graphics_draw_sprites_OLD(const struct crayon_spritesheet *ss,
	const struct crayon_animation *anim, uint16_t *draw_coords, uint16_t *frame_data, uint16_t fd_size,
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

	//Use these to merge the two palette if's into one
	// PVR_TXRFMT_PAL4BPP   (5 << 27)
	// PVR_TXRFMT_PAL8BPP   (6 << 27)
	//(6 << 27) == 110 << 27 == 11000 00000 00000 00000 00000 00000
	//(5 << 27) == 110 << 27 == 10100 00000 00000 00000 00000 00000
	//paletteNumber ranges from 0 to 63 or 0 to 15 depending on BPP
	//   (8BPP) 3  << 25 == 00011 00000 00000 00000 00000 00000
	//   (4BPP) 63 << 21 == 00011 11110 00000 00000 00000 00000
	//PVR_TXRFMT_PAL4BPP == 10100 00000 00000 00000 00000 00000
	//PVR_TXRFMT_PAL8BPP == 11000 00000 00000 00000 00000 00000

	//Think about converting the texture format back into what it is originally XD

	//pvr_sprite_cxt_txr(context, TR/OP/PT list, int textureformat, dimX, dimY, texture, filter)

	//Once the spritesheet/font format is fixed:
	//uint8_t filter = PVR_FILTER_NONE;	//Have param to change this possibly
	//int textureformat = ss->spritesheet_format;
	//if(4BPP){
	//		textureformat |= ((paletteNumber) << 21); 
	// }
	//if(8BPP){
	//		textureformat |= ((paletteNumber) << 25); 
	// }
	//pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, textureformat,
	//	 ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, filter);

	//Then remove all that messy junk below for this and single draw (Confirm above idea would work)

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

// draw "num_sprites" amount of times using all "draw_pos" and the relevant frames/scales/rotations/colour/z (Read options)
// filter mode is in the struct and format is taken from spritesheet struct
// palette_num ais obvious
// poly_list_mode is used to draw opaque, punchthrough or transparent stuff depending on current list
extern uint8_t graphics_draw_sprites(crayon_sprite_array_t *sprite_array, const struct crayon_spritesheet *ss, uint8_t poly_list_mode){
// extern uint8_t graphics_draw_sprites(crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode){

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;
	pvr_sprite_cxt_txr(&context, PVR_LIST_OP_POLY, (ss->spritesheet_format) << 27, 128, 128, ss->spritesheet_texture, PVR_FILTER_NONE);

	// if(sprite_array->ss->spritesheet_format == 6){  //PAL8BPP format
	// pvr_sprite_cxt_txr(&context, poly_list_mode, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(sprite_array->palette_num),
	// 	sprite_array->ss->spritesheet_dims, sprite_array->ss->spritesheet_dims,
	// 	sprite_array->ss->spritesheet_texture, sprite_array->filter);
	// }
	// else if(sprite_array->ss->spritesheet_format == 5){ //PAL4BPP format
	// pvr_sprite_cxt_txr(&context, poly_list_mode, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(sprite_array->palette_num),
	// 	sprite_array->ss->spritesheet_dims, sprite_array->ss->spritesheet_dims,
	// 	sprite_array->ss->spritesheet_texture, sprite_array->filter);
	// }
	// else if(sprite_array->ss->spritesheet_format == 0 || sprite_array->ss->spritesheet_format == 1 ||
	// sprite_array->ss->spritesheet_format == 2){  //ARGB1555, RGB565 and RGB4444
	// 	pvr_sprite_cxt_txr(&context, poly_list_mode, (sprite_array->ss->spritesheet_format) << 27,
	// 	sprite_array->ss->spritesheet_dims, sprite_array->ss->spritesheet_dims,
	// 	sprite_array->ss->spritesheet_texture, sprite_array->filter);
	// }
	// else{ //Unknown format
	// 	return 1;
	// }

	pvr_sprite_txr_t vert = {
		.flags = PVR_CMD_VERTEX_EOL,
	};

	if(!(sprite_array->options & (1 << 0))){	//All share the same z
		// vert.az = sprite_array->draw_z[0];
		// vert.bz = sprite_array->draw_z[0];
		// vert.cz = sprite_array->draw_z[0];
	}
	if(!(sprite_array->options & (1 << 1))){	//All share the same frame
		// u0 = sprite_array->frame_coords_map[sprite_array->frame_coords_keys[0]] / (float)sprite_array->ss->spritesheet_dims;
		// v0 = sprite_array->frame_coords_map[sprite_array->frame_coords_keys[0] + 1] / (float)sprite_array->ss->spritesheet_dims;
		// u1 = u0 + sprite_array->anim->animation_frame_width / (float)sprite_array->ss->spritesheet_dims;
		// v1 = v0 + sprite_array->anim->animation_frame_height / (float)sprite_array->ss->spritesheet_dims;
		// vert.auv = PVR_PACK_16BIT_UV(u0, v0);
		// vert.buv = PVR_PACK_16BIT_UV(u1, v0);
		// vert.cuv = PVR_PACK_16BIT_UV(u1, v1);
	}

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));
	uint16_t i;
	for(i = 0; i < sprite_array->num_sprites; i++){
		if(0){
			//The floor command breaks stuff...? Not anymore atleast
			vert.ax = floor(sprite_array->draw_pos[2 * i]);	//We floor the values since we're doing 2D and they'll look messed up if we have position "11.5", however scales can mess this up
			vert.ay = floor(sprite_array->draw_pos[(2 * i) + 1]);
			vert.bx = floor(sprite_array->draw_pos[2 * i] + sprite_array->anim->animation_frame_width) * sprite_array->scales[2 * i * !!(sprite_array->options & (1 << 2))];
			vert.by = floor(sprite_array->draw_pos[(2 * i) + 1]);
			vert.cx = floor(sprite_array->draw_pos[2 * i] + sprite_array->anim->animation_frame_width) * sprite_array->scales[2 * i * !!(sprite_array->options & (1 << 2))];
			vert.cy = floor(sprite_array->draw_pos[(2 * i) + 1] + sprite_array->anim->animation_frame_height) * sprite_array->scales[(2 * i * !!(sprite_array->options & (1 << 2))) + 1];
			vert.dx = floor(sprite_array->draw_pos[2 * i]);
			vert.dy = floor(sprite_array->draw_pos[(2 * i) + 1] + sprite_array->anim->animation_frame_height) * sprite_array->scales[(2 * i * !!(sprite_array->options & (1 << 2))) + 1];
		}
		else if(1){
			vert.ax = 0;
			vert.ay = 0;
			vert.bx = 128;
			vert.by = 0;
			vert.cx = 128;
			vert.cy = 128;
			vert.dx = 0;
			vert.dy = 128;
		}
		else{
			//Better, but the widths aren't being added right
			// vert.ax = sprite_array->draw_pos[2 * i];
			// vert.ay = sprite_array->draw_pos[(2 * i) + 1];
			// vert.bx = sprite_array->draw_pos[2 * i] + (sprite_array->anim->animation_frame_width * sprite_array->scales[2 * i * !!(sprite_array->options & (1 << 2))]);
			// vert.by = sprite_array->draw_pos[(2 * i) + 1];
			// vert.cx = sprite_array->draw_pos[2 * i] + (sprite_array->anim->animation_frame_width * sprite_array->scales[2 * i * !!(sprite_array->options & (1 << 2))]);
			// vert.cy = sprite_array->draw_pos[(2 * i) + 1] + (sprite_array->anim->animation_frame_height * sprite_array->scales[(2 * i * !!(sprite_array->options & (1 << 2))) + 1]);
			// vert.dx = sprite_array->draw_pos[2 * i];
			// vert.dy = sprite_array->draw_pos[(2 * i) + 1] + (sprite_array->anim->animation_frame_height * sprite_array->scales[(2 * i * !!(sprite_array->options & (1 << 2))) + 1]);
		}

		if(sprite_array->options & (1 << 0)){	//z
			// vert.az = (float)sprite_array->draw_z[i];
			// vert.bz = (float)sprite_array->draw_z[i];
			// vert.cz = (float)sprite_array->draw_z[i];
		}
		vert.az = 1.0f;
		vert.bz = 1.0f;
		vert.cz = 1.0f;

		if(sprite_array->options & (1 << 1)){	//frame
			// u0 = sprite_array->frame_coords_map[(2 * sprite_array->frame_coords_keys[i])] / (float)sprite_array->ss->spritesheet_dims;
			// v0 = sprite_array->frame_coords_map[(2 * sprite_array->frame_coords_keys[i]) + 1] / (float)sprite_array->ss->spritesheet_dims;
			// u1 = u0 + sprite_array->anim->animation_frame_width / (float)sprite_array->ss->spritesheet_dims;
			// v1 = v0 + sprite_array->anim->animation_frame_height / (float)sprite_array->ss->spritesheet_dims;
			// vert.auv = PVR_PACK_16BIT_UV(u0, v0);
			// vert.buv = PVR_PACK_16BIT_UV(u1, v0);
			// vert.cuv = PVR_PACK_16BIT_UV(u1, v1);

			u0 = 0;
			v0 = 0;
			u1 = 1;
			v1 = 1;
			vert.auv = PVR_PACK_16BIT_UV(u0, v0);
			vert.buv = PVR_PACK_16BIT_UV(u1, v0);
			vert.cuv = PVR_PACK_16BIT_UV(u1, v1);
		}

		if(sprite_array->options & (1 << 3)){	//rotations (Unimplemented)
			;
		}
		else{
			;
		}

		if(sprite_array->options & (1 << 4)){	//colour (Unimplemented)
			;
		}
		else{
			;
		}

		pvr_prim(&vert, sizeof(vert));
	}

	return 0;
}

extern uint8_t graphics_draw_polys(crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode){
	return 0;
}

extern uint8_t graphics_draw_text_mono(const struct crayon_font_mono *fm, float draw_x, float draw_y,
	float draw_z, float scale_x, float scale_y,	uint8_t paletteNumber, char * string){

	float x0 = draw_x;
	float y0 = draw_y;
	const float z = draw_z;

	//x1 and y1 depend on the letter
	float x1 = draw_x + fm->char_width * scale_x;
	float y1 = draw_y + fm->char_height * scale_y;

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;
	if(fm->texture_format == 6){  //PAL8BPP format
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(paletteNumber),
		fm->fontsheet_dim, fm->fontsheet_dim, fm->fontsheet_texture, PVR_FILTER_NONE);
	}
	else if(fm->texture_format == 5){ //PAL4BPP format
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(paletteNumber),
		fm->fontsheet_dim, fm->fontsheet_dim, fm->fontsheet_texture, PVR_FILTER_NONE);
	}
	else if(fm->texture_format == 0 || fm->texture_format == 1 || fm->texture_format == 2){  //ARGB1555, RGB565 and RGB4444
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (fm->texture_format) << 27,
		fm->fontsheet_dim, fm->fontsheet_dim, fm->fontsheet_texture, PVR_FILTER_NONE);
	}
	else{ //Unknown format
		return 1;
	}

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	float prop_width = (float)fm->char_width / fm->fontsheet_dim;
	float prop_height = (float)fm->char_height / fm->fontsheet_dim;
	while(1){	//First char seems to be drawn higher than others
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line (Doesn't seem to work right)
			x0 = draw_x;
			x1 = draw_x + fm->char_width * scale_x;
			y0 = y1;
			y1 += fm->char_height * scale_y;
			i++;
			continue;
		}
		uint8_t distance_from_space = string[i] - ' ';

		uint8_t row = distance_from_space / fm->num_columns;
		uint8_t column = distance_from_space - (row * fm->num_columns);

		u0 = column * prop_width;
		v0 = row * prop_height;
		u1 = u0 + prop_width;
		v1 = v0 + prop_height;

		pvr_sprite_txr_t vert = {
			.flags = PVR_CMD_VERTEX_EOL,
			.ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(u0, v0),
			.bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(u1, v0),
			.cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(u1, v1),
			.dx = x0, .dy = y1
		};
		pvr_prim(&vert, sizeof(vert));

		x0 = x1;
		x1 += fm->char_width * scale_x;
		i++;
	}

	return 0;
}

extern uint8_t graphics_draw_text_prop(const struct crayon_font_prop *fp, float draw_x, float draw_y,
	float draw_z, float scale_x, float scale_y,	uint8_t paletteNumber, char * string){

	float x0 = draw_x;
	float y0 = draw_y;
	const float z = draw_z;
	float x1 = draw_x;
	float y1 = draw_y + fp->char_height * scale_y;
	float v0 = 0;
	float v1 = 0;
	float percent_height = (float)fp->char_height / fp->fontsheet_dim;

	// float percent_width;
	float u0, u1;

	pvr_sprite_cxt_t context;
	if(fp->texture_format == 6){  //PAL8BPP format
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(paletteNumber),
		fp->fontsheet_dim, fp->fontsheet_dim, fp->fontsheet_texture, PVR_FILTER_NONE);
	}
	else if(fp->texture_format == 5){ //PAL4BPP format
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(paletteNumber),
		fp->fontsheet_dim, fp->fontsheet_dim, fp->fontsheet_texture, PVR_FILTER_NONE);
	}
	else if(fp->texture_format == 0 || fp->texture_format == 1 || fp->texture_format == 2){  //ARGB1555, RGB565 and RGB4444
		pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (fp->texture_format) << 27,
		fp->fontsheet_dim, fp->fontsheet_dim, fp->fontsheet_texture, PVR_FILTER_NONE);
	}
	else{ //Unknown format
		return 1;
	}

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	while(1){	//First char seems to be drawn higher than others
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line (Doesn't seem to work right)
			x0 = draw_x;
			x1 = draw_x;
			y0 = y1;
			y1 += fp->char_height * scale_y;
			i++;
			continue;
		}
		uint8_t distance_from_space = string[i] - ' ';

		x1 += fp->char_width[distance_from_space] * scale_x;	//get the width of the display char

		u0 = (float)fp->char_x_coord[distance_from_space] / fp->fontsheet_dim;
		u1 = u0 + ((float)fp->char_width[distance_from_space] / fp->fontsheet_dim);

		//Can this section be optimised? Maybe replace it with binary search?
		int j;
		int char_count = 0;
		for(j = 0; j < fp->num_rows; j++){
			char_count += fp->chars_per_row[j];
			if(distance_from_space < char_count){
				v0 = j * percent_height;
				v1 = v0 + percent_height;
				break;
			}
		}

		pvr_sprite_txr_t vert = {
			.flags = PVR_CMD_VERTEX_EOL,
			.ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(u0, v0),
			.bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(u1, v0),
			.cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(u1, v1),
			.dx = x0, .dy = y1
		};
		pvr_prim(&vert, sizeof(vert));

		x0 = x1;
		i++;
	}

	return 0;
}
