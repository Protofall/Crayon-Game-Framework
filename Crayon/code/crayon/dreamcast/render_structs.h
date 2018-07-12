#ifndef RENDER_STRUCTS_CRAYON_H
#define RENDER_STRUCTS_CRAYON_H

;
/*

Ideas of what this should looke liek:

Its designed for graphics_draw_sprites/polys() first (I think I'll remove the single version)
If you want to draw a single thing with this struct then I think you'll still need to go through
the multi-draw. It shouldn't be too much slower if any.

rough look:

typedef struct render{
	uint16_t * draw_x;	//Width extracted from anim/frame data, Each group of 2 is for one sub-texture
	uint16_t * draw_y;	//Height extracted from anim/frame data, Each group of 2 is for one sub-texture
	uint16_t * frameCoords;	//Each group of 4 elements is one sub-texture to draw
	//Something for scale (x/y)
	//Something for rotation	//(Probs a list of z rots and sprite mode rounds to nearest 90 degree or something or just
								//make the sprite and poly modes work differently. Poly uses angles, sprite uses
								//booleans/flip bits)

	uint16_t numSubTextures;	//This tells the draw function how many sprites/polys to draw.

	uint8_t options;	//Format XXXX XFSR, Basically 3 booleans optios relating to frameCoords, scaling and rotation
						//if that bit is set to true, then we use the first element of F/S/R array for all sub-textures
						//Else we assume each sub-texture has its own unique F/S/R value
						//Maybe incorperate filter into this to save on space...if so move FSR to front and can use
						//modulo to easily get the filter bit

	uint8_t paletteNum;	//Also ask if palettes can start at not multiples of 16 or 256
	uint8_t draw_z;
	uint8_t filter;	//Nearest neighbour, linear, bilinear, etc
	spritesheet * ss;
	animation * anim;
} render_t;

*/

#endif
