#ifndef RENDER_STRUCTS_CRAYON_H
#define RENDER_STRUCTS_CRAYON_H

#include "texture_structs.h"  // For the spritesheet and anim structs
#include "vector_structs.h"

#define CRAY_REF_LIST (1 << 0)	// Tells the functions that we want each element to have a reference node
#define CRAY_MULTI_FRAME (1 << 1)
#define CRAY_MULTI_SCALE (1 << 2)
#define CRAY_MULTI_DIM (1 << 2)
#define CRAY_MULTI_FLIP (1 << 3)
#define CRAY_MULTI_ROTATE (1 << 4)
#define CRAY_MULTI_COLOUR (1 << 5) // Use this one for untextured polys
#define CRAY_MULTI_COLOUR_ADD ((1 << 6) | CRAY_MULTI_COLOUR)
#define CRAY_MULTI_COLOUR_BLEND CRAY_MULTI_COLOUR
#define CRAY_NO_MULTIS 0	// Just so when checking the code its easier to read

// Use these if you don't want multis, but want to specify these
#define CRAY_COLOUR_BLEND (0 << 6)
#define CRAY_COLOUR_ADD (1 << 6)

// Internal usage
#define CRAY_HAS_TEXTURE (1 << 7)

// The camera, controls where on screen to render and which part of the world to draw
typedef struct crayon_viewport_t {
	// Top left of the world region and dimensions
	float world_x;	// Must be a float since the draw position array is a bunch of floats
	float world_y;
	uint16_t world_width;
	uint16_t world_height;

	// The scrolling modifier
	float world_movement_factor;

	// Top left of where to render to on screen and dimensions
		// For DC these can all be uint16_t's. For the PC port I think uint16_t is still fine even with
		// larger monitors because a uint16_t is 65535 at most and that still supports 8K (And possibly higher)
	uint16_t window_x;
	uint16_t window_y;
	uint16_t window_width;
	uint16_t window_height;
} crayon_viewport_t;

typedef struct crayon_sprite_array_reference {
	uint16_t id;
	struct crayon_sprite_array_reference *next;
} crayon_sprite_array_reference_t;

// This is designed for the multi-draw functions. If you want to draw a single thing with this struct
// then I you'll still need to go through the multi-draw. It shouldn't be too much slower if any.
typedef struct crayon_sprite_array {
	vec2_f_t *coord;			// Width then Height extracted from anim/frame data,
								// Each element is one pair of coordinates
	uint8_t *layer;				// The layer to help deal with overlapping sprites/polys
	uint8_t *frame_id;			// ids for a pair of frame_uv elements and uses that for
								// drawing. frame_id[i] refers to frame_uv[i].x/y
	vec2_u16_t *frame_uv;		// Each element is UV for one frame of an animation (U is x, V is Y)
	uint32_t *colour;			// For poly mode this dictates the rgb and alpha of a polygon
	uint8_t *fade;				// A part of the colour array. Tells it how to transition between
								// The base colour and the one from the colour array
	vec2_f_t *scale;			// Float incase you want to shrink or enlarge it. For untextured polys this
								// is the height and width of the polys

	uint8_t *flip;				// Mirror the sprite along x-axis (Then rotation is applied)

	float *rotation;			// Poly uses angles to rotate on Z axis, sprite uses
								// booleans/flip bits. Decide what type this should be...
	uint8_t *visible;			// If 1 then the sprite is rendered. Else its skipped
	uint16_t size;				// This tells the draw function how many sprites/polys to draw.
	uint8_t frames_used;		// The number of frames you want to use. Minimum 1

	uint8_t options;			// Format TCCR (Flip)SF(Ref), Basically some booleans options relating to
								// fade/colour, rotation, flip, scale, frame_ids and the reference list
								// If that bit is set to true, then we use the first element of
								// arrays (except map) for all sub-textures
								// Else we assume each sprite has its own unique value
								// T stands for "Textured". Changes how we handle the struct
									// Untextured polys only use The right Colour bit, Rotation and Scale
								// If the bit for reference list is enabled, then init and expand will create
									// new nodes for the new elements

								// Note that the 1st colour bit tells it to use either Blend or Add
								// mode when applying the colour
								// Blend is the "Fade to colour" mode
								// Add just gives a coloured translucent overlay to the sprite

	uint8_t filter;				// 0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2

	crayon_spritesheet_t *spritesheet;
	crayon_animation_t *animation;
	crayon_palette_t *palette;	// Also ask if palettes can start at not multiples of 16 or 256

	crayon_sprite_array_reference_t *head;
} crayon_sprite_array_t;

// Fade in means we're going from full effect, back to the scene (Entering a menu)
// Fade out is reverse (Going back a menu)
#define CRAY_FADE_STATE_NONE 0
#define CRAY_FADE_STATE_IN 1
#define CRAY_FADE_STATE_OUT 2

// Used by crayon_graphics_transistion_resting_state() To tell if we've finished a
// transition or not and where we are
#define CRAY_FADE_STATE_NOT_RESTING 0
#define CRAY_FADE_STATE_RESTING_IN 1
#define CRAY_FADE_STATE_RESTING_OUT 2

typedef struct crayon_transition {
	uint8_t curr_state;	// The state we are going to 0 for not fading, 1 for fade-in and 2 for fade-out
	uint8_t resting_state;	// Is NOT_RESTING if its not resting, RESTING_STATE_IN if its finished fading in
							// and RESTING_STATE_OUT if its finished fading out
	uint32_t duration_fade_in;	// The time in frames it takes to complete the effect fading in
	uint32_t duration_fade_out;	// The time it takes to fade out
	uint32_t curr_duration;	// It will start at zero and go up to either "duration_fade_in" or
							// "duration_fade_out" depending on if its fading in or out
	uint32_t prev_duration;	// Like above, but records the previous frame. Might be useful

	crayon_sprite_array_t *draw;	// If you want you can pass a pointer to an array of them and use them in your function
	void (*f)(struct crayon_transition *effect, void *);	// The function that applies the effect that the user requests
} crayon_transition_t;

#endif
