#include "graphics.h"

//There are 4 palettes for 8BPP and 64 palettes for 4BPP
extern uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp){
	if(cp->palette_id < 0 || cp->bpp < 0 || cp->bpp * cp->palette_id >= 1024){	//Invalid format/palette not properly set
		return 1;
	}
	int entries;
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

extern void crayon_graphics_frame_coordinates(const struct crayon_animation *anim, uint16_t *frame_x, uint16_t *frame_y, uint8_t frame){
	int framesPerRow = anim->animation_sheet_width/anim->animation_frame_width;
	int colNum = frame%framesPerRow; //Gets the column (Zero indexed)
	int rowNum = frame/framesPerRow;  //Gets the row (Zero indexed)

	*frame_x = anim->animation_x + (colNum) * anim->animation_frame_width;
	*frame_y = anim->animation_y + (rowNum) * anim->animation_frame_height;

	return;
}

//uint8_t poly_list_mode

extern void crayon_graphics_draw_untextured_poly(uint16_t draw_x, uint16_t draw_y, uint16_t draw_z, uint16_t dim_x,
  uint16_t dim_y, uint32_t colour, uint8_t poly_list_mode){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;

	pvr_poly_cxt_col(&cxt, poly_list_mode);
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
extern void crayon_graphics_draw_untextured_array(crayon_untextured_array_t *poly_array){
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


extern uint8_t crayon_graphics_draw_sprite(const struct crayon_spritesheet *ss,
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
	uint8_t texture_format = (((1 << 3) - 1) & (ss->spritesheet_format >> (28 - 1)));	//Gets the Pixel format
																													//https://github.com/tvspelsfreak/texconv
	int textureformat = ss->spritesheet_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((paletteNumber) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((paletteNumber) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, textureformat, ss->spritesheet_dims,
		ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);

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

// 	//Use these to merge the two palette if's into one
// 	// PVR_TXRFMT_PAL4BPP   (5 << 27)
// 	// PVR_TXRFMT_PAL8BPP   (6 << 27)
// 	//(6 << 27) == 110 << 27 == 11000 00000 00000 00000 00000 00000
// 	//(5 << 27) == 110 << 27 == 10100 00000 00000 00000 00000 00000
// 	//paletteNumber ranges from 0 to 63 or 0 to 15 depending on BPP
// 	//   (8BPP) 3  << 25 == 00011 00000 00000 00000 00000 00000
// 	//   (4BPP) 63 << 21 == 00011 11110 00000 00000 00000 00000
// 	//PVR_TXRFMT_PAL4BPP == 10100 00000 00000 00000 00000 00000
// 	//PVR_TXRFMT_PAL8BPP == 11000 00000 00000 00000 00000 00000

// 	//pvr_sprite_cxt_txr(context, TR/OP/PT list, int textureformat, dimX, dimY, texture, filter)

// 	//Once the spritesheet/font format is fixed:
// 	//uint8_t filter = PVR_FILTER_NONE;	//Have param to change this possibly
// 	//int textureformat = ss->spritesheet_format;
// 	//if(4BPP){
// 	//		textureformat |= ((paletteNumber) << 21); 
// 	// }
// 	//if(8BPP){
// 	//		textureformat |= ((paletteNumber) << 25); 
// 	// }
// 	//pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, textureformat,
// 	//	 ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, filter);


//Need to come back and do rotations and maybe colour?
extern uint8_t crayon_graphics_draw_sprites(crayon_textured_array_t *sprite_array, uint8_t poly_list_mode){

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;
	uint8_t texture_format = (((1 << 3) - 1) & (sprite_array->spritesheet->spritesheet_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = sprite_array->spritesheet->spritesheet_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((sprite_array->palette->palette_id) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((sprite_array->palette->palette_id) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, sprite_array->spritesheet->spritesheet_dims,
		sprite_array->spritesheet->spritesheet_dims, sprite_array->spritesheet->spritesheet_texture, sprite_array->filter);

	pvr_sprite_txr_t vert = {
		.flags = PVR_CMD_VERTEX_EOL
	};

	if(!(sprite_array->options & (1 << 0))){	//All share the same z
		vert.az = sprite_array->draw_z[0];
		vert.bz = sprite_array->draw_z[0];
		vert.cz = sprite_array->draw_z[0];
	}
	if(!(sprite_array->options & (1 << 1))){	//All share the same frame
		u0 = sprite_array->frame_coord_map[sprite_array->frame_coord_keys[0]] / (float)sprite_array->spritesheet->spritesheet_dims;
		v0 = sprite_array->frame_coord_map[sprite_array->frame_coord_keys[0] + 1] / (float)sprite_array->spritesheet->spritesheet_dims;
		u1 = u0 + sprite_array->animation->animation_frame_width / (float)sprite_array->spritesheet->spritesheet_dims;
		v1 = v0 + sprite_array->animation->animation_frame_height / (float)sprite_array->spritesheet->spritesheet_dims;
		vert.auv = PVR_PACK_16BIT_UV(u0, v0);
		vert.buv = PVR_PACK_16BIT_UV(u1, v0);
		vert.cuv = PVR_PACK_16BIT_UV(u1, v1);
	}

	//Assistant vars. Just making them so its easier to read and maybe less mem accesses
	uint8_t multi_scales = !!(sprite_array->options & (1 << 2));

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));
	uint16_t i;
	for(i = 0; i < sprite_array->num_sprites; i++){
		//We floor the values since we're doing 2D and they'll look messed up if we have position "11.5", however scales can mess this up
		vert.ax = floor(sprite_array->positions[2 * i]);
		vert.ay = floor(sprite_array->positions[(2 * i) + 1]);
		vert.bx = floor(sprite_array->positions[2 * i] +
			(sprite_array->animation->animation_frame_width * sprite_array->scales[2 * i * multi_scales]));
		vert.by = floor(sprite_array->positions[(2 * i) + 1]);
		vert.cx = floor(sprite_array->positions[2 * i] +
			(sprite_array->animation->animation_frame_width * sprite_array->scales[2 * i * multi_scales]));
		vert.cy = floor(sprite_array->positions[(2 * i) + 1] +
			(sprite_array->animation->animation_frame_height * sprite_array->scales[(2 * i * multi_scales) + 1]));
		vert.dx = floor(sprite_array->positions[2 * i]);
		vert.dy = floor(sprite_array->positions[(2 * i) + 1] +
			(sprite_array->animation->animation_frame_height * sprite_array->scales[(2 * i * multi_scales) + 1]));

		if(sprite_array->options & (1 << 0)){	//z
			vert.az = (float)sprite_array->draw_z[i];
			vert.bz = (float)sprite_array->draw_z[i];
			vert.cz = (float)sprite_array->draw_z[i];
		}

		if(sprite_array->options & (1 << 1)){	//frame
			u0 = sprite_array->frame_coord_map[(2 * sprite_array->frame_coord_keys[i])] / (float)sprite_array->spritesheet->spritesheet_dims;
			v0 = sprite_array->frame_coord_map[(2 * sprite_array->frame_coord_keys[i]) + 1] / (float)sprite_array->spritesheet->spritesheet_dims;
			u1 = u0 + sprite_array->animation->animation_frame_width / (float)sprite_array->spritesheet->spritesheet_dims;
			v1 = v0 + sprite_array->animation->animation_frame_height / (float)sprite_array->spritesheet->spritesheet_dims;
			vert.auv = PVR_PACK_16BIT_UV(u0, v0);
			vert.buv = PVR_PACK_16BIT_UV(u1, v0);
			vert.cuv = PVR_PACK_16BIT_UV(u1, v1);
		}

		if(sprite_array->options & (1 << 3)){	//flips (Unimplemented)
			;
		}
		else{
			;
		}

		if(sprite_array->options & (1 << 4)){	//rotations (Unimplemented)
			;
		}
		else{
			;
		}

		if(sprite_array->options & (1 << 5)){	//colour (Unimplemented)
			;
		}
		else{
			;
		}

		pvr_prim(&vert, sizeof(vert));
	}

	return 0;
}

//I'll come back to this later
extern uint8_t crayon_graphics_draw_polys(crayon_textured_array_t *sprite_array, uint8_t poly_list_mode){
	return 0;
}

extern uint8_t crayon_graphics_draw_text_mono(const struct crayon_font_mono *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber, char * string){

	float x0 = draw_x;
	float y0 = draw_y;
	const float z = draw_z;

	//x1 and y1 depend on the letter
	float x1 = draw_x + fm->char_width * scale_x;
	float y1 = draw_y + fm->char_height * scale_y;

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;

	uint8_t texture_format = (((1 << 3) - 1) & (fm->texture_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = fm->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((paletteNumber) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((paletteNumber) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fm->fontsheet_dim,
		fm->fontsheet_dim, fm->fontsheet_texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	float prop_width = (float)fm->char_width / fm->fontsheet_dim;
	float prop_height = (float)fm->char_height / fm->fontsheet_dim;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line
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

extern uint8_t crayon_graphics_draw_text_prop(const struct crayon_font_prop *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber, char * string){

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

	uint8_t texture_format = (((1 << 3) - 1) & (fp->texture_format >> (28 - 1)));	//Gets the Pixel format
																												//https://github.com/tvspelsfreak/texconv
	int textureformat = fp->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((paletteNumber) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((paletteNumber) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fp->fontsheet_dim,
		fp->fontsheet_dim, fp->fontsheet_texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line
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
