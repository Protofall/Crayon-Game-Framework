#ifndef RENDER_STRUCTS_CRAYON_H
#define RENDER_STRUCTS_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and anim structs

/*
Its designed for the multi-draw functions. If you want to draw a single
thing with this struct then I think you'll still need to go through
the multi-draw. It shouldn't be too much slower if any.
*/

//In revision 1 just skip colour and rotation since I still need to plan them more
	//colours and rotations are currently unused
typedef struct crayon_textured_array{
	float *positions;			//Width then Height extracted from anim/frame data,
								//Each group of 2 is for one sub-texture
	uint8_t *frame_coord_keys;	//Contains element ids a for group of two elements of
								//frame_coords_map and uses that for drawing
	uint16_t *frame_coord_map;	//Each group of 2 elements is one frame of an animation
	uint32_t *colours;			//For poly mode this dictates the rgb and alpha of a polygon
								//(Might be usable in Sprite mode?)
	float *scales;				//Float incase you want to shrink it, better for enlarging though

	uint8_t *flips;				//UNUSED, will mirror the sprite (Then rotation is applied)

	float *rotations;			//Poly uses angles to rotate on Z axis, sprite uses
								//booleans/flip bits. Decide what type this should be...
	uint8_t *draw_z;			//The layer to help deal with overlapping sprites/polys
	uint16_t num_sprites;		//This tells the draw function how many sprites/polys to draw.
	uint8_t unique_frames;		//The number of UVs in frame_coords_map

	uint8_t options;			//Format XXCR (Flips)SFZ, Basically some booleans options relating to
								//colour, rotations, flips, scales, frame_coords, z coord (layer)
								//If that bit is set to true, then we use the first element of
								//arrays (except map) for all sub-textures
								//Else we assume each sprite has its own unique value

	uint8_t filter;				//0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2


	crayon_spritesheet_t *spritesheet;
	crayon_animation_t *animation;
	crayon_palette_t *palette;	//Also ask if palettes can start at not multiples of 16 or 256
} crayon_textured_array_t;

//Used for rendering many untextured polys
typedef struct crayon_untextured_array{
	float *positions;		//Group of 2 per poly (top left coord)
	uint8_t *draw_z;			//The layer to help deal with overlapping sprites/polys
	uint32_t *colours;			//Dictates the rgb and alpha of a polygon
	uint16_t *draw_dims;		//The x and y dims of each poly
	float *rotations;			//Poly uses angles to rotate on Z axis
	uint16_t num_polys;
	uint8_t options;			//---- ZCDR. If Z,C,D,R is 1, then we use all elements in their lists. If 0 then we only use the first element.
} crayon_untextured_array_t;

#endif
