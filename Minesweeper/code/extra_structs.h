#ifndef MS_EXTRA_STRUCTS_H
#define MS_EXTRA_STRUCTS_H

#include "../../Crayon/code/dreamcast/texture_structs.h"
#include "savefile.h"

//This file exist just to make the main file a little bit more clean

typedef struct MinesweeperOS{
	//OS stuff + region and sd
	crayon_textured_array_t **assets;
	uint8_t num_assets;
	uint8_t lang_id;	//Used for detecting a press on the icon
	uint8_t tabs_y;
	crayon_textured_array_t sd;
	crayon_textured_array_t region;

	uint16_t clock_x;
	uint16_t clock_y;
	crayon_palette_t * clock_palette;	//The palette for the clock to use

} MinesweeperOS_t;

//Contains the logic for a board
typedef struct MinesweeperGrid{
	uint8_t *logic_grid;
	crayon_textured_array_t draw_grid;	//Contains the tile positions, frames, UVs and more

	//Set these 3 with the alternate board details for easy language switching
	crayon_spritesheet_t * alt_ss;
	crayon_animation_t * alt_anim;
	crayon_palette_t * alt_pal;

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

	uint8_t difficulty;	//1 = beginner, 2 = intermediate, 3 = expert. 0 means its a custom map

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
	uint8_t htz;				//Whatever refresh rate you chose. Currently unused

	uint8_t focus;	//0 = normal game, 1 = type name (High score), 2 = display high scores, 3 = options,
					//4 = controls, 5 = about

	//For the options page (Apply only affects these 3 and not the checkboxes)
	uint8_t disp_x;
	uint8_t disp_y;
	uint8_t disp_mines;
	char x_buffer[4], y_buffer[4], m_buffer[4];

	crayon_textured_array_t buttons;
	crayon_textured_array_t checkers;
	crayon_textured_array_t number_changers;

	uint16_t tabs_x[5];
	uint8_t tabs_width[5];

	SaveFileDetails_t save;
	int8_t vmu_present;		//1 is there's a valid VMU plugged in, 0 if there isn't (Doesn't check if VMU has enough space)

	// MinesweeperSaveFile_t save_file;	//Not a pointer since this struct needs the same save file always

} MinesweeperOptions_t;

//Contains the data related to the keyboard where you enter a high score
typedef struct MinesweeperKeyboard{
	char type_buffer[16];
	int8_t chars_typed;
	uint16_t type_box_x, type_box_y;
	crayon_textured_array_t mini_buttons, key_big_buttons;
	int8_t caps;

	const char * upper_keys;
	const char * lower_keys;

	int8_t record_index;
} MinesweeperKeyboard_t;

#endif
