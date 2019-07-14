#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehsheet and anim structs
#include "render_structs.h"  //For the crayon_sprite_array struct

#include <stdlib.h>
#include <string.h>	//For length of string
#include <math.h>

//Sets a palette for a spritesheet
extern uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp);

//Give a render struct, map index, and animation frame and it will populate the coordinates
extern void crayon_graphics_frame_coordinates(const crayon_sprite_array_t *draw_list, uint8_t index, uint8_t frame);

//This will get the size of the draw element as it appears on screen. It takes scale into consideration
	//Later might want to trunc it and also add viewport support when that happens
extern float crayon_graphics_get_draw_element_width(const crayon_sprite_array_t *draw_list, uint8_t id);
extern float crayon_graphics_get_draw_element_height(const crayon_sprite_array_t *draw_list, uint8_t id);


//------------------Drawing Untextured polys------------------//


//Queue a colour/alpha poly to be rendered. I recommend the array version, but for simple implementation this is good
extern void crayon_graphics_draw_untextured_poly(float draw_x, float draw_y, uint8_t layer,
	uint16_t dim_x, uint16_t dim_y, uint32_t colour, uint8_t poly_list_mode);

//Draw all coloured polys in the struct's list
extern void crayon_graphics_draw_untextured_array(crayon_untextured_array_t *poly_array, uint8_t poly_list_mode);

extern void crayon_graphics_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t layer, uint32_t colour,
	uint8_t poly_list_mode);


//------------------Drawing Sprites from Spritesheets------------------//


//Queue a texture to be rendered, draw mode right most bit is true for enhanced (false for non)
//And second right most bit will later be used for camera mode control
extern uint8_t crayon_graphics_draw(crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode, uint8_t draw_mode);


//------------------Internal Drawing Functions------------------//


//poly_list mode is for the tr/pt/op render list macro we want to use.
extern uint8_t crayon_graphics_draw_sprites(crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode);

//The version with polygons (Use this if your spritesheet is bigger than 256 by 256)
	//For DC this uses "poly mode"
extern uint8_t crayon_graphics_draw_sprites_enhanced(crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode);

//------------------Rotation Stuff------------------//


extern uint8_t crayon_graphics_almost_equals(float a, float b, float epsilon);


//------------------Drawing Fonts------------------//


//Draw string using mono font (string must be null-terminated)
extern uint8_t crayon_graphics_draw_text_mono(char * string, const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);

//Draw string using propertional font (string must be null-terminated)
extern uint8_t crayon_graphics_draw_text_prop(char * string, const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);


//------------------String info------------------//


//Checks to see if a string has any illegal characters in it
	//This means characters like tabs are considerred illegal
extern uint8_t crayon_graphics_valid_string(const char *string, uint8_t num_chars);

//This gets the drawn-length of a string using a certain font
//If newlines is true then it will only return the longest line
extern uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char * string, uint8_t newlines);	//UNTESTED
extern uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char * string, uint8_t newlines);


//------------------Internal graphics functions------------------//

extern float crayon_graphics_rotate_point_x(float center_x, float center_y, float orbit_x, float orbit_y, float radians);
extern float crayon_graphics_rotate_point_y(float center_x, float center_y, float orbit_x, float orbit_y, float radians);


#endif
