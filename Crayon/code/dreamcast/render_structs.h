#ifndef RENDER_STRUCTS_CRAYON_H
#define RENDER_STRUCTS_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and anim structs

/*
Its designed for the multi-draw functions. If you want to draw a single
thing with this struct then I think you'll still need to go through
the multi-draw. It shouldn't be too much slower if any.
*/

//Unfinished and currently unused
//CHANGE THIS TO HAVE A UV MAP THING
// typedef struct crayon_sprite_array{
// 	uint16_t * draw_pos;	//Width then Height extracted from anim/frame data, Each group of 2 is for one sub-texture
// 	uint16_t * frame_coords;	//Each group of 4 elements is one sub-texture to draw
// 	uint8_t * scales;	//I think 8 bits is good enough for most cases
// 	float * rotations;	//Poly uses angles to rotate on Z axis, sprite uses booleans/flip bits. Decide what type this should be...
// 	uint16_t num_sprites;	//This tells the draw function how many sprites/polys to draw.

// 	uint8_t options;	//Format FSRX XFFF, Basically 3 booleans options relating to frameCoords, scales and rotations
// 						//if that bit is set to true, then we use the first element of F/S/R array for all sub-textures
// 						//Else we assume each sub-texture has its own unique F/S/R value
// 						//The 3 F's at the end are for the filtering mode. Can easily access it with a modulo 8 operation
// 						//0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2

// 	uint8_t draw_z;	//The layer to help deal with overlapping sprites/polys
// 	uint8_t palette_num;	//Also ask if palettes can start at not multiples of 16 or 256
// 	uint32_t colour;	//For poly mode this dictates the rgb and alpha of a polygon
// 	crayon_spritesheet_t * ss;
// 	crayon_animation_t * anim;
// } crayon_sprite_array_t;

//In revision 1 just skip colour and rotation since I still need to plan them more
typedef struct crayon_sprite_array{
	uint16_t *draw_pos = NULL;			//Width then Height extracted from anim/frame data,
										//Each group of 2 is for one sub-texture
	uint8_t *frame_coords_map = NULL;	//Contains element ids for group of 4 frame_coords (why not 2?)
										//and uses that for drawing
	uint16_t *frame_coords = NULL;		//Each group of 4 elements is one frame of an animation
	uint32_t *colour = NULL;			//For poly mode this dictates the rgb and alpha of a polygon
										//(Might be usable in Sprite mode?)
	uint8_t *scales = NULL;				//I think 8 bits is good enough for most cases
	float *rotations = NULL;			//Poly uses angles to rotate on Z axis, sprite uses
										//booleans/flip bits. Decide what type this should be...
	uint8_t *draw_z = NULL;				//The layer to help deal with overlapping sprites/polys
	uint16_t num_sprites;				//This tells the draw function how many sprites/polys to draw.

	//Maybe update this to a uint16_t later
	uint8_t options;					//Format FCSR ZFFF, Basically 3 booleans options relating to
										//frame_coords, colour, scales, rotations, draw_z if that bit is set to true,
										//then we use the first element of F/C/S/R/Z array for all sub-textures
										//Else we assume each sub-texture has its own unique F/S/R value
										//The 3 F's at the end are
										//for the filtering mode. Can easily access it with a modulo 8 operation
										//0 = none, 2 = Bilinear, 4 = Trilinear1, 6 = Trilinear2

	uint8_t palette_num;				//Also ask if palettes can start at not multiples of 16 or 256
	crayon_spritesheet_t *ss = NULL;
	crayon_animation_t *anim = NULL;
} crayon_sprite_array_t;

//Used for rendering many untextured polys
typedef struct crayon_untextured_array{
	uint16_t *draw_pos = NULL;	//Group of 2 per poly (top left coord)
	uint8_t *draw_z = NULL;		//The layer to help deal with overlapping sprites/polys
	uint32_t *colours = NULL;	//Dictates the rgb and alpha of a polygon
	uint16_t *draw_dims = NULL;	//The x and y dims of each poly
	float *rotations = NULL;	//Poly uses angles to rotate on Z axis
	uint16_t num_polys;
	uint8_t options;			//---O ZCDR. If Z,C,D,R is 1, then we use all elements in their lists. If 0 then we only use the first element.
								//For O. If 1 its opaque, else its transparent (If your polys are fully opaque, set this to 1 (Its more efficient))
} crayon_untextured_array_t;

#endif
