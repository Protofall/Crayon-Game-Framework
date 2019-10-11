#ifndef RENDER_STRUCTS_CRAYON_H
#define RENDER_STRUCTS_CRAYON_H

#include "texture_structs.h"  //For the spritesheet and anim structs
#include "vector_structs.h"

#define CRAY_MULTI_LAYER (1 << 0)
#define CRAY_MULTI_FRAME (1 << 1)
#define CRAY_MULTI_SCALE (1 << 2)
#define CRAY_MULTI_DIM (1 << 2)
#define CRAY_MULTI_FLIP (1 << 3)
#define CRAY_MULTI_ROTATE (1 << 4)
#define CRAY_MULTI_COLOUR_ADD (3 << 5) // 0110 0000
#define CRAY_MULTI_COLOUR_BLEND (1 << 5) // 0010 0000
#define CRAY_MULTI_COLOUR (1 << 5) //Use this one for untextured polys

//Internal usage
#define CRAY_COLOUR_BLEND (0 << 6)
#define CRAY_COLOUR_ADD (1 << 6)
#define CRAY_HAS_TEXTURE (1 << 7)

//The camera, controls where on screen to render and which part of the world to draw
typedef struct crayon_viewport_t{
	//Top left of the world region and dimensions
	float world_x;	//Must be a float since the draw position array is a bunch of floats
	float world_y;
	uint16_t world_width;
	uint16_t world_height;

	//The scrolling modifier
	float world_movement_factor;

	//Top left of where to render to on screen and dimensions
		//For DC these can all be uint16_t's. For the PC port I think uint16_t is still fine even with larger monitors because a uint16_t is 65535 at most and that still supports 8K (And possibly higher)
	uint16_t window_x;
	uint16_t window_y;
	uint16_t window_width;
	uint16_t window_height;
} crayon_viewport_t;

//This is designed for the multi-draw functions. If you want to draw a single thing with this struct
//then I you'll still need to go through the multi-draw. It shouldn't be too much slower if any.
typedef struct crayon_sprite_array{
	vec2_f_t *coord;			//Width then Height extracted from anim/frame data,
								//Each element is one pair of coordinates
	uint8_t *frame_id;			//ids for a pair of frame_uv elements and uses that for
								//drawing. frame_id[i] refers to frame_uv[i].x/y
	vec2_u16_t *frame_uv;		//Each element is UV for one frame of an animation (U is x, V is Y)
	uint32_t *colour;			//For poly mode this dictates the rgb and alpha of a polygon
	uint8_t *fade;				//A part of the colour array. Tells it how to transition between
								//The base colour and the one from the colour array
	vec2_f_t *scale;			//Float incase you want to shrink or enlarge it. For untextured polys this
								//is the height and width of the polys

	uint8_t *flip;				//Mirror the sprite along x-axis (Then rotation is applied)

	float *rotation;			//Poly uses angles to rotate on Z axis, sprite uses
								//booleans/flip bits. Decide what type this should be...
	uint8_t *layer;				//The layer to help deal with overlapping sprites/polys
	uint16_t list_size;			//This tells the draw function how many sprites/polys to draw.
	uint8_t frames_used;		//The number of frames you want to use. Minimum 1

	uint8_t options;			//Format TCCR (flip)SFZ, Basically some booleans options relating to
								//fade/colour, rotation, flip, scale, frame_ids, z coord (layer)
								//If that bit is set to true, then we use the first element of
								//arrays (except map) for all sub-textures
								//Else we assume each sprite has its own unique value
								//T stands for "Textured". Changes how we handle the struct
									//Untextured polys only use The right Colour bit, Rotation, Scale
									//and Layer (Z)

								//Note that the 1st colour bit tells it to use either Blend or Add
								//mode when applying the colour
								//Blend is the "Fade to colour" mode
								//Add just gives a coloured translucent overlay to the sprite

	uint8_t filter;				//0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2

	crayon_spritesheet_t *spritesheet;
	crayon_animation_t *animation;
	crayon_palette_t *palette;	//Also ask if palettes can start at not multiples of 16 or 256
} crayon_sprite_array_t;

#endif
