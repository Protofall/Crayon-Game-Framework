#ifndef MEMORY_CRAYON_H
#define MEMORY_CRAYON_H

#include "texture_structs.h"  //For the spritehseet and animation structs
#include "render_structs.h"  //For the crayon_sprite_array struct
#include "assist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kos/fs_romdisk.h> //For romdisk swapping
#include <zlib/zlib.h>
extern int zlib_getlength(char *filename);	//Because zlib.h won't declare it for us

typedef struct dtex_header{
	uint8_t magic[4]; //magic number "DTEX"
	uint16_t   width; //texture width in pixels
	uint16_t  height; //texture height in pixels
	uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
	uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
	uint8_t     magic[4]; //magic number "DPAL"
	uint32_t color_count; //number of 32-bit ARGB palette entries
} dpal_header_t;

//------------------Checking memory------------------//

//Gets the index of an animation
	//UNTESTED
extern int16_t crayon_memory_get_animation_id(char * name, crayon_spritesheet_t * ss);

//------------------Allocating memory------------------//

//Reads a file, loads it into a dtex
extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, uint16_t *texture_width, uint16_t *texture_height, uint32_t *format,
	char *texture_path);

//Loads a "crayon_spritesheet" spritesheet into memory
extern uint8_t crayon_memory_load_spritesheet(crayon_spritesheet_t *ss, crayon_palette_t *cp, int8_t palette_id, char *path);

//Loads a proportionally-spaced fontsheet into memory
extern uint8_t crayon_memory_load_prop_font_sheet(crayon_font_prop_t *fp, crayon_palette_t *cp, int8_t palette_id, char *path);

//Loads a mono-spaced fontsheet into memory
extern uint8_t crayon_memory_load_mono_font_sheet(crayon_font_mono_t *fm, crayon_palette_t *cp, int8_t palette_id, char *path);

//If given a valid path and a crayon_palette object, it will populate the palette object with the correct data
extern uint8_t crayon_memory_load_palette(crayon_palette_t *cp, int8_t bpp, char *path);

//This will make a new palette struct thats a copy of another one.
extern void crayon_memory_clone_palette(crayon_palette_t *original, crayon_palette_t *copy, int8_t palette_id);

//Set initial array sizes and options for your sprite_array
//Option's format is TCCR (flip)SF-
	//If T is set, we ignore the flip, frames and Left colour bits
//If making an untextured array, set the spritesheet pointer to NULL
extern void crayon_memory_init_sprite_array(crayon_sprite_array_t *sprite_array, crayon_spritesheet_t *ss,
	uint8_t animation_id, crayon_palette_t *pal, uint16_t list_size, uint8_t frames_used, uint8_t options,
	uint8_t filter, uint8_t set_defaults);

extern void crayon_memory_init_camera(crayon_viewport_t *camera, vec2_f_t world_coord, vec2_u16_t world_dim,
	vec2_u16_t window_coord, vec2_u16_t window_dim, float world_movement_factor);


//------------------Modifying memory----------------//


//This function attempts to search for colour1 in the palette and swaps it for colour2
//If _continue is false then it only replaces the first instance of colour1, else it replaces all instances
//It returns 0 if colour1 wasn't found, otherwise it returns the number of "colour1"s that were swapped
extern uint16_t crayon_memory_swap_colour(crayon_palette_t *cp, uint32_t colour1, uint32_t colour2, uint8_t _continue);

//WARNING. This function assumes the "indexes" list is sorted in ascending order (To save on computation time)
	//If you pass in an unsorted list, idk what madness will happen
//It will go through all the elements from the "indexes" list and remove them from the sprite array
extern uint8_t crayon_memory_remove_sprite_array_elements(crayon_sprite_array_t *sprite_array, uint16_t * indexes,
	uint16_t indexes_length);

//Takes a sprite array and sets its size (Bigger or smaller). It doesn't handle value initialisation though
	//INTERNAL USAGE
extern uint8_t crayon_memory_allocate_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t size);

//Reallocs and extends with "elements" amount of new elements. If "set_defaults" is on then it sets the defaults
extern uint8_t crayon_memory_extend_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t elements, uint8_t set_defaults);

//Sets the defaults. If going from 0 to list_size - 1, this will also set the non-multis
extern void crayon_memory_set_defaults_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t start, uint16_t end);


//------------------Freeing memory------------------//


//Free up all memory from a spritesheet struct. if free_palette is true, it will also free the palette
extern uint8_t crayon_memory_free_spritesheet(crayon_spritesheet_t *ss);

//Same as above, but for mono-spaced fontsheets
extern uint8_t crayon_memory_free_prop_font_sheet(crayon_font_prop_t *fp);

//Same as above, but for proportionally-spaced fontsheets
extern uint8_t crayon_memory_free_mono_font_sheet(crayon_font_mono_t *fm);

//Frees a palette
extern uint8_t crayon_memory_free_palette(crayon_palette_t *cp);

//Frees a draw array
extern uint8_t crayon_memory_free_sprite_array(crayon_sprite_array_t *sprite_array);


//------------------Mounting romdisks------------------//


//Mount a regular img romdisk
extern uint8_t crayon_memory_mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk (Apparently its kinda dodgy)
extern uint8_t crayon_memory_mount_romdisk_gz(char *filename, char *mountpoint);


//------------------Render Struct Getters------------------//


// For ease of use, you can access the array directly if you want
	//UNTESTED
extern float crayon_memory_get_coord_x(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern float crayon_memory_get_coord_y(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern vec2_f_t crayon_memory_get_coords(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern uint32_t crayon_memory_get_colour(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern uint8_t crayon_memory_get_fade(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern float crayon_memory_get_scale_x(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern float crayon_memory_get_scale_y(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern vec2_f_t crayon_memory_get_scales(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern uint8_t crayon_memory_get_flip(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern float crayon_memory_get_rotation(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern uint8_t crayon_memory_get_layer(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern uint8_t crayon_memory_get_frame_id(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);
extern vec2_u16_t crayon_memory_get_frame_uv(crayon_sprite_array_t * sprites, uint16_t index, uint8_t * error);


//------------------Render Struct Setters------------------//

//UNTESTED
extern uint8_t crayon_memory_set_coord_x(crayon_sprite_array_t * sprites, uint16_t index, float value);
extern uint8_t crayon_memory_set_coord_y(crayon_sprite_array_t * sprites, uint16_t index, float value);
extern uint8_t crayon_memory_set_coords(crayon_sprite_array_t * sprites, uint16_t index, vec2_f_t value);
extern uint8_t crayon_memory_set_colour(crayon_sprite_array_t * sprites, uint16_t index, uint32_t value);
extern uint8_t crayon_memory_set_fade(crayon_sprite_array_t * sprites, uint16_t index, uint8_t value);
extern uint8_t crayon_memory_set_scale_x(crayon_sprite_array_t * sprites, uint16_t index, float value);
extern uint8_t crayon_memory_set_scale_y(crayon_sprite_array_t * sprites, uint16_t index, float value);
extern uint8_t crayon_memory_set_scales(crayon_sprite_array_t * sprites, uint16_t index, vec2_f_t value);
extern uint8_t crayon_memory_set_flip(crayon_sprite_array_t * sprites, uint16_t index, uint8_t value);
extern uint8_t crayon_memory_set_rotation(crayon_sprite_array_t * sprites, uint16_t index, float value);
extern uint8_t crayon_memory_set_layer(crayon_sprite_array_t * sprites, uint16_t index, uint8_t value);
extern uint8_t crayon_memory_set_frame_id(crayon_sprite_array_t * sprites, uint16_t index, uint8_t value);
extern uint8_t crayon_memory_set_frame_uv(crayon_sprite_array_t * sprites, uint16_t index, uint8_t frame_id);


//--------------------------Misc--------------------------//

extern void crayon_memory_move_camera_x(crayon_viewport_t * camera, float x);
extern void crayon_memory_move_camera_y(crayon_viewport_t * camera, float y);

#endif
