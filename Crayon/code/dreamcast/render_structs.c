#include "render_structs.h"

extern float crayon_get_coord_x(crayon_sprite_array_t * sprites, uint16_t index){
	return (index < sprites->list_size) ? sprites->coord[index].x : 0;
}

extern float crayon_get_coord_y(crayon_sprite_array_t * sprites, uint16_t index){
	return (index < sprites->list_size) ? sprites->coord[index].y : 0;
}

extern vec2_f_t crayon_get_coords(crayon_sprite_array_t * sprites, uint16_t index){
	return (index < sprites->list_size) ? sprites->coord[index] : (vec2_f_t){0, 0};
}

extern uint32_t crayon_get_colour(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_COLOUR) && index == 0) || index < sprites->list_size){
		return sprites->colour[index];
	}
	return 0;
}

extern uint8_t crayon_get_fade(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_COLOUR) && index == 0) || index < sprites->list_size){
		return sprites->fade[index];
	}
	return 0;
}

extern float crayon_get_scale_x(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_SCALE) && index == 0) || index < sprites->list_size){
		return sprites->scale[index].x;
	}
	return 0;
}

extern float crayon_get_scale_y(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_SCALE) && index == 0) || index < sprites->list_size){
		return sprites->scale[index].y;
	}
	return 0;
}

extern vec2_f_t crayon_get_scales(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_SCALE) && index == 0) || index < sprites->list_size){
		return sprites->scale[index];
	}
	return (vec2_f_t){0, 0};
}

extern uint8_t crayon_get_flip(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_FLIP) && index == 0) || index < sprites->list_size){
		return sprites->flip[index];
	}
	return 0;
}

extern float crayon_get_rotation(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_ROTATE) && index == 0) || index < sprites->list_size){
		return sprites->rotation[index];
	}
	return 0;
}

extern uint8_t crayon_get_layer(crayon_sprite_array_t * sprites, uint16_t index){
	if(((sprites->options & CRAY_MULTI_LAYER) && index == 0) || index < sprites->list_size){
		return sprites->layer[index];
	}
	return 0;
}

extern uint8_t crayon_get_frame_coord_key(crayon_sprite_array_t * sprites, uint16_t index){
	return (index < sprites->frames_used) ? sprites->frame_coord_key[index] : 0;
}

extern vec2_u16_t crayon_get_frame_coord_map(crayon_sprite_array_t * sprites, uint16_t index){
	return (index < sprites->frames_used) ? sprites->frame_coord_map[index] : (vec2_u16_t){0, 0};
}
