#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehsheet and anim structs
#include "render_structs.h"  //For the crayon_sprite_array struct
#include "vector_structs.h"  //For the rotate function struct

#include <stdlib.h>
#include <string.h>	//For length of string
#include <math.h>

#define CRAY_OP_LIST PVR_LIST_OP_POLY	//No alpha
#define CRAY_TR_LIST PVR_LIST_TR_POLY	//Alpha is either full on or off
#define CRAY_PT_LIST PVR_LIST_PT_POLY	//Varying alpha

#define CRAY_FILTER_NEAREST PVR_FILTER_NONE
#define CRAY_FILTER_LINEAR PVR_FILTER_LINEAR

//Might want to replace these with inline functions later
#define crayon_graphics_wait_ready() pvr_wait_ready()
#define crayon_graphics_scene_begin() pvr_scene_begin()
#define crayon_graphics_list_begin(list) pvr_list_begin(list)
#define crayon_graphics_list_finish() pvr_list_finish()
#define crayon_graphics_scene_finish() pvr_scene_finish()

#define CRAY_SCREEN_DRAW_SIMPLE 0
#define CRAY_SCREEN_DRAW_ENHANCED 1
#define CRAY_CAMERA_DRAW_SIMPLE (1 << 1)	//10 in binary
#define CRAY_CAMERA_DRAW_ENHANCED 3 //This is 11 in binary

//Internal usage
#define CRAY_USING_CAMERA (1 << 1)

//Sets a palette for a spritesheet
extern uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp);

//This will get the size of the draw element as it appears on screen. It takes scale into consideration
	//Later might want to trunc it and also add viewport support when that happens
extern float crayon_graphics_get_draw_element_width(const crayon_sprite_array_t *sprite_array, uint8_t id);
extern float crayon_graphics_get_draw_element_height(const crayon_sprite_array_t *sprite_array, uint8_t id);


//------------------Drawing Sprites from Spritesheets------------------//


//Queue a texture to be rendered, draw mode right most bit is true for enhanced (false for now)
//And second right most bit will later be used for camera mode control
extern int8_t crayon_graphics_draw_sprites(const crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode, uint8_t draw_mode);


//------------------Internal Drawing Functions------------------//


//poly_list mode is for the tr/pt/op render list macro we want to use.
extern uint8_t crayon_graphics_draw_sprites_simple(const crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode);

//The version with polygons (Use this if your spritesheet is bigger than 256 by 256)
	//For DC this uses "poly mode"
extern uint8_t crayon_graphics_draw_sprites_enhanced(const crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode);

//This will draw untextured polys (Sprite_arrays with no texture set)
extern uint8_t crayon_graphics_draw_untextured_array(const crayon_sprite_array_t *sprite_array, uint8_t poly_list_mode);


//------------------Drawing Fonts------------------//


//Draw string using mono font (string must be null-terminated)
extern uint8_t crayon_graphics_draw_text_mono(char * string, const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);

//Draw string using propertional font (string must be null-terminated)
extern uint8_t crayon_graphics_draw_text_prop(char * string, const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);


//------------------Drawing Lines------------------//


extern void crayon_graphics_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t layer, uint32_t colour,
	uint8_t poly_list_mode);


//------------------String Info------------------//


//Checks to see if a string has any illegal characters in it
	//This means characters like tabs are considerred illegal
extern uint8_t crayon_graphics_valid_string(const char *string, uint8_t num_chars);

//This gets the drawn-length of a string using a certain font
//If newlines is true then it will only return the longest line
extern uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char * string, uint8_t newlines);	//UNTESTED
extern uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char * string, uint8_t newlines);


//------------------Misc. Internal Functions------------------//

//Returns the point "center_x/y" rotated "radian" degrees around "orbit_x/y"
extern vec2_f_t crayon_graphics_rotate_point(vec2_f_t center, vec2_f_t orbit, float radians);

//Checks if float a is equal to b (+ or -) epsilon
extern uint8_t crayon_graphics_almost_equals(float a, float b, float epsilon);


#endif
