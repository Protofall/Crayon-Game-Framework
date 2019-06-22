#ifndef RENDER_STRUCTS_CRAYON_H
#define RENDER_STRUCTS_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and anim structs

/*
This is designed for the multi-draw functions. If you want to draw a single thing with this struct
then I you'll still need to go through the multi-draw. It shouldn't be too much slower if any.
*/

//Sprite mode is complete (I hope), poly mode is missing colour and true rotation support
typedef struct crayon_sprite_array{
	float *pos;					//Width then Height extracted from anim/frame data,
								//Each group of 2 is for one sub-texture
	uint8_t *frame_coord_key;	//Contains element ids a for group of two elements of
								//frame_coords_map and uses that for drawing
	uint16_t *frame_coord_map;	//Each group of 2 elements is one frame of an animation
	uint32_t *colour;			//For poly mode this dictates the rgb and alpha of a polygon
	uint8_t *fade;				//A part of the colour array. Tells it how to transition between
								//The base colour and the one from the colour array (UNUSED)
	float *scale;				//Float incase you want to shrink or enlarge it

	uint8_t *flip;				//Mirror the sprite along x-axis (Then rotation is applied)

	float *rotation;			//Poly uses angles to rotate on Z axis, sprite uses
								//booleans/flip bits. Decide what type this should be...
	uint8_t *layer;				//The layer to help deal with overlapping sprites/polys
	uint16_t list_size;			//This tells the draw function how many sprites/polys to draw.
	uint8_t frames_used;		//The number of frames you want to use. Minimum 1

	uint8_t options;			//Format -CCR (flip)SFZ, Basically some booleans options relating to
								//fade/colour, rotation, flip, scale, frame_coords, z coord (layer)
								//If that bit is set to true, then we use the first element of
								//arrays (except map) for all sub-textures
								//Else we assume each sprite has its own unique value

								//Note that the 1st colour bit tells it to use either Blend or Add
								//mode when applying the colour
								//Blend is the "Fade to colour" mode
								//Add just gives a coloured translucent overlay to the sprite

	uint8_t filter;				//0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2

	crayon_spritesheet_t *spritesheet;
	crayon_animation_t *animation;
	crayon_palette_t *palette;	//Also ask if palettes can start at not multiples of 16 or 256
} crayon_sprite_array_t;

//Used for rendering many untextured polys
typedef struct crayon_untextured_array{
	float *pos;					//Group of 2 per poly (top left coord)
	uint8_t *layer;				//The layer to help deal with overlapping sprites/polys
	uint32_t *colour;			//Dictates the rgb and alpha of a polygon
	uint16_t *dimensions;		//The x and y dims of each poly
	float *rotation;			//Poly uses angles to rotate on Z axis (CURRENTLY UNUSED)
	uint16_t list_size;
	uint8_t options;			//---- ZCDR. If Z,C,D,R is 1, then we use all elements in their lists. If 0 then we only use the first element.
} crayon_untextured_array_t;

#endif
