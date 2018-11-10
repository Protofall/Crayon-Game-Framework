#ifndef MS_EXTRA_STRUCTS_H
#define MS_EXTRA_STRUCTS_H

#include "../../Crayon/code/dreamcast/texture_structs.h"

//This file exist just to make the main file a little bit more clean

//Should add some code here to move the sd and region icons (Or store their x/y's here)
typedef struct MinesweeperOS{
	uint8_t *ids;	//Animation ids
	uint16_t *coords_pos;	//The x and y for placing it
	uint16_t *coords_frame;	//The frame coordinates for UV
	uint16_t *scale;	//The scale factor
	uint8_t sprite_count;	//Number of sprites to draw
	crayon_spritesheet_t *windows_ss;	//The spritesheet for the windows assets

	//These two are for stuff that appear in both OSes, but vary depending on which OS you use
	uint16_t *variant_pos;	//Records the x and y for all the OS dependent stuff not in OS spritesheet
	uint8_t clock_palette;	//The palette for the clock to use
} MinesweeperOS_t;

/*

OS dependent stuff

[Menu_X] [Menu_Y]	//Game, Options, About
[Clock_X] [Clock_Y]
[Icon_Eject_X] [Icon_Eject_Y]
[Icon_DC_X] [Icon_DC_Y]
[Icon_BS_X] [Icon_BS_Y]

*/

typedef struct MinesweeperSaveFile{
	uint8_t BS_Mode;	//Bulletsweeper mode. 0 for never won, 1 for Beat with 1 player, 2 for beat with 2 players, etc.
	uint8_t checkbox_options;	//Sound and Question marks
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[11][6];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint8_t pref_mines;
} MinesweeperSaveFile_t;

//Contains the logic for a board
typedef struct MinesweeperGrid{
	crayon_spritesheet_t tile_ss;
	crayon_animation_t tile_anim;
	uint8_t *logic_grid;	//The following 3 arrays are in the crayon_sprite_array_t

	//Next 3 pointers will later be replaced with the crayon_sprite_array object
	uint16_t *coord_grid;
	uint8_t *frame_grid_key;	//Contains element id's for the frame grid that contains the
								//texel coords for all 16 anim frames
	uint16_t *frame_grid;
	crayon_sprite_array_t grid;

	uint8_t x;
	uint8_t y;
	uint16_t start_x;
	uint16_t start_y;
	uint16_t num_mines;
	uint8_t first_reveal;

	uint16_t non_mines_left;	//When this variable equals zero, the game is won
	int num_flags;	//More like "number of flags in the pool"
	uint8_t over_mode;	//0 = ready for new game, 1 = game just ended, 2 = loss (ded), 3 = win
	uint8_t game_live;	//Is true when the timer is turning
	uint8_t revealed;
	int time_sec;

	// uint8_t players_allowed;	//1111, if you want to make a competative mode this can help
								//It goes P1, P2, P3, P4. I'm made it so all players can work
								//on any grid. Defeault 15
} MinesweeperGrid_t;

//Contains game options and focus (Windows tab)
typedef struct MinesweeperOptions{
	uint8_t sd_present;			//If an ext2 formatted SD card is detected, this this becomes true
								//and modifies some textures/coords and allows R to save screenshots
	uint8_t question_enabled;	//Enable the use of question marking
	uint8_t sound_enabled;		//Toggle the sound
	uint8_t operating_system;	//0 for 2000, 1 for XP
	uint8_t language;			//0 for English, 1 for Italian. This affects the font language and
								//the Minesweeper/Prato fiorito themes

	uint8_t focus;

	//For the options page (Apply only affects these 3 and not the checkboxes)
	uint8_t disp_x;
	uint8_t disp_y;
	uint8_t disp_mines;
	char x_buffer[4], y_buffer[4], m_buffer[4];

	//Add some new draw struct pointers/or stack allocated, idk

	crayon_spritesheet_t *check_box;	//The anim for the checkbox...I might remove this later
	MinesweeperSaveFile_t savefile;	//Not a pointer since this struct needs the same save file always
} MinesweeperOptions_t;

//Add a cursor/button combo struct?

#endif
