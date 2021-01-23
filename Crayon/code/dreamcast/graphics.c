#include "graphics.h"

float __CRAYON_GRAPHICS_DEBUG_VARS[16] = {0};

uint16_t __htz = 60;
float __htz_adjustment = 1;

// Later integrate this into crayon_graphics_init()

// Currently set to this since this appears to be the default viewport on Dreamcast (Nothing)
	// Change the last two values to crayon_graphics_get_window_width/height for PC and other platforms
int __clip_window[4] = {0, 0, -1, -1};

// crayon_viewport_t __default_camera;

uint8_t crayon_graphics_init(uint8_t poly_modes){
	pvr_init_params_t pvr_params;
	pvr_params.opb_sizes[0] = (poly_modes & CRAYON_ENABLE_OP) ? PVR_BINSIZE_16 : PVR_BINSIZE_0;
	pvr_params.opb_sizes[1] = PVR_BINSIZE_0;
	pvr_params.opb_sizes[2] = (poly_modes & CRAYON_ENABLE_TR) ? PVR_BINSIZE_16 : PVR_BINSIZE_0;
	pvr_params.opb_sizes[3] = PVR_BINSIZE_0;
	pvr_params.opb_sizes[4] = (poly_modes & CRAYON_ENABLE_PT) ? PVR_BINSIZE_16 : PVR_BINSIZE_0;

	pvr_params.vertex_buf_size = 512 * 1024;
	pvr_params.dma_enabled = 0;
	pvr_params.fsaa_enabled = 0;	// Horizontal scaling (?)
	pvr_params.autosort_disabled = 0;

	pvr_init(&pvr_params);

	if(vid_check_cable() == CT_VGA){
		vid_set_mode(DM_640x480_VGA, PM_RGB565);	// 60Hz
		__htz = 60;
		__htz_adjustment = 1;
	}
	else if(flashrom_get_region() == FLASHROM_REGION_EUROPE){
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		// 50Hz
		__htz = 50;
		__htz_adjustment = 1.2;
	}
	else{
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	// 60Hz
		__htz = 60;
		__htz_adjustment = 1;
	}

	// // Initialise the default camera to use when no camer is specified
	// crayon_memory_init_camera(&__default_camera, (vec2_f_t){0, 0},
	// 	(vec2_u16_t){crayon_graphics_get_window_width(), crayon_graphics_get_window_height()},
	// 	(vec2_u16_t){0, 0},
	// 	(vec2_u16_t){crayon_graphics_get_window_width(), crayon_graphics_get_window_height()},
	// 	1
	// );

	// Set the screen size to full screen by default
	vec2_u16_t values[2] = {
		{0, 0},
		{crayon_graphics_get_window_width(), crayon_graphics_get_window_height()}
	};
	crayon_clipping_cmd_t clip = crayon_graphics_clamp_hardware_clip(values);

	// // We also need to submit it to take effect
	// pvr_scene_begin();
	// pvr_list_begin(PVR_LIST_OP_POLY);
	// crayon_graphics_set_hardware_clip(&clip);
	// pvr_list_finish();
	// pvr_scene_finish();

	return 0;
}

void crayon_graphics_shutdown(){
	pvr_shutdown();
}

// There are 4 palettes for 8BPP and 64 palettes for 4BPP
uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp){
	if(cp->palette_id < 0 || cp->bpp < 0 || cp->bpp * cp->palette_id >= 1024){	// Invalid format/palette not properly set
		return 1;
	}
	uint16_t entries;
	if(cp->bpp == 4){
		entries = 16;
	}
	else if(cp->bpp == 8){
		entries = 256;
	}
	else{	// When OpenGL port arrives, this might change if I allow more than 64 entries
		return 2;
	}

	pvr_set_pal_format(PVR_PAL_ARGB8888);
	unsigned int i;
	for(i = 0; i < cp->colour_count; ++i){
		pvr_set_pal_entry(i + entries * cp->palette_id, cp->palette[i]);
	}
	return 0;
}

float crayon_graphics_get_draw_element_width(const crayon_sprite_array_t *sprite_array, uint8_t id){
	if(!(sprite_array->options & CRAYON_MULTI_SCALE)){id = 0;}	// When there's only one scale
	if(sprite_array->options & CRAYON_HAS_TEXTURE){
		return sprite_array->animation->frame_width * sprite_array->scale[id].x;
	}
	else{
		return sprite_array->scale[id].x;
	}
}

float crayon_graphics_get_draw_element_height(const crayon_sprite_array_t *sprite_array, uint8_t id){
	if(!(sprite_array->options & CRAYON_MULTI_SCALE)){id = 0;}	// When there's only one scale
	if(sprite_array->options & CRAYON_HAS_TEXTURE){
		return sprite_array->animation->frame_height * sprite_array->scale[id].y;
	}
	else{
		return sprite_array->scale[id].y;
	}
}

// Right now on Dreamcast I have no idea how to check the current dimensions, so I just assume this. Its usually right
uint32_t crayon_graphics_get_window_width(){
	return 640;
}

uint32_t crayon_graphics_get_window_height(){
	return 480;
}

uint8_t crayon_graphics_is_hardware_clip_exact(const vec2_u16_t *values){
	if((values[0].x & ((1 << 5) - 1)) || (values[0].y & ((1 << 5) - 1)) ||
		(values[1].x & ((1 << 5) - 1)) || (values[1].y & ((1 << 5) - 1))){
		return 0;
	}

	return 1;
}

crayon_clipping_cmd_t crayon_graphics_clamp_hardware_clip(const vec2_u16_t *values){
	// Make a copy since we might need to modify it
	vec2_u16_t copy[2];
	memcpy(copy, values, sizeof(vec2_u16_t) * 2);

	// We can't crop this far, so bring it back
	if(copy[1].x > 1280){
		copy[1].x = 1280;
	}
	if(copy[1].y > 480){
		copy[1].y = 480;
	}

	// Make sure that min is min and max is max. If they go over, just set them to the same thing
	if(copy[0].x > copy[1].x){
		copy[0].x = copy[1].x;
	}
	if(copy[0].y > copy[1].y){
		copy[0].y = copy[1].y;
	}

	// Specify this as a user clip command
	crayon_clipping_cmd_t clip;
	clip.cmd = PVR_CMD_USERCLIP;

	// Discard lowest 5 bits (Round down)
	clip.minx = copy[0].x >> 5;
	clip.miny = copy[0].y >> 5;

	// Round up to nearest tile (Multiple of 32)
	clip.maxx = (copy[1].x >> 5);

	// If none of the last 5 bits are set (Perfect multiple of 32)
	// NOTE. For max we have that minus 1 since "int * 32" is technically the start of the next tile
	if(!(copy[1].x & ((1 << 5) - 1))){
		clip.maxx -= 1;
	}

	clip.maxy = (copy[1].y >> 5);
	if(!(copy[1].y & ((1 << 5) - 1))){
		clip.maxy -= 1;
	}

	return clip;
}

void crayon_graphics_set_hardware_clip(crayon_clipping_cmd_t *clip){
	// If this is a new dimension, update last dimension and submit TA
	if(memcmp(&__clip_window, &clip->minx, sizeof(int) * 4)){
		memcpy(&__clip_window, &clip->minx, sizeof(int) * 4);

		pvr_prim(clip, sizeof(crayon_clipping_cmd_t));
	}

	return;
}


//---------------------------------------------------------------------//


// Similar to PVR_PACK_16BIT_UV, except they only add either the U or V
	// and they leave the other coord intact
static inline uint32_t PVR_PACK_16BIT_U(uint32_t sprite_uv, float u) {
	union {
		float f;
		uint32_t i;
	} u2;

	u2.f = u;

	return (u2.i & 0xFFFF0000) | (sprite_uv & 0xFFFF);
}

static inline uint32_t PVR_PACK_16BIT_V(uint32_t sprite_uv, float v) {
	union {
		float f;
		uint32_t i;
	} v2;

	v2.f = v;

	return (sprite_uv & 0xFFFF0000) | (v2.i >> 16);
}

// draw_option == ---- SHOM
// M is for draw mode (1 for enhanced, 0 for simple)
// O is for OOB checks
// H is for hardware cropping (32 by 32 tiles on Dreamcast, Scissor test on PC)
// S is for softare cropping (On PC this defaults to hardware cropping)
int8_t crayon_graphics_draw_sprites(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
		uint8_t poly_list_mode, uint8_t draw_option){
	if(sprite_array->size == 0){	// Don't render anything
		return 2;
	}

	crayon_viewport_t default_camera;
	if(camera == NULL){	// No Camera, use the default one
		crayon_memory_init_camera(&default_camera, (vec2_f_t){0, 0},
			(vec2_u16_t){crayon_graphics_get_window_width(), crayon_graphics_get_window_height()},
			(vec2_u16_t){0, 0},
			(vec2_u16_t){crayon_graphics_get_window_width(), crayon_graphics_get_window_height()}, 1);
		camera = &default_camera;
		// camera = &__default_camera;
	}

	// // Set the clipping-region/scissor-test
	// if(draw_option & CRAYON_DRAW_HARDWARE_CROP){
	// 	vec2_u16_t values[2] = {
	// 		{camera->window_x, camera->window_y},
	// 		{camera->window_x + camera->window_width, camera->window_y + camera->window_height}
	// 	};
	// 	crayon_clipping_cmd_t clip = crayon_graphics_clamp_hardware_clip(values);
	// 	crayon_graphics_set_hardware_clip(&clip);

	// 	// No need to software crop if this happens
	// 	if(crayon_graphics_is_hardware_clip_exact(values)){
	// 		draw_option &= ~CRAYON_DRAW_SOFTWARE_CROP;
	// 	}
	// }

	if(sprite_array->options & CRAYON_HAS_TEXTURE){	// Textured
		if(draw_option & CRAYON_DRAW_ENHANCED){
			return crayon_graphics_draw_sprites_enhanced(sprite_array, camera, poly_list_mode, draw_option);
		}
		return crayon_graphics_draw_sprites_simple(sprite_array, camera, poly_list_mode, draw_option);
	}
	return crayon_graphics_draw_untextured_sprites(sprite_array, camera, poly_list_mode, draw_option);
}

uint8_t crayon_graphics_draw_sprites_simple(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
		uint8_t poly_list_mode, uint8_t options){
	// Set the texture format including the palette id
	int pvr_txr_fmt = sprite_array->spritesheet->texture_format;
	switch(DTEX_TXRFMT(sprite_array->spritesheet->texture_format)){
		case 5:
			pvr_txr_fmt |= PVR_TXRFMT_4BPP_PAL(sprite_array->palette->palette_id);
			break;
		case 6:
			pvr_txr_fmt |= PVR_TXRFMT_8BPP_PAL(sprite_array->palette->palette_id);
	}

	// Setup the sprite context and hdr info
	pvr_sprite_cxt_t context;
	pvr_sprite_cxt_txr(&context, poly_list_mode, pvr_txr_fmt, sprite_array->spritesheet->texture_width,
		sprite_array->spritesheet->texture_height, sprite_array->spritesheet->texture, sprite_array->filter);

	// Override the clipping parameter
	context.gen.clip_mode = PVR_USERCLIP_INSIDE;

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	// Dreamcast sprites only take a single "vert" struct. This contains all 4 points
	pvr_sprite_txr_t sprite = {
		.flags = PVR_CMD_VERTEX_EOL
	};

	// We can use "vert_ptr[index * 3]" to access the x and "vert_ptr[(index * 3) + 1]" for y
		// NOTE: There is no dz, but we won't be modifying the Z coordinates with this

	// typedef struct {
	//     uint32  flags;
	//     float   ax;
	//     float   ay;
	//     float   az;
	//     float   bx;
	//     float   by;
	//     float   bz;
	//     float   cx;
	//     float   cy;
	//     float   cz;
	//     float   dx;
	//     float   dy;
	//     uint32 dummy;
	//     uint32 auv;
	//     uint32 buv;
	//     uint32 cuv;
	// } pvr_sprite_txr_t;

	float *vert_ptr = &(sprite.ax);
	uint32_t *uv_ptr = &(sprite.auv);

	// The vertexes for the sprite and camera
	vec2_f_t camera_verts[2], sprite_verts[2];
	camera_verts[0] = (vec2_f_t){camera->window_x, camera->window_y};
	camera_verts[1] = (vec2_f_t){camera->window_x + camera->window_width, camera->window_y + camera->window_height};

	// So I can access them like the uvs array
	float *sprite_ptr = &(sprite_verts[0].x);
	float *camera_ptr = &(camera_verts[0].x);

	// The main loop's index and the two for software cropping
		// Can we remove some of these?
	unsigned int i, j, k, l, crop_side;

	// Used for stuff like figuring out "rotation_val".and software cropping UVs
	float holder;

	// Used for cropping
	uint8_t flip_val = 0;	// Our current flip status
	uint8_t rotation_val = 0;	// Our current rotation status

	// So we don't have to do the same division every loop
	vec2_f_t camera_scale = (vec2_f_t){camera->window_width / (float)camera->world_width,
		camera->window_height / (float)camera->world_height};

	// Used for calculating the rotated vertexes for 90 and 270 degree sprites
	vec2_f_t mid, offset;

	float uvs[4] = {0};	// u0, v0, u1, v1 (Set to zero to avoid compiler warnings)

	// Used to determine if the array has 1 or "sprite_array->size" number of elements
	uint8_t multi_frame = !!(sprite_array->options & CRAYON_MULTI_FRAME);
	uint8_t multi_scale = !!(sprite_array->options & CRAYON_MULTI_SCALE);
	uint8_t multi_flip = !!(sprite_array->options & CRAYON_MULTI_FLIP);
	uint8_t multi_rotate = !!(sprite_array->options & CRAYON_MULTI_ROTATE);

	for(i = 0; i < sprite_array->size; i++){
		// These if statements will trigger once if we have a single element (i == 0)
			// and every time for a multi-list

		if(!sprite_array->visible[i] && i != 0){
			continue;	// We need the defaults to be set on first loop
		}

		// Update if the UVs for every frame index
		if(i == 0 || multi_frame ){	// frame
			uvs[0] = sprite_array->frame_uv[sprite_array->frame_id[multi_frame ? i : 0]].x / (float)sprite_array->spritesheet->texture_width;
			uvs[1] = sprite_array->frame_uv[sprite_array->frame_id[multi_frame ? i : 0]].y / (float)sprite_array->spritesheet->texture_height;
			uvs[2] = uvs[0] + sprite_array->animation->frame_width / (float)sprite_array->spritesheet->texture_width;
			uvs[3] = uvs[1] + sprite_array->animation->frame_height / (float)sprite_array->spritesheet->texture_height;
		}

		if(i == 0 || multi_flip){
			flip_val = (sprite_array->flip[i] != 0);
		}

		if(i == 0 || multi_rotate){
			if(sprite_array->rotation[i] != 0){
				holder = fmod(sprite_array->rotation[i], 360.0);	// If angle is more than 360 degrees, this fixes that
				if(holder < 0){	// fmod has range of about -359 to +359, this changes it to 0 to +359
					holder += 360.0;
				}

				// For sprite mode don't simply "rotate" the verts because we want to avoid sin/cos, instead we need to change the uv
				if(crayon_misc_almost_equals(holder, 90.0, 45.0)){
					rotation_val = 3;
				}
				else if(crayon_misc_almost_equals(holder, 180.0, 45.0)){
					rotation_val = 2;
				}
				else if(crayon_misc_almost_equals(holder, 270.0, 45.0)){
					rotation_val = 1;
				}
				else{	// Seems there's some cases where this picks up stuff it shouldn't
					rotation_val = 0;
				}
			}
			else{	// Don't bother doing extra calculations
				rotation_val = 0;
			}
		}

		// If the first element is invisible, now skip vertex setting, cropping and rendering
		if(i == 0 && !sprite_array->visible[i]){
			continue;
		}

		// This section acts as the rotation
		if(rotation_val == 0){	// 0 degrees
			// NOTE: we don't need to floor the camera's window vars because they're all ints
			sprite.ax = floor((floor(sprite_array->coord[i].x) - floor(camera->world_x)) * camera_scale.x) + camera->window_x;
			sprite.ay = floor((floor(sprite_array->coord[i].y) - floor(camera->world_y)) * camera_scale.y) + camera->window_y;
			sprite.bx = sprite.ax + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * camera_scale.x);
			sprite.by = sprite.ay;
			sprite.cx = sprite.bx;
			sprite.cy = sprite.ay + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * camera_scale.y);
			sprite.dx = sprite.ax;
			sprite.dy = sprite.cy;
		}
		else if(rotation_val == 2){	// 180 degrees
			sprite.cx = floor((floor(sprite_array->coord[i].x) - floor(camera->world_x)) * camera_scale.x) + camera->window_x;
			sprite.cy = floor((floor(sprite_array->coord[i].y) - floor(camera->world_y)) * camera_scale.y) + camera->window_y;
			sprite.dx = sprite.cx + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * camera_scale.x);
			sprite.dy = sprite.cy;
			sprite.ax = sprite.dx;
			sprite.ay = sprite.cy + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * camera_scale.y);
			sprite.bx = sprite.cx;
			sprite.by = sprite.ay;
		}
		else{	// 90 and 270 degree rotations
			// The sprite's original boundries
			sprite.ax = floor((floor(sprite_array->coord[i].x) - floor(camera->world_x)) * camera_scale.x) + camera->window_x;
			sprite.ay = floor((floor(sprite_array->coord[i].y) - floor(camera->world_y)) * camera_scale.y) + camera->window_y;
			sprite.bx = sprite.ax + floor(sprite_array->animation->frame_width * sprite_array->scale[i * multi_scale].x * camera_scale.x);
			sprite.cy = sprite.ay + floor(sprite_array->animation->frame_height * sprite_array->scale[i * multi_scale].y * camera_scale.y);

			// We have to do this to simulate the "cos/sin" rotation of enhanced where it rotates around the mid point
				// Otherwise there can be an inaccuracy in the center of the sprite between simple and enhanced renderers
			mid.x = ((sprite.bx - sprite.ax) * 0.5) + sprite.ax;
			offset.x = sprite.bx - mid.x;

			mid.y = ((sprite.cy - sprite.ay) * 0.5) + sprite.ay;
			offset.y = sprite.cy - mid.y;

			if(rotation_val == 3){	// 90 degrees
				sprite.dx = mid.x - offset.y;
				sprite.dy = mid.y - offset.x;
				sprite.ax = mid.x + offset.y;
				sprite.ay = sprite.dy;
				sprite.bx = sprite.ax;
				sprite.by = mid.y + offset.x;
				sprite.cx = sprite.dx;
				sprite.cy = sprite.by;
			}
			else{	// 270 degrees
				sprite.bx = mid.x - offset.y;
				sprite.by = mid.y - offset.x;
				sprite.cx = mid.x + offset.y;
				sprite.cy = sprite.by;
				sprite.dx = sprite.cx;
				sprite.dy = mid.y + offset.x;
				sprite.ax = sprite.bx;
				sprite.ay = sprite.dy;
			}
		}

		sprite.az = sprite.bz = sprite.cz = sprite_array->layer[i];

		// Gets the Top Left and the Bottom Right vertex coords. Takes rotation into consideration.
		sprite_verts[0].x = vert_ptr[rotation_val * 3];	// TL
		sprite_verts[0].y = vert_ptr[(rotation_val * 3) + 1];
		j = (rotation_val + 2) % 4;
		sprite_verts[1].x = vert_ptr[j * 3];	// BR
		sprite_verts[1].y = vert_ptr[(j * 3) + 1];

		// If we have the software cropping or OOB detection check active
			// Then check if they don't overlap and if so move to next element
		if((options & (CRAYON_DRAW_CHECK_OOB | CRAYON_DRAW_SOFTWARE_CROP)) &&
				!crayon_graphics_aabb_aabb_overlap(sprite_verts, camera_verts)){
			continue;
		}

		// If software cropping, Modify the verts/UVs
		if((options & CRAYON_DRAW_SOFTWARE_CROP)){
			// rotation_val:
			// - 0: 0 degrees    a-vert: TL   ORDER (k, j): (0,3), (1,0), (2,1), (3,2)
			// - 1: 270 degrees  a-vert: BL   ORDER       : (1,0), (2,1), (3,2), (0,3)
			// - 2: 180 degrees  a-vert: BR   ORDER       : (2,1), (3,2), (0,3), (1,0)
			// - 3: 90 degrees   a-vert: TR   ORDER       : (3,2), (0,3), (1,0), (2,1)

			// We will start with the LHS verts
			k = rotation_val;
			j = (k != 0) ? k - 1 : 3;	// Vert before k.

			for(l = 0; l < 4; l++){
				// Check to see if the side we want to check needs cropping
					// LTRB order
				if((l < 2 && vert_ptr[(3 * k) + (l & 1)] < camera_ptr[l]) ||
						(l > 1 && vert_ptr[(3 * k) + (l & 1)] > camera_ptr[l])){

					vert_ptr[(3 * k) + (l & 1)] = camera_ptr[l];
					vert_ptr[(3 * j) + (l & 1)] = camera_ptr[l];

					// Percentage of sprite off screen
					holder = ((camera_ptr[l] - sprite_ptr[l]) /
						(sprite_ptr[2 + (l & 1)] - sprite_ptr[0 + (l & 1)]));
					if(l > 1){
						holder *= -1;
					}
				}
				else{	// Not off screen, 0% offscreen
					holder = 0;
				}

				// The amount of "UV" offscreen
				holder *= ((k & 1) == 0) ? (uvs[2] - uvs[0]) : (uvs[3] - uvs[1]);

				// Change the target UV to the opposite side when flipped and a U/X
				crop_side = k;
				if(flip_val){
					if(crop_side == 0){
						crop_side = 2;
					}
					else if(crop_side == 2){
						crop_side = 0;
					}
				}

				// Get the exact UV value we need
				switch(crop_side){
					case 0:	// left side
						holder = uvs[0] + holder;
						break;
					case 1:	// Top side
						holder = uvs[1] + holder;
						break;
					case 2:	// Right side
						holder = uvs[2] - holder;
						break;
					case 3:	// Bottom side
						holder = uvs[3] - holder;
						break;
				}

				// Making sure we don't try to set "duv" because it DNE
				if(k < 3){
					if(crop_side == 0 || crop_side == 2){
						uv_ptr[k] = PVR_PACK_16BIT_U(uv_ptr[k], holder);
					}
					else{
						uv_ptr[k] = PVR_PACK_16BIT_V(uv_ptr[k], holder);
					}
				}
				if(j < 3){
					if(crop_side == 0 || crop_side == 2){
						uv_ptr[j] = PVR_PACK_16BIT_U(uv_ptr[j], holder);
					}
					else{
						uv_ptr[j] = PVR_PACK_16BIT_V(uv_ptr[j], holder);
					}
				}

				// Just optimisation, no need to do the below if its the last loop
				if(l == 3){
					break;
				}

				// Get the next vert ids
				j = k;
				k = (k < 3) ? k + 1 : 0;
			}
		}
		else{
			if(flip_val){
				sprite.auv = PVR_PACK_16BIT_UV(uvs[2], uvs[1]);
				sprite.buv = PVR_PACK_16BIT_UV(uvs[0], uvs[1]);
				sprite.cuv = PVR_PACK_16BIT_UV(uvs[0], uvs[3]);
			}
			else{
				sprite.auv = PVR_PACK_16BIT_UV(uvs[0], uvs[1]);
				sprite.buv = PVR_PACK_16BIT_UV(uvs[2], uvs[1]);
				sprite.cuv = PVR_PACK_16BIT_UV(uvs[2], uvs[3]);
			}
		}

		// Pass the sprite struct
		pvr_prim(&sprite, sizeof(sprite));
	}

	return 0;
}

uint8_t crayon_graphics_draw_sprites_enhanced(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
		uint8_t poly_list_mode, uint8_t options){
	int pvr_txr_fmt = sprite_array->spritesheet->texture_format;
	switch(DTEX_TXRFMT(sprite_array->spritesheet->texture_format)){
		case 5:
			pvr_txr_fmt |= PVR_TXRFMT_4BPP_PAL(sprite_array->palette->palette_id);
			break;
		case 6:
			pvr_txr_fmt |= PVR_TXRFMT_8BPP_PAL(sprite_array->palette->palette_id);
	}

	// Need to set them to zero to prevent a bunch of compiler warnings...
	vec2_f_t uv[2] = {{0,0}};

	pvr_poly_cxt_t cxt;
	pvr_poly_cxt_txr(&cxt, poly_list_mode, pvr_txr_fmt, sprite_array->spritesheet->texture_width,
		sprite_array->spritesheet->texture_height, sprite_array->spritesheet->texture, sprite_array->filter);

	// Override the clipping parameter
	cxt.gen.clip_mode = PVR_USERCLIP_INSIDE;

	pvr_poly_hdr_t hdr;
	pvr_poly_compile(&hdr, &cxt);
	hdr.cmd |= 4;	// Enable oargb
	pvr_prim(&hdr, sizeof(hdr));

	// The verts
	uint8_t poly_verts = 4;
	#define _MAX_POLY_VERTS 8	// With Sutherland-Hodgman I belive it can create up-to 8 vert polys for a 4 sided object
	pvr_vertex_t vert[_MAX_POLY_VERTS];
	vec2_f_t vert_coords[(_MAX_POLY_VERTS * 2)];	// We double that because of Sutherland-Hodgman alg
	#undef _MAX_POLY_VERTS

	// Setting the first vert's command
	vert[0].flags = PVR_CMD_VERTEX;

	// min_x, min_y, max_x, max_y
	float window_coords[4];
	window_coords[0] = camera->window_x;
	window_coords[1] = camera->window_y;
	window_coords[2] = camera->window_x + camera->window_width;
	window_coords[3] = camera->window_y + camera->window_height;

	// Contains a bunch of random vectors. Unrotated sprite top left and bottom right verts,
		// mid point, unrotated point
	#define __SPRITE_BOUND_TL 0
	#define __SPRITE_BOUND_BR 1
	#define __MID_POINT 2
	#define __UNROTATED 3
	#define __CAMERA_SCALE 5
	vec2_f_t vec[5] = {{0,0}};

	// Set the camera scaling boundries
	vec[__CAMERA_SCALE].x = camera->window_width / (float)camera->world_width;
	vec[__CAMERA_SCALE].y = camera->window_height / (float)camera->world_height;

	// Quick commands to check if its multi or not
		// I wonder if its more efficient to use these vars or copy their
		// formula everywhere its used
	uint8_t multi_colour = !!(sprite_array->options & CRAYON_MULTI_COLOUR);
	uint8_t multi_rotation = !!(sprite_array->options & CRAYON_MULTI_ROTATE);
	uint8_t multi_scale = !!(sprite_array->options & CRAYON_MULTI_DIM);
	uint8_t multi_frame = !!(sprite_array->options & CRAYON_MULTI_FRAME);
	uint8_t multi_flip = !!(sprite_array->options & CRAYON_MULTI_FLIP);

	float angle = 0;

	// Used for the various colour calulations
	uint8_t f, a, r, g, b;

	uint8_t skip = 0;

	unsigned int i, j, k, index;	// Indexes
	for(i = 0; i < sprite_array->size; i++){
		// If element is invisible (Max fade and zero alpha effectively make it invisible too)
			// For the first element, we need to initialise our vars, otherwise we just skip to the next element
		if((sprite_array->fade[multi_colour ? i : 0] == 0xFF && (sprite_array->colour[multi_colour ? i : 0] >> 24) == 0) ||
				!sprite_array->visible[i]){
			if(i != 0){continue;}
			skip = 1;
		}

		// These if statements will trigger once if we have a single element (i == 0)
			// and every time for a multi-list

		if(i == 0 || multi_frame){	// frame
			uv[0].x = sprite_array->frame_uv[sprite_array->frame_id[i]].x / (float)sprite_array->spritesheet->texture_width;
			uv[0].y = sprite_array->frame_uv[sprite_array->frame_id[i]].y / (float)sprite_array->spritesheet->texture_height;
			uv[1].x = uv[0].x + (sprite_array->animation->frame_width / (float)sprite_array->spritesheet->texture_width);
			uv[1].y = uv[0].y + (sprite_array->animation->frame_height / (float)sprite_array->spritesheet->texture_height);
		}

		// Update rotation angle if needed
		if(i == 0 || multi_rotation){
			angle = fmod(sprite_array->rotation[i], 360.0);	// If angle is more than 360 degrees, this fixes that
			if(angle < 0){angle += 360.0;}	// fmod has range -359 to +359, this changes it to 0 to +359
			angle = (angle * M_PI) / 180.0f;
		}

		if(i == 0 || multi_colour){
			f = sprite_array->fade[i];
			a = (sprite_array->colour[i] >> 24) & 0xFF;

			r = (((sprite_array->colour[i] >> 16) & 0xFF) * f) / 255.0f;
			g = (((sprite_array->colour[i] >> 8) & 0xFF) * f) / 255.0f;
			b = (((sprite_array->colour[i]) & 0xFF) * f) / 255.0f;
			if(sprite_array->options & CRAYON_COLOUR_ADD){	// If Adding
				vert[0].argb = (a << 24) + 0x00FFFFFF;
			}
			else{	// If Blending
				f = 255 - f;	// Inverse f
				vert[0].argb = (a << 24) + (f << 16) + (f << 8) + f;
			}
			vert[0].oargb = (a << 24) + (r << 16) + (g << 8) + b;
		}

		if(skip){
			skip = 0;
			continue;
		}

		// Reason for the order of the floor-ing
			// For top left vert, We remove any float variance from the sprite vert and also the world coord since all sprites should
			// be locked to the same grind and same for cameras. We then floor the result of timesing by the scale so its now locked
			// to the grid. We don't floor the scale because we might want to scale by 2.5 or something. Finally window_x/y is an int
		// Also notice how the verts are in clockwise order. This makes it easier for sutherland hodgman algorithm
		vert_coords[0].x = floor((floor(sprite_array->coord[i].x) - floor(camera->world_x)) * vec[__CAMERA_SCALE].x) + camera->window_x;
		vert_coords[0].y = floor((floor(sprite_array->coord[i].y) - floor(camera->world_y)) * vec[__CAMERA_SCALE].y) + camera->window_y;
		vert_coords[1].x = vert_coords[0].x +
			floor(sprite_array->animation->frame_width * sprite_array->scale[multi_scale ? i : 0].x * vec[__CAMERA_SCALE].x);
		vert_coords[1].y = vert_coords[0].y;
		vert_coords[2].x = vert_coords[1].x;
		vert_coords[2].y = vert_coords[0].y +
			floor(sprite_array->animation->frame_height * sprite_array->scale[multi_scale ? i : 0].y * vec[__CAMERA_SCALE].y);
		vert_coords[3].x = vert_coords[0].x;
		vert_coords[3].y = vert_coords[2].y;

		// Store the unrotated vertex boundries for later if we're doing software cropping
		if((options & CRAYON_DRAW_SOFTWARE_CROP)){
			vec[__SPRITE_BOUND_TL].x = vert_coords[0].x;
			vec[__SPRITE_BOUND_TL].y = vert_coords[0].y;
			vec[__SPRITE_BOUND_BR].x = vert_coords[1].x;
			vec[__SPRITE_BOUND_BR].y = vert_coords[3].y;
		}

		// If we don't want to do rotations (Rotation == 0.0), then skip it
		if(sprite_array->rotation[multi_rotation ? i : 0] != 0.0f){

			// Gets the true midpoint
			vec[__MID_POINT].x = ((vert_coords[1].x - vert_coords[0].x) * 0.5) + vert_coords[0].x;
			vec[__MID_POINT].y = ((vert_coords[2].y - vert_coords[0].y) * 0.5) + vert_coords[0].y;

			// Update the vert x and y positions
			for(j = 0; j < 4; j++){
				vert_coords[j] = crayon_misc_rotate_point(vec[__MID_POINT], vert_coords[j], angle);
			}
		}

		// OOB check
		if(options & CRAYON_DRAW_CHECK_OOB){
			// If they don't overlap then no point progressing
			if(!crayon_graphics_aabb_obb_overlap(vert_coords, window_coords)){
				continue;
			}
		}

		// Perform software cropping (But only if we can't do hardware cropping)
		if((options & CRAYON_DRAW_SOFTWARE_CROP)){
			poly_verts = crayon_graphics_sutherland_hodgman(vert_coords, window_coords);

			// This will only really trigger is the OOB check was disabled
			if(poly_verts < 3){continue;}	// Usually only ever triggers for "0" so we don't render junk

			// For a 4-vert poly we expect the vert order to be "0, 1, 3, 2"
			// We have to swap the vertex order since this is how its done to render stuff
				// To generate the UVs we must rotate the point by "-angle" to find the x/y offset of the vertex
			vert[0].x = vert_coords[0].x;
			vert[0].y = vert_coords[0].y;
			if(angle != 0){vec[__UNROTATED] = crayon_misc_rotate_point(vec[__MID_POINT], vert_coords[0], -angle);}
			else{vec[__UNROTATED] = vert_coords[0];}	// Since "mid" wasn't set

			// Due to inaccuracies with the sin/cos functions, the numbers will be off a bit and occationally will be OOB
			// So this just makes sure they are within
			if(vec[__UNROTATED].x < vec[__SPRITE_BOUND_TL].x){vec[__UNROTATED].x = vec[__SPRITE_BOUND_TL].x;}
			else if(vec[__UNROTATED].x > vec[__SPRITE_BOUND_BR].x){vec[__UNROTATED].x = vec[__SPRITE_BOUND_BR].x;}
			if(vec[__UNROTATED].y < vec[__SPRITE_BOUND_TL].y){vec[__UNROTATED].y = vec[__SPRITE_BOUND_TL].y;}
			else if(vec[__UNROTATED].y > vec[__SPRITE_BOUND_BR].y){vec[__UNROTATED].y = vec[__SPRITE_BOUND_BR].y;}

			// Find the UV offset
				// Only "x" responds to flip
			vert[0].u = (uv[1].x - uv[0].x) * ((vec[__UNROTATED].x - vec[__SPRITE_BOUND_TL].x) /
				(vec[__SPRITE_BOUND_BR].x - vec[__SPRITE_BOUND_TL].x));
			if(!sprite_array->flip[multi_flip ? i : 0]){	// uv[0].x + Part
				vert[0].u += uv[0].x;
			}
			else{	// uv[1].x - Part
				vert[0].u *= -1;
				vert[0].u += uv[1].x;
			}
			vert[0].v = uv[0].y + ((uv[1].y - uv[0].y) * ((vec[__UNROTATED].y - vec[__SPRITE_BOUND_TL].y) /
				(vec[__SPRITE_BOUND_BR].y - vec[__SPRITE_BOUND_TL].y)));

			k = 1;	// vert_coords index goes in order, 0, 1, -1, 2, -2, etc
			for(j = 1; j < poly_verts; j++){
				f = j % 2 != 1;	// Starts at 1 (Recyclign register for "f" rather thank making a new var)
				index = f ? poly_verts - k : k;

				vert[j].x = vert_coords[index].x;
				vert[j].y = vert_coords[index].y;
				if(angle != 0){vec[__UNROTATED] = crayon_misc_rotate_point(vec[__MID_POINT], vert_coords[index], -angle);}
				else{vec[__UNROTATED] = vert_coords[index];}	// Since "mid" wasn't set

				// OOB correction
				if(vec[__UNROTATED].x < vec[__SPRITE_BOUND_TL].x){vec[__UNROTATED].x = vec[__SPRITE_BOUND_TL].x;}
				else if(vec[__UNROTATED].x > vec[__SPRITE_BOUND_BR].x){vec[__UNROTATED].x = vec[__SPRITE_BOUND_BR].x;}
				if(vec[__UNROTATED].y < vec[__SPRITE_BOUND_TL].y){vec[__UNROTATED].y = vec[__SPRITE_BOUND_TL].y;}
				else if(vec[__UNROTATED].y > vec[__SPRITE_BOUND_BR].y){vec[__UNROTATED].y = vec[__SPRITE_BOUND_BR].y;}

				vert[j].u = (uv[1].x - uv[0].x) * ((vec[__UNROTATED].x - vec[__SPRITE_BOUND_TL].x) /
					(vec[__SPRITE_BOUND_BR].x - vec[__SPRITE_BOUND_TL].x));
				if(!sprite_array->flip[multi_flip ? i : 0]){	// uv[0].x + Part
					vert[j].u += uv[0].x;
				}
				else{	// uv[1].x - Part
					vert[j].u *= -1;
					vert[j].u += uv[1].x;
				}
				vert[j].v = uv[0].y + ((uv[1].y - uv[0].y) * ((vec[__UNROTATED].y - vec[__SPRITE_BOUND_TL].y) /
					(vec[__SPRITE_BOUND_BR].y - vec[__SPRITE_BOUND_TL].y)));

				if(f){k++;}
			}
		}
		else{
			poly_verts = 4;

			vert[0].x = vert_coords[0].x;
			vert[0].y = vert_coords[0].y;
			vert[1].x = vert_coords[1].x;
			vert[1].y = vert_coords[1].y;
			vert[2].x = vert_coords[3].x;
			vert[2].y = vert_coords[3].y;
			vert[3].x = vert_coords[2].x;
			vert[3].y = vert_coords[2].y;

			// Apply the UVs
				// If its flipped, swap the x UVs
			if(sprite_array->flip[multi_flip ? i : 0]){
				vert[0].u = uv[1].x;
				vert[1].u = uv[0].x;
				vert[2].u = uv[1].x;
				vert[3].u = uv[0].x;
			}
			else{
				vert[0].u = uv[0].x;
				vert[1].u = uv[1].x;
				vert[2].u = uv[0].x;
				vert[3].u = uv[1].x;
			}
			vert[0].v = uv[0].y;
			vert[1].v = uv[0].y;
			vert[2].v = uv[1].y;
			vert[3].v = uv[1].y;
		}

		// Apply these to all verts
		for(j = 0; j < poly_verts; j++){
			vert[j].argb = vert[0].argb;
			vert[j].oargb = vert[0].oargb;
			vert[j].z = sprite_array->layer[i];
			vert[j].flags = PVR_CMD_VERTEX;	// Don't bother setting this for the first one since it should never change
		}
		vert[poly_verts - 1].flags = PVR_CMD_VERTEX_EOL;	// This must be EOL

		pvr_prim(&vert, sizeof(pvr_vertex_t) * poly_verts);
	}

	return 0;
}

uint8_t crayon_graphics_draw_untextured_sprites(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t options){
	// This var exist so we don't need to worry about constantly floor-ing the camera's world points
		// The sprite points are also floor-ed before calculations are done
	vec2_f_t camera_scale = (vec2_f_t){camera->window_width / (float)camera->world_width,
		camera->window_height / (float)camera->world_height};

	// The verts
	uint8_t poly_verts = 4;
	#define _MAX_POLY_VERTS 8	// With Sutherland-Hodgman I belive it can create up-to 8 vert polys for a 4 sided object
	pvr_vertex_t vert[_MAX_POLY_VERTS];
	vec2_f_t vert_coords[_MAX_POLY_VERTS * 2];	// We double that because of Sutherland-Hodgman alg
	#undef _MAX_POLY_VERTS

	// Setting the first vert's command
	vert[0].flags = PVR_CMD_VERTEX;

	// min_x, min_y, max_x, max_y
	float window_coords[4];
	window_coords[0] = camera->window_x;
	window_coords[1] = camera->window_y;
	window_coords[2] = camera->window_x + camera->window_width;
	window_coords[3] = camera->window_y + camera->window_height;

	pvr_poly_cxt_t cxt;
	pvr_poly_cxt_col(&cxt, poly_list_mode);

	// Override the clipping parameter
	cxt.gen.clip_mode = PVR_USERCLIP_INSIDE;

	pvr_poly_hdr_t hdr;	
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	// Rotation stuff
		// Using vert_coords indexes 4 for the mid-point
	float angle = 0;

	// --CR -D--
	uint8_t multi_colour = !!(sprite_array->options & CRAYON_MULTI_COLOUR);
	uint8_t multi_rotation = !!(sprite_array->options & CRAYON_MULTI_ROTATE);
	uint8_t multi_dim = !!(sprite_array->options & CRAYON_MULTI_DIM);

	// Invisible or 0 alpha poly
	uint8_t skip = 0;

	// We use full ints because its faster than uint16_t apparently
	unsigned int i, j, k;
	for(i = 0; i < sprite_array->size; i++){
		if(sprite_array->colour[multi_colour ? i : 0] >> 24 == 0 || sprite_array->visible[i] == 0){	// Don't draw alpha-less or invisible stuff
			if(i != 0){continue;}	// For the first element, we need to initialise our vars, otherwise we just skip to the next element
			skip = 1;	// We need the defaults to be set on first loop
		}

		// Update rotation part if needed
		if(i == 0 || multi_rotation){
			angle = fmod(sprite_array->rotation[i], 360.0);	// If angle is more than 360 degrees, this fixes that
			if(angle < 0){angle += 360.0;}	// fmod has range -359 to +359, this changes it to 0 to +359
			angle = (angle * M_PI) / 180.0f;	// Convert from degrees to ratians
		}

		if(i == 0 || multi_colour){
			vert[0].argb = sprite_array->colour[i];
		}

		if(skip){
			skip = 0;
			continue;
		}

		// Reason for the order of the floor-ing
			// For top left vert, We remove any float variance from the sprite vert and also the world coord since all sprites should
			// be locked to the same grind and same for cameras. We then floor the result of timesing by the scale so its now locked
			// to the grid. We don't floor the scale because we might want to scale by 2.5 or something. Finally window_x/y is an int
		// Also notice how the verts are in clockwise order. This makes it easier for sutherland hodgman algorithm
		vert_coords[0].x = floor((floor(sprite_array->coord[i].x) - floor(camera->world_x)) * camera_scale.x) + camera->window_x;
		vert_coords[0].y = floor((floor(sprite_array->coord[i].y) - floor(camera->world_y)) * camera_scale.y) + camera->window_y;
		vert_coords[1].x = vert_coords[0].x + floor(sprite_array->scale[multi_dim ? i : 0].x * camera_scale.x);
		vert_coords[1].y = vert_coords[0].y;
		vert_coords[2].x = vert_coords[1].x;
		vert_coords[2].y = vert_coords[0].y + floor(sprite_array->scale[multi_dim ? i : 0].y * camera_scale.y);
		vert_coords[3].x = vert_coords[0].x;
		vert_coords[3].y = vert_coords[2].y;

		// Rotate the poly
		if(sprite_array->rotation[multi_rotation ? i : 0] != 0.0){
			// Gets the midpoint
			#define _CRAYON_SPARE_INDEX 4	// Rather than making a new variable, lets use an exisiting one to save on registers
			vert_coords[_CRAYON_SPARE_INDEX].x = ((vert_coords[1].x - vert_coords[0].x) * 0.5) + vert_coords[0].x;
			vert_coords[_CRAYON_SPARE_INDEX].y = ((vert_coords[2].y - vert_coords[0].y) * 0.5) + vert_coords[0].y;

			// Rotate the verts around the midpoint
			for(j = 0; j < 4; j++){
				vert_coords[j] = crayon_misc_rotate_point(vert_coords[_CRAYON_SPARE_INDEX], vert_coords[j], angle);
			}
			#undef _CRAYON_MID_POINT
		}

		// OOB check
		if(options & CRAYON_DRAW_CHECK_OOB){
			// If they don't overlap then no point progressing
			if(!crayon_graphics_aabb_obb_overlap(vert_coords, window_coords)){
				continue;
			}
		}

		// Perform software cropping (But only if we can't do hardware cropping)
		if((options & CRAYON_DRAW_SOFTWARE_CROP)){
			poly_verts = crayon_graphics_sutherland_hodgman(vert_coords, window_coords);

			// This will only really trigger is the OOB check was disabled
			if(poly_verts < 3){continue;}	// Usually only ever triggers for "0" so we don't render junk
		}
		else{
			poly_verts = 4;
		}

		// For a 4-vert poly we expect the vert order to be "0, 1, 3, 2"
			// We have to swap the vertex order since this is how its done to render stuff
		vert[0].x = vert_coords[0].x;
		vert[0].y = vert_coords[0].y;
		k = 1;	// Order, 0, 1, -1, 2, -2
		for(j = 1; j < poly_verts; j++){
			skip = j % 2 != 1;	// Starts at 1
			vert[j].x = vert_coords[skip ? poly_verts - k : k].x;
			vert[j].y = vert_coords[skip ? poly_verts - k : k].y;
			if(skip){k++;}
		}
		skip = 0;	// Since this is used for other things too

		// Set the Z, colour and flags for all the verts
		vert[0].z = sprite_array->layer[i];
		for(j = 1; j < poly_verts; j++){
			vert[j].z = vert[0].z;
			vert[j].argb = vert[0].argb;
			vert[j].flags = PVR_CMD_VERTEX;	// Don't bother setting this for the first one since it should never change
		}
		vert[poly_verts - 1].flags = PVR_CMD_VERTEX_EOL;	// This must be EOL

		// Submit the verts to the PVR for rendering
		pvr_prim(&vert, sizeof(pvr_vertex_t) * poly_verts);
	}

	return 0;
}


//---------------------------------------------------------------------//


uint8_t crayon_graphics_draw_text_mono(char *string, const crayon_font_mono_t *fm, uint8_t poly_list_mode,
	float draw_x, float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number){

	float x0 = floor(draw_x);
	float y0 = floor(draw_y);
	const float z = layer;

	// x1 and y1 depend on the letter
	float x1 = x0 + fm->char_width * scale_x;
	float y1 = y0 + fm->char_height * scale_y;

	float u0, v0, u1, v1;

	pvr_sprite_cxt_t context;

	int pvr_txr_fmt = fm->texture_format;
	uint8_t texture_format = DTEX_TXRFMT(fm->texture_format);
	if(texture_format == 5){	// 4BPP
		pvr_txr_fmt |= PVR_TXRFMT_4BPP_PAL(palette_number);
	}
	else if(texture_format == 6){	// 8BPP
		pvr_txr_fmt |= PVR_TXRFMT_8BPP_PAL(palette_number);
	}

	pvr_sprite_cxt_txr(&context, poly_list_mode, pvr_txr_fmt, fm->texture_width,
		fm->texture_height, fm->texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	unsigned int i = 0;
	float prop_width = (float)fm->char_width / fm->texture_width;
	float prop_height = (float)fm->char_height / fm->texture_height;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	// This should be able to do a new line
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

uint8_t crayon_graphics_draw_text_prop(char *string, const crayon_font_prop_t *fp, uint8_t poly_list_mode,
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

	int pvr_txr_fmt = fp->texture_format;
	uint8_t texture_format = DTEX_TXRFMT(fp->texture_format);
	if(texture_format == 5){	// 4BPP
		pvr_txr_fmt |= PVR_TXRFMT_4BPP_PAL(palette_number);
	}
	else if(texture_format == 6){	// 8BPP
		pvr_txr_fmt |= PVR_TXRFMT_8BPP_PAL(palette_number);
	}

	pvr_sprite_cxt_txr(&context, poly_list_mode, pvr_txr_fmt, fp->texture_width,
		fp->texture_height, fp->texture, PVR_FILTER_NONE);

	pvr_sprite_hdr_t header;
	pvr_sprite_compile(&header, &context);
	pvr_prim(&header, sizeof(header));

	unsigned int i = 0;
	while(1){
		if(string[i] == '\0'){
			break;
		}
		if(string[i] == '\n'){	// This should be able to do a new line
			x0 = floor(draw_x);
			x1 = x0;
			y0 = y1 + floor(fp->char_spacing.y * scale_y);
			y1 = y0 + floor(fp->char_height * scale_y);
			i++;
			continue;
		}
		uint8_t distance_from_space = string[i] - ' ';

		x1 = x0 + floor(fp->char_width[distance_from_space] * scale_x);	// get the width of the display char

		u0 = (float)fp->char_x_coord[distance_from_space] / fp->texture_width;
		u1 = u0 + ((float)fp->char_width[distance_from_space] / fp->texture_width);

		// Can this section be optimised? Maybe replace it with binary search?
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


void crayon_graphics_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t layer, uint32_t colour,
	uint8_t poly_list_mode){
	if(colour >> 24 == 0){return;}	// Don't draw alpha-less stuff
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert[4];

	pvr_poly_cxt_col(&cxt, poly_list_mode);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	unsigned int i;
	for(i = 0; i < 4; i++){
		vert[i].argb = colour;
		vert[i].oargb = 0;
		vert[i].z = layer;
		vert[i].flags = PVR_CMD_VERTEX;
	}
	vert[3].flags = PVR_CMD_VERTEX_EOL;

	int16_t y_diff = y2 - y1;
	int16_t x_diff = x2 - x1;

	// Absolute them
	if(y_diff < 0){y_diff *= -1;}
	if(x_diff < 0){x_diff *= -1;}

	// We flip them if its in between SW and NE (Going clockwise)

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

	if(x_diff > y_diff){	// Wider than taller
		// Top right
		vert[1].x = x2 + 1;
		vert[1].y = y2;

		// Bottom left
		vert[2].x = x1;
		vert[2].y = y1 + 1;

		// Bottom right
		vert[3].x = x2 + 1;
		vert[3].y = y2 + 1;

	}
	else{	// taller than wider
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


uint8_t crayon_graphics_valid_string(const char *string, uint8_t num_chars){
	unsigned int i = 0;
	while(string[i] != '\0'){
		if(string[i] == '\n'){
			i++;
			continue;
		}

		if(string[i] < ' ' || string[i] > ' ' + num_chars - 1){	// Outside the fontsheet's charset
			return 1;
		}

		i++;
	}

	return 0;
}

uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char *string){
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
		if(string[i] == '\n'){	// This should be able to do a new line (Doesn't seem to work right)
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

	if(i > 0){best_length -= fm->char_spacing.x;}	// Since we have char - 1 spaces

	return best_length;
}

uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char *string){
	uint16_t current_length = 0;
	uint16_t best_length = 0;

	uint8_t distance_from_space;

	unsigned int i = 0;
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

	// Have one less one
	if(i > 0){best_length -= fp->char_spacing.x;}	// Since we have char - 1 spaces

	return best_length;
}


//---------------------------------------------------------------------//


void crayon_graphics_transistion_init(crayon_transition_t *effect, crayon_sprite_array_t *sprite_array,
	void (*f)(crayon_transition_t *, void *), uint32_t duration_in, uint32_t duration_out){

	effect->f = f;

	effect->curr_state = CRAYON_FADE_STATE_NONE;
	effect->resting_state = CRAYON_FADE_STATE_NONE;
	effect->duration_fade_in = duration_in;
	effect->duration_fade_out = duration_out;
	effect->curr_duration = 0;
	effect->prev_duration = 0;

	effect->draw = sprite_array;
	return;
}

void crayon_graphics_transistion_skip_to_state(crayon_transition_t *effect, void *params, uint8_t state){
	if(state != CRAYON_FADE_STATE_IN && state != CRAYON_FADE_STATE_OUT){return;}
	effect->curr_state = state;

	// We set the duration to the end of the state we gave it
	effect->curr_duration = (state == CRAYON_FADE_STATE_IN) ? effect->duration_fade_in : effect->duration_fade_out;
	effect->prev_duration = effect->curr_duration;

	(*effect->f)(effect, params);
	effect->resting_state = ((state == CRAYON_FADE_STATE_OUT)) ? CRAYON_FADE_STATE_RESTING_OUT : CRAYON_FADE_STATE_RESTING_IN;
	effect->curr_state = CRAYON_FADE_STATE_NONE;
	return;
}

void crayon_graphics_transistion_change_state(crayon_transition_t *effect, uint8_t state){
	if(state != CRAYON_FADE_STATE_IN && state != CRAYON_FADE_STATE_OUT){return;}
	effect->curr_state = state;
	effect->resting_state = CRAYON_FADE_STATE_NOT_RESTING;

	effect->curr_duration = 0;

	return;
}

void crayon_graphics_transistion_apply(crayon_transition_t *effect, void *params){
	if(effect->curr_state != CRAYON_FADE_STATE_IN && effect->curr_state != CRAYON_FADE_STATE_OUT){return;}

	effect->prev_duration = effect->curr_duration;
	effect->curr_duration++;
	(*effect->f)(effect, params);

	// Check if the transition has finished
	if((effect->curr_state == CRAYON_FADE_STATE_OUT && effect->curr_duration == effect->duration_fade_out)){
		effect->resting_state = CRAYON_FADE_STATE_RESTING_OUT;
		effect->curr_state = CRAYON_FADE_STATE_NONE;
	}
	else if(effect->curr_state == CRAYON_FADE_STATE_IN && effect->curr_duration == effect->duration_fade_in){
		effect->resting_state = CRAYON_FADE_STATE_RESTING_IN;
		effect->curr_state = CRAYON_FADE_STATE_NONE;
	}
	else{
		effect->resting_state = CRAYON_FADE_STATE_NOT_RESTING;
	}

	return;
}

double crayon_graphics_transition_get_curr_percentage(crayon_transition_t *effect){
	if(effect->curr_state == CRAYON_FADE_STATE_IN){
		return (effect->duration_fade_in - effect->curr_duration) / (double)effect->duration_fade_in;
	}

	// Fade out
	return effect->curr_duration / (double)effect->duration_fade_out;
}

double crayon_graphics_transition_get_prev_percentage(crayon_transition_t *effect){
	if(effect->curr_state == CRAYON_FADE_STATE_IN){
		return (effect->duration_fade_in - effect->prev_duration) / (double)effect->duration_fade_in;
	}

	// Fade out
	return effect->prev_duration / (double)effect->duration_fade_out;
}


//---------------------------------------------------------------------//


// If side is true, we check >=, else its <
static uint8_t _crayon_graphics_point_inbounds(float val, float axis, uint8_t side){
	if(side){return (val >= axis);}
	return (val < axis);
}

// If we are going here, we're assuming the lines DO intersect somewhere
	// If given x value, we're trying to find y and vica versa
static float _crayon_graphic_line_plane_intersect(vec2_f_t *v0, vec2_f_t *v1, float curr_val, uint8_t vertical_plane){
	float m, c;
	if(vertical_plane){  // Have x, finding y
		// To avoid divide by zero error
		if(v0->x == v1->x){  // In our use case, this should never trigger since we know the line couldn't be paralle
			return v0->y;
		}

		// Turn our line segment into "y = mx + c" then plug in x (curr_val) and we have y
		m = (v1->y - v0->y) / (v1->x - v0->x);
		c = v0->y - (m * v0->x);

		// We want to get the y value and we have the x value
			// This version doesn't work, but if it did it would be faster than the other method
		//float ratio = abs(x0 - curr_val) / abs(x0 - x1);  // Ratio of how far the point is between x0 and x1
		//return y0 + (abs(y0 - y1) * ratio);   // The y coord
	}
	else{  // Have y, finding x
		// To avoid divide by zero error
		if(v0->y == v1->y){  // Never used for us
			return v0->x;
		}

		// Rather than doing "y = mx + c", I instead do "x = my + c" and this still gets us the right result.
		m = (v1->x - v0->x) / (v1->y - v0->y);
		c = v0->x - (m * v0->y);
	}

	return (m * curr_val) + c;
}

uint8_t crayon_graphics_sutherland_hodgman(vec2_f_t *vert_coords, float *camera_coords){
	// L, T, R, B
	uint8_t crop_len = 4;
	uint8_t new_len;	// Set to 0 later

	// Some vars
	uint8_t in1, in2;  // 1 if value is in, 0 if not
	uint8_t offset1, offset2;
	uint8_t axis = 1;	// 1 = Data is from indexes 0 - 7, 0 = indexes 8 - 15
	unsigned int i, j, prev_vert_index;
	for(j = 0; j < 4; j++){
		new_len = 0;
		#define _CRAYON_VERT_ARRAY_MID 8
		offset1 = axis ? _CRAYON_VERT_ARRAY_MID : 0;
		offset2 = axis ? 0 : _CRAYON_VERT_ARRAY_MID;
		#undef _CRAYON_VERT_ARRAY_MID
		prev_vert_index = offset2 + (crop_len - 1);	// The last vert in the list

		// Checking if the vertex is in or out of bounds
		in1 = _crayon_graphics_point_inbounds(axis ? vert_coords[prev_vert_index].x : vert_coords[prev_vert_index].y, camera_coords[j], (j < 2));

		for(i = 0; i < crop_len; i++){
			in2 = _crayon_graphics_point_inbounds(axis ? vert_coords[offset2 + i].x : vert_coords[offset2 + i].y, camera_coords[j], (j < 2));

			if(in1 && in2){	// both IN, Save vert i
				if(new_len >= 8){printf("AN ERROR OCCURED"); __CRAYON_GRAPHICS_DEBUG_VARS[0] = 0; return 0;}	// This should never trigger
				vert_coords[offset1 + new_len].x = vert_coords[offset2 + i].x;
				vert_coords[offset1 + new_len].y = vert_coords[offset2 + i].y;
				new_len++;
			}
			else if(in1 || in2){	// One IN, other OUT. We'll make v1' and save it
				if(new_len >= 8){printf("AN ERROR OCCURED"); __CRAYON_GRAPHICS_DEBUG_VARS[0] = 1; return 0;}	// This should never trigger
				if(axis){
					vert_coords[offset1 + new_len].x = camera_coords[j];
					vert_coords[offset1 + new_len].y = _crayon_graphic_line_plane_intersect(&vert_coords[prev_vert_index],
						&vert_coords[offset2 + i], camera_coords[j], axis);
				}
				else{
					vert_coords[offset1 + new_len].x = _crayon_graphic_line_plane_intersect(&vert_coords[prev_vert_index],
						&vert_coords[offset2 + i], camera_coords[j], axis);
					vert_coords[offset1 + new_len].y = camera_coords[j];
				}

				if(in2){	// OUT-IN, save v2 aswell
					vert_coords[offset1 + new_len + 1].x = vert_coords[offset2 + i].x;
					vert_coords[offset1 + new_len + 1].y = vert_coords[offset2 + i].y;
					new_len += 2;
				}
				else{	// IN-OUT
					new_len++;
				}
			}
			// If both are OUT, we discard both

			// Update the prev vert stuff
			in1 = in2;
			prev_vert_index = i + offset2;
		}
		if(new_len < 3){	// If we disabled OOB check and poly is entirely OOB, this can happen. Also don't render lines/points
			return new_len;
		}
		crop_len = new_len;
		axis = !axis;  // True on j = 0 and 2
	}

	return new_len;
}

uint8_t crayon_graphics_aabb_obb_overlap(vec2_f_t *obb, float *aabb){
	vec2_f_t *range = crayon_graphics_get_range(obb);

	// Check with aabb's normals
	if(range[1].x < aabb[0] || range[1].y < aabb[1] || range[0].x > aabb[2] || range[0].y > aabb[3]){
		return 0;
	}

	vec2_f_t normal_1 = crayon_graphics_unit_vector(obb[1].x - obb[0].x, obb[1].y - obb[0].y);  // Left side's normal
	vec2_f_t normal_2 = crayon_graphics_unit_vector(obb[3].x - obb[0].x, obb[3].y - obb[0].y);  // Top side's normal

	// // Quicker, but they aren't *unit* vectors So there might be issues with really big numbers
	// normal_1.x = -(obb[1]->x - obb[0]->x);
	// normal_1.y = obb[1]->y - obb[0]->y;

	// normal_2.x = -(obb[2]->x - obb[1]->x);
	// normal_2.y = obb[2]->y - obb[1]->y;

	if(!crayon_graphics_seperating_axis_theorem(obb, aabb, &normal_1)){
		return 0;
	}

	if(!crayon_graphics_seperating_axis_theorem(obb, aabb, &normal_2)){
		return 0;
	}

	return 1;
}

uint8_t crayon_graphics_seperating_axis_theorem(vec2_f_t *obb, float *aabb, vec2_f_t *normal){
	vec2_f_t range[2];	// min_obb, max_obb, min_aabb, max_aabb
	range[0].x = crayon_graphics_dot_product(obb[0].x, obb[0].y, normal->x, normal->y);
	range[0].y = range[0].x;
	range[1].x = crayon_graphics_dot_product(aabb[0], aabb[1], normal->x, normal->y);
	range[1].y = range[1].x;

	unsigned int i;
	float dot_value;
	for(i = 1; i < 4; i++){
		dot_value = crayon_graphics_dot_product(obb[i].x, obb[i].y, normal->x, normal->y);
		if(dot_value < range[0].x){range[0].x = dot_value;}
		if(dot_value > range[0].y){range[0].y = dot_value;}

		// i = 0: min/min, 1: max/min, 2: max/max, 3: min/max
		dot_value = crayon_graphics_dot_product(aabb[(i < 3) ? 2 : 0], aabb[(i > 1 ? 3 : 1)], normal->x, normal->y);
		if(dot_value < range[1].x){range[1].x = dot_value;}
		if(dot_value > range[1].y){range[1].y = dot_value;}
	}

	// max_obb < min_aabb || min_obb > max_aabb
	if(range[0].y < range[1].x || range[0].x > range[1].y){
		return 0;
	}

	return 1;
}

vec2_f_t *crayon_graphics_get_range(vec2_f_t *vals){
	static vec2_f_t ret[2];	// min x, min y, max x, max y
	ret[0].x = vals[0].x;
	ret[0].y = vals[0].y;
	ret[1].x = vals[0].x;
	ret[1].y = vals[0].y;

	unsigned int i;
	for(i = 1; i < 4; i++){	// Skip the first element since we've already checked it
		if(vals[i].x < ret[0].x){ret[0].x = vals[i].x;}
		if(vals[i].x > ret[1].x){ret[1].x = vals[i].x;}

		if(vals[i].y < ret[0].y){ret[0].y = vals[i].y;}
		if(vals[i].y > ret[1].y){ret[1].y = vals[i].y;}
	}
	return ret;
}

float crayon_graphics_dot_product(float x1, float y1, float x2, float y2){
	return (x1 * x2) + (y1 * y2);
}

// Only ever called once (In crayon_graphics_unit_vector() )
inline float crayon_graphics_magnitude(float x, float y){
	return sqrt(pow(x, 2) + pow(y, 2));
}

vec2_f_t crayon_graphics_unit_vector(float x, float y){
	float div = 1 / crayon_graphics_magnitude(x, y);
	return (vec2_f_t){x * div, y * div};
}

// How to check if OOB of two axis aligned boundry boxes
	// We pass in the boundries of the polys. Since they're all axis aligned, this is fine
	// Eg if The left X vert of the camera is 150 and all the sprite X verts are less than 150 then its OOB
uint8_t crayon_graphics_aabb_aabb_overlap(vec2_f_t *vS, vec2_f_t *vC){
	if(vS[1].x < vC[0].x || vS[0].x > vC[1].x || vS[1].y < vC[0].y || vS[0].y > vC[1].y){
		return 0;
	}

	// No OOBs detected
	return 1;
}

uint8_t crayon_graphics_round_way(float value){
	return (value - (int)value > 0.5) ? 1 : 0;
}
