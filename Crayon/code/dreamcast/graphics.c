#include "graphics.h"

//There are 4 palettes for 8BPP and 64 palettes for 4BPP
extern uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp){
	if(cp->palette_id < 0 || cp->bpp < 0 || cp->bpp * cp->palette_id >= 1024){	//Invalid format/palette not properly set
		return 1;
	}
	uint16_t entries;
	if(cp->bpp == 4){
		entries = 16;
	}
	else if(cp->bpp == 8){
		entries = 256;
	}
	else{	//When OpenGL port arrives, this might change if I allow more than 64 entries
		return 2;
	}

	pvr_set_pal_format(PVR_PAL_ARGB8888);
	uint16_t i; //Can't this be a uint8_t instead? 0 to 255 and max 256 entries per palette
	//...but then again how would the loop be able to break? since it would overflow back to 0
	for(i = 0; i < cp->colour_count; ++i){
		pvr_set_pal_entry(i + entries * cp->palette_id, cp->palette[i]);
	}
	return 0;
}

extern void crayon_graphics_frame_coordinates(const crayon_draw_array_t *draw_list, uint8_t index, uint8_t frame){
	crayon_animation_t * anim = draw_list->animation;
	uint8_t frames_per_row = anim->sheet_width / anim->frame_width;
	uint8_t column_number = frame % frames_per_row;
	uint8_t row_number = frame / frames_per_row;

	draw_list->frame_coord_map[index].x = anim->x + (column_number * anim->frame_width);
	draw_list->frame_coord_map[index].y = anim->y + (row_number * anim->frame_height);
	return;
}

extern float crayon_graphics_get_draw_element_width(const crayon_draw_array_t *draw_list, uint8_t id){
	if(!(draw_list->options & CRAY_MULTI_SCALE)){id = 0;}	//When there's only one scale
	if(draw_list->options & CRAY_HAS_TEXTURE){
		return draw_list->animation->frame_width * draw_list->scale[id].x;
	}
	else{
		return draw_list->scale[id].x;
	}
}

extern float crayon_graphics_get_draw_element_height(const crayon_draw_array_t *draw_list, uint8_t id){
	if(!(draw_list->options & CRAY_MULTI_SCALE)){id = 0;}	//When there's only one scale
	if(draw_list->options & CRAY_HAS_TEXTURE){
		return draw_list->animation->frame_height * draw_list->scale[id].y;
	}
	else{
		return draw_list->scale[id].y;
	}
}

extern uint8_t crayon_graphics_draw_untextured_array(crayon_draw_array_t *draw_array, uint8_t poly_list_mode){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert[4];

	pvr_poly_cxt_col(&cxt, poly_list_mode);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	//--CR -D-Z
	uint8_t multiple_rotation = (draw_array->options >> 4) & 1;
	uint8_t multiple_dims = (draw_array->options >> 2) & 1;
	uint8_t multiple_colour = (draw_array->options >> 5) & 1;
	uint8_t multiple_z = draw_array->options & 1;

	//All this just for rotations
	uint16_t *rotation_index;
	uint16_t zero = 0;
	float angle = 0;
	float mid_x = 0;
	float mid_y = 0;
	uint8_t skip = 0;

	uint16_t i, j;
	if(multiple_rotation){rotation_index = &i;}
	else{rotation_index = &zero;}

	for(i = 0; i < 3; i++){
		vert[i].flags = PVR_CMD_VERTEX;
	}
	vert[3].flags = PVR_CMD_VERTEX_EOL;

	for(i = 0; i < draw_array->list_size; i++){
		if(draw_array->colour[multiple_colour * i] >> 24 == 0){	//Don't draw alpha-less stuff
			if(i != 0){continue;}	//For the first element, we need to initialise our vars, otherwise we just skip to the next element
			skip = 1;
		}
		vert[0].argb = draw_array->colour[multiple_colour * i];	//If only one colour, this is forced to colour zero
		vert[0].oargb = 0;
		vert[0].z = draw_array->layer[multiple_z * i];

		vert[0].x = trunc(draw_array->coord[i].x);
		vert[0].y = trunc(draw_array->coord[i].y);
		vert[1].x = vert[0].x + trunc(draw_array->scale[multiple_dims * i].x);	//If using one dim, multiple dims reduces it to the first value
		vert[1].y = vert[0].y;
		vert[2].x = vert[0].x;
		vert[2].y = vert[0].y + trunc(draw_array->scale[multiple_dims * i].y);
		vert[3].x = vert[1].x;
		vert[3].y = vert[2].y;

		//Update rotation part if needed
		if(*rotation_index == i){
			angle = fmod(draw_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
			if(angle < 0){angle += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359
			angle = (angle * M_PI) / 180.0f;	//Convert from degrees to ratians
		}

		//Rotate the poly
		if(draw_array->rotation[*rotation_index] != 0.0f){
			//Gets the midpoint
			mid_x = ((vert[1].x - vert[0].x)/2.0f) + vert[0].x;
			mid_y = ((vert[2].y - vert[0].y)/2.0f) + vert[0].y;

			//Rotate the verts around the midpoint
			for(j = 0; j < 4; j++){
				crayon_graphics_rotate_point(mid_x, mid_y, &vert[j].x, &vert[j].y, angle);
			}
		}

		for(j = 1; j < 4; j++){
			vert[j].z = vert[0].z;
			vert[j].argb = vert[0].argb;
			vert[j].oargb = vert[0].oargb;
		}

		if(skip){
			skip = 0;
			continue;
		}
		pvr_prim(&vert, sizeof(pvr_vertex_t) * 4);
	}
	return 0;
}

extern void crayon_graphics_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t layer, uint32_t colour,
	uint8_t poly_list_mode){
	if(colour >> 24 == 0){return;}	//Don't draw alpha-less stuff
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert[4];

	pvr_poly_cxt_col(&cxt, poly_list_mode);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	uint8_t i;
	for(i = 0; i < 4; i++){
		vert[i].argb = colour;
		vert[i].oargb = 0;
		vert[i].z = layer;
		vert[i].flags = PVR_CMD_VERTEX;
	}
	vert[3].flags = PVR_CMD_VERTEX_EOL;

	int16_t y_diff = y2 - y1;
	int16_t x_diff = x2 - x1;

	//Absolute them
	if(y_diff < 0){y_diff *= -1;}
	if(x_diff < 0){x_diff *= -1;}

	//We flip them if its in between SW and NE (Going clockwise)

	if((x1 > x2 && x_diff > y_diff) || (y1 > y2 && x_diff < y_diff)){
		uint16_t temp = x2;
		x2 = x1;
		x1 = temp;

		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	// Top left
	vert[0].x = x1;
	vert[0].y = y1;

	if(x_diff > y_diff){	//Wider than taller
		//Top right
		vert[1].x = x2 + 1;
		vert[1].y = y2;

		//Bottom left
		vert[2].x = x1;
		vert[2].y = y1 + 1;

		//Bottom right
		vert[3].x = x2 + 1;
		vert[3].y = y2 + 1;

	}
	else{	//taller than wider
		vert[1].x = x1 + 1;
		vert[1].y = y1;

		vert[2].x = x2;
		vert[2].y = y2 + 1;

		vert[3].x = x2 + 1;
		vert[3].y = y2 + 1;
	}

	pvr_prim(&vert, sizeof(pvr_vertex_t) * 4);

	return;
}

//---- --CM
//C is for camera mode (Unimplemented)
//M is for draw mode
extern int8_t crayon_graphics_draw(crayon_draw_array_t *draw_array, uint8_t poly_list_mode, uint8_t draw_mode){
	if(!(draw_mode & CRAY_USING_CAMERA)){	//No Camera
		if(draw_array->options & CRAY_HAS_TEXTURE){	//Textured
			if(!(draw_mode & CRAY_DRAW_ENHANCED)){	//Simple draw
				return crayon_graphics_draw_sprites_simple(draw_array, poly_list_mode);
			}
			return crayon_graphics_draw_sprites_enhanced(draw_array, poly_list_mode);
		}
		return crayon_graphics_draw_untextured_array(draw_array, poly_list_mode);
	}
	else{	//Camera
		if(draw_array->options & CRAY_HAS_TEXTURE){	//Textured
			;
		}
		else{
			if(!(draw_mode & CRAY_DRAW_ENHANCED)){
				;
			}
			else{
				;
			}
		}
		return -1;
	}
}

// 	//Use these to merge the two palette if's into one
// 	// PVR_TXRFMT_PAL4BPP   (5 << 27)
// 	// PVR_TXRFMT_PAL8BPP   (6 << 27)
// 	//(6 << 27) == 110 << 27 == 11000 00000 00000 00000 00000 00000
// 	//(5 << 27) == 110 << 27 == 10100 00000 00000 00000 00000 00000
// 	//palette_number ranges from 0 to 63 or 0 to 15 depending on BPP
// 	//   (8BPP) 3  << 25 == 00011 00000 00000 00000 00000 00000
// 	//   (4BPP) 63 << 21 == 00011 11110 00000 00000 00000 00000
// 	//PVR_TXRFMT_PAL4BPP == 10100 00000 00000 00000 00000 00000
// 	//PVR_TXRFMT_PAL8BPP == 11000 00000 00000 00000 00000 00000

// 	//pvr_sprite_cxt_txr(context, TR/OP/PT list, int textureformat, dimX, dimY, texture, filter)

// 	//Once the spritesheet/font format is fixed:
// 	//uint8_t filter = PVR_FILTER_NONE;	//Have param to change this possibly
// 	//int textureformat = ss->texture_format;
// 	//if(4BPP){
// 	//		textureformat |= ((palette_number) << 21); 
// 	// }
// 	//if(8BPP){
// 	//		textureformat |= ((palette_number) << 25); 
// 	// }
// 	//pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, textureformat,
// 	//	 ss->dimensions, ss->dimensions, ss->texture, filter);


extern uint8_t crayon_graphics_draw_sprites_simple(crayon_draw_array_t *draw_array, uint8_t poly_list_mode){
	float u0, v0, u1, v1;
	uint32_t duv;	//duv is used to assist in the rotations
	u0 = 0; v0 = 0; u1 = 0; v1 = 0; duv = 0;	//Needed if you want to prevent a bunch of compiler warnings...

	pvr_sprite_cxt_t context;
	uint8_t texture_format = (((1 << 3) - 1) & (draw_array->spritesheet->texture_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = draw_array->spritesheet->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((draw_array->palette->palette_id) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((draw_array->palette->palette_id) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, draw_array->spritesheet->dimensions,
		draw_array->spritesheet->dimensions, draw_array->spritesheet->texture, draw_array->filter);

	pvr_sprite_txr_t vert = {
		.flags = PVR_CMD_VERTEX_EOL
	};

	//Easily lets us use the right index for each array
		//That way 1-length arrays only get calculated once and each element for a multi list is calculated
	uint16_t *rotation_index, *flip_index, *frame_index, *z_index;
	uint16_t i;	//The main loop's index
	uint16_t zero = 0;
	float rotation_under_360;
	if(draw_array->options & CRAY_MULTI_LAYER){
		z_index = &i;
	}
	else{
		z_index = &zero;
	}

	if(draw_array->options & CRAY_MULTI_FRAME){
		frame_index = &i;
	}
	else{
		frame_index = &zero;
	}

	if(draw_array->options & CRAY_MULTI_FLIP){
		flip_index = &i;
	}
	else{
		flip_index = &zero;
	}

	if(draw_array->options & CRAY_MULTI_ROTATE){
		rotation_index = &i;
	}
	else{
		rotation_index = &zero;
	}

	uint8_t multi_scale = !!(draw_array->options & CRAY_MULTI_SCALE);

	uint8_t multi_flip = !!(draw_array->options & CRAY_MULTI_FLIP);
	uint8_t multi_rotate = !!(draw_array->options & CRAY_MULTI_ROTATE);
	uint8_t multi_frame = !!(draw_array->options & CRAY_MULTI_FRAME);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));
	for(i = 0; i < draw_array->list_size; i++){
		//These if statements will trigger once if we have a single element (i == 0)
			//and every time for a multi-list
		if(*z_index == i){	//z
			vert.az = (float)draw_array->layer[*z_index];
			vert.bz = (float)draw_array->layer[*z_index];
			vert.cz = (float)draw_array->layer[*z_index];
		}

		if(*frame_index == i){	//frame
			u0 = draw_array->frame_coord_map[draw_array->frame_coord_key[*frame_index]].x / (float)draw_array->spritesheet->dimensions;
			v0 = draw_array->frame_coord_map[draw_array->frame_coord_key[*frame_index]].y / (float)draw_array->spritesheet->dimensions;
			u1 = u0 + draw_array->animation->frame_width / (float)draw_array->spritesheet->dimensions;
			v1 = v0 + draw_array->animation->frame_height / (float)draw_array->spritesheet->dimensions;
		}

		//Basically enter if first element or either the flip/rotate/frame changed
			//The multi_blah's are there to prevent checks on draw params that aren't multi/won't change
		if(i == 0 || (multi_flip && (draw_array->flip[i] != draw_array->flip[i - 1])) ||
			(multi_rotate && (draw_array->rotation[i] != draw_array->rotation[i - 1])) ||
			(multi_frame &&
			((draw_array->frame_coord_map[i].x != draw_array->frame_coord_map[i - 1].x) ||
			(draw_array->frame_coord_map[i].y != draw_array->frame_coord_map[i - 1].y)))
			){
			
			if(draw_array->flip[*flip_index] & (1 << 0)){	//Is flipped?
				vert.auv = PVR_PACK_16BIT_UV(u1, v0);
				vert.buv = PVR_PACK_16BIT_UV(u0, v0);
				vert.cuv = PVR_PACK_16BIT_UV(u0, v1);
				duv = PVR_PACK_16BIT_UV(u1, v1);	//Used for possible rotation calculations
			}
			else{
				vert.auv = PVR_PACK_16BIT_UV(u0, v0);
				vert.buv = PVR_PACK_16BIT_UV(u1, v0);
				vert.cuv = PVR_PACK_16BIT_UV(u1, v1);
				duv = PVR_PACK_16BIT_UV(u0, v1);
			}

			//Don't both rotating if the value is zero
			if(draw_array->rotation[*rotation_index] != 0){
				rotation_under_360 = fmod(draw_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
				if(rotation_under_360 < 0){rotation_under_360 += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359

				//For sprite mode we can't simply "rotate" the verts, instead we need to change the uv
				if(crayon_graphics_almost_equals(rotation_under_360, 90.0, 45.0)){
					vert.cuv = vert.buv;
					vert.buv = vert.auv;
					vert.auv = duv;

					goto verts_rotated;
				}
				else if(crayon_graphics_almost_equals(rotation_under_360, 180.0, 45.0)){
					vert.buv = duv;
					duv = vert.auv;
					vert.auv = vert.cuv;
					vert.cuv = duv;
				}
				else if(crayon_graphics_almost_equals(rotation_under_360, 270.0, 45.0)){
					vert.auv = vert.buv;
					vert.buv = vert.cuv;
					vert.cuv = duv;

					goto verts_rotated;
				}
			}
		}

		//Imagine a "goto verts_normal;" for this little bit
			//I couldn't actually do that since the verts wouldn't be set if the rotation aren't checked
			//Hence it just flows here naturally

		vert.ax = trunc(draw_array->coord[i].x);
		vert.ay = trunc(draw_array->coord[i].y);
		vert.bx = vert.ax + trunc(draw_array->animation->frame_width * draw_array->scale[i * multi_scale].x);
		vert.by = vert.ay;
		vert.cx = vert.bx;
		vert.cy = vert.ay + trunc(draw_array->animation->frame_height * draw_array->scale[i * multi_scale].y);
		vert.dx = vert.ax;
		vert.dy = vert.cy;

		//For 90 and 270 modes we need different vert positions aswell
			//for cases where frame width != frame height
		if(0){
			verts_rotated:	;	//The semi-colon is there because a label can't be followed by a declaration (Compiler thing)
								//So instead we trick it and give an empty statement :P

			//Both vars are uint16_t and lengths can't be negative or more than 1024 (Largest texture size for DC)
				//Therfore storing the result in a int16_t is perfectly fine
			int16_t diff = draw_array->animation->frame_width - draw_array->animation->frame_height;

			vert.ax = trunc(draw_array->coord[i].x) + ((draw_array->scale[i * multi_scale].y * diff) / 2);
			vert.ay = trunc(draw_array->coord[i].y) - ((draw_array->scale[i * multi_scale].x * diff) / 2);
			vert.bx = vert.ax + trunc(draw_array->animation->frame_height * draw_array->scale[i * multi_scale].y);
			vert.by = vert.ay;
			vert.cx = vert.bx;
			vert.cy = vert.ay + trunc(draw_array->animation->frame_width * draw_array->scale[i * multi_scale].x);
			vert.dx = vert.ax;
			vert.dy = vert.cy;
		}

		pvr_prim(&vert, sizeof(vert));
	}

	return 0;
}

extern uint8_t crayon_graphics_draw_sprites_enhanced(crayon_draw_array_t *draw_array, uint8_t poly_list_mode){
	float u0, v0, u1, v1;
	u0 = 0; v0 = 0; u1 = 0; v1 = 0;	//Needed if you want to prevent a bunch of compiler warnings...

	uint8_t texture_format = (((1 << 3) - 1) & (draw_array->spritesheet->texture_format >> (28 - 1)));	//Gets the Pixel format
																											//https://github.com/tvspelsfreak/texconv
	int textureformat = draw_array->spritesheet->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((draw_array->palette->palette_id) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((draw_array->palette->palette_id) << 25);	//Update the later to use KOS' macros
	}

	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_txr(&cxt, poly_list_mode, textureformat,
		draw_array->spritesheet->dimensions, draw_array->spritesheet->dimensions,
		draw_array->spritesheet->texture, draw_array->filter);
	pvr_poly_compile(&hdr, &cxt);
	hdr.cmd |= 4;	//Enable oargb
	pvr_prim(&hdr, sizeof(hdr));

	pvr_vertex_t vert[4];	//4 verts per sprite

	//Easily lets us use the right index for each array
		//That way 1-length arrays only get calculated once and each element for a multi list is calculated
	uint16_t *rotation_index, *flip_index, *frame_index, *z_index, *colour_index;
	uint8_t multi_scale = !!(draw_array->options & CRAY_MULTI_SCALE);
	uint16_t zero = 0;
	float angle = 0;
	float mid_x = 0;
	float mid_y = 0;
	uint8_t skip = 0;

	uint16_t i, j;	//Indexes
	if(draw_array->options & CRAY_MULTI_LAYER){z_index = &i;}
	else{z_index = &zero;}

	if(draw_array->options & CRAY_MULTI_FRAME){frame_index = &i;}
	else{frame_index = &zero;}

	if(draw_array->options & CRAY_MULTI_FLIP){flip_index = &i;}
	else{flip_index = &zero;}

	if(draw_array->options & CRAY_MULTI_ROTATE){rotation_index = &i;}
	else{rotation_index = &zero;}

	if(draw_array->options & CRAY_MULTI_COLOUR){colour_index = &i;}
	else{colour_index = &zero;}

	//Set the flags
	for(j = 0; j < 3; j++){
		vert[j].flags = PVR_CMD_VERTEX;
	}
	vert[3].flags = PVR_CMD_VERTEX_EOL;

	uint8_t invf, f, a, r, g, b;
	for(i = 0; i < draw_array->list_size; i++){
		if(draw_array->colour[*colour_index] >> 24 == 0){	//Don't draw alpha-less stuff
			if(i != 0){continue;}	//For the first element, we need to initialise our vars, otherwise we just skip to the next element
			skip = 1;
		}
		//These if statements will trigger once if we have a single element (i == 0)
			//and every time for a multi-list
		if(*z_index == i){	//z
			vert[0].z = (float)draw_array->layer[*z_index];
		}

		if(*frame_index == i){	//frame
			u0 = draw_array->frame_coord_map[draw_array->frame_coord_key[*frame_index]].x / (float)draw_array->spritesheet->dimensions;
			v0 = draw_array->frame_coord_map[draw_array->frame_coord_key[*frame_index]].y / (float)draw_array->spritesheet->dimensions;
			u1 = u0 + draw_array->animation->frame_width / (float)draw_array->spritesheet->dimensions;
			v1 = v0 + draw_array->animation->frame_height / (float)draw_array->spritesheet->dimensions;
		}

		if(*flip_index == i || *frame_index == i){	//UV
			if(draw_array->flip[*flip_index] & (1 << 0)){	//Is flipped
				vert[0].u = u1;
				vert[0].v = v0;
				vert[1].u = u0;
				vert[1].v = v0;
				vert[2].u = u1;
				vert[2].v = v1;
				vert[3].u = u0;
				vert[3].v = v1;
			}
			else{
				vert[0].u = u0;
				vert[0].v = v0;
				vert[1].u = u1;
				vert[1].v = v0;
				vert[2].u = u0;
				vert[2].v = v1;
				vert[3].u = u1;
				vert[3].v = v1;
			}
		}

		if(*colour_index == i){
			f = draw_array->fade[*colour_index];
			a = (draw_array->colour[*colour_index] >> 24) & 0xFF;

			r = (((draw_array->colour[*colour_index] >> 16) & 0xFF) * f)/255.0f;
			g = (((draw_array->colour[*colour_index] >> 8) & 0xFF) * f)/255.0f;
			b = (((draw_array->colour[*colour_index]) & 0xFF) * f)/255.0f;
			if(draw_array->options & CRAY_COLOUR_ADD){	//If Adding
				vert[0].argb = (a << 24) + 0x00FFFFFF;
			}
			else{	//If Blending
				invf = 255 - f;
				vert[0].argb = (a << 24) + (invf << 16) + (invf << 8) + invf;
			}
			vert[0].oargb = (a << 24) + (r << 16) + (g << 8) + b;
		}

		vert[0].x = trunc(draw_array->coord[i].x);
		vert[0].y = trunc(draw_array->coord[i].y);
		vert[1].x = vert[0].x + trunc(draw_array->animation->frame_width * draw_array->scale[i * multi_scale].x);
		vert[1].y = vert[0].y;
		vert[2].x = vert[0].x;
		vert[2].y = vert[0].y + trunc(draw_array->animation->frame_height * draw_array->scale[i * multi_scale].y);
		vert[3].x = vert[1].x;
		vert[3].y = vert[2].y;

		//Update rotation part if needed
		if(*rotation_index == i){
			angle = fmod(draw_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
			if(angle < 0){angle += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359
			angle = (angle * M_PI) / 180.0f;
		}

		//If we don't want to do rotations (Rotation == 0.0), then skip it
		if(draw_array->rotation[*rotation_index] != 0.0f){

			//Gets the true midpoint
			mid_x = ((vert[1].x - vert[0].x)/2.0f) + vert[0].x;
			mid_y = ((vert[2].y - vert[0].y)/2.0f) + vert[0].y;

			//Update the vert x and y positions
			for(j = 0; j < 4; j++){
				crayon_graphics_rotate_point(mid_x, mid_y, &vert[j].x, &vert[j].y, angle);
			}
		}

		//Apply these to all verts
		for(j = 1; j < 4; j++){
			vert[j].argb = vert[0].argb;
			vert[j].oargb = vert[0].oargb;
			vert[j].z = vert[0].z;
		}

		if(skip){
			skip = 0;
			continue;
		}
		pvr_prim(&vert, sizeof(pvr_vertex_t) * 4);
	}

	return 0;
}

extern uint8_t crayon_graphics_almost_equals(float a, float b, float epsilon){
	return fabs(a-b) < epsilon;
}

extern uint8_t crayon_graphics_draw_text_mono(char * string, const struct crayon_font_mono *fm, uint8_t poly_list_mode,
	float draw_x, float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number){

	float x0 = trunc(draw_x);
	float y0 = trunc(draw_y);
	const float z = layer;

	//x1 and y1 depend on the letter
	float x1 = x0 + fm->char_width * scale_x;
	float y1 = y0 + fm->char_height * scale_y;

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;

	uint8_t texture_format = (((1 << 3) - 1) & (fm->texture_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = fm->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((palette_number) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((palette_number) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fm->dimensions,
		fm->dimensions, fm->texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	float prop_width = (float)fm->char_width / fm->dimensions;
	float prop_height = (float)fm->char_height / fm->dimensions;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line
			x0 = trunc(draw_x);
			x1 = x0 + fm->char_width * scale_x;
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

extern uint8_t crayon_graphics_draw_text_prop(char * string, const struct crayon_font_prop *fp, uint8_t poly_list_mode,
	float draw_x, float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number){

	float x0 = trunc(draw_x);
	float y0 = trunc(draw_y);
	const float z = layer;
	float x1 = x0;
	float y1 = y0 + fp->char_height * scale_y;
	float v0 = 0;
	float v1 = 0;
	float percent_height = (float)fp->char_height / fp->dimensions;

	// float percent_width;
	float u0, u1;

	pvr_sprite_cxt_t context;

	uint8_t texture_format = (((1 << 3) - 1) & (fp->texture_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = fp->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((palette_number) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((palette_number) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fp->dimensions,
		fp->dimensions, fp->texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line
			x0 = trunc(draw_x);
			x1 = x0;
			y0 = y1;
			y1 += fp->char_height * scale_y;
			i++;
			continue;
		}
		uint8_t distance_from_space = string[i] - ' ';

		x1 += fp->char_width[distance_from_space] * scale_x;	//get the width of the display char

		u0 = (float)fp->char_x_coord[distance_from_space] / fp->dimensions;
		u1 = u0 + ((float)fp->char_width[distance_from_space] / fp->dimensions);

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

extern uint8_t crayon_graphics_valid_string(const char *string, uint8_t num_chars){
	uint16_t i = 0;
	while(string[i] != '\0'){
		if(string[i] == '\n'){
			i++;
			continue;
		}
		if(string[i] < ' ' || string[i] > ' ' + num_chars - 1){	//Outside the fontsheet's charset
			return 1;
		}
		i++;
	}

	return 0;
}

extern uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char * string, uint8_t newlines){
	if(!newlines){
		return strlen(string) * fm->char_width;	//Since all chars are the same width, we can just do this
	}

	uint16_t current_length = 0;
	uint16_t best_length = 0;
	int i = 0;
	while(1){
		if(string[i] == '\0'){
			if(current_length > best_length){
				best_length = current_length;
			}
			i++;
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line (Doesn't seem to work right)
			if(current_length > best_length){
				best_length = current_length;
			}
			current_length = 0;
			i++;
			continue;
		}

		current_length += fm->char_width;
		i++;
	}

	return best_length;
}

extern uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char * string, uint8_t newlines){
	uint16_t current_length = 0;
	uint16_t best_length = 0;

	int i = 0;
	while(1){
		if(string[i] == '\0'){
			if(current_length > best_length){
				best_length = current_length;
			}
			current_length = 0;
			break;
		}
		if(string[i] == '\n'){
			if(newlines){
				if(current_length > best_length){
					best_length = current_length;
				}
				current_length = 0;
			}
			i++;
			continue;
		}

		uint8_t distance_from_space = string[i] - ' ';
		current_length += fp->char_width[distance_from_space];

		i++;
	}

	return best_length;
}

extern void crayon_graphics_rotate_point(float center_x, float center_y, float * orbit_x, float * orbit_y, float radians){
	float sin_theta = sin(radians);
	float cos_theta = cos(radians);
	float new_x = (cos_theta * (*orbit_x - center_x)) - (sin_theta * (*orbit_y - center_y)) + center_x;
	*orbit_y = (sin_theta * (*orbit_x - center_x)) + (cos_theta * (*orbit_y - center_y)) + center_y;
	*orbit_x = new_x;
	return;

}
