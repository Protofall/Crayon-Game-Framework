#ifndef GRAPHICS_CRAYON_H
#define GRAPHICS_CRAYON_H

#include "misc.h"
#include "texture_structs.h"  // For the spritehsheet and anim structs
#include "render_structs.h"  // For the crayon_sprite_array struct
#include "vector_structs.h"
#include "memory.h"  // For the rotate function struct

#include <stdlib.h>
#include <string.h>	// For length of string
#include <math.h>

// For region and htz stuff
#include <dc/flashrom.h>

#define CRAYON_OP_LIST PVR_LIST_OP_POLY	// No alpha
#define CRAYON_TR_LIST PVR_LIST_TR_POLY	// Alpha is either full on or off
#define CRAYON_PT_LIST PVR_LIST_PT_POLY	// Varying alpha

#define CRAYON_FILTER_NEAREST PVR_FILTER_NONE
#define CRAYON_FILTER_LINEAR PVR_FILTER_LINEAR

// The draw_mode options
#define CRAYON_DRAW_SIMPLE (0 << 0)
#define CRAYON_DRAW_ENHANCED (1 << 0)
#define CRAYON_DRAW_CHECK_OOB (1 << 1)	// If the sprite is entirely OOB, then go to next sprite
#define CRAYON_DRAW_HARDWARE_CROP (1 << 2)	// On PC uses Scissor Test, DC uses TA for 32x32 tiles
#define CRAYON_DRAW_SOFTWARE_CROP (1 << 3)	// Only used for systems where we can't use hardware cropping everywhere
#define CRAYON_DRAW_CROP CRAYON_DRAW_SOFTWARE_CROP | CRAYON_DRAW_HARDWARE_CROP	// It will attempt to use hardware cropping if all the edges line up
#define CRAYON_DRAW_FULL_CROP CRAYON_DRAW_CROP | CRAYON_DRAW_CHECK_OOB

// This var's purpose is to make debugging the render-ers and other graphics function much easier
	// Since I currently can't print any text while rendering an object, instead I can set vars to
	// this variable and render them later since this is global
extern float __CRAYON_GRAPHICS_DEBUG_VARS[16];

extern uint16_t __htz;
extern float __htz_adjustment;

#define CRAYON_ENABLE_OP (1 << 0)
#define CRAYON_ENABLE_TR (1 << 1)
#define CRAYON_ENABLE_PT (1 << 2)
uint8_t crayon_graphics_init(uint8_t poly_modes);
void crayon_graphics_shutdown();

// Sets a palette for a spritesheet
uint8_t crayon_graphics_setup_palette(const crayon_palette_t *cp);

// This will get the size of the draw element as it appears on screen. It takes scale into consideration
	// Later might want to trunc it and also add viewport support when that happens
float crayon_graphics_get_draw_element_width(const crayon_sprite_array_t *sprite_array, uint8_t id);
float crayon_graphics_get_draw_element_height(const crayon_sprite_array_t *sprite_array, uint8_t id);

// This gets the width and height of the screen in pixels
uint32_t crayon_graphics_get_window_width();
uint32_t crayon_graphics_get_window_height();


//------------------Drawing Sprites from Spritesheets------------------//


// Queue a texture to be rendered, draw mode right most bit is true for enhanced
int8_t crayon_graphics_draw_sprites(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t draw_option);


//------------------Internal Drawing Functions------------------//


// poly_list mode is for the tr/pt/op render list macro we want to use.
// Uses the Dreamcast sprites/quads for faster/more efficient rendering
uint8_t crayon_graphics_draw_sprites_simple(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t options);

// The version with polygons (Use this if your spritesheet is bigger than 256 by 256)
uint8_t crayon_graphics_draw_sprites_enhanced(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t options);

// This will draw untextured polys (Sprite_arrays with no texture set)
uint8_t crayon_graphics_draw_untextured_sprites(const crayon_sprite_array_t *sprite_array, const crayon_viewport_t *camera,
	uint8_t poly_list_mode, uint8_t options);


//------------------Drawing Fonts------------------//


// Draw string using mono font (string must be null-terminated)
uint8_t crayon_graphics_draw_text_mono(char *string, const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);

// Draw string using propertional font (string must be null-terminated)
uint8_t crayon_graphics_draw_text_prop(char *string, const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number);


//------------------Drawing Other------------------//


void crayon_graphics_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t layer, uint32_t colour,
	uint8_t poly_list_mode);


//------------------String Info------------------//


// Checks to see if a string has any illegal characters in it
	// This means characters like tabs are considerred illegal
uint8_t crayon_graphics_valid_string(const char *string, uint8_t num_chars);

// This gets the drawn-length of a string using a certain font. But only the longest line
uint16_t crayon_graphics_string_get_length_mono(const crayon_font_mono_t *fm, char *string);
uint16_t crayon_graphics_string_get_length_prop(const crayon_font_prop_t *fp, char *string);


//------------------Transition Effects------------------//


void crayon_graphics_transistion_init(crayon_transition_t *effect, crayon_sprite_array_t *sprite_array,
	void (*f)(crayon_transition_t *, void *), uint32_t duration_in, uint32_t duration_out);
		// Thats how you pass in a function. But unsure if that'll work if I want to add it as param for the struct
		// Note: We assume the sprite_array is already initialised

// This will skip to the end of a transition
void crayon_graphics_transistion_skip_to_state(crayon_transition_t *effect, void *params, uint8_t state);

// You'll need to call apply to see the change
void crayon_graphics_transistion_change_state(crayon_transition_t *effect, uint8_t state);

// This will bring it close to either fade-in or fade-out
void crayon_graphics_transistion_apply(crayon_transition_t *effect, void *params);

// Will give you a percentage where 0 is fully faded in and 1 is fully faded out
double crayon_graphics_transition_get_curr_percentage(crayon_transition_t *effect);
double crayon_graphics_transition_get_prev_percentage(crayon_transition_t *effect);

uint8_t crayon_graphics_transistion_resting_state(crayon_transition_t *effect);

//  #define crayon_savefile_get_valid_screens(effect, camera, poly_list_mode, draw_mode)  crayon_graphics_draw_sprites(effect->draw, camera, poly_list_mode, draw_mode)

// This is just a call to the regular draw function
//  inline uint8_t crayon_graphics_draw_transistion(const crayon_transition_t *effect, const crayon_viewport_t *camera,
//  	uint8_t poly_list_mode, uint8_t draw_mode){
//  	return crayon_graphics_draw_sprites(effect->draw, camera, poly_list_mode, draw_mode);
//  }


//------------------Misc. Internal Functions------------------//


// Will take a 4-sided poly and return a poly that is cropped so its entirely within the camera window
  // This video taught it so well, very easy to follow:
  // https://www.youtube.com/watch?v=Euuw72Ymu0M
uint8_t crayon_graphics_sutherland_hodgman(vec2_f_t *vert_coords, float *camera_coords);

// Sees if a sprite (rectangle obb) is overlapping with the camera (Assumes camera)
	// It assumes the verts are in clockwise order. But *might* work for other orders by chance

// Returns 1 if the shapes overlap, 0 otherwise
// AABB = Axis Aligned Boundry Box (Camera). OBB = Oriented Boundry Box (Sprite/Poly)
// Two shapes only overlap if they have overlap when "projected" onto every normal axis of both shapes.
// OBB-OBB uses 8 normal/seperating axises, one for every side. However AABB-OBB only uses 4 since 4 pairs of axises are always parallel
// For them to overlap, the shapes "crushed" vertexes have to overlap on every axis
  // So if theres even one axis where the crushed vertexes don't intersect, then the whole thing doesn't overlap.
// obb is an array of 4 vec2_f_ts. aabb is just 4 floats, the min_x, min_y, max_x, max_y of the camera verts
uint8_t crayon_graphics_aabb_obb_overlap(vec2_f_t *obb, float *aabb);

// Returns 1 if overlap, 0 if they don't
uint8_t crayon_graphics_seperating_axis_theorem(vec2_f_t *obb, float *aabb, vec2_f_t *normal);

// Gets the min and max x and y values and returns them. Assumes vals is an array of 4 structs
	// NOTE. The return value is a static array that persists after the function ends. However calling the function again will override
	// the old data. However currently its only used once so this isn't an issue
vec2_f_t *crayon_graphics_get_range(vec2_f_t *vals);

// The dot product. Produces a scalar (Single value).
// The dot product of X and Y is the length of the projection of A onto B multiplied by the length of B (or the other way around).
// https://physics.stackexchange.com/questions/14082/what-is-the-physical-significance-of-dot-cross-product-of-vectors-why-is-divi
float crayon_graphics_dot_product(float x1, float y1, float x2, float y2);

vec2_f_t crayon_graphics_unit_vector(float x, float y);

// Checks if float a is equal to b (+ or -) epsilon
uint8_t crayon_graphics_almost_equals(float a, float b, float epsilon);

// Returns a crop range to see which edges need to be cropped
// ---- BRTL (Bottom, Right, Top, Left)
	// Arrays are just the top left and bottom right verts of the boxes
uint8_t crayon_graphics_check_intersect(vec2_f_t *vS, vec2_f_t *vC);

// Checks if vS is partially inside of vC.
	// Arrays are just the top left and bottom right verts of the boxes
uint8_t crayon_graphics_aabb_aabb_overlap(vec2_f_t *vS, vec2_f_t *vC);

// This function will return 0 (0.5 or less) if the number would be rounded down and
// returns 1 ( more than 0.5) if it would be rounded up.
	// CURRENTLY UNUSED
uint8_t crayon_graphics_round_way(float value);

// vert is the nth vert (Backwards C shaped)
vec2_f_t crayon_graphics_get_sprite_vert(pvr_sprite_txr_t *sprite, uint8_t vert);

#endif
