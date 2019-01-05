#ifndef MS_EXTRA_STRUCTS_H
#define MS_EXTRA_STRUCTS_H

#include <crayon/texture_structs.h>
#include <crayon/savefile.h>

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

//+ save icon should be 2 blocks long
typedef struct minesweeper_savefile{
	uint8_t options;	//XXXH LOSQ (Refresh rate (Hz), Language, OS, Sound, Questions)
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[6][16];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint16_t pref_mines;

	uint8_t bulletsweeper_beaten;	//Bulletsweeper mode. 0 for never won, 1 for Beat with 1 player, 2 for beat with 2 players, etc.
} minesweeper_savefile_t;

//This is the original savefile format, I forgot that pref_mines needs to be a uint16_t so I had to update the format later on (Also 2 blocks long)
typedef struct pre_1_3_0_minesweeper_savefile{
	uint8_t options;	//XXXH LOSQ (Refresh rate (Hz), Language, OS, Sound, Questions)
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[6][16];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint8_t pref_mines;
} pre_1_3_0_minesweeper_savefile_t;

//Contains game options and focus (Windows tab)
typedef struct MinesweeperOptions{
	uint8_t sd_present;			//If an ext2 formatted SD card is detected, this this becomes true
								//and modifies some textures/coords and allows R to save screenshots
	uint8_t question_enabled;	//Enable the use of question marking
	uint8_t sound_enabled;		//Toggle the sound
	uint8_t operating_system;	//0 for 2000, 1 for XP
	uint8_t language;			//0 for English, 1 for Italian. This affects the font language and
								//the Minesweeper/Prato fiorito themes
	uint8_t htz;				//Whatever refresh rate you chose. 0 = 50, 1 = 60

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

	crayon_savefile_details_t savefile_details;
	minesweeper_savefile_t savefile;

} MinesweeperOptions_t;

//Contains the data related to the keyboard where you enter a high score
typedef struct MinesweeperKeyboard{
	uint16_t keyboard_start_x;
	uint16_t keyboard_start_y;
	uint16_t keyboard_width;
	uint16_t keyboard_height;

	char type_buffer[16];
	int8_t chars_typed;
	uint16_t type_box_x, type_box_y;
	crayon_textured_array_t mini_buttons, medium_buttons, big_buttons;
	int8_t caps;
	int8_t special;	//true/false. Has priority over caps
	uint16_t name_length;	//Used for the flashing text cursor
	crayon_font_prop_t * fontsheet;	//Used for the flashing line

	const char * upper_keys;
	const char * lower_keys;
	const char * special_keys;

	int8_t record_index;
} MinesweeperKeyboard_t;

#endif
