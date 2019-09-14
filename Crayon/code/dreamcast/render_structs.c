#include "render_structs.h"

extern float crayon_get_coord_x(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->coord[index].x;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern float crayon_get_coord_y(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->coord[index].y;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern vec2_f_t crayon_get_coords(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->coord[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return (vec2_f_t){0, 0};
}

extern uint32_t crayon_get_colour(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_COLOUR) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->colour[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern uint8_t crayon_get_fade(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_COLOUR) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->fade[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern float crayon_get_scale_x(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_SCALE) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->scale[index].x;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern float crayon_get_scale_y(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_SCALE) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->scale[index].y;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern vec2_f_t crayon_get_scales(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_SCALE) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->scale[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return (vec2_f_t){0, 0};
}

extern uint8_t crayon_get_flip(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_FLIP) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->flip[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern float crayon_get_rotation(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_ROTATE) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->rotation[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern uint8_t crayon_get_layer(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_LAYER) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->layer[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern uint8_t crayon_get_frame_id(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(((sprites->options & CRAY_MULTI_FRAME) && index == 0) || index < sprites->list_size){
		if(error){*error = 0;}
		return sprites->frame_coord_key[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

extern vec2_u16_t crayon_get_frame_uv(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error){
	if(index < sprites->frames_used){
		if(error){*error = 0;}
		return sprites->frame_coord_map[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return (vec2_u16_t){0, 0};
}
