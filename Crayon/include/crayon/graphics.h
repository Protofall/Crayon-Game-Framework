#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "texture_structs.h"  //For the spritehsheet and anim structs
#include "render_structs.h"  //For the crayon_sprite_array struct
#include "vector_structs.h"  //For the rotate function struct
#include "memory.h"  //For the rotate function struct

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

#define CRAY_DRAW_SIMPLE 0
#define CRAY_DRAW_ENHANCED 1

//This var's purpose is to make debugging the render-ers and other graphics function much easier
	//Since I currently can't print any text while rendering an object, instead I can set vars to
	//this variable and render them later since this is global
float __GRAPHICS_DEBUG_VARIABLES[16];

//Sets a palette for a spritesheet
extern uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp);

//This will get the size of the draw element as it appears on screen. It takes scale into consideration
	//Later might want to trunc it and also add viewport support when that happens
extern float crayon_graphics_get_draw_element_width(const crayon_sprite_array_t *sprite_array, uint8_t id);
extern float crayon_graphics_get_draw_element_height(const crayon_sprite_array_t *sprite_array, uint8_t id);

//This gets the width and height of the screen in pixels
extern uint32_t crayon_graphics_get_window_width();
extern uint32_t crayon_graphics_get_window_height();


//------------------Drawing Sprites from Spritesheets------------------//


//Queue a texture to be rendered, draw mode right most bit is true for enhanced
extern int8_t crayon_graphics_draw_sprites(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t draw_mode);


//------------------Internal Drawing Functions------------------//


//poly_list mode is for the tr/pt/op render list macro we want to use.
//The version with polygons (Use this if your spritesheet is bigger than 256 by 256)
	//For DC this uses "poly mode"
extern uint8_t crayon_graphics_draw_sprites_enhanced(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode);

//This will draw untextured polys (Sprite_arrays with no texture set)
extern uint8_t crayon_graphics_draw_untextured_array(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode);

//Like the other simple draw one, but this uses a camera to control where on screen to render and what region to show
extern uint8_t crayon_graphics_draw_sprites_simple(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode);

//DELETE THIS LATER
extern uint8_t crayon_graphics_draw_sprites_simple_POLY_TEST(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode);


//------------------Drawing Fonts------------------//


//Draw string using mono font (string must be null-terminated)
extern uint8_t crayon_graphics_draw_text_mono(char * string, const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);

//Draw string using propertional font (string must be null-terminated)
extern uint8_t crayon_graphics_draw_text_prop(char * string, const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);


//------------------Drawing Other------------------//


extern void crayon_graphics_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t layer, uint32_t colour,
	uint8_t poly_list_mode);


//------------------String Info------------------//


//Checks to see if a string has any illegal characters in it
	//This means characters like tabs are considerred illegal
extern uint8_t crayon_graphics_valid_string(const char *string, uint8_t num_chars);

//This gets the drawn-length of a string using a certain font. But only the longest line
extern uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char * string);
extern uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char * string);


//------------------Transition Effects------------------//


extern void crayon_graphics_transistion_init(crayon_transition_t * effect, crayon_sprite_array_t * sprite_array,
	void (*f)(crayon_transition_t *, void *), uint32_t duration_in, uint32_t duration_out);
		//Thats how you pass in a function. But unsure if that'll work if I want to add it as param for the struct
		//Note: We assume the sprite_array is already initialised

//This will skip to the end of a transition
extern void crayon_graphics_transistion_skip_to_state(crayon_transition_t * effect, void * params, uint8_t state);

//You'll need to call apply to see the change
extern void crayon_graphics_transistion_change_state(crayon_transition_t * effect, uint8_t state);

//This will bring it close to either fade-in or fade-out
extern void crayon_graphics_transistion_apply(crayon_transition_t * effect, void * params);

//Will give you a percentage where 0 is fully faded in and 1 is fully faded out
extern double crayon_graphics_transition_get_curr_percentage(crayon_transition_t * effect);
extern double crayon_graphics_transition_get_prev_percentage(crayon_transition_t * effect);

//Returns NOT_RESTING if its not resting, RESTING_STATE_IN if its finished fading in and RESTING_STATE_OUT if its finished fading out
extern uint8_t crayon_graphics_transistion_resting_state(crayon_transition_t * effect);

//This is just a call to the regular draw function
// inline uint8_t crayon_graphics_draw_transistion(const crayon_transition_t * effect, const crayon_viewport_t * camera,
// 	uint8_t poly_list_mode, uint8_t draw_mode){
// 	return crayon_graphics_draw_sprites(effect->draw, camera, poly_list_mode, draw_mode);
// }



//------------------Misc. Internal Functions------------------//

//Returns the point "center_x/y" rotated "radian" degrees around "orbit_x/y"
extern vec2_f_t crayon_graphics_rotate_point(vec2_f_t center, vec2_f_t orbit, float radians);

//Checks if float a is equal to b (+ or -) epsilon
extern uint8_t crayon_graphics_almost_equals(float a, float b, float epsilon);

//Returns a crop range to see which edges need to be cropped
//---- BRTL (Bottom, Right, Top, Left)
//NOTE: Verts must be passed in in Z order
extern uint8_t crayon_graphics_check_intersect(vec2_f_t vC[4], vec2_f_t vS[4]);

//Checks if an element is entirely outside of another. Verts are in the Z order
extern uint8_t crayon_graphics_check_oob(vec2_f_t vC[4], vec2_f_t vS[4], uint8_t mode);

//This function will return 0 (0.5 or less) if the number would be rounded down and
//returns 1 ( more than 0.5) if it would be rounded up.
	//CURRENTLY UNUSED
extern uint8_t round_way(float value);

//vert is the nth vert (Backwards C shaped)
extern vec2_f_t crayon_graphics_get_sprite_vert(pvr_sprite_txr_t sprite, uint8_t vert);

extern void crayon_graphics_set_sprite_vert_x(pvr_sprite_txr_t *sprite, uint8_t vert, float value);
extern void crayon_graphics_set_sprite_vert_y(pvr_sprite_txr_t *sprite, uint8_t vert, float value);

//Given a 90-degree-increment rotation, flip flag and side, it will return the correct index to modify
extern uint8_t crayon_get_uv_index(uint8_t side, uint8_t rotation_val, uint8_t flip_val);

extern float crayon_graphics_get_texture_divisor(uint8_t side, uint8_t rotation_val, vec2_f_t dims);

extern float crayon_graphics_get_texture_offset(uint8_t side, vec2_f_t * vert, vec2_f_t * scale, const crayon_viewport_t *camera);


#endif
