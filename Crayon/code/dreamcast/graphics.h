#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehsheet and anim structs
#include "render_structs.h"  //For the crayon_textured_array struct

#include <stdlib.h>
#include <math.h>

//Sets a palette for a spritesheet
extern int graphics_setup_palette(const crayon_palette_t *cp);

//Give an animation struct, frame, coordinates and frame and it will populate the coordinates
extern void graphics_frame_coordinates(const struct crayon_animation *anim, uint16_t *frame_x,
	uint16_t *frame_y, uint8_t frame);


//------------------Drawing Untextured polys------------------//


//Queue a colour/alpha poly to be rendered. I recomment the array version, but for simple implementation this is good
extern void graphics_draw_untextured_poly(uint16_t draw_x, uint16_t draw_y, uint16_t draw_z,
	uint16_t dim_x, uint16_t dim_y, uint32_t colour, uint8_t opaque);

//Draw all coloured polys in the struct's list
extern void graphics_draw_untextured_array(crayon_untextured_array_t *poly_array);


//------------------Drawing Spritesheets------------------//


//Queue a texture to be rendered (If RGB565 or ARGB4444 then paletteNumber is never read)
extern uint8_t graphics_draw_sprite(const struct crayon_spritesheet *ss,
	const struct crayon_animation *anim, float draw_x, float draw_y, float draw_z,
	float scale_x, float scale_y, uint16_t frame_x, uint16_t frame_y,
	uint8_t paletteNumber);

//Testing drawing multiple of same thing CHANGE fd_size TO "BOOLEAN" (0/1)
	//Will be removed soon
extern uint8_t graphics_draw_sprites_OLD(const struct crayon_spritesheet *ss,
	const struct crayon_animation *anim, uint16_t *draw_coords, uint16_t *frame_data, uint16_t fd_size,
	uint16_t num_sprites, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber);

//(Delete other multi sprite draw after and keep original as a learner version for noobies)
//poly_list mode is for the tr/pt/op render list macro we want to use. Might move this param into the struct...
extern uint8_t crayon_graphics_draw_sprites(crayon_textured_array_t *sprite_array, uint8_t poly_list_mode);	//UNFINISHED (No colour/rotation yet)

//The version with polygons (Use this if your spritesheet is bigger than 256 by 256)
extern uint8_t crayon_graphics_draw_polys(crayon_textured_array_t *sprite_array, uint8_t poly_list_mode);	//UNIMPLEMENTED

//------------------Drawing Fonts------------------//


//Draw string using mono font (string must be null-terminated)
extern uint8_t graphics_draw_text_mono(const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber, char * string);

//Draw string using propertional font (string must be null-terminated)
extern uint8_t graphics_draw_text_prop(const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, float draw_z, float scale_x, float scale_y, uint8_t paletteNumber, char * string);


#endif
