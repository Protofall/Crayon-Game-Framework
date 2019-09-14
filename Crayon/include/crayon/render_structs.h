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

//This is designed for the multi-draw functions. If you want to draw a single thing with this struct
//then I you'll still need to go through the multi-draw. It shouldn't be too much slower if any.
typedef struct crayon_sprite_array{
	vec2_f_t *coord;			//Width then Height extracted from anim/frame data,
								//Each element is one pair of coordinates
	uint8_t *frame_coord_key;	//ids for a pair of frame_coords_map elements and uses that for
								//drawing. frame_coord_key[i] refers to frame_coord_map[i].x/y
	vec2_u16_t *frame_coord_map;//Each element is UV for one frame of an animation (U is x, V is Y)
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
								//fade/colour, rotation, flip, scale, frame_coords, z coord (layer)
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

// Getters, for ease of use, you can access the array directly if you want
	//They will all return values of 0 if it isn't valid, but can still return 0 if its valid...
	//UNTESTED
extern float crayon_get_coord_x(crayon_sprite_array_t * sprites, uint16_t index);
extern float crayon_get_coord_y(crayon_sprite_array_t * sprites, uint16_t index);
extern vec2_f_t crayon_get_coords(crayon_sprite_array_t * sprites, uint16_t index);
extern uint32_t crayon_get_colour(crayon_sprite_array_t * sprites, uint16_t index);
extern uint8_t crayon_get_fade(crayon_sprite_array_t * sprites, uint16_t index);
extern float crayon_get_scale_x(crayon_sprite_array_t * sprites, uint16_t index);
extern float crayon_get_scale_y(crayon_sprite_array_t * sprites, uint16_t index);
extern vec2_f_t crayon_get_scales(crayon_sprite_array_t * sprites, uint16_t index);
extern uint8_t crayon_get_flip(crayon_sprite_array_t * sprites, uint16_t index);
extern float crayon_get_rotation(crayon_sprite_array_t * sprites, uint16_t index);
extern uint8_t crayon_get_layer(crayon_sprite_array_t * sprites, uint16_t index);
extern uint8_t crayon_get_frame_coord_id(crayon_sprite_array_t * sprites, uint16_t index);
extern vec2_u16_t crayon_get_frame_coord_uv(crayon_sprite_array_t * sprites, uint16_t index);

#endif
