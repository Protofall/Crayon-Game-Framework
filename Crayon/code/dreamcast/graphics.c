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

extern float crayon_graphics_get_draw_element_width(const crayon_sprite_array_t *sprite_array, uint8_t id){
	if(!(sprite_array->options & CRAY_MULTI_SCALE)){id = 0;}	//When there's only one scale
	if(sprite_array->options & CRAY_HAS_TEXTURE){
		return sprite_array->animation->frame_width * sprite_array->scale[id].x;
	}
	else{
		return sprite_array->scale[id].x;
	}
}

extern float crayon_graphics_get_draw_element_height(const crayon_sprite_array_t *sprite_array, uint8_t id){
	if(!(sprite_array->options & CRAY_MULTI_SCALE)){id = 0;}	//When there's only one scale
	if(sprite_array->options & CRAY_HAS_TEXTURE){
		return sprite_array->animation->frame_height * sprite_array->scale[id].y;
	}
	else{
		return sprite_array->scale[id].y;
	}
}

//Right now on Dreamcast I have no idea how to check the current dimensions, so I just assume this. Its usually right
extern uint32_t crayon_graphics_get_window_width(){
	return 640;
}

extern uint32_t crayon_graphics_get_window_height(){
	return 480;
}


//---------------------------------------------------------------------//


//draw_mode == ---- ---M
//M is for draw mode (1 for enhanced, 0 for simple)
extern int8_t crayon_graphics_draw_sprites(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t draw_mode){
	if(sprite_array->list_size == 0){return 2;}	//Don't render nothing
	crayon_viewport_t default_camera;
	if(camera == NULL){	//No Camera, use the default one
		crayon_memory_init_camera(&default_camera, (vec2_f_t){0, 0},
			(vec2_u16_t){crayon_graphics_get_window_width(), crayon_graphics_get_window_height()},
			(vec2_u16_t){0, 0},
			(vec2_u16_t){crayon_graphics_get_window_width(), crayon_graphics_get_window_height()}, 1);
		camera = &default_camera;
	}

	if(sprite_array->options & CRAY_HAS_TEXTURE){	//Textured
		if(draw_mode & CRAY_DRAW_ENHANCED){
			return crayon_graphics_draw_sprites_enhanced(sprite_array, camera, poly_list_mode);
		}
		return crayon_graphics_draw_sprites_simple(sprite_array, camera, poly_list_mode);
	}
	return crayon_graphics_draw_untextured_array(sprite_array, camera, poly_list_mode);
}

//DELETE THIS LATER
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

extern uint8_t crayon_graphics_draw_sprites_enhanced(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode){
	
	float u0, v0, u1, v1;
	u0 = 0; v0 = 0; u1 = 0; v1 = 0;	//Needed if you want to prevent a bunch of compiler warnings...

	uint8_t texture_format = (((1 << 3) - 1) & (sprite_array->spritesheet->texture_format >> (28 - 1)));	//Gets the Pixel format
																										//https://github.com/tvspelsfreak/texconv
	int textureformat = sprite_array->spritesheet->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((sprite_array->palette->palette_id) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((sprite_array->palette->palette_id) << 25);	//Update the later to use KOS' macros
	}

	// uint8_t crop_edges = (1 << 4) - 1;	//---- BRTL
	// 									//---- 1111 (Crop on all edges)

	// //DELETE THIS LATER. A basic optimisation for now
	// if(camera->window_x == 0){crop_edges = crop_edges & (~ (1 << 0));}
	// if(camera->window_y == 0){crop_edges = crop_edges & (~ (1 << 1));}
	// if(camera->window_width == crayon_graphics_get_window_width()){crop_edges = crop_edges & (~ (1 << 2));}
	// if(camera->window_height == crayon_graphics_get_window_height()){crop_edges = crop_edges & (~ (1 << 3));}

	//Used for cropping
	// uint8_t bounds = 0;
	// uint8_t cropped = 0;

	//This var exist so we don't need to worry about constantly floor-ing the camera's world points
		//The sprite points are also floor-ed before calculations are done
	vec2_f_t world_coord = (vec2_f_t){floor(camera->world_x),floor(camera->world_y)};

	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_txr(&cxt, poly_list_mode, textureformat,
		sprite_array->spritesheet->texture_width, sprite_array->spritesheet->texture_height,
		sprite_array->spritesheet->texture, sprite_array->filter);
	pvr_poly_compile(&hdr, &cxt);
	hdr.cmd |= 4;	//Enable oargb
	pvr_prim(&hdr, sizeof(hdr));

	pvr_vertex_t vert[4];	//4 verts per sprite
	vec2_f_t rotated_values;

	//Easily lets us use the right index for each array
		//That way 1-length arrays only get calculated once and each element for a multi list is calculated
	uint16_t *rotation_index, *flip_index, *frame_index, *colour_index;
	uint8_t multi_scale = !!(sprite_array->options & CRAY_MULTI_SCALE);
	uint16_t zero = 0;
	float angle = 0;
	float mid_x = 0;
	float mid_y = 0;
	uint8_t skip = 0;

	uint16_t i, j;	//Indexes

	if(sprite_array->options & CRAY_MULTI_FRAME){frame_index = &i;}
	else{frame_index = &zero;}

	if(sprite_array->options & CRAY_MULTI_FLIP){flip_index = &i;}
	else{flip_index = &zero;}

	if(sprite_array->options & CRAY_MULTI_ROTATE){rotation_index = &i;}
	else{rotation_index = &zero;}

	if(sprite_array->options & CRAY_MULTI_COLOUR){colour_index = &i;}
	else{colour_index = &zero;}

	//Set the flags
	for(j = 0; j < 3; j++){
		vert[j].flags = PVR_CMD_VERTEX;
	}
	vert[3].flags = PVR_CMD_VERTEX_EOL;

	uint8_t invf, f, a, r, g, b;
	for(i = 0; i < sprite_array->list_size; i++){
		if(sprite_array->colour[*colour_index] >> 24 == 0){	//Don't draw alpha-less stuff
			if(i != 0){continue;}	//For the first element, we need to initialise our vars, otherwise we just skip to the next element
			skip = 1;
		}

		if(sprite_array->visible[i] == 0){
			if(i != 0){continue;}
			else{skip = 1;}	//We need the defaults to be set on first loop
		}

		//These if statements will trigger once if we have a single element (i == 0)
			//and every time for a multi-list

		if(*frame_index == i){	//frame
			u0 = sprite_array->frame_uv[sprite_array->frame_id[*frame_index]].x / (float)sprite_array->spritesheet->texture_width;
			v0 = sprite_array->frame_uv[sprite_array->frame_id[*frame_index]].y / (float)sprite_array->spritesheet->texture_height;
			u1 = u0 + sprite_array->animation->frame_width / (float)sprite_array->spritesheet->texture_width;
			v1 = v0 + sprite_array->animation->frame_height / (float)sprite_array->spritesheet->texture_height;
		}

		if(*flip_index == i || *frame_index == i){	//UV
			if(sprite_array->flip[*flip_index] & (1 << 0)){	//Is flipped
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
			f = sprite_array->fade[*colour_index];
			a = (sprite_array->colour[*colour_index] >> 24) & 0xFF;

			r = (((sprite_array->colour[*colour_index] >> 16) & 0xFF) * f)/255.0f;
			g = (((sprite_array->colour[*colour_index] >> 8) & 0xFF) * f)/255.0f;
			b = (((sprite_array->colour[*colour_index]) & 0xFF) * f)/255.0f;
			if(sprite_array->options & CRAY_COLOUR_ADD){	//If Adding
				vert[0].argb = (a << 24) + 0x00FFFFFF;
			}
			else{	//If Blending
				invf = 255 - f;
				vert[0].argb = (a << 24) + (invf << 16) + (invf << 8) + invf;
			}
			vert[0].oargb = (a << 24) + (r << 16) + (g << 8) + b;
		}

		//Update rotation part if needed
		if(*rotation_index == i){
			angle = fmod(sprite_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
			if(angle < 0){angle += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359
			angle = (angle * M_PI) / 180.0f;
		}

		if(skip){skip = 0; continue;}

		vert[0].x = floor((floor(sprite_array->coord[i].x) - world_coord.x) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
		vert[0].y = floor((floor(sprite_array->coord[i].y) - world_coord.y) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
		vert[1].x = vert[0].x + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_width / (float)camera->world_width));
		vert[1].y = vert[0].y;
		vert[2].x = vert[0].x;
		vert[2].y = vert[0].y + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_height / (float)camera->world_height));
		vert[3].x = vert[1].x;
		vert[3].y = vert[2].y;

		vert[0].z = (float)sprite_array->layer[i];

		//If we don't want to do rotations (Rotation == 0.0), then skip it
		if(sprite_array->rotation[*rotation_index] != 0.0f){

			//Gets the true midpoint
			mid_x = ((vert[1].x - vert[0].x)/2.0f) + vert[0].x;
			mid_y = ((vert[2].y - vert[0].y)/2.0f) + vert[0].y;

			//Update the vert x and y positions
			for(j = 0; j < 4; j++){
				rotated_values = crayon_graphics_rotate_point((vec2_f_t){mid_x, mid_y}, (vec2_f_t){vert[j].x, vert[j].y}, angle);
				vert[j].x = rotated_values.x;
				vert[j].y = rotated_values.y;
			}
		}

		//Apply these to all verts
		for(j = 1; j < 4; j++){
			vert[j].argb = vert[0].argb;
			vert[j].oargb = vert[0].oargb;
			vert[j].z = vert[0].z;
		}

		pvr_prim(&vert, sizeof(pvr_vertex_t) * 4);
	}

	return 0;
}

extern uint8_t crayon_graphics_draw_untextured_array(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode){

	// uint8_t crop_edges = (1 << 4) - 1;	//---- BRTL
	// 									//---- 1111 (Crop on all edges)

	// //DELETE THIS LATER. A basic optimisation for now
	// if(camera->window_x == 0){crop_edges = crop_edges & (~ (1 << 0));}
	// if(camera->window_y == 0){crop_edges = crop_edges & (~ (1 << 1));}
	// if(camera->window_width == crayon_graphics_get_window_width()){crop_edges = crop_edges & (~ (1 << 2));}
	// if(camera->window_height == crayon_graphics_get_window_height()){crop_edges = crop_edges & (~ (1 << 3));}

	//Used for cropping
	// uint8_t bounds = 0;
	// uint8_t cropped = 0;

	//This var exist so we don't need to worry about constantly floor-ing the camera's world points
		//The sprite points are also floor-ed before calculations are done
	vec2_f_t world_coord = (vec2_f_t){floor(camera->world_x),floor(camera->world_y)};

	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert[4];
	vec2_f_t rotated_values;

	pvr_poly_cxt_col(&cxt, poly_list_mode);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	//All this just for rotations
	uint16_t *rotation_index, *colour_index;
	uint8_t multi_dim = !!(sprite_array->options & CRAY_MULTI_DIM);
	uint16_t zero = 0;
	float angle = 0;
	float mid_x = 0;
	float mid_y = 0;
	uint8_t skip = 0;

	uint16_t i, j;
	//--CR -D--
	if(sprite_array->options & CRAY_MULTI_ROTATE){rotation_index = &i;}
	else{rotation_index = &zero;}
	if(sprite_array->options & CRAY_MULTI_COLOUR){colour_index = &i;}
	else{colour_index = &zero;}

	for(i = 0; i < 3; i++){
		vert[i].flags = PVR_CMD_VERTEX;
	}
	vert[3].flags = PVR_CMD_VERTEX_EOL;

	//Unused param, set to 0
	vert[0].oargb = 0;
	for(i = 1; i < 4; i++){
		vert[i].oargb = vert[0].oargb;
	}

	for(i = 0; i < sprite_array->list_size; i++){
		if(sprite_array->colour[*colour_index] >> 24 == 0){	//Don't draw alpha-less stuff
			if(i != 0){continue;}	//For the first element, we need to initialise our vars, otherwise we just skip to the next element
			skip = 1;
		}

		if(sprite_array->visible[i] == 0){
			if(i != 0){continue;}
			else{skip = 1;}	//We need the defaults to be set on first loop
		}

		//Update rotation part if needed
		if(*rotation_index == i){
			angle = fmod(sprite_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
			if(angle < 0){angle += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359
			angle = (angle * M_PI) / 180.0f;	//Convert from degrees to ratians
		}

		if(*colour_index == i){
			vert[0].argb = sprite_array->colour[i];
		}

		if(skip){skip = 0; continue;}

		vert[0].z = sprite_array->layer[i];

		vert[0].x = floor((floor(sprite_array->coord[i].x) - world_coord.x) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
		vert[0].y = floor((floor(sprite_array->coord[i].y) - world_coord.y) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
		vert[1].x = vert[0].x + floor(sprite_array->scale[i * multi_dim].x * (camera->window_width / (float)camera->world_width));
		vert[1].y = vert[0].y;
		vert[2].x = vert[0].x;
		vert[2].y = vert[0].y + floor(sprite_array->scale[i * multi_dim].y * (camera->window_height / (float)camera->world_height));
		vert[3].x = vert[1].x;
		vert[3].y = vert[2].y;

		//Rotate the poly
		if(sprite_array->rotation[*rotation_index] != 0.0f){
			//Gets the midpoint
			mid_x = ((vert[1].x - vert[0].x)/2.0f) + vert[0].x;
			mid_y = ((vert[2].y - vert[0].y)/2.0f) + vert[0].y;

			//Rotate the verts around the midpoint
			for(j = 0; j < 4; j++){
				rotated_values = crayon_graphics_rotate_point((vec2_f_t){mid_x, mid_y}, (vec2_f_t){vert[j].x, vert[j].y}, angle);
				vert[j].x = rotated_values.x;
				vert[j].y = rotated_values.y;
			}
		}

		for(j = 1; j < 4; j++){
			vert[j].z = vert[0].z;
			vert[j].argb = vert[0].argb;
		}

		pvr_prim(&vert, sizeof(pvr_vertex_t) * 4);
	}
	return 0;
}

extern uint8_t crayon_graphics_draw_sprites_simple(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode){

	uint8_t crop_edges = (1 << 4) - 1;	//---- BRTL
										//---- 1111 (Crop on all edges)

	//DELETE THIS LATER. A basic optimisation for now
	if(camera->window_x == 0){crop_edges = crop_edges & (~ (1 << 0));}
	if(camera->window_y == 0){crop_edges = crop_edges & (~ (1 << 1));}
	if(camera->window_width == crayon_graphics_get_window_width()){crop_edges = crop_edges & (~ (1 << 2));}
	if(camera->window_height == crayon_graphics_get_window_height()){crop_edges = crop_edges & (~ (1 << 3));}

	//Used for cropping
	uint8_t bounds = 0;
	uint8_t cropped = 0;
	uint8_t flip_val = 0;
	uint8_t rotation_val = 0;

	//This var exist so we don't need to worry about constantly floor-ing the camera's world points
		//The sprite points are also floor-ed before calculations are done
	vec2_f_t world_coord = (vec2_f_t){floor(camera->world_x),floor(camera->world_y)};

	//Used in calcuating the new UV
	float texture_offset;
	float texture_divider;
	vec2_f_t selected_vert;
	uint8_t uv_index;

	float uvs[4] = {0};	//u0, v0, u1, v1 (Set to zero to avoid compiler warnings)
	vec2_f_t camera_verts[4], sprite_verts[4];
	camera_verts[0] = (vec2_f_t){camera->window_x,camera->window_y};
	camera_verts[1] = (vec2_f_t){camera->window_x+camera->window_width,camera->window_y};
	camera_verts[2] = (vec2_f_t){camera->window_x,camera->window_y+camera->window_height};
	camera_verts[3] = (vec2_f_t){camera->window_x+camera->window_width,camera->window_y+camera->window_height};

	pvr_sprite_cxt_t context;
	uint8_t texture_format = (((1 << 3) - 1) & (sprite_array->spritesheet->texture_format >> (28 - 1)));	//Gets the Pixel format
																										//https://github.com/tvspelsfreak/texconv
	int textureformat = sprite_array->spritesheet->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((sprite_array->palette->palette_id) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((sprite_array->palette->palette_id) << 25);	//Update the later to use KOS' macros
	}
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, sprite_array->spritesheet->texture_width,
		sprite_array->spritesheet->texture_height, sprite_array->spritesheet->texture, sprite_array->filter);

	pvr_sprite_txr_t vert = {
		.flags = PVR_CMD_VERTEX_EOL
	};

	//Easily lets us use the right index for each array
		//That way 1-length arrays only get calculated once and each element for a multi list is calculated
	uint16_t *rotation_index, *flip_index, *frame_index;
	uint16_t i;	//The main loop's index
	uint16_t zero = 0;
	float rotation_under_360 = 0;

	if(sprite_array->options & CRAY_MULTI_FRAME){
		frame_index = &i;
	}
	else{
		frame_index = &zero;
	}

	if(sprite_array->options & CRAY_MULTI_FLIP){
		flip_index = &i;
	}
	else{
		flip_index = &zero;
	}

	if(sprite_array->options & CRAY_MULTI_ROTATE){
		rotation_index = &i;
	}
	else{
		rotation_index = &zero;
	}

	uint8_t multi_scale = !!(sprite_array->options & CRAY_MULTI_SCALE);

	uint8_t multi_flip = !!(sprite_array->options & CRAY_MULTI_FLIP);
	uint8_t multi_rotate = !!(sprite_array->options & CRAY_MULTI_ROTATE);
	uint8_t multi_frame = !!(sprite_array->options & CRAY_MULTI_FRAME);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));
	for(i = 0; i < sprite_array->list_size; i++){
		//These if statements will trigger once if we have a single element (i == 0)
			//and every time for a multi-list
			//and some of them trigger if we just cropped a UV

		if(sprite_array->visible[i] == 0){
			if(i != 0){continue;}	//We need the defaults to be set on first loop
		}

		if(*frame_index == i || cropped){	//frame
			uvs[0] = sprite_array->frame_uv[sprite_array->frame_id[*frame_index]].x / (float)sprite_array->spritesheet->texture_width;
			uvs[1] = sprite_array->frame_uv[sprite_array->frame_id[*frame_index]].y / (float)sprite_array->spritesheet->texture_height;
			uvs[2] = uvs[0] + sprite_array->animation->frame_width / (float)sprite_array->spritesheet->texture_width;
			uvs[3] = uvs[1] + sprite_array->animation->frame_height / (float)sprite_array->spritesheet->texture_height;
			cropped = 0;	//Reset cropped from previous element
		}

		//Basically enter if first element or either the flip/rotate/frame changed
			//The multi_blah's are there to prevent checks on draw params that aren't multi/won't change
		if(i == 0 || (multi_flip && (sprite_array->flip[i] != sprite_array->flip[i - 1])) ||
			(multi_rotate && (sprite_array->rotation[i] != sprite_array->rotation[i - 1])) ||
			(multi_frame &&
			((sprite_array->frame_uv[i].x != sprite_array->frame_uv[i - 1].x) ||
			(sprite_array->frame_uv[i].y != sprite_array->frame_uv[i - 1].y)))
			){

			//Is flipped?
			if(sprite_array->flip[*flip_index] & (1 << 0)){flip_val = 1;}
			else{flip_val = 0;}

			//Don't bother doing extra calculations
			if(sprite_array->rotation[*rotation_index] != 0){
				rotation_under_360 = fmod(sprite_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
				if(rotation_under_360 < 0){rotation_under_360 += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359

				//For sprite mode we can't simply "rotate" the verts, instead we need to change the uv
				if(crayon_graphics_almost_equals(rotation_under_360, 90.0, 45.0)){
					rotation_val = 1;
				}
				else if(crayon_graphics_almost_equals(rotation_under_360, 180.0, 45.0)){
					rotation_val = 2;
				}
				else if(crayon_graphics_almost_equals(rotation_under_360, 270.0, 45.0)){
					rotation_val = 3;
				}
				else{rotation_val = 0;}
			}
			else{rotation_val = 0;}
		}

		//These blocks act as the rotation
		if(rotation_val == 0){
			//NOTE: we don't need to floor the camera's window vars because they're all ints
			vert.ax = floor((floor(sprite_array->coord[i].x) - world_coord.x) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.ay = floor((floor(sprite_array->coord[i].y) - world_coord.y) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.bx = vert.ax + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_width / (float)camera->world_width));
			vert.by = vert.ay;
			vert.cx = vert.bx;
			vert.cy = vert.ay + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_height / (float)camera->world_height));
			vert.dx = vert.ax;
			vert.dy = vert.cy;
		}
		else if(rotation_val == 1){
			//Both vars are uint16_t and lengths can't be negative or more than 1024 (Largest texture size for DC)
				//Therfore storing the result in a int16_t is perfectly fine
			int16_t diff = sprite_array->animation->frame_width - sprite_array->animation->frame_height;

			vert.dx = floor((floor(sprite_array->coord[i].x) - world_coord.x + (sprite_array->scale[i * multi_scale].y * diff / 2)) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.dy = floor((floor(sprite_array->coord[i].y) - world_coord.y - (sprite_array->scale[i * multi_scale].x * diff / 2)) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.ax = vert.dx + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_width / (float)camera->world_width));
			vert.ay = vert.dy;
			vert.bx = vert.ax;
			vert.by = vert.dy + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_height / (float)camera->world_height));
			vert.cx = vert.dx;
			vert.cy = vert.by;
		}
		else if(rotation_val == 2){
			vert.cx = floor((floor(sprite_array->coord[i].x) - world_coord.x) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.cy = floor((floor(sprite_array->coord[i].y) - world_coord.y) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.dx = vert.cx + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_width / (float)camera->world_width));
			vert.dy = vert.cy;
			vert.ax = vert.dx;
			vert.ay = vert.cy + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_height / (float)camera->world_height));
			vert.bx = vert.cx;
			vert.by = vert.ay;
		}
		else if(rotation_val == 3){
			int16_t diff = sprite_array->animation->frame_width - sprite_array->animation->frame_height;

			vert.bx = floor((floor(sprite_array->coord[i].x) - world_coord.x + (sprite_array->scale[i * multi_scale].y * diff / 2)) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.by = floor((floor(sprite_array->coord[i].y) - world_coord.y - (sprite_array->scale[i * multi_scale].x * diff / 2)) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.cx = vert.bx + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_width / (float)camera->world_width));
			vert.cy = vert.by;
			vert.dx = vert.cx;
			vert.dy = vert.by + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_height / (float)camera->world_height));
			vert.ax = vert.bx;
			vert.ay = vert.dy;
		}

		//The first element if invisible now skips cropping and rendering
		if(i == 0 && sprite_array->visible[i] == 0){continue;}

		vert.az = (float)sprite_array->layer[i];
		vert.bz = (float)sprite_array->layer[i];
		vert.cz = (float)sprite_array->layer[i];

		//Verts c and d (Or 2 and 3) are swapped so its in Z order instead of "Backwards C" order
		sprite_verts[0] = crayon_graphics_get_sprite_vert(vert, (4 + 0 - rotation_val) % 4);
		sprite_verts[1] = crayon_graphics_get_sprite_vert(vert, (4 + 1 - rotation_val) % 4);
		sprite_verts[2] = crayon_graphics_get_sprite_vert(vert, (4 + 3 - rotation_val) % 4);
		sprite_verts[3] = crayon_graphics_get_sprite_vert(vert, (4 + 2 - rotation_val) % 4);

		//If OOB then don't draw
		if(crayon_graphics_check_oob(camera_verts, sprite_verts, CRAY_DRAW_SIMPLE)){continue;}

		//If we don't need to crop at all, don't both doing the checks. bounds is zero by default
		if(crop_edges){
			bounds = crayon_graphics_check_intersect(camera_verts, sprite_verts);
			bounds &= crop_edges;		//To simplify the if checks
		}

		//Replace below with function calls such as
		//func(&uvs[], flip, rotate, side, camera, scale, animation, &vert)
		//and it modifies the uv element if required
			//Will need to make a function to get last param of crayon_graphics_set_sprite_vert_x/y
		if(bounds & (1 << 0)){	//Left side
			//Get the vert that's currently on the left side
			uv_index = crayon_get_uv_index(0, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 0 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(0, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(0, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(2, rotation_val, flip_val)] - uvs[uv_index]);

			// if(i == 0 && __GRAPHICS_DEBUG_VARIABLES[0] == 1){
			// 	__GRAPHICS_DEBUG_VARIABLES[1] = uvs[uv_index];
			// 	__GRAPHICS_DEBUG_VARIABLES[2] = uv_index;
			// 	__GRAPHICS_DEBUG_VARIABLES[3] = selected_vert.x;
			// 	__GRAPHICS_DEBUG_VARIABLES[4] = selected_vert.y;
			// 	__GRAPHICS_DEBUG_VARIABLES[5] = texture_divider;	//Is at 8
			// 	__GRAPHICS_DEBUG_VARIABLES[6] = texture_offset;		//Is at zero
			// }

			//Set the vert
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 0 - rotation_val) % 4, camera->window_x);
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 3 - rotation_val) % 4, camera->window_x);
		}
		if(bounds & (1 << 1)){	//Top side
			//Get uv thats on top side
			uv_index = crayon_get_uv_index(1, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 1 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(1, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(1, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(3, rotation_val, flip_val)] - uvs[uv_index]);
			// uvs[uv_index] += (texture_offset / texture_divider) / sprite_array->scale[i * multi_scale].y;

			//If we remove the scale from texture_offset
			// uvs[uv_index] += texture_offset / (texture_divider * sprite_array->scale[i * multi_scale].y * sprite_array->scale[i * multi_scale].y);

			//Set the vert
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 1 - rotation_val) % 4, camera->window_y);
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 0 - rotation_val) % 4, camera->window_y);
		}
		if(bounds & (1 << 2)){	//Right side
			//I don't fully understand why we use the magic number 2, maybe its the opposite of 0 (0 == R, 2 == L)

			//Get the vert that's currently on the right side
			uv_index = crayon_get_uv_index(2, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 2 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(2, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(2, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(0, rotation_val, flip_val)] - uvs[uv_index]);

			//Set the vert
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 2 - rotation_val) % 4, camera->window_x + camera->window_width);
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 1 - rotation_val) % 4, camera->window_x + camera->window_width);
		}
		if(bounds & (1 << 3)){	//Bottom side
			//Get the vert that's currently on the bottom side
			uv_index = crayon_get_uv_index(3, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 3 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(3, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(3, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(1, rotation_val, flip_val)] - uvs[uv_index]);

			//Set the vert
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 3 - rotation_val) % 4, camera->window_y + camera->window_height);
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 2 - rotation_val) % 4, camera->window_y + camera->window_height);
		}

		if(flip_val){
			vert.auv = PVR_PACK_16BIT_UV(uvs[2], uvs[1]);
			vert.buv = PVR_PACK_16BIT_UV(uvs[0], uvs[1]);
			vert.cuv = PVR_PACK_16BIT_UV(uvs[0], uvs[3]);
		}
		else{
			vert.auv = PVR_PACK_16BIT_UV(uvs[0], uvs[1]);
			vert.buv = PVR_PACK_16BIT_UV(uvs[2], uvs[1]);
			vert.cuv = PVR_PACK_16BIT_UV(uvs[2], uvs[3]);
		}

		//Signal to next item we just modified the uvs and verts via cropping so we need to recalculate them
		cropped = (bounds) ? 1 : 0;

		//Draw the sprite
		pvr_prim(&vert, sizeof(vert));

	}

	return 0;
}

//Below is a function that does the same stuff as the simple camera, except its using polys and hence 32-bit UVs.
	//This means all known cases of shimmering and jittering disapear
extern uint8_t crayon_graphics_draw_sprites_simple_POLY_TEST(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode){
	pvr_sprite_txr_t vert = {
		.flags = PVR_CMD_VERTEX_EOL
	};

	uint8_t crop_edges = (1 << 4) - 1;	//---- BRTL
										//---- 1111 (Crop on all edges)

	//DELETE THIS LATER. A basic optimisation for now
	if(camera->window_x == 0){crop_edges = crop_edges & (~ (1 << 0));}
	if(camera->window_y == 0){crop_edges = crop_edges & (~ (1 << 1));}
	if(camera->window_width == crayon_graphics_get_window_width()){crop_edges = crop_edges & (~ (1 << 2));}
	if(camera->window_height == crayon_graphics_get_window_height()){crop_edges = crop_edges & (~ (1 << 3));}

	//Used for cropping
	uint8_t bounds = 0;
	uint8_t cropped = 0;
	uint8_t flip_val = 0;
	uint8_t rotation_val = 0;

	//This var exist so we don't need to worry about constantly floor-ing the camera's world points
		//The sprite points are also floor-ed before calculations are done
	vec2_f_t world_coord = (vec2_f_t){floor(camera->world_x),floor(camera->world_y)};

	//Used in calcuating the new UV
	float texture_offset;
	float texture_divider;
	vec2_f_t selected_vert;
	uint8_t uv_index;

	float uvs[4] = {0};	//u0, v0, u1, v1 (Set to zero to avoid compiler warnings)
	vec2_f_t camera_verts[4], sprite_verts[4];
	camera_verts[0] = (vec2_f_t){camera->window_x,camera->window_y};
	camera_verts[1] = (vec2_f_t){camera->window_x+camera->window_width,camera->window_y};
	camera_verts[2] = (vec2_f_t){camera->window_x,camera->window_y+camera->window_height};
	camera_verts[3] = (vec2_f_t){camera->window_x+camera->window_width,camera->window_y+camera->window_height};

	uint8_t texture_format = (((1 << 3) - 1) & (sprite_array->spritesheet->texture_format >> (28 - 1)));	//Gets the Pixel format
																										//https://github.com/tvspelsfreak/texconv
	int textureformat = sprite_array->spritesheet->texture_format;
	if(texture_format == 5){	//4BPP
			textureformat |= ((sprite_array->palette->palette_id) << 21);	//Update the later to use KOS' macros
	}
	if(texture_format == 6){	//8BPP
			textureformat |= ((sprite_array->palette->palette_id) << 25);	//Update the later to use KOS' macros
	}

	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_txr(&cxt, poly_list_mode, textureformat,
		sprite_array->spritesheet->texture_width, sprite_array->spritesheet->texture_height,
		sprite_array->spritesheet->texture, sprite_array->filter);
	pvr_poly_compile(&hdr, &cxt);
	// hdr.cmd |= 4;	//Enable oargb
	pvr_prim(&hdr, sizeof(hdr));

	pvr_vertex_t vert2[4];	//4 verts per sprite
	//Set the flags
	uint8_t j;
	for(j = 0; j < 3; j++){
		vert2[j].flags = PVR_CMD_VERTEX;
	}
	vert2[3].flags = PVR_CMD_VERTEX_EOL;
	vert2[0].argb = 0xFFFFFFFF;
	vert2[1].argb = 0xFFFFFFFF;
	vert2[2].argb = 0xFFFFFFFF;
	vert2[3].argb = 0xFFFFFFFF;

	//Easily lets us use the right index for each array
		//That way 1-length arrays only get calculated once and each element for a multi list is calculated
	uint16_t *rotation_index, *flip_index, *frame_index;
	uint16_t i;	//The main loop's index
	uint16_t zero = 0;
	float rotation_under_360 = 0;

	if(sprite_array->options & CRAY_MULTI_FRAME){
		frame_index = &i;
	}
	else{
		frame_index = &zero;
	}

	if(sprite_array->options & CRAY_MULTI_FLIP){
		flip_index = &i;
	}
	else{
		flip_index = &zero;
	}

	if(sprite_array->options & CRAY_MULTI_ROTATE){
		rotation_index = &i;
	}
	else{
		rotation_index = &zero;
	}

	uint8_t multi_scale = !!(sprite_array->options & CRAY_MULTI_SCALE);

	uint8_t multi_flip = !!(sprite_array->options & CRAY_MULTI_FLIP);
	uint8_t multi_rotate = !!(sprite_array->options & CRAY_MULTI_ROTATE);
	uint8_t multi_frame = !!(sprite_array->options & CRAY_MULTI_FRAME);

	for(i = 0; i < sprite_array->list_size; i++){
		//These if statements will trigger once if we have a single element (i == 0)
			//and every time for a multi-list
			//and some of them trigger if we just cropped a UV

		if(*frame_index == i || cropped){	//frame
			uvs[0] = sprite_array->frame_uv[sprite_array->frame_id[*frame_index]].x / (float)sprite_array->spritesheet->texture_width;
			uvs[1] = sprite_array->frame_uv[sprite_array->frame_id[*frame_index]].y / (float)sprite_array->spritesheet->texture_height;
			uvs[2] = uvs[0] + sprite_array->animation->frame_width / (float)sprite_array->spritesheet->texture_width;
			uvs[3] = uvs[1] + sprite_array->animation->frame_height / (float)sprite_array->spritesheet->texture_height;
		}

		//Basically enter if first element or either the flip/rotate/frame changed or was cropped
			//The multi_blah's are there to prevent checks on draw params that aren't multi/won't change
		if(i == 0 || cropped || (multi_flip && (sprite_array->flip[i] != sprite_array->flip[i - 1])) ||
			(multi_rotate && (sprite_array->rotation[i] != sprite_array->rotation[i - 1])) ||
			(multi_frame &&
			((sprite_array->frame_uv[i].x != sprite_array->frame_uv[i - 1].x) ||
			(sprite_array->frame_uv[i].y != sprite_array->frame_uv[i - 1].y)))
			){
			cropped = 0;

			//Is flipped?
			if(sprite_array->flip[*flip_index] & (1 << 0)){flip_val = 1;}
			else{flip_val = 0;}

			//Don't bother doing extra calculations
			if(sprite_array->rotation[*rotation_index] != 0){
				rotation_under_360 = fmod(sprite_array->rotation[*rotation_index], 360.0);	//If angle is more than 360 degrees, this fixes that
				if(rotation_under_360 < 0){rotation_under_360 += 360.0;}	//fmod has range -359 to +359, this changes it to 0 to +359

				//For sprite mode we can't simply "rotate" the verts, instead we need to change the uv
				if(crayon_graphics_almost_equals(rotation_under_360, 90.0, 45.0)){
					rotation_val = 1;
					goto verts_rotated_90;
				}
				else if(crayon_graphics_almost_equals(rotation_under_360, 180.0, 45.0)){
					rotation_val = 2;
					goto verts_rotated_180;
				}
				else if(crayon_graphics_almost_equals(rotation_under_360, 270.0, 45.0)){
					rotation_val = 3;
					goto verts_rotated_270;
				}
				else{rotation_val = 0;}
			}
			else{rotation_val = 0;}
		}

		//Imagine a "goto verts_rotated_0;" for this little bit
			//I couldn't actually do since it just flows here naturally
		//NOTE: we don't need to floor the camera's window vars because they're all ints
		vert.ax = floor((floor(sprite_array->coord[i].x) - world_coord.x) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
		vert.ay = floor((floor(sprite_array->coord[i].y) - world_coord.y) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
		vert.bx = vert.ax + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_width / (float)camera->world_width));
		vert.by = vert.ay;
		vert.cx = vert.bx;
		vert.cy = vert.ay + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_height / (float)camera->world_height));
		vert.dx = vert.ax;
		vert.dy = vert.cy;

		//These blocks act as the rotation
		if(0){
			verts_rotated_90:	;	//The semi-colon is there because a label can't be followed by a declaration (Compiler thing)
								//So instead we trick it and give an empty statement :P

			//Both vars are uint16_t and lengths can't be negative or more than 1024 (Largest texture size for DC)
				//Therfore storing the result in a int16_t is perfectly fine
			int16_t diff = sprite_array->animation->frame_width - sprite_array->animation->frame_height;

			vert.dx = floor((floor(sprite_array->coord[i].x) - world_coord.x + (sprite_array->scale[i * multi_scale].y * diff / 2)) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.dy = floor((floor(sprite_array->coord[i].y) - world_coord.y - (sprite_array->scale[i * multi_scale].x * diff / 2)) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.ax = vert.dx + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_width / (float)camera->world_width));
			vert.ay = vert.dy;
			vert.bx = vert.ax;
			vert.by = vert.dy + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_height / (float)camera->world_height));
			vert.cx = vert.dx;
			vert.cy = vert.by;
		}
		if(0){
			verts_rotated_180:	;

			vert.cx = floor((floor(sprite_array->coord[i].x) - world_coord.x) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.cy = floor((floor(sprite_array->coord[i].y) - world_coord.y) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.dx = vert.cx + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_width / (float)camera->world_width));
			vert.dy = vert.cy;
			vert.ax = vert.dx;
			vert.ay = vert.cy + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_height / (float)camera->world_height));
			vert.bx = vert.cx;
			vert.by = vert.ay;
		}
		if(0){
			verts_rotated_270:	;

			int16_t diff = sprite_array->animation->frame_width - sprite_array->animation->frame_height;

			vert.bx = floor((floor(sprite_array->coord[i].x) - world_coord.x + (sprite_array->scale[i * multi_scale].y * diff / 2)) * (camera->window_width / (float)camera->world_width)) + camera->window_x;
			vert.by = floor((floor(sprite_array->coord[i].y) - world_coord.y - (sprite_array->scale[i * multi_scale].x * diff / 2)) * (camera->window_height / (float)camera->world_height)) + camera->window_y;
			vert.cx = vert.bx + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * (camera->window_width / (float)camera->world_width));
			vert.cy = vert.by;
			vert.dx = vert.cx;
			vert.dy = vert.by + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * (camera->window_height / (float)camera->world_height));
			vert.ax = vert.bx;
			vert.ay = vert.dy;
		}

		vert2[0].z = (float)sprite_array->layer[i];
		//Apply these to all verts
		for(j = 1; j < 4; j++){
			vert2[j].z = vert2[0].z;
		}

		//Verts c and d (Or 2 and 3) are swapped so its in Z order instead of "Backwards C" order
		sprite_verts[0] = crayon_graphics_get_sprite_vert(vert, (4 + 0 - rotation_val) % 4);
		sprite_verts[1] = crayon_graphics_get_sprite_vert(vert, (4 + 1 - rotation_val) % 4);
		sprite_verts[2] = crayon_graphics_get_sprite_vert(vert, (4 + 3 - rotation_val) % 4);
		sprite_verts[3] = crayon_graphics_get_sprite_vert(vert, (4 + 2 - rotation_val) % 4);

		//If OOB then don't draw
		if(crayon_graphics_check_oob(camera_verts, sprite_verts, CRAY_DRAW_SIMPLE)){continue;}

		//If we don't need to crop at all, don't both doing the checks. bounds is zero by default
		if(crop_edges){
			bounds = crayon_graphics_check_intersect(camera_verts, sprite_verts);
			bounds &= crop_edges;		//To simplify the if checks
		}

		//Replace below with function calls such as
		//func(&uvs[], flip, rotate, side, camera, scale, animation, &vert)
		//and it modifies the uv element if required
			//Will need to make a function to get last param of crayon_graphics_set_sprite_vert_x/y
		if(bounds & (1 << 0)){	//Left side
			//Get the vert that's currently on the left side
			uv_index = crayon_get_uv_index(0, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 0 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(0, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(0, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(2, rotation_val, flip_val)] - uvs[uv_index]);

			//Set the vert
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 0 - rotation_val) % 4, camera->window_x);
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 3 - rotation_val) % 4, camera->window_x);
		}
		if(bounds & (1 << 1)){	//Top side
			//Get uv thats on top side
			uv_index = crayon_get_uv_index(1, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 1 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(1, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(1, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(3, rotation_val, flip_val)] - uvs[uv_index]);
			// uvs[uv_index] += (texture_offset / texture_divider) / sprite_array->scale[i * multi_scale].y;

			//If we remove the scale from texture_offset
			// uvs[uv_index] += texture_offset / (texture_divider * sprite_array->scale[i * multi_scale].y * sprite_array->scale[i * multi_scale].y);

			//Set the vert
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 1 - rotation_val) % 4, camera->window_y);
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 0 - rotation_val) % 4, camera->window_y);
		}
		if(bounds & (1 << 2)){	//Right side
			//I don't fully understand why we use the magic number 2, maybe its the opposite of 0 (0 == R, 2 == L)

			//Get the vert that's currently on the right side
			uv_index = crayon_get_uv_index(2, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 2 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(2, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(2, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(0, rotation_val, flip_val)] - uvs[uv_index]);

			//Set the vert
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 2 - rotation_val) % 4, camera->window_x + camera->window_width);
			crayon_graphics_set_sprite_vert_x(&vert, (4 + 1 - rotation_val) % 4, camera->window_x + camera->window_width);
		}
		if(bounds & (1 << 3)){	//Bottom side
			//Get the vert that's currently on the bottom side
			uv_index = crayon_get_uv_index(3, rotation_val, flip_val);
			selected_vert = crayon_graphics_get_sprite_vert(vert, (4 + 3 - rotation_val) % 4);
			texture_offset = crayon_graphics_get_texture_offset(3, &selected_vert, &sprite_array->scale[i * multi_scale], camera);
			texture_divider = crayon_graphics_get_texture_divisor(3, rotation_val,
				(vec2_f_t){sprite_array->animation->frame_width,sprite_array->animation->frame_height});

			uvs[uv_index] += (texture_offset / texture_divider) * (uvs[crayon_get_uv_index(1, rotation_val, flip_val)] - uvs[uv_index]);

			//Set the vert
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 3 - rotation_val) % 4, camera->window_y + camera->window_height);
			crayon_graphics_set_sprite_vert_y(&vert, (4 + 2 - rotation_val) % 4, camera->window_y + camera->window_height);
		}

		if(flip_val){
			vert2[0].u = uvs[2];
			vert2[1].u = uvs[0];
			vert2[2].u = uvs[2];
			vert2[3].u = uvs[0];
		}
		else{
			vert2[0].u = uvs[0];
			vert2[1].u = uvs[2];
			vert2[2].u = uvs[0];
			vert2[3].u = uvs[2];
		}

		//V's are constant since we only flip the U's
		vert2[0].v = uvs[1];
		vert2[1].v = uvs[1];
		vert2[2].v = uvs[3];
		vert2[3].v = uvs[3];

		//Signal to next item we just modified the uvs and verts via cropping so we need to recalculate them
		cropped = (bounds) ? 1 : 0;

		sprite_verts[0] = crayon_graphics_get_sprite_vert(vert, 0);
		sprite_verts[1] = crayon_graphics_get_sprite_vert(vert, 1);
		sprite_verts[2] = crayon_graphics_get_sprite_vert(vert, 3);
		sprite_verts[3] = crayon_graphics_get_sprite_vert(vert, 2);

		for(j = 0; j < 4; j++){
			vert2[j].x = sprite_verts[j].x;
			vert2[j].y = sprite_verts[j].y;
		}

		//Draw the sprite
		// pvr_prim(&vert, sizeof(vert));
		pvr_prim(&vert2, sizeof(pvr_vertex_t) * 4);

	}

	return 0;
}


//---------------------------------------------------------------------//


extern uint8_t crayon_graphics_draw_text_mono(char * string, const crayon_font_mono_t *fm, uint8_t poly_list_mode,
	float draw_x, float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number){

	float x0 = floor(draw_x);
	float y0 = floor(draw_y);
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
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fm->texture_width,
		fm->texture_height, fm->texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	float prop_width = (float)fm->char_width / fm->texture_width;
	float prop_height = (float)fm->char_height / fm->texture_height;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line
			x0 = floor(draw_x);
			x1 = x0 + floor(fm->char_width * scale_x);
			y0 = y1 + floor(fm->char_spacing.y * scale_y);
			y1 = y0 + floor(fm->char_height * scale_y);
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

		x0 = x1 + (fm->char_spacing.x * scale_x);
		x1 += floor((fm->char_width + fm->char_spacing.x) * scale_x);
		i++;
	}

	return 0;
}

extern uint8_t crayon_graphics_draw_text_prop(char * string, const crayon_font_prop_t *fp, uint8_t poly_list_mode,
	float draw_x, float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number){

	float x0 = floor(draw_x);
	float y0 = floor(draw_y);
	const float z = layer;
	// float x1 = x0 - floor(fp->char_height * scale_x);
	float x1 = x0;
	float y1 = y0 + floor(fp->char_height * scale_y);
	float v0 = 0;
	float v1 = 0;
	float percent_height = (float)fp->char_height / fp->texture_height;

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
	pvr_sprite_cxt_txr(&context, poly_list_mode, textureformat, fp->texture_width,
		fp->texture_height, fp->texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	int i = 0;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	//This should be able to do a new line
			x0 = floor(draw_x);
			x1 = x0;
			y0 = y1 + floor(fp->char_spacing.y * scale_y);
			y1 = y0 + floor(fp->char_height * scale_y);
			i++;
			continue;
		}
		uint8_t distance_from_space = string[i] - ' ';

		x1 = x0 + floor(fp->char_width[distance_from_space] * scale_x);	//get the width of the display char

		u0 = (float)fp->char_x_coord[distance_from_space] / fp->texture_width;
		u1 = u0 + ((float)fp->char_width[distance_from_space] / fp->texture_width);

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

		x0 = x1 + (fp->char_spacing.x * scale_x);
		i++;
	}

	return 0;
}


//---------------------------------------------------------------------//


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


//---------------------------------------------------------------------//


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

extern uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char * string){
	uint16_t current_length = 0;
	uint16_t best_length = 0;
	
	uint16_t i = 0;
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
		current_length += fm->char_spacing.x;
		i++;
	}

	if(i > 0){best_length -= fm->char_spacing.x;}	//Since we have char - 1 spaces

	return best_length;
}

extern uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char * string){
	uint16_t current_length = 0;
	uint16_t best_length = 0;

	uint8_t distance_from_space;

	uint16_t i = 0;
	while(1){
		if(string[i] == '\0'){
			if(current_length > best_length){
				best_length = current_length;
			}
			break;
		}
		if(string[i] == '\n'){
			if(current_length > best_length){
				best_length = current_length;
			}
			current_length = 0;
			i++;
			continue;
		}

		distance_from_space = string[i] - ' ';
		current_length += fp->char_width[distance_from_space];
		current_length += fp->char_spacing.x;

		i++;
	}

	//Have one less one
	if(i > 0){best_length -= fp->char_spacing.x;}	//Since we have char - 1 spaces

	return best_length;
}


//---------------------------------------------------------------------//


extern void crayon_graphics_transistion_init(crayon_transition_t * effect, crayon_sprite_array_t * sprite_array,
	void (*f)(crayon_transition_t *, void *), uint32_t duration_in, uint32_t duration_out){

	effect->f = f;

	effect->state = CRAY_FADE_STATE_NONE;
	effect->duration_fade_in = duration_in;
	effect->duration_fade_out = duration_out;
	effect->curr_duration = 0;
	effect->prev_duration = 0;

	effect->draw = sprite_array;
	return;
}

extern void crayon_graphics_transistion_skip_to_state(crayon_transition_t * effect, void * params, uint8_t state){
	if(state != CRAY_FADE_STATE_IN && state != CRAY_FADE_STATE_OUT){return;}
	effect->state = state;

	//We set the duration to the end of the state we gave it
	effect->curr_duration = (state == CRAY_FADE_STATE_IN) ? effect->duration_fade_in : effect->duration_fade_out;
	effect->prev_duration = effect->curr_duration;

	(*effect->f)(effect, params);
	effect->state = CRAY_FADE_STATE_NONE;
	return;
}

extern void crayon_graphics_transistion_change_state(crayon_transition_t * effect, uint8_t state){
	if(state != CRAY_FADE_STATE_IN && state != CRAY_FADE_STATE_OUT){return;}
	effect->state = state;

	effect->curr_duration = 0;

	return;
}

extern void crayon_graphics_transistion_apply(crayon_transition_t * effect, void * params){
	if(effect->state != CRAY_FADE_STATE_IN && effect->state != CRAY_FADE_STATE_OUT){return;}

	effect->prev_duration = effect->curr_duration;
	effect->curr_duration++;
	(*effect->f)(effect, params);

	//The transition seems to have finished
	if(crayon_graphics_transistion_resting_state(effect) != CRAY_FADE_NOT_RESTING){
		effect->state = CRAY_FADE_STATE_NONE;
	}

	return;
}

extern double crayon_graphics_transition_get_state_percentage(crayon_transition_t * effect){
	if(effect->state == CRAY_FADE_STATE_IN){
		return (effect->duration_fade_in - effect->curr_duration) / (double)effect->duration_fade_in;
	}

	//Fade out
	return effect->curr_duration / (double)effect->duration_fade_out;
}

extern uint8_t crayon_graphics_transistion_resting_state(crayon_transition_t * effect){
	if((effect->state == CRAY_FADE_STATE_OUT && effect->curr_duration == effect->duration_fade_out)){
		return CRAY_FADE_RESTING_STATE_OUT;
	}
	else if(effect->state == CRAY_FADE_STATE_IN && effect->curr_duration == effect->duration_fade_in){
		return CRAY_FADE_RESTING_STATE_IN;
	}
	return CRAY_FADE_NOT_RESTING;
}


//---------------------------------------------------------------------//


extern vec2_f_t crayon_graphics_rotate_point(vec2_f_t center, vec2_f_t orbit, float radians){
	float sin_theta = sin(radians);
	float cos_theta = cos(radians);
	return (vec2_f_t){(cos_theta * (orbit.x - center.x)) - (sin_theta * (orbit.y - center.y)) + center.x,
		(sin_theta * (orbit.x - center.x)) + (cos_theta * (orbit.y - center.y)) + center.y};
}

extern uint8_t crayon_graphics_almost_equals(float a, float b, float epsilon){
	return fabs(a-b) < epsilon;
}

//Verts order: Top left, Top right, bottom left, bottom right. Z order
	//Return is of format "---- BRTL"
extern uint8_t crayon_graphics_check_intersect(vec2_f_t vC[4], vec2_f_t vS[4]){
	uint8_t bounds = 0;
	if(vS[0].y < vC[0].y || vS[1].y < vC[0].y){bounds |= (1 << 1);}
	if(vS[2].y > vC[2].y || vS[3].y > vC[2].y){bounds |= (1 << 3);}
	if(vS[0].x < vC[0].x || vS[2].x < vC[0].x){bounds |= (1 << 0);}
	if(vS[1].x > vC[1].x || vS[3].x > vC[1].x){bounds |= (1 << 2);}

	return bounds;
}

//How to check if OOB for Simple mode (And Enhanced with no rotation)
	//All verts are further away than one of the camera verts
	//So if The left X vert of the camera is 150 and all the sprite X verts are less than 150 then its OOB
//How to check if OOB for Enhanced mode
	// -
extern uint8_t crayon_graphics_check_oob(vec2_f_t vC[4], vec2_f_t vS[4], uint8_t mode){
	if(mode == CRAY_DRAW_SIMPLE){	//Assumes Axis aligned and Z-order verts. Also used by enhanced when rotation is zero
		if(vS[1].x < vC[0].x){
			return 1;
		}
		//right verts
		if(vS[0].x > vC[1].x){
			return 1;
		}
		//top verts
		if(vS[2].y < vC[0].y){
			return 1;
		}
		//bottom verts
		if(vS[0].y > vC[2].y){
			return 1;
		}

		//No OOBs detected
		return 0;
	}
	else{
		;

		//No OOBs detected
		return 0;
	}
}

extern uint8_t round_way(float value){
	return (value - (int)value > 0.5) ? 1 : 0;
}

//NOTE: This is in the backwards C format (C is bottom right and D is bottom left verts)
extern vec2_f_t crayon_graphics_get_sprite_vert(pvr_sprite_txr_t sprite, uint8_t vert){
	switch(vert){
		case 0:
		return (vec2_f_t){sprite.ax,sprite.ay};
		case 1:
		return (vec2_f_t){sprite.bx,sprite.by};
		case 2:
		return (vec2_f_t){sprite.cx,sprite.cy};
		case 3:
		return (vec2_f_t){sprite.dx,sprite.dy};
	}
	return (vec2_f_t){0,0};
}

extern void crayon_graphics_set_sprite_vert_x(pvr_sprite_txr_t *sprite, uint8_t vert, float value){
	switch(vert){
		case 0:
		sprite->ax = value;
		break;
		case 1:
		sprite->bx = value;
		break;
		case 2:
		sprite->cx = value;
		break;
		case 3:
		sprite->dx = value;
		break;
	}
	return;
}

extern void crayon_graphics_set_sprite_vert_y(pvr_sprite_txr_t *sprite, uint8_t vert, float value){
	switch(vert){
		case 0:
		sprite->ay = value;
		break;
		case 1:
		sprite->by = value;
		break;
		case 2:
		sprite->cy = value;
		break;
		case 3:
		sprite->dy = value;
		break;
	}
	return;
}

//Side is in format LTRB, 0123
extern uint8_t crayon_get_uv_index(uint8_t side, uint8_t rotation_val, uint8_t flip_val){
	//rotate it *back* by rotation_val
	side -= rotation_val;
	if(side > 3){side += 4;}	//If it underflows, bring it back in

	//L becomes R and R becomes L. T and B don't change
	if(flip_val && side % 2 == 0){
		side = (side + 2) % 4;
	}

	return side;
}

extern float crayon_graphics_get_texture_divisor(uint8_t side, uint8_t rotation_val, vec2_f_t dims){
    if((side % 2 == 0 && rotation_val % 2 == 0) || (side % 2 == 1 && rotation_val % 2 == 1)){
        return dims.x;  //width
    }
    return dims.y;   //height
}

extern float crayon_graphics_get_texture_offset(uint8_t side, vec2_f_t * vert, vec2_f_t * scale, const crayon_viewport_t *camera){
	switch(side){
		case 0:
		return (camera->world_width/(float)camera->window_width) * (camera->window_x - vert->x)/scale->x;
		case 1:
		return (camera->world_height/(float)camera->window_height) * (camera->window_y - vert->y)/scale->y;
		case 2:
		return (camera->world_width/(float)camera->window_width) * (vert->x - (camera->window_x + camera->window_width))/scale->x;
		case 3:
		return (camera->world_height/(float)camera->window_height) * (vert->y - (camera->window_y + camera->window_height))/scale->y;
	}
	return 0;	//Shouldn't get here
}
