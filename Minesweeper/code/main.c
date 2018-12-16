//Crayon libraries
#include <crayon/memory.h>

#include "extra_structs.h"
#include "setup.h"
#include "custom_polys.h"

//For the controller and mouse
#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/maple/mouse.h>

//For the timer
#include <arch/timer.h>
#include <time.h>

//for tolower()
#include <ctype.h>

//For the sound effects
#include <dc/sound/stream.h>
#include <dc/sound/sfxmgr.h>

//To get region info
#include <dc/flashrom.h>

#define CRAYON_DEBUG 0
//LOWER BUTTON TOP BOUND BECAUSE OF XP MODE

#if CRAYON_SD_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <ext2/fs_ext2.h>
#endif

#if CRAYON_SD_MODE == 1
	#define MNT_MODE FS_EXT2_MOUNT_READWRITE	//Might manually change it so its not a define anymore

	static void unmount_ext2_sd(){
		fs_ext2_unmount("/sd");
		fs_ext2_shutdown();
		sd_shutdown();
	}

	static int mount_ext2_sd(){
		kos_blockdev_t sd_dev;
		uint8 partition_type;

		// Initialize the sd card if its present
		if(sd_init()){
			return 1;
		}

		// Grab the block device for the first partition on the SD card. Note that
		// you must have the SD card formatted with an MBR partitioning scheme
		if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
			return 2;
		}

		// Check to see if the MBR says that we have a Linux partition
		if(partition_type != 0x83){
			return 3;
		}

		// Initialize fs_ext2 and attempt to mount the device
		if(fs_ext2_init()){
			return 4;
		}

		//Mount the SD card to the sd dir in the VFS
		if(fs_ext2_mount("/sd", &sd_dev, MNT_MODE)){
			return 5;
		}
		return 0;
	}
#endif

// N0 N1 N2
// N3 S  N4
// N5 N6 N7
// N are neighbours, S is source (ele_x, ele_y)
// The returned value is in the format
// N7N6N5N4N3N2N1N0 where each bit is true if neighbour is valid
uint8_t neighbouring_tiles(MinesweeperGrid_t * grid, int ele_x, int ele_y){
	uint8_t ret_val = 255;
	if(ele_x == 0){
		ret_val &= ~(1 << 0);	//Clearing bits which can't be right
		ret_val &= ~(1 << 3);
		ret_val &= ~(1 << 5);
	}
	else if(ele_x >= grid->x - 1){
		ret_val &= ~(1 << 2);
		ret_val &= ~(1 << 4);
		ret_val &= ~(1 << 7);
	}
	if(ele_y == 0){
		ret_val &= ~(1 << 0);
		ret_val &= ~(1 << 1);
		ret_val &= ~(1 << 2);
	}
	else if(ele_y >= grid->y - 1){
		ret_val &= ~(1 << 5);
		ret_val &= ~(1 << 6);
		ret_val &= ~(1 << 7);
	}

	return ret_val;
}

//Pass it in the params and it will modify them based on valids and which tile chosen
uint8_t neightbours_to_adjustments(int8_t * tile_offset_x, int8_t * tile_offset_y, uint8_t valids, uint8_t tile){
	if(valids & (1 << tile)){
		if(tile == 0){
			*tile_offset_x = -1;
			*tile_offset_y = -1;
		}
		else if(tile == 1){
			*tile_offset_x = 0;
			*tile_offset_y = -1;
		}
		else if(tile == 2){
			*tile_offset_x = 1;
			*tile_offset_y = -1;
		}
		else if(tile == 3){
			*tile_offset_x = -1;
			*tile_offset_y = 0;
		}
		else if(tile == 4){
			*tile_offset_x = 1;
			*tile_offset_y = 0;
		}
		else if(tile == 5){
			*tile_offset_x = -1;
			*tile_offset_y = 1;
		}
		else if(tile == 6){
			*tile_offset_x = 0;
			*tile_offset_y = 1;
		}
		else if(tile == 7){
			*tile_offset_x = 1;
			*tile_offset_y = 1;
		}
		return 0;
	}
	return 1;	//Not valid? tell sender
}

//Initially called by clear_grid where x and y are always a valid numbers
void populate_logic(MinesweeperGrid_t * grid, int ele_x, int ele_y){
	int ele_logic = ele_x + grid->x * ele_y;
	if(grid->logic_grid[ele_logic] % (1 << 4) == 9){	//Is mine
		return;
	}

	uint8_t valids = neighbouring_tiles(grid, ele_x, ele_y);
	uint8_t sum = 0;	//A tiles value

	if((valids & (1 << 0)) && grid->logic_grid[ele_x - 1 + ((ele_y- 1) * grid->x)] % (1 << 4) == 9){sum++;}		//Top Left
	if((valids & (1 << 1)) && grid->logic_grid[ele_x + ((ele_y - 1) * grid->x)] % (1 << 4) == 9){sum++;}		//Top centre
	if((valids & (1 << 2)) && grid->logic_grid[ele_x + 1 + ((ele_y - 1) * grid->x)] % (1 << 4) == 9){sum++;}	//Top right
	if((valids & (1 << 3)) && grid->logic_grid[ele_x - 1 + (ele_y * grid->x)] % (1 << 4) == 9){sum++;}			//Mid Left
	if((valids & (1 << 4)) && grid->logic_grid[ele_x + 1 + (ele_y * grid->x)] % (1 << 4) == 9){sum++;}			//Mid Right
	if((valids & (1 << 5)) && grid->logic_grid[ele_x - 1 + ((ele_y + 1) * grid->x)] % (1 << 4) == 9){sum++;}	//Bottom left
	if((valids & (1 << 6)) && grid->logic_grid[ele_x + ((ele_y + 1) * grid->x)] % (1 << 4) == 9){sum++;}		//Bottom centre
	if((valids & (1 << 7)) && grid->logic_grid[ele_x + 1 + ((ele_y + 1) * grid->x)] % (1 << 4) == 9){sum++;}	//Bottom right

	grid->logic_grid[ele_logic] += sum;

	return;
}


uint8_t true_prob(double p){
	return rand() < p * (RAND_MAX + 1.0);
}

//Blanks out grid then fills with mines, but doesn't number them
void clear_grid(MinesweeperGrid_t * grid){
	grid->num_flags = 0;

	int i = 0;
	uint16_t mines_left = grid->num_mines;
	uint16_t tiles_left = grid->draw_grid.num_sprites;

	while(i < grid->draw_grid.num_sprites){	//Populate board
		double prob = (double)mines_left / (double)tiles_left;	//Can I do better than using a division?
		grid->logic_grid[i] = 9 * true_prob(prob);
		if(grid->logic_grid[i]){
			mines_left--;
			grid->num_flags++;
		}
		tiles_left--;
		i++;
	}

	for(i = 0; i < grid->draw_grid.num_sprites; i++){
		grid->draw_grid.frame_coord_keys[i] = 0;
	}

	grid->game_live = 0;
	grid->over_mode = 0;
	grid->revealed = 0;
	grid->time_sec = 0;
	grid->first_reveal = 0;

	grid->non_mines_left = grid->draw_grid.num_sprites - grid->num_flags;

	return;
}

//When called it changes the grid size and calls clear
//Max grid size is 38, 21
void reset_grid(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, uint8_t x, uint8_t y, uint16_t mine_count){
	grid->x = x;
	grid->y = y;
	grid->num_mines = mine_count;

	options->savefile.pref_height = y;
	options->savefile.pref_width = x;
	options->savefile.pref_mines = mine_count;

	//Used for checking and setting high scores
	if(x == 9 && y == 9 && mine_count == 10){	//Beginner
		grid->difficulty = 1;
	}
	else if(x == 16 && y == 16 && mine_count == 40){	//Intermediate
		grid->difficulty = 2;
	}
	else if(x == 30 && y == 16 && mine_count == 99){	//Expert
		grid->difficulty = 3;
	}
	else{	//Custom grid
		grid->difficulty = 0;
	}

	if(grid->logic_grid != NULL){
		free(grid->logic_grid);
	}

	//New
	// if(grid->draw_grid.positions != NULL){
		// free(grid->draw_grid.positions);
	// }
	// if(grid->draw_grid.frame_coord_keys != NULL){
		// free(grid->draw_grid.frame_coord_keys);
	// }

	grid->draw_grid.num_sprites = x * y;
	grid->logic_grid = (uint8_t *) malloc(grid->draw_grid.num_sprites * sizeof(uint8_t));

	//New
	// grid->draw_grid.positions = (float *) malloc(2 * grid->draw_grid.num_sprites * sizeof(float));
	// grid->draw_grid.frame_coord_keys = (uint8_t *) malloc(grid->draw_grid.num_sprites * sizeof(uint8_t));
	// grid->draw_grid.positions = (float *) malloc(2 * grid->draw_grid.num_sprites * sizeof(float));
	// grid->draw_grid.frame_coord_keys = (uint8_t *) malloc(grid->draw_grid.num_sprites * sizeof(uint8_t));

	//Calculate some start_x stuff. Default for expert is 80, 104
	grid->start_x = 320 - (grid->x * 8);
	grid->start_y = 104 + 160 - (grid->y * 8);
	if(grid->start_y < 104){
		grid->start_y = 104;
	}

	int i, j;

	for(j = 0; j < grid->y; j++){
		for(i = 0; i < grid->x; i++){   //i is x, j is y
			uint16_t ele = (j * grid->x * 2) + (2 * i);

			grid->draw_grid.positions[ele] = (float)(grid->start_x + (i * 16));
			grid->draw_grid.positions[ele + 1] = (float)(grid->start_y + (j * 16));
		}
	}

	clear_grid(grid);

	options->disp_x = grid->x;
	options->disp_y = grid->y;
	options->disp_mines = grid->num_mines;

	sprintf(options->x_buffer, "%d", options->disp_x);
	sprintf(options->y_buffer, "%d", options->disp_y);
	sprintf(options->m_buffer, "%d", options->disp_mines);

	return;
}

//Numbers every tile in the grid
void number_grid(MinesweeperGrid_t * grid){
	int i, j;
	for(j = 0; j < grid->y; j++){
		for(i = 0; i < grid->x; i++){
			populate_logic(grid, i, j);
		}
	}

	return;
}

//If you initially press a tile, move the mines around so you get a "free first press"
void adjust_grid(MinesweeperGrid_t * grid, int ele_logic){
	grid->logic_grid[ele_logic] = 0;
	// non_mines_left++;	//I commented these out because normally they aren't needed, but if you had a grid with all mines (why?) then this matters
	// num_flags--;
	int i = 0;
	for(i = 0; i < grid->x * grid->y; i++){
		if(i != ele_logic && grid->logic_grid[i] % (1 << 4) != 9){
			grid->logic_grid[i] += 9;
			break;
			// non_mines_left--;
			// num_flags++;
		}
	}

	return;
}

//It fills it out differently depending on over_mode
void reveal_map(MinesweeperGrid_t * grid){
	int i;
	if(grid->over_mode == 2){
		for(i = 0; i < grid->x * grid->y; i++){
			if(grid->logic_grid[i] == 9 || grid->logic_grid[i] == 41){	//Untouched or question marked
				grid->draw_grid.frame_coord_keys[i] = 3;
			}
			if(grid->logic_grid[i] != 73 && grid->logic_grid[i] & 1<<6){	//Untouched or question marked
				grid->draw_grid.frame_coord_keys[i] = 5;
			}
		}
	}
	else if(grid->over_mode == 3){
		grid->num_flags = 0;
		for(i = 0; i < grid->x * grid->y; i++){
			if(grid->logic_grid[i] % (1 << 5) == 9){
				grid->draw_grid.frame_coord_keys[i] = 1;
			}
		}
	}
	grid->revealed = 1;
	return;
}

void discover_tile(MinesweeperGrid_t * grid, int ele_x, int ele_y){
	int ele_logic = ele_x + grid->x * ele_y;
	if(!(grid->logic_grid[ele_logic] & 1 << 6)){	//If not flagged
		if(grid->logic_grid[ele_logic] & 1 << 7){	//Already discovered
			return;
		}
		if(grid->logic_grid[ele_logic] & 1 << 5){	//If questioned, remove the question mark and set it to a normal tile
			grid->logic_grid[ele_logic] &= ~(1 << 5);
			grid->draw_grid.frame_coord_keys[ele_logic] = 0;
		}
		if(grid->logic_grid[ele_logic] == 9){	//If mine
			grid->draw_grid.frame_coord_keys[ele_logic] = 4;
			grid->game_live = 0;
			grid->over_mode = 1;
		}
		else{
			grid->draw_grid.frame_coord_keys[ele_logic] = 7 + grid->logic_grid[ele_logic];
			grid->non_mines_left--;
		}
		grid->logic_grid[ele_logic] |= (1 << 7);
		if(grid->logic_grid[ele_logic] % (1 << 5) == 0){
			uint8_t valids = neighbouring_tiles(grid, ele_x, ele_y);
			int i;
			for(i = 0; i < 8; i++){
				if(valids & (1 << i)){	//If the tile is valid
					int8_t xVarriant = 0;	//X is -1 if 0,3,5. +1 if 2,4,7 and 0 if 1 or 6
					if(i == 0 || i == 3 || i == 5){
						xVarriant = -1;
					}
					else if(i == 2 || i == 4 || i == 7){
						xVarriant = 1;
					}
					int8_t yVariant = 0;	//Y is -1 if i < 3. +1 if i > 4, and 0 otherwise
					if(i < 3){
						yVariant = -1;
					}
					else if(i > 4){
						yVariant = 1;
					}
					discover_tile(grid, ele_x + xVarriant, ele_y + yVariant);
				}
			}
		}
	}
	return;
}

void x_press(MinesweeperGrid_t * grid, int ele_x, int ele_y){
	int ele_logic = ele_x + grid->x * ele_y;
	if((grid->logic_grid[ele_logic] & 1<<7)){	//If revealed

		uint8_t valids = neighbouring_tiles(grid, ele_x, ele_y);
		uint8_t flag_sum = 0;	//A tiles value

		if((valids & (1 << 0)) && grid->logic_grid[ele_x - 1 + ((ele_y- 1) * grid->x)] & (1 << 6)){flag_sum++;}	//Top Left
		if((valids & (1 << 1)) && grid->logic_grid[ele_x + ((ele_y - 1) * grid->x)]  & (1 << 6)){flag_sum++;}		//Top centre
		if((valids & (1 << 2)) && grid->logic_grid[ele_x + 1 + ((ele_y - 1) * grid->x)]  & (1 << 6)){flag_sum++;}	//Top right
		if((valids & (1 << 3)) && grid->logic_grid[ele_x - 1 + (ele_y * grid->x)]  & (1 << 6)){flag_sum++;}		//Mid Left
		if((valids & (1 << 4)) && grid->logic_grid[ele_x + 1 + (ele_y * grid->x)]  & (1 << 6)){flag_sum++;}		//Mid Right
		if((valids & (1 << 5)) && grid->logic_grid[ele_x - 1 + ((ele_y + 1) * grid->x)]  & (1 << 6)){flag_sum++;}	//Bottom left
		if((valids & (1 << 6)) && grid->logic_grid[ele_x + ((ele_y + 1) * grid->x)]  & (1 << 6)){flag_sum++;}		//Bottom centre
		if((valids & (1 << 7)) && grid->logic_grid[ele_x + 1 + ((ele_y + 1) * grid->x)]  & (1 << 6)){flag_sum++;}	//Bottom right

		if(grid->logic_grid[ele_logic] % (1 << 4) == flag_sum){
			//Execute the X-press on all adjacent
			int i;
			for(i = 0; i < 8; i++){
				if(valids & (1 << i)){	//If the tile is valid
					int8_t xVarriant = 0;	//X is -1 if 0,3,5. +1 if 2,4,7 and 0 if 1 or 6
					if(i == 0 || i == 3 || i == 5){
						xVarriant = -1;
					}
					else if(i == 2 || i == 4 || i == 7){
						xVarriant = 1;
					}
					int8_t yVariant = 0;	//Y is -1 if i < 3. +1 if i > 4, and 0 otherwise
					if(i < 3){
						yVariant = -1;
					}
					else if(i > 4){
						yVariant = 1;
					}
					discover_tile(grid, ele_x + xVarriant, ele_y + yVariant);
				}
			}
		}
	}
	return;
}

//If we use a flag that decrements the flag count, leaving flag will increment it
void b_press(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, uint16_t ele_logic){
	if(!(grid->logic_grid[ele_logic] & (1 << 7))){	//Not discovered
		uint8_t status = 0;	//0 = normal, 1 = flag, 2 = question icons
		if(grid->logic_grid[ele_logic] & (1 << 6)){	//If flagged
			grid->logic_grid[ele_logic] &= ~(1 << 6);	//Clears the flag bit
			if(options->question_enabled){
				grid->logic_grid[ele_logic] |= (1 << 5);	//Sets the question bit
				status = 2;
			}
			grid->num_flags++;
		}
		else{	//Not flagged, but maybe questioned
			if(grid->logic_grid[ele_logic] & (1 << 5)){	//Normal it
				grid->logic_grid[ele_logic] &= ~(1 << 5);
				status = 0;
			}
			else{	//Flag it
				grid->logic_grid[ele_logic] |= (1 << 6);
				status = 1;
				grid->num_flags--;
			}
		}
		grid->draw_grid.frame_coord_keys[ele_logic] = status;
	}
	return;
}

//Used for displaying the counter for flags and timer
	//Display is 0 for the mine count or 1 for the timer
void digit_set(crayon_textured_array_t * ren_list, int num, uint8_t display){
	if(num < -99){
		num = -99;
	}
	else if(num > 999){
		num = 999;
	}

	uint8_t * draw_key = ren_list->frame_coord_keys + (display * 3);	//Gives us the pointer to the right display

	char num_char[3];
	sprintf(num_char, "%03d", num);	//Using this since its easier to convert to string than try and read from the int...
	uint8_t i;
	for(i = 0; i < 3; i++){
		if(num_char[i] == '-'){
			draw_key[i] = 10;
		}
		else{
			draw_key[i] = (int)num_char[i] - 48;
		}
	}

	return;
}

//control_type = 0 for normal controller and 1 for mouse
void grid_indent_logic(MinesweeperGrid_t * grid, int ele_x, int ele_y, uint8_t in_grid, int id, uint16_t *press_data, uint8_t A_held, uint8_t X_held,
	uint8_t B_held, uint8_t control_type){
	if(grid->over_mode != 0 || (!A_held && !X_held) || !in_grid){
		press_data[3 * id] = 0;
		return;
	}
	if(X_held || (control_type && (A_held && B_held))){	//X press has priority (If using mouse, A+B counts are X press)
		press_data[3 * id] = 2;
	}
	else if(A_held){	//A press
		press_data[3 * id] = 1;
	}
	press_data[(3 * id) + 1] = ele_x;
	press_data[(3 * id) + 2] = ele_y;
}

//Pass in a pointer to the beginning of the queried cursor's position
void face_logic(MinesweeperGrid_t * grid, crayon_textured_array_t * face, float *cursor_position, uint8_t A_held, uint8_t X_held){
	if(A_held || X_held){
		if(!grid->over_mode && !face->frame_coord_keys[0]){	//Basically face_frame_id != 2, 3 or 4
			face->frame_coord_keys[0] = 1;
		}
		if(A_held && (cursor_position[0] <= face->positions[0] + 26) && (cursor_position[1] <= face->positions[1] + 26)
			&& cursor_position[0] >= face->positions[0] && cursor_position[1] >= face->positions[1]){	//If hovering over face and holding on it
			face->frame_coord_keys[0] = 4;
		}
	}
}

//Handles the interaction logic with the grid
void grid_ABX_logic(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, int ele_x, int ele_y, uint8_t button_action, uint32_t *start_time, uint32_t *start_ms_time){
	if(grid->over_mode == 0){
		if(button_action & (1 << 0)){	//For A press
			if(!grid->game_live){
				timer_ms_gettime(start_time, start_ms_time);
				grid->game_live = 1;
			}
			if(grid->first_reveal == 0){
				int ele_logic = ele_x + grid->x * ele_y;
				if(!(grid->logic_grid[ele_logic] & (1 << 6))){
					if(grid->logic_grid[ele_logic] % (1 << 4) == 9){	//If this is the first reveal and its not flagged and its a mine
						adjust_grid(grid, ele_logic);
					}
					number_grid(grid);
					grid->first_reveal = 1;
				}
			}
			discover_tile(grid, ele_x, ele_y);
		}
		if(button_action & (1 << 1)){	//For X press
			if(!grid->game_live){
				timer_ms_gettime(start_time, start_ms_time);
				grid->game_live = 1;
			}
			x_press(grid, ele_x, ele_y);
		}
		if(button_action & (1 << 2)){	//For B press
			b_press(grid, options, ele_x + grid->x * ele_y);
		}
	}
}

//Its purpose is to check if the player's cursor is on the grid when doing an A/B/X action
void cursor_on_grid(MinesweeperGrid_t * grid, uint8_t *in_grid, int *ele_x, int *ele_y, uint8_t button_action, int id, float *cursor_position){
	if((button_action % (1 << 3)) || !grid->over_mode){
		*in_grid = (cursor_position[2 * id] < grid->start_x + (grid->x * 16)) &&
		(cursor_position[(2 * id) + 1] < grid->start_y + (grid->y * 16)) &&
		cursor_position[2 * id] >= grid->start_x && cursor_position[(2 * id) + 1] >= grid->start_y;
		if(*in_grid){
			*ele_x = (cursor_position[2 * id]  - grid->start_x) / 16;
			*ele_y = (cursor_position[(2 * id) + 1]  - grid->start_y) / 16;
		}
	}
}

//keyboard logic
uint8_t keyboard_logic(MinesweeperKeyboard_t * keyboard, MinesweeperOptions_t * options, float *cursor_pos, uint8_t A_released, uint16_t time){
	if(A_released){
		if(options->focus == 1){	//simplify this to use a char array index
			uint8_t mini_button_pressed = 0;
			uint8_t i;
			for(i = 0; i < keyboard->mini_buttons.num_sprites; i++){
				if((cursor_pos[0] >= keyboard->mini_buttons.positions[(2 * i)]) && (cursor_pos[1] >= keyboard->mini_buttons.positions[(2 * i) + 1])
					&& cursor_pos[0] <= keyboard->mini_buttons.positions[(2 * i)] + keyboard->mini_buttons.animation->animation_frame_width &&
					cursor_pos[1] <= keyboard->mini_buttons.positions[(2 * i) + 1] + keyboard->mini_buttons.animation->animation_frame_height){
					mini_button_pressed = 1;
					break;
				}
			}

			if(!mini_button_pressed){
				if((cursor_pos[0] >= keyboard->key_big_buttons.positions[0]) && (cursor_pos[1] >= keyboard->key_big_buttons.positions[1])
					&& cursor_pos[0] <= keyboard->key_big_buttons.positions[0] + keyboard->key_big_buttons.animation->animation_frame_width &&
					cursor_pos[1] <= keyboard->key_big_buttons.positions[1] + keyboard->key_big_buttons.animation->animation_frame_height){	//Enter pressed

					//Record names must have at least one char
					if(keyboard->chars_typed <= 0){
						return 0;
					}

					//Save this record
					options->savefile.times[keyboard->record_index] = time;
					strcpy(options->savefile.record_names[keyboard->record_index], keyboard->type_buffer);

					//Reset the buffer for the next score
					keyboard->type_buffer[0] = '\0';
					keyboard->chars_typed = 0;

					//It makes sense to save the record immediately
					options->savefile.options = options->question_enabled + (options->sound_enabled << 1)
						+ (options->operating_system << 2) + (options->language << 3) + (options->htz << 4);
					crayon_savefile_save_uncompressed_save(&options->savefile_details);

					//Resume focus 0 (Normal game play)
					options->focus = 0;	//Change to 2 later when I make that screen
				}
				else if((cursor_pos[0] >= keyboard->key_big_buttons.positions[2]) && (cursor_pos[1] >= keyboard->key_big_buttons.positions[3])
					&& cursor_pos[0] <= keyboard->key_big_buttons.positions[2] + keyboard->key_big_buttons.animation->animation_frame_width &&
					cursor_pos[1] <= keyboard->key_big_buttons.positions[3] + keyboard->key_big_buttons.animation->animation_frame_height){	//Space pressed
					if(keyboard->chars_typed < 15){
						keyboard->type_buffer[keyboard->chars_typed + 1] = '\0';
						keyboard->type_buffer[keyboard->chars_typed] = ' ';
						keyboard->chars_typed++;
					}
				}
				return 0;
			}

			if(mini_button_pressed){
				//if shift then toggle caps
				if(i == 20 || i == 30){
					keyboard->caps = !keyboard->caps;
					return 0;
				}

				//if back then remove a char
				if(i == 10){
					if(keyboard->chars_typed > 0){
						keyboard->type_buffer[keyboard->chars_typed - 1] = '\0';
						keyboard->chars_typed--;
					}
					return 0;
				}

				//else append the new char
				if(keyboard->chars_typed < 15){
					keyboard->type_buffer[keyboard->chars_typed + 1] = '\0';
					if(keyboard->caps){
						keyboard->type_buffer[keyboard->chars_typed] = keyboard->upper_keys[i];
					}
					else{
						keyboard->type_buffer[keyboard->chars_typed] = keyboard->lower_keys[i];
					}
					keyboard->chars_typed++;
				}
			}

			return 0;
		}
	}
	return 1;
}

//Handles focus related things
//cursor_pos is a pointer to the beginning of the cursor we are checking with
uint8_t button_press_logic_buttons(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, MinesweeperOS_t *os, crayon_textured_array_t * face,
	float *cursor_pos, uint32_t previous_buttons, uint32_t buttons){
	//CLEAN UP THE MAGIC NUMBERS
	if(options->focus != 1){	//When we get a new score, we don't want to change focus easily
		if((buttons & CONT_A) && !(previous_buttons & CONT_A)){
			if(cursor_pos[1] <= os->tabs_y + 13 && cursor_pos[1] >= os->tabs_y - 4){
				if((cursor_pos[0] <= options->tabs_x[0] + options->tabs_width[0] + 4) && 
						cursor_pos[0] >= options->tabs_x[0] - 4){
					options->focus = 0;
				}
				else if((cursor_pos[0] <= options->tabs_x[1] + options->tabs_width[1] + 4) && 
						cursor_pos[0] >= options->tabs_x[1] - 4){
					options->focus = 2;
				}
				else if((cursor_pos[0] <= options->tabs_x[2] + options->tabs_width[2] + 4) && 
						cursor_pos[0] >= options->tabs_x[2] - 4){
					options->focus = 3;
				}
				else if((cursor_pos[0] <= options->tabs_x[3] + options->tabs_width[3] + 4) && 
						cursor_pos[0] >= options->tabs_x[3] - 4){
					options->focus = 4;
				}
				else if((cursor_pos[0] <= options->tabs_x[4] + options->tabs_width[4] + 4) && 
						cursor_pos[0] >= options->tabs_x[4] - 4){
					options->focus = 5;
				}
			}
		}
	}

	if(options->focus == 2){
		if((buttons & CONT_A) && !(previous_buttons & CONT_A)){	//When we get a new score, we don't want to change focus easily
			if((cursor_pos[0] <= options->number_changers.positions[0] + 16) && (cursor_pos[1] <= options->number_changers.positions[1] + 9)
					&& cursor_pos[0] >= options->number_changers.positions[0] && cursor_pos[1] >= options->number_changers.positions[1]){	//Incrementer Y
				if(options->disp_y < 21){
					options->disp_y++;
					sprintf(options->y_buffer, "%d", options->disp_y);
				}
			}
			else if((cursor_pos[0] <= options->number_changers.positions[0] + 16) && (cursor_pos[1] <= options->number_changers.positions[1] + 19)
					&& cursor_pos[0] >= options->number_changers.positions[0] && cursor_pos[1] >= options->number_changers.positions[1] + 10){	//Decrementer Y
				if(options->disp_y > 9){
					options->disp_y--;
					sprintf(options->y_buffer, "%d", options->disp_y);
				}
			}
			else if((cursor_pos[0] <= options->number_changers.positions[2] + 16) && (cursor_pos[1] <= options->number_changers.positions[3] + 9)
					&& cursor_pos[0] >= options->number_changers.positions[2] && cursor_pos[1] >= options->number_changers.positions[3]){	//Incrementer X
				if(options->disp_x < 38){
					options->disp_x++;
					sprintf(options->x_buffer, "%d", options->disp_x);
				}
			}
			else if((cursor_pos[0] <= options->number_changers.positions[2] + 16) && (cursor_pos[1] <= options->number_changers.positions[3] + 19)
					&& cursor_pos[0] >= options->number_changers.positions[2] && cursor_pos[1] >= options->number_changers.positions[3] + 10){	//Decrementer X
				if(options->disp_x > 9){
					options->disp_x--;
					sprintf(options->x_buffer, "%d", options->disp_x);
				}
			}
			else if((cursor_pos[0] <= options->number_changers.positions[4] + 16) && (cursor_pos[1] <= options->number_changers.positions[5] + 9)
					&& cursor_pos[0] >= options->number_changers.positions[4] && cursor_pos[1] >= options->number_changers.positions[5]){	//Incrementer Num Mines
				if(options->disp_mines < (options->disp_x - 1) * (options->disp_y - 1)){
					options->disp_mines++;
				}
				else{
					options->disp_mines = (options->disp_x - 1) * (options->disp_y - 1);
				}
				sprintf(options->m_buffer, "%d", options->disp_mines);
			}
			else if((cursor_pos[0] <= options->number_changers.positions[4] + 16) && (cursor_pos[1] <= options->number_changers.positions[5] + 19)
					&& cursor_pos[0] >= options->number_changers.positions[4] && cursor_pos[1] >= options->number_changers.positions[5] + 10){	//Decrementer Num Mines
				if(options->disp_mines > 10){
					options->disp_mines--;
					sprintf(options->m_buffer, "%d", options->disp_mines);
				}
			}
			else if((cursor_pos[0] <= options->buttons.positions[0] + 75) && (cursor_pos[1] <= options->buttons.positions[1] + 23)
					&& cursor_pos[0] >= options->buttons.positions[0] && cursor_pos[1] >= options->buttons.positions[1]){	//Beginner
				sprintf(options->x_buffer, "%d", options->disp_x);
				sprintf(options->y_buffer, "%d", options->disp_y);
				sprintf(options->m_buffer, "%d", options->disp_mines);
				reset_grid(grid, options, 9, 9, 10);
				face->frame_coord_keys[0] = 0;	//Reset the face
			}
			else if((cursor_pos[0] <= options->buttons.positions[2] + 75) && (cursor_pos[1] <= options->buttons.positions[3] + 23)
					&& cursor_pos[0] >= options->buttons.positions[2] && cursor_pos[1] >= options->buttons.positions[3]){	//Intermediate
				sprintf(options->x_buffer, "%d", options->disp_x);
				sprintf(options->y_buffer, "%d", options->disp_y);
				sprintf(options->m_buffer, "%d", options->disp_mines);
				reset_grid(grid, options, 16, 16, 40);
				face->frame_coord_keys[0] = 0;
			}
			else if((cursor_pos[0] <= options->buttons.positions[4] + 75) && (cursor_pos[1] <= options->buttons.positions[5] + 23)
					&& cursor_pos[0] >= options->buttons.positions[4] && cursor_pos[1] >= options->buttons.positions[5]){	//Expert
				sprintf(options->x_buffer, "%d", options->disp_x);
				sprintf(options->y_buffer, "%d", options->disp_y);
				sprintf(options->m_buffer, "%d", options->disp_mines);
				reset_grid(grid, options, 30, 16, 99);
				face->frame_coord_keys[0] = 0;
			}
			else if((cursor_pos[0] <= options->buttons.positions[6] + 75) && (cursor_pos[1] <= options->buttons.positions[7] + 23)
					&& cursor_pos[0] >= options->buttons.positions[6] && cursor_pos[1] >= options->buttons.positions[7]){	//Save to VMU
				options->savefile.options = options->question_enabled + (options->sound_enabled << 1)
					+ (options->operating_system << 2) + (options->language << 3) + (options->htz << 4);
				crayon_savefile_save_uncompressed_save(&options->savefile_details);
			}
			else if((cursor_pos[0] <= options->buttons.positions[8] + 75) && (cursor_pos[1] <= options->buttons.positions[9] + 23)
					&& cursor_pos[0] >= options->buttons.positions[8] && cursor_pos[1] >= options->buttons.positions[9]){	//Apply
				if(options->disp_mines > (options->disp_x - 1) * (options->disp_y - 1)){
					options->disp_mines = (options->disp_x - 1) * (options->disp_y - 1);
					sprintf(options->m_buffer, "%d", options->disp_mines);
				}
				reset_grid(grid, options, options->disp_x, options->disp_y, options->disp_mines);
				face->frame_coord_keys[0] = 0;
			}
			else if((cursor_pos[0] <= options->checkers.positions[0] + 13) && (cursor_pos[1] <= options->checkers.positions[1] + 13)
					&& cursor_pos[0] >= options->checkers.positions[0] && cursor_pos[1] >= options->checkers.positions[1]){	//Sound checker
				options->sound_enabled = !options->sound_enabled;
				options->checkers.frame_coord_keys[0] = options->sound_enabled;	//Update to show the right frame
			}
			else if((cursor_pos[0] <= options->checkers.positions[2] + 13) && (cursor_pos[1] <= options->checkers.positions[3] + 13)
					&& cursor_pos[0] >= options->checkers.positions[2] && cursor_pos[1] >= options->checkers.positions[3]){	//Question mark checker
				options->question_enabled = !options->question_enabled;
				options->checkers.frame_coord_keys[1] = options->question_enabled;	//Update to show the right frame
			}
		}
	}

	return 0;
}

//This handles all the buttons you can press aside from the grid itself. Currently only does the face, your re-add the buttons variant here
uint8_t button_press_logic(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, uint8_t button_action, int id, float *cursor_position,
	uint32_t *previous_buttons, uint32_t buttons){
	if(options->focus == 0){
		if((button_action & (1 << 0)) && (cursor_position[2 * id] <= 307 + 26) && (cursor_position[(2 * id) + 1] <= 64 + 26)
			&& cursor_position[2 * id] >= 307 && cursor_position[(2 * id) + 1] >= 64){	//If face is released on
			clear_grid(grid);
			previous_buttons[id] = buttons;	//Store the previous button press
			//Originally it set the face id to zero, but I removed it because it felt pointless
			return 1;
		}
	}

	return 0;
}

uint8_t button_hover(float cursor_x, float cursor_y, MinesweeperOS_t * os, MinesweeperOptions_t * options){
	if(cursor_y > os->tabs_y + 13 || cursor_y < os->tabs_y - 3){	//If not on the box layer, just return instantly
		//!(var <= x && var >= y) == var > x || var < y
		//var <= 5 && var >= 0 becomes var > 5 || var < 0
		return 0;

	}

	uint8_t i;
	for(i = 0; i < 5; i++){
		if(cursor_x >= options->tabs_x[i] - 4 && cursor_x <= options->tabs_x[i] + options->tabs_width[i] + 4){
			return (1 << i);
		}
	}

	return 0;
}

//switches you between English and Italian modes
	//Maybe make this a part of button_press_logic?
void language_swap(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, MinesweeperOS_t * os, float * cursor_pos, uint8_t a_press){
	if(cursor_pos[0] > os->assets[os->lang_id]->positions[0] && cursor_pos[1] > os->assets[os->lang_id]->positions[1] &&
		cursor_pos[0] < os->assets[os->lang_id]->positions[0] + os->assets[os->lang_id]->animation->animation_frame_width &&
		cursor_pos[1] < os->assets[os->lang_id]->positions[1] + os->assets[os->lang_id]->animation->animation_frame_height &&
		a_press){
		options->language = !options->language;

		//Switch between english and italian grid ss, anim and pals
		crayon_spritesheet_t * temp_ss = grid->alt_ss;
		crayon_animation_t * temp_anim = grid->alt_anim;
		crayon_palette_t * temp_pal = grid->alt_pal;
		grid->alt_ss = grid->draw_grid.spritesheet;
		grid->alt_anim = grid->draw_grid.animation;
		grid->alt_pal = grid->draw_grid.palette;
		grid->draw_grid.spritesheet = temp_ss;
		grid->draw_grid.animation = temp_anim;
		grid->draw_grid.palette = temp_pal;

		//Swap out grid frame coords
		uint8_t i;
		for(i = 0; i < grid->draw_grid.unique_frames; i++){
			graphics_frame_coordinates(grid->draw_grid.animation, grid->draw_grid.frame_coord_map + (2 * i), grid->draw_grid.frame_coord_map + (2 * i) + 1, i);
		}
		//Use alternative frame coords for OS assets
		for(i = 0; i < os->num_assets; i++){
			if(os->assets[i]->unique_frames > 1){
				os->assets[i]->frame_coord_keys[0] = options->language;
			}
		}

	}
	return;
}

pvr_init_params_t pvr_params = {
		// Enable opaque, translucent and punch through polygons with size 16
			//To better explain, the opb_sizes or Object Pointer Buffer sizes
			//Work like this: Set to zero to disable. Otherwise the higher the
			//number the more space used (And more efficient under higher loads)
			//The lower the number, the less space used and less efficient under
			//high loads. You can choose 0, 8, 16 or 32
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },

		// Vertex buffer size 512K
		512 * 1024,

		// No DMA
		0,

		// No FSAA
		0,

		// Translucent Autosort enabled
		0
};

int main(){
	MinesweeperGrid_t MS_grid;	//Contains a bunch of variables related to the grid
	MinesweeperOptions_t MS_options;	//Contains a bunch of vars related to options
	MinesweeperKeyboard_t MS_keyboard;	//The struct containing everything keyboard related

	//Setting some default variables
	MS_grid.over_mode = 0;
	MS_grid.game_live = 0;
	MS_options.sd_present = 0;
	MS_options.focus = 0;

	//Load the VMU icon data (The icon appears distorted in SD mode though...I don't see why)
	#if CRAYON_SD_MODE == 1
		crayon_memory_mount_romdisk("/sd/SaveFile.img", "/Save");
	#else
		crayon_memory_mount_romdisk("/cd/SaveFile.img", "/Save");
	#endif

	crayon_savefile_load_icon(&MS_options.savefile_details, "/Save/IMAGE.BIN", "/Save/PALLETTE.BIN");

	fs_romdisk_unmount("/SaveFile");

	//Setting the default save_detail vars
	crayon_savefile_init_savefile_details(&MS_options.savefile_details, (uint8_t *)&MS_options.savefile,
		sizeof(minesweeper_savefile_t), 1, 0, "Made with Crayon by Protofall\0", "Minesweeper\0",
		"Proto_Minesweep\0", "MINESWEEPER.s\0");

	//Pre 1.2.0 savefiles has an incorrect app_id, this function updates older save files
	uint8_t old_saves = setup_update_old_saves(&MS_options.savefile_details);

	MS_options.savefile_details.valid_vmus = crayon_savefile_get_valid_vmus(&MS_options.savefile_details);
	MS_options.savefile_details.valid_saves = crayon_savefile_get_valid_saves(&MS_options.savefile_details);
	MS_options.savefile_details.valid_vmu_screens = crayon_savefile_get_valid_screens();

	uint8_t first_time = !MS_options.savefile_details.valid_saves;

	//Find the first savefile (if it exists)
	int iter;
	int jiter;
	for(iter = 0; iter <= 3; iter++){
		for(jiter = 1; jiter <= 2; jiter++){
			if(crayon_savefile_get_vmu_bitmap(MS_options.savefile_details.valid_saves, iter, jiter)){	//Use the left most VMU
				MS_options.savefile_details.port = iter;
				MS_options.savefile_details.slot = jiter;
				goto Exit_loop_1;
			}
		}
	}
	Exit_loop_1:

	//If a save already exists
	crayon_savefile_load_uncompressed_save(&MS_options.savefile_details);
	if(MS_options.savefile_details.valid_vmus && MS_options.savefile_details.port != -1 && MS_options.savefile_details.slot != -1){
		MS_options.question_enabled = !!(MS_options.savefile.options & (1 << 0));
		MS_options.sound_enabled = !!(MS_options.savefile.options & (1 << 1));
		MS_options.operating_system = !!(MS_options.savefile.options & (1 << 2));
		MS_options.language = !!(MS_options.savefile.options & (1 << 3));
		MS_options.htz = !!(MS_options.savefile.options & (1 << 4));
	}
	else{	//No VMU isn't present or no savefile yet
		//If we don't already have a savefile, choose a VMU
		if(MS_options.savefile_details.valid_vmus){
			for(iter = 0; iter <= 3; iter++){
				for(jiter = 1; jiter <= 2; jiter++){
					if(crayon_savefile_get_vmu_bitmap(MS_options.savefile_details.valid_vmus, iter, jiter)){	//Use the left most VMU
						MS_options.savefile_details.port = iter;
						MS_options.savefile_details.slot = jiter;
						goto Exit_loop_2;
					}
				}
			}
		}
		Exit_loop_2:

		MS_options.question_enabled = 1;
		MS_options.sound_enabled = 1;
		MS_options.operating_system = 0;
		MS_options.language = 0;
		MS_options.htz = 1;

		MS_options.savefile.options = MS_options.question_enabled + (MS_options.sound_enabled << 1)
				+ (MS_options.operating_system << 2) + (MS_options.language << 3) + (MS_options.htz << 4);

		int8_t i;
		for(i = 0; i < 6; i++){
			strcpy(MS_options.savefile.record_names[i], "Anonymous\0");
			// strcpy(MS_options.savefile.record_names[i], "%%%%%%%%%%%%%%%\0");	//Longest possible name
			MS_options.savefile.times[i] = 999;
		}
		MS_options.savefile.pref_width = 30;
		MS_options.savefile.pref_height = 16;
		MS_options.savefile.pref_mines = 99;

		//Make a new savefile if one isn't present
			//If we have a valid vmu, port and slot won't be minus 1
			//But if we don't they will be minus 1 and the save won't be made
		crayon_savefile_save_uncompressed_save(&MS_options.savefile_details);
	}

	//Currently this is the only way to access some of the hidden features
	//Later OS and htz will be chosen in BIOS
	MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
	if(st->buttons & CONT_B){		//B press
		MS_options.operating_system = !MS_options.operating_system;
	}

	if(st->buttons & CONT_A){		//A press
		MS_options.htz = !MS_options.htz;
	}
	MAPLE_FOREACH_END()

	float htz_adjustment = 1;	//Default, 60Hz, 60/60Hz

	//The dreamcast logo to be displayed on the windows taskbar
	uint8_t region = flashrom_get_region();
	if(region < 0){	//If error we just default to green swirl. Apparently its possible for some DCs to return -1
		region = 0;	//Invalid region
	}

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		if(first_time && 0){	//REMEMBER TO CHANGE THIS IN THE BIOS UPDATE (Currently disabled due to already doing the options before)
			if(region != FLASHROM_REGION_EUROPE){
				vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
			}
			else{
				vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
				if(MS_options.htz == 0){
					htz_adjustment = 1.2;	//60/50Hz
				}
			}
		}
		else{
			if(MS_options.htz){
				vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
			}
			else{
				vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
				if(MS_options.htz == 0){
					htz_adjustment = 1.2;	//60/50Hz
				}
			}
		}
	}

	pvr_init(&pvr_params);

	//Have Hz select menu here, followed by a vid_set_mode call (if need be)

	//Call the "BIOS_bootup_sequence" function. Select OS there

	//Call Windows_load function showing either the XP or 2000 bootup screen

	srand(time(0));	//Set the seed for rand()
	time_t os_clock;	//Stores the current time
	struct tm *time_struct;
	int8_t hour = -1;
	int8_t minute = -1;
	char time_buffer[9];

	float cursor_position[8];
	cursor_position[0] = 100;
	cursor_position[1] = 66;
	cursor_position[2] = 200;
	cursor_position[3] = 66;
	cursor_position[4] = 414;
	cursor_position[5] = 66;
	cursor_position[6] = 524;
	cursor_position[7] = 66;

	crayon_spritesheet_t Board, Icons, Controllers, Windows;
	crayon_palette_t Board_P, Icons_P, Controllers_P, Windows_P, Tahoma_P, White_Tahoma_P,
		cursor_red, cursor_yellow, cursor_green, cursor_blue;
	// crayon_palette_t BIOS_P;
	// crayon_font_mono_t BIOS_font;
	crayon_font_prop_t Tahoma_font;
	Board.spritesheet_texture = NULL;
	Icons.spritesheet_texture = NULL;
	Windows.spritesheet_texture = NULL;
	// BIOS_font.fontsheet_texture = NULL;
	Tahoma_font.fontsheet_texture = NULL;

	crayon_untextured_array_t Bg_polys, Option_polys;	//Contains some of the untextured polys that will be drawn.
	sfxhnd_t Sound_Tick, Sound_Death, Sound_Death_Italian, Sound_Win;	//Sound effect handles. Might add more later for startup sounds or maybe put them in cdda? (Note this is a uint32_t)
	snd_stream_init();	//Needed otherwise snd_sfx calls crash

	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes == 0){
			MS_options.sd_present = 1;
		}
	#endif

	#if CRAYON_SD_MODE == 1
		crayon_memory_mount_romdisk("/sd/Minesweeper.img", "/Minesweeper");
	#else
		crayon_memory_mount_romdisk("/cd/Minesweeper.img", "/Minesweeper");
	#endif

	//I like to put the font's paletets at the very back of the system (But really, its probably better standard to go first)
	// crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 63, "/BIOS/BIOS_font.dtex");
	crayon_memory_load_prop_font_sheet(&Tahoma_font, &Tahoma_P, 62, "/Minesweeper/Tahoma_font.dtex");
	crayon_memory_load_spritesheet(&Board, &Board_P, 0, "/Minesweeper/Board.dtex");
	crayon_memory_load_spritesheet(&Icons, &Icons_P, 1, "/Minesweeper/Icons.dtex");	
	crayon_memory_load_spritesheet(&Controllers, &Controllers_P, 2, "/Minesweeper/Controllers.dtex");

	Sound_Tick = snd_sfx_load("/Minesweeper/Sounds/tick.wav");
	Sound_Death = snd_sfx_load("/Minesweeper/Sounds/death.wav");
	Sound_Death_Italian = snd_sfx_load("/Minesweeper/Sounds/deathItalian.wav");
	Sound_Win = snd_sfx_load("/Minesweeper/Sounds/win.wav");
	fs_romdisk_unmount("/Minesweeper");

	//Load the OS assets
	if(MS_options.operating_system){
		#if CRAYON_SD_MODE == 1
			crayon_memory_mount_romdisk("/sd/XP.img", "/XP");
		#else
			crayon_memory_mount_romdisk("/cd/XP.img", "/XP");
		#endif
		crayon_memory_load_spritesheet(&Windows, NULL, -1, "/XP/Windows.dtex");	//XP doesn't use paletted textures so we don't pass in a palette
		fs_romdisk_unmount("/XP");
	}
	else{
		#if CRAYON_SD_MODE == 1
			crayon_memory_mount_romdisk("/sd/2000.img", "/2000");
		#else
			crayon_memory_mount_romdisk("/cd/2000.img", "/2000");
		#endif
		crayon_memory_load_spritesheet(&Windows, &Windows_P, 1, "/2000/Windows.dtex");
		fs_romdisk_unmount("/2000");
	}

	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	//Make the OS struct and populate it
	MinesweeperOS_t os;

	//Populate OS struct
	setup_OS_assets(&os, &Windows, &Windows_P, MS_options.operating_system, MS_options.language, MS_options.sd_present);
	setup_OS_assets_icons(&os, &Icons, &Icons_P, MS_options.operating_system, region);
	MS_options.tabs_x[0] = 9;	//Game
	MS_options.tabs_width[0] = 27;
	MS_options.tabs_x[1] = MS_options.tabs_x[0] + MS_options.tabs_width[0] + 15;	//Options
	MS_options.tabs_width[1] = 37;
	MS_options.tabs_x[2] = MS_options.tabs_x[1] + MS_options.tabs_width[1] + 15 ;	//Best Times
	MS_options.tabs_width[2] = 51;
	MS_options.tabs_x[3] = MS_options.tabs_x[2] + MS_options.tabs_width[2] + 15 ;	//Controls
	MS_options.tabs_width[3] = 40;
	MS_options.tabs_x[4] = MS_options.tabs_x[3] + MS_options.tabs_width[3] + 15 ;	//About
	MS_options.tabs_width[4] = 29;

	//Add the clock palette
	if(MS_options.operating_system){
		os.clock_palette = &White_Tahoma_P;
	}
	else{
		os.clock_palette = &Tahoma_P;
	}

	//Setting size to 1 since reset_grid will reset it soon anyways
	//Also due to lang thing we don't know the spritesheet, animation or palette
	// crayon_memory_set_sprite_array(&MS_grid.draw_grid, 1, 16, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL);
	crayon_memory_set_sprite_array(&MS_grid.draw_grid, 38 * 21, 16, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL);	//Technically there's no need to make change the size after this

	//Setting up the language spritesheet, animation and palette pointers
	uint8_t tile_id = 0;
	for(iter = 0; iter < Board.spritesheet_animation_count; iter++){
		if(!strcmp(Board.spritesheet_animation_array[iter].animation_name, "tiles")){
			tile_id = iter;
			break;
		}
	}
	if(!MS_options.language){	//English
		MS_grid.draw_grid.spritesheet = &Board;
		MS_grid.draw_grid.animation = &Board.spritesheet_animation_array[tile_id];
		MS_grid.draw_grid.palette = &Board_P;
	}
	else{	//Italian
		MS_grid.alt_ss = &Board;
		MS_grid.alt_anim = &Board.spritesheet_animation_array[tile_id];
		MS_grid.alt_pal = &Board_P;
	}

	for(iter = 0; iter < Windows.spritesheet_animation_count; iter++){
		if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "italianTiles")){
			tile_id = iter;
			break;
		}
	}

	if(MS_options.language){	//Italian
		MS_grid.draw_grid.spritesheet = &Windows;
		MS_grid.draw_grid.animation = &Windows.spritesheet_animation_array[tile_id];
		MS_grid.draw_grid.palette = &Windows_P;	//I think this is fine in XP mode
	}
	else{	//English
		MS_grid.alt_ss = &Windows;
		MS_grid.alt_anim = &Windows.spritesheet_animation_array[tile_id];
		MS_grid.alt_pal = &Windows_P;
	}

	//Indent draw list
	crayon_textured_array_t indented_tiles;
	crayon_memory_set_sprite_array(&indented_tiles, 8 * 4, 2, 0, 1, 0, 0, 0, 0, 0, MS_grid.alt_ss, MS_grid.alt_anim, MS_grid.alt_pal);
	indented_tiles.draw_z[0] = 18;
	indented_tiles.scales[0] = 1;
	indented_tiles.scales[1] = 1;
	indented_tiles.flips[0] = 0;
	indented_tiles.rotations[0] = 0;
	indented_tiles.colours[0] = 0;
	graphics_frame_coordinates(indented_tiles.animation, indented_tiles.frame_coord_map + 0, indented_tiles.frame_coord_map + 1, 6);	//Indent question
	graphics_frame_coordinates(indented_tiles.animation, indented_tiles.frame_coord_map + 2, indented_tiles.frame_coord_map + 3, 7);	//Indent blank

	//Setting defaults for the grid (These won't ever change again)
	MS_grid.draw_grid.draw_z[0] = 17;
	MS_grid.draw_grid.scales[0] = 1;
	MS_grid.draw_grid.scales[1] = 1;
	MS_grid.draw_grid.flips[0] = 0;
	MS_grid.draw_grid.rotations[0] = 0;
	MS_grid.draw_grid.colours[0] = 0;
	for(iter = 0; iter < MS_grid.draw_grid.unique_frames; iter++){
		graphics_frame_coordinates(MS_grid.draw_grid.animation, MS_grid.draw_grid.frame_coord_map + (2 * iter),
			MS_grid.draw_grid.frame_coord_map + (2 * iter) + 1, iter);
	}

	//Get the info for num_changer
	uint8_t num_changer_id = 4;
	uint8_t button_id = 4;
	uint8_t miniButton_id = 4;
	uint8_t checker_id = 4;
	for(iter = 0; iter < Windows.spritesheet_animation_count; iter++){
		if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "button")){
			button_id = iter;
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "miniButton")){
			miniButton_id = iter;
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "checker")){
			checker_id = iter;
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "numberChanger")){
			num_changer_id = iter;
		}
	}

	//Controller pixel art on the 
	crayon_textured_array_t devices;
	crayon_memory_set_sprite_array(&devices, 2, 2, 0, 1, 0, 0, 0, 0, 0, &Controllers, &Controllers.spritesheet_animation_array[0], &Controllers_P);
	devices.scales[0] = 1;
	devices.scales[1] = 1;
	devices.flips[0] = 0;
	devices.rotations[0] = 0;
	devices.colours[0] = 0;
	devices.positions[0] = 40;
	devices.positions[1] = 108;
	devices.positions[2] = 640 - 256 - 20;
	devices.positions[3] = devices.positions[1];
	devices.draw_z[0] = 31;
	devices.frame_coord_keys[0] = 0;
	devices.frame_coord_keys[1] = 1;
	graphics_frame_coordinates(devices.animation, devices.frame_coord_map + 0, devices.frame_coord_map + 1, 0);
	graphics_frame_coordinates(devices.animation, devices.frame_coord_map + 2, devices.frame_coord_map + 3, 1);

	//Dot for legend numbers (5 for gamepad and 3 for mouse legend, 1 for face, 11 in the legend)
	crayon_textured_array_t legend_dot;
	crayon_memory_set_sprite_array(&legend_dot, 20, 1, 0, 0, 0, 0, 0, 0, 0, &Controllers, &Controllers.spritesheet_animation_array[1], &Controllers_P);
	legend_dot.scales[0] = 1;
	legend_dot.scales[1] = 1;
	legend_dot.flips[0] = 0;
	legend_dot.rotations[0] = 0;
	legend_dot.colours[0] = 0;

	//Mouse
	legend_dot.positions[0] = 380;
	legend_dot.positions[1] = 140;
	legend_dot.positions[2] = 588;
	legend_dot.positions[3] = 140;
	legend_dot.positions[4] = 380;
	legend_dot.positions[5] = 182;

	//Controller
	legend_dot.positions[6] = 320;
	legend_dot.positions[7] = 182;
	legend_dot.positions[8] = legend_dot.positions[6];
	legend_dot.positions[9] = legend_dot.positions[7] + 21;
	legend_dot.positions[10] = legend_dot.positions[6];
	legend_dot.positions[11] = legend_dot.positions[7] + 41;
	legend_dot.positions[12] = legend_dot.positions[6];
	legend_dot.positions[13] = legend_dot.positions[7] + 61;
	legend_dot.positions[14] = 161;
	legend_dot.positions[15] = legend_dot.positions[6] - 2;

	//Face
	legend_dot.positions[16] = 344;
	legend_dot.positions[17] = 70;

	//Legend
	legend_dot.positions[18] = 200;
	legend_dot.positions[19] = 366;
	legend_dot.positions[20] = legend_dot.positions[18] + 23;
	legend_dot.positions[21] = legend_dot.positions[19];

	legend_dot.positions[22] = legend_dot.positions[18];
	legend_dot.positions[23] = legend_dot.positions[19] + 16;
	legend_dot.positions[24] = legend_dot.positions[18] + 23;
	legend_dot.positions[25] = legend_dot.positions[19] + 16;

	legend_dot.positions[26] = legend_dot.positions[18];
	legend_dot.positions[27] = legend_dot.positions[19] + 32;
	legend_dot.positions[28] = legend_dot.positions[18] + 23;
	legend_dot.positions[29] = legend_dot.positions[19] + 32;
	legend_dot.positions[30] = legend_dot.positions[18] + 23 + 23 + 6;
	legend_dot.positions[31] = legend_dot.positions[19] + 32;
	legend_dot.positions[32] = legend_dot.positions[18] + 23 + 23 + 6 + 23 + 7;
	legend_dot.positions[33] = legend_dot.positions[19] + 32;

	legend_dot.positions[34] = legend_dot.positions[18];
	legend_dot.positions[35] = legend_dot.positions[19] + 48;

	legend_dot.positions[36] = legend_dot.positions[18];
	legend_dot.positions[37] = legend_dot.positions[19] + 64;
	legend_dot.positions[38] = legend_dot.positions[18] + 23;
	legend_dot.positions[39] = legend_dot.positions[19] + 64;

	legend_dot.draw_z[0] = 33;
	legend_dot.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(legend_dot.animation, legend_dot.frame_coord_map + 0, legend_dot.frame_coord_map + 1, 0);

	//Controller swirl
	crayon_textured_array_t cont_swirl;
	crayon_memory_set_sprite_array(&cont_swirl, 1, 1, 0, 0, 0, 0, 0, 0, 0, &Controllers, &Controllers.spritesheet_animation_array[2], &Controllers_P);
	cont_swirl.scales[0] = 1;
	cont_swirl.scales[1] = 1;
	cont_swirl.flips[0] = 0;
	cont_swirl.rotations[0] = 0;
	cont_swirl.colours[0] = 0;
	cont_swirl.positions[0] = devices.positions[0] + 118;
	cont_swirl.positions[1] = devices.positions[1] + 10;
	cont_swirl.draw_z[0] = 32;
	cont_swirl.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(cont_swirl.animation, cont_swirl.frame_coord_map + 0, cont_swirl.frame_coord_map + 1, region);

	crayon_textured_array_t digit_display;
	crayon_memory_set_sprite_array(&digit_display, 6, 11, 0, 1, 0, 0, 0, 0, 0, &Board, &Board.spritesheet_animation_array[1], &Board_P);
	digit_display.scales[0] = 1;
	digit_display.scales[1] = 1;
	digit_display.flips[0] = 0;
	digit_display.rotations[0] = 0;
	digit_display.colours[0] = 0;
	digit_display.positions[0] = 20;
	digit_display.positions[1] = 65;
	digit_display.positions[2] = digit_display.positions[0] + digit_display.animation->animation_frame_width;
	digit_display.positions[3] = digit_display.positions[1];
	digit_display.positions[4] = digit_display.positions[2] + digit_display.animation->animation_frame_width;
	digit_display.positions[5] = digit_display.positions[1];
	digit_display.positions[6] = 581;
	digit_display.positions[7] = digit_display.positions[1];
	digit_display.positions[8] = digit_display.positions[6] + digit_display.animation->animation_frame_width;
	digit_display.positions[9] = digit_display.positions[1];
	digit_display.positions[10] = digit_display.positions[8] + digit_display.animation->animation_frame_width;
	digit_display.positions[11] = digit_display.positions[1];
	digit_display.draw_z[0] = 17;
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 0, digit_display.frame_coord_map + 1, 0);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 2, digit_display.frame_coord_map + 3, 1);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 4, digit_display.frame_coord_map + 5, 2);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 6, digit_display.frame_coord_map + 7, 3);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 8, digit_display.frame_coord_map + 9, 4);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 10, digit_display.frame_coord_map + 11, 5);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 12, digit_display.frame_coord_map + 13, 6);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 14, digit_display.frame_coord_map + 15, 7);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 16, digit_display.frame_coord_map + 17, 8);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 18, digit_display.frame_coord_map + 19, 9);
	graphics_frame_coordinates(digit_display.animation, digit_display.frame_coord_map + 20, digit_display.frame_coord_map + 21, 10);

	crayon_textured_array_t face;
	crayon_memory_set_sprite_array(&face, 1, 5, 0, 1, 0, 0, 0, 0, 0, &Board, &Board.spritesheet_animation_array[0], &Board_P);
	face.scales[0] = 1;
	face.scales[1] = 1;
	face.flips[0] = 0;
	face.rotations[0] = 0;
	face.colours[0] = 0;
	face.positions[0] = 307;
	face.positions[1] = 64;
	face.draw_z[0] = 16;
	face.frame_coord_keys[0] = 0;	//Neutral face
	graphics_frame_coordinates(face.animation, face.frame_coord_map + 0, face.frame_coord_map + 1, 0);
	graphics_frame_coordinates(face.animation, face.frame_coord_map + 2, face.frame_coord_map + 3, 1);
	graphics_frame_coordinates(face.animation, face.frame_coord_map + 4, face.frame_coord_map + 5, 2);
	graphics_frame_coordinates(face.animation, face.frame_coord_map + 6, face.frame_coord_map + 7, 3);
	graphics_frame_coordinates(face.animation, face.frame_coord_map + 8, face.frame_coord_map + 9, 4);

	//Some defaults for the keyboard struct
	MS_keyboard.type_buffer[0] = '\0';
	MS_keyboard.chars_typed = 0;
	MS_keyboard.caps = 1;

	//26 letters, 2 shifts, and ", !" and ". ?" buttons
	crayon_memory_set_sprite_array(&MS_keyboard.mini_buttons, 31, 1, 0, 0, 0, 0, 0, 0, 0, &Windows, &Windows.spritesheet_animation_array[miniButton_id], &Windows_P);
	MS_keyboard.mini_buttons.scales[0] = 1;
	MS_keyboard.mini_buttons.scales[1] = 1;
	MS_keyboard.mini_buttons.flips[0] = 0;
	MS_keyboard.mini_buttons.rotations[0] = 0;
	MS_keyboard.mini_buttons.colours[0] = 0;
	MS_keyboard.mini_buttons.draw_z[0] = 30;
	MS_keyboard.mini_buttons.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(MS_keyboard.mini_buttons.animation, MS_keyboard.mini_buttons.frame_coord_map, MS_keyboard.mini_buttons.frame_coord_map + 1, 0);

	setup_keys(&MS_keyboard);

	uint8_t keyboard_start_x = 161;
	uint16_t keyboard_start_y = 250;
	MS_keyboard.type_box_x = keyboard_start_x - 8 + 71;
	MS_keyboard.type_box_y = keyboard_start_y - 88 + 56;
	//Bix box is 334 wide. half is 167. 11 times 10 is 110. + 10 (Each side) is 120. 167 - 60 = 107

	for(iter = 0; iter < 31; iter++){
		if(iter < 11){
			MS_keyboard.mini_buttons.positions[2 * iter] = keyboard_start_x + (iter * 27) + 12;
			MS_keyboard.mini_buttons.positions[(2 * iter) + 1] = keyboard_start_y;
		}
		else if(iter < 20){
			MS_keyboard.mini_buttons.positions[2 * iter] = keyboard_start_x + ((iter - 11) * 27);
			MS_keyboard.mini_buttons.positions[(2 * iter) + 1] = keyboard_start_y + 30 - 3;
		}
		else{
			MS_keyboard.mini_buttons.positions[2 * iter] = keyboard_start_x + ((iter - 20) * 27) + 12;
			MS_keyboard.mini_buttons.positions[(2 * iter) + 1] = keyboard_start_y + 60 - 6;	
		}
	}

	//Space and Enter
	crayon_memory_set_sprite_array(&MS_keyboard.key_big_buttons, 2, 1, 0, 0, 0, 0, 0, 0, 0, &Windows, &Windows.spritesheet_animation_array[button_id], &Windows_P);
	MS_keyboard.key_big_buttons.scales[0] = 1;
	MS_keyboard.key_big_buttons.scales[1] = 1;
	MS_keyboard.key_big_buttons.flips[0] = 0;
	MS_keyboard.key_big_buttons.rotations[0] = 0;
	MS_keyboard.key_big_buttons.colours[0] = 0;
	MS_keyboard.key_big_buttons.draw_z[0] = 30;
	MS_keyboard.key_big_buttons.frame_coord_keys[0] = 0;
	MS_keyboard.key_big_buttons.positions[0] = keyboard_start_x + 243;	//Enter
	MS_keyboard.key_big_buttons.positions[1] = keyboard_start_y + 27;
	MS_keyboard.key_big_buttons.positions[2] = keyboard_start_x + 121;	//Space
	MS_keyboard.key_big_buttons.positions[3] = keyboard_start_y + 81;
	graphics_frame_coordinates(MS_keyboard.key_big_buttons.animation, MS_keyboard.key_big_buttons.frame_coord_map, MS_keyboard.key_big_buttons.frame_coord_map + 1, 0);

	//Options draw structs
	crayon_memory_set_sprite_array(&MS_options.buttons, 5, 1, 0, 0, 0, 0, 0, 0, 0, &Windows, &Windows.spritesheet_animation_array[button_id], &Windows_P);
	crayon_memory_set_sprite_array(&MS_options.checkers, 2, 2, 0, 1, 0, 0, 0, 0, 0, &Windows, &Windows.spritesheet_animation_array[checker_id], &Windows_P);
	crayon_memory_set_sprite_array(&MS_options.number_changers, 3, 1, 0, 0, 0, 0, 0, 0, 0, &Windows, &Windows.spritesheet_animation_array[num_changer_id], &Windows_P);

	//Buttons
	MS_options.buttons.draw_z[0] = 30;
	MS_options.buttons.scales[0] = 1;
	MS_options.buttons.scales[1] = 1;
	MS_options.buttons.flips[0] = 0;
	MS_options.buttons.rotations[0] = 0;
	MS_options.buttons.colours[0] = 0;
	MS_options.buttons.positions[0] = 408;	//Beginner
	MS_options.buttons.positions[1] = 113 + (2 * MS_options.operating_system);
	MS_options.buttons.positions[2] = MS_options.buttons.positions[0];			//Intermediate
	MS_options.buttons.positions[3] = MS_options.buttons.positions[1] + 50;
	MS_options.buttons.positions[4] = MS_options.buttons.positions[0];			//Expert
	MS_options.buttons.positions[5] = MS_options.buttons.positions[3] + 50;
	MS_options.buttons.positions[6] = MS_options.buttons.positions[0] - 247;	//Save to VMU
	MS_options.buttons.positions[7] = 185;
	MS_options.buttons.positions[8] = MS_options.buttons.positions[6];			//Update Grid
	MS_options.buttons.positions[9] = MS_options.buttons.positions[7] + 30;
	MS_options.buttons.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(MS_options.buttons.animation, MS_options.buttons.frame_coord_map, MS_options.buttons.frame_coord_map + 1, 0);

	//Checkers
	MS_options.checkers.draw_z[0] = 30;
	MS_options.checkers.scales[0] = 1;
	MS_options.checkers.scales[1] = 1;
	MS_options.checkers.flips[0] = 0;
	MS_options.checkers.rotations[0] = 0;
	MS_options.checkers.colours[0] = 0;
	MS_options.checkers.positions[0] = MS_options.buttons.positions[6] + 60;	//Sound
	MS_options.checkers.positions[1] = MS_options.buttons.positions[7] - 66;
	MS_options.checkers.positions[2] = MS_options.checkers.positions[0];		//Questions
	MS_options.checkers.positions[3] = MS_options.checkers.positions[1] + 40;
	MS_options.checkers.frame_coord_keys[0] = MS_options.sound_enabled;
	MS_options.checkers.frame_coord_keys[1] = MS_options.question_enabled;
	graphics_frame_coordinates(MS_options.checkers.animation, MS_options.checkers.frame_coord_map, MS_options.checkers.frame_coord_map + 1, 0);
	graphics_frame_coordinates(MS_options.checkers.animation, MS_options.checkers.frame_coord_map + 2, MS_options.checkers.frame_coord_map + 3, 1);

	//Number_changers
	MS_options.number_changers.draw_z[0] = 30;
	MS_options.number_changers.scales[0] = 1;
	MS_options.number_changers.scales[1] = 1;
	MS_options.number_changers.flips[0] = 0;
	MS_options.number_changers.rotations[0] = 0;
	MS_options.number_changers.colours[0] = 0;
	MS_options.number_changers.positions[0] = MS_options.checkers.positions[0] + 128;	//Height
	MS_options.number_changers.positions[1] = MS_options.buttons.positions[1] + 2;
	MS_options.number_changers.positions[2] = MS_options.number_changers.positions[0];	//Width
	MS_options.number_changers.positions[3] = MS_options.number_changers.positions[1] + 50;
	MS_options.number_changers.positions[4] = MS_options.number_changers.positions[0];	//Mines
	MS_options.number_changers.positions[5] = MS_options.number_changers.positions[1] + 100;
	MS_options.number_changers.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(MS_options.number_changers.animation, MS_options.number_changers.frame_coord_map, MS_options.number_changers.frame_coord_map + 1, 0);

	MS_grid.logic_grid = NULL;
	reset_grid(&MS_grid, &MS_options, MS_options.savefile.pref_width, MS_options.savefile.pref_height,
		MS_options.savefile.pref_mines);

	//Setup the untextured poly structs
	setup_bg_untextured_poly(&Bg_polys, MS_options.operating_system, MS_options.sd_present);
	setup_option_untextured_poly(&Option_polys, &MS_options.number_changers, MS_options.operating_system);

	//For the timer
	uint32_t current_time = 0;
	uint32_t current_ms_time = 0;
	uint32_t start_time = 0;
	uint32_t start_ms_time = 0;

	//For the "start to reset"
	uint8_t start_primed = 0;	//Format -PVA 4321 where the numbers are if the player is holding either nothing or just start,
								//A somewhere start is being pressed. V is invalidated, P is someone pressed combo
	uint8_t player_active = 0;	//Used to confirm if a controller is being used

	//Stores the button combination from the previous button press
	uint32_t previous_buttons[4] = {0};
	uint8_t button_action = 0;	// ---S YBXA format, each maple loop the variable is overritten with released statuses

	//1st element is type of click, 2nd element is x, 3rd element is y and repeat for all 4 controllers
	//1st, 0 = none, 1 = A, 2 = X, X press has priority over A press
	//the x and y are zero indexed like a normal array
	uint16_t press_data[12] = {0};
	uint8_t player_movement = 0;	//This tells whether a player is using a joystick (0) or D-Pad (1) Only last 4 bits are used

	float thumb_x = 0;
	float thumb_y = 0;
	uint8_t thumb_active;

	//The palette for XP's clock
	crayon_memory_clone_palette(&Tahoma_P, &White_Tahoma_P, 61);
	crayon_memory_swap_colour(&White_Tahoma_P, 0xFF000000, 0xFFFFFFFF, 0);	//Swapps black for white

	//The cursor colours
	crayon_memory_clone_palette(&Icons_P, &cursor_red, 8);
	crayon_memory_clone_palette(&Icons_P, &cursor_yellow, 9);
	crayon_memory_clone_palette(&Icons_P, &cursor_green, 10);
	crayon_memory_clone_palette(&Icons_P, &cursor_blue, 11);
	crayon_memory_swap_colour(&cursor_red, 0xFFFFFFFF, 0xFFFF0000, 0);
	crayon_memory_swap_colour(&cursor_yellow, 0xFFFFFFFF, 0xFFFFFF00, 0);
	crayon_memory_swap_colour(&cursor_green, 0xFFFFFFFF, 0xFF008000, 0);
	crayon_memory_swap_colour(&cursor_blue, 0xFFFFFFFF, 0xFF4D87D0, 0);

	#if CRAYON_DEBUG == 1
	//Stuff for debugging/performance testing
		pvr_stats_t pvr_stats;
		uint8_t last_30_FPS[30] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		uint8_t FPS_array_iter = 0;
		int fps_ave, fps_min, fps_max;
	#endif

	while(1){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		if(st->joyx > 0){	//Converting from -128, 127 to -1, 1 CHECK IF I'M DIVIDING RIGHT (Should they be all 127 or 128 or this?)
			thumb_x = (float) st->joyx / 127;
		}
		else{
			thumb_x = (float) st->joyx / 128;
		}
		if(st->joyy > 0){
			thumb_y = (float) st->joyy / 127;
		}
		else{
			thumb_y = (float) st->joyy / 128;
		}

		thumb_active = (thumb_x * thumb_x) + (thumb_y * thumb_y) > 0.16;	//When its moving

		if(start_primed && (st->buttons != CONT_START || thumb_active) && (st->buttons & CONT_START)){	//Player is pressing inpure start

			start_primed |= (1 << (__dev->port));	//Number bits
		}

		//It triggers ONLY when start is the only input
		if(st->buttons == CONT_START && !thumb_active){	//Atleast someone is only pressing start
			start_primed |= (1 << 6);	//P bit
		}

		if(!(player_active & (1 << __dev->port))){	//Player is there, but hasn't been activated yet
			if(st->buttons != 0 || thumb_active){	//Input detected
				player_active |= (1 << __dev->port);
			}
			else{
				continue;
			}
		}

		//Use the buttons previously pressed to control what happens here
		button_action = 0;
		if((previous_buttons[__dev->port] & CONT_X) && !(st->buttons & CONT_X)){	//X released
			button_action |= (1 << 1);
		}
		else{
			if((previous_buttons[__dev->port] & CONT_A) && !(st->buttons & CONT_A)){	//A released
				button_action |= (1 << 0);
			}
			if(!(previous_buttons[__dev->port] & CONT_B) && (st->buttons & CONT_B)){	//B pressed
				button_action |= (1 << 2);
			}
		}
		button_action |= (!(previous_buttons[__dev->port] & CONT_Y) && (st->buttons & CONT_Y)) << 3;	//Y pressed
		button_action |= ((start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && !(start_primed & (1 << 4))) << 4;	//If we press start, but we haven't done active prime yet and we aren't invalidated

		if(button_press_logic(&MS_grid, &MS_options, button_action, __dev->port, cursor_position, previous_buttons, st->buttons)){break;}
		button_press_logic_buttons(&MS_grid, &MS_options, &os, &face, cursor_position + (2 * __dev->port), previous_buttons[__dev->port], st->buttons);
		language_swap(&MS_grid, &MS_options, &os, cursor_position + (2 * __dev->port), button_action & (1 << 0));
		keyboard_logic(&MS_keyboard, &MS_options, cursor_position + (2 * __dev->port), button_action & (1 << 0), MS_grid.time_sec);

		//These are only ever set if The cursor is on the grid and A/B/X is/was pressed
		int ele_x = -1;
		int ele_y = -1;
		uint8_t in_grid = 0;

		if(MS_options.focus == 0){
			cursor_on_grid(&MS_grid, &in_grid, &ele_x, &ele_y, button_action, __dev->port, cursor_position);
			if(in_grid){grid_ABX_logic(&MS_grid, &MS_options, ele_x, ele_y, button_action, &start_time, &start_ms_time);}

			//Start press code
			if(button_action & (1 << 4)){
				start_primed |= (1 << 4);	//A bit
			}
		}

		//Y press code
		if(button_action & (1 << 3)){
			player_movement ^= (1 << __dev->port);
		}

		//Movement code
		if(player_movement & (1 << __dev->port)){	//D-Pad
			float movement_speed = 1.0f;
			//On emulator when mapping keyboard keys to the controller, this gimps DPad while the thumbstick is unlimited
			//If using a real thumbstick then this code should be good
			if(((st->buttons & CONT_DPAD_UP) || (st->buttons & CONT_DPAD_DOWN)) &&
				((st->buttons & CONT_DPAD_LEFT) || (st->buttons & CONT_DPAD_RIGHT))){	//When doing diagonals, don't move as far
				movement_speed = 0.70710678118f;
			}

			if(st->buttons & CONT_DPAD_UP){
				cursor_position[(2 * __dev->port) + 1] -= 2 * movement_speed * htz_adjustment;
				if(cursor_position[(2 * __dev->port) + 1] < 0){
					cursor_position[(2 * __dev->port) + 1] = 0;
				}
			}
			if(st->buttons & CONT_DPAD_DOWN){
				cursor_position[(2 * __dev->port) + 1] += 2 * movement_speed * htz_adjustment;
				if(cursor_position[(2 * __dev->port) + 1] > 480){
					cursor_position[(2 * __dev->port) + 1] = 480;
				}
			}
			if(st->buttons & CONT_DPAD_LEFT){
				cursor_position[2 * __dev->port] -= 2 * movement_speed * htz_adjustment;
				if(cursor_position[2 * __dev->port] < 0){
					cursor_position[2 * __dev->port] = 0;
				}
			}
			if(st->buttons & CONT_DPAD_RIGHT){
				cursor_position[2 * __dev->port] += 2 * movement_speed * htz_adjustment;
				if(cursor_position[2 * __dev->port] > 640){
					cursor_position[2 * __dev->port] = 640;
				}
			}
		}
		else{	//Thumbstick
			if(thumb_active){	//The thumbstick is outside of the 40% radius
				cursor_position[2 * __dev->port] += 2 * thumb_x * htz_adjustment;
				if(cursor_position[2 * __dev->port] < 0){
					cursor_position[2 * __dev->port] = 0;
				}
				else if(cursor_position[2 * __dev->port] > 640){
					cursor_position[2 * __dev->port] = 640;
				}
				cursor_position[(2 * __dev->port) + 1] += 2 * thumb_y * htz_adjustment;
				if(cursor_position[(2 * __dev->port) + 1] < 0){
					cursor_position[(2 * __dev->port) + 1] = 0;
				}
				else if(cursor_position[(2 * __dev->port) + 1] > 480){
					cursor_position[(2 * __dev->port) + 1] = 480;
				}
			}
		}

		//Using thumbstick on emulator:
		//50Hz	5.68 secs, around 284 frames
		//60Hz	5.72 seconds, around 343 frames

		//343/284 ~= 1.204 ~= 60/50. This is good

		face_logic(&MS_grid, &face, cursor_position + (2 * __dev->port), !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)));

		if(MS_options.focus == 0){
			grid_indent_logic(&MS_grid, ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)), !!(st->buttons & (CONT_B)), 0);
		}

		previous_buttons[__dev->port] = st->buttons;	//Store the previous button presses

		MAPLE_FOREACH_END()

		//NOTE TO SELF: Break up some of the controller code into functions since its shared between the controller and the mouse
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_MOUSE, mouse_state_t, st)	//Mouse support

		//In emulators like lxdream this will usually trigger instantly since it doesn't do mouse buttons that well
		if(!(player_active & (1 << __dev->port))){	//Player is there, but hasn't been activated yet
			if(st->buttons != 0 || st->dx != 0 || st->dy != 0){	//Input detected
				player_active |= (1 << __dev->port);
			}
			else{
				continue;
			}
		}

		button_action = 0;
		//Side was released OR (L and R were pressed, but now (L OR R) is released)
		if(((previous_buttons[__dev->port] & MOUSE_SIDEBUTTON) && !(st->buttons & MOUSE_SIDEBUTTON)) ||
			((previous_buttons[__dev->port] & MOUSE_LEFTBUTTON) && (previous_buttons[__dev->port] & MOUSE_RIGHTBUTTON) &&
				(!(st->buttons & MOUSE_LEFTBUTTON) || !(st->buttons & MOUSE_RIGHTBUTTON)))){	//X released
			button_action |= (1 << 1);
		}
		else{
			if((previous_buttons[__dev->port] & MOUSE_LEFTBUTTON) && !(st->buttons & MOUSE_LEFTBUTTON)){	//A released
				button_action |= (1 << 0);
			}
			if(!(previous_buttons[__dev->port] & MOUSE_RIGHTBUTTON) && (st->buttons & MOUSE_RIGHTBUTTON)){	//B pressed
				button_action |= (1 << 2);
			}
		}

		if(button_press_logic(&MS_grid, &MS_options, button_action, __dev->port, cursor_position, previous_buttons, st->buttons)){break;}
		button_press_logic_buttons(&MS_grid, &MS_options, &os, &face, cursor_position + (2 * __dev->port), previous_buttons[__dev->port], st->buttons);
		language_swap(&MS_grid, &MS_options, &os, cursor_position + (2 * __dev->port), button_action & (1 << 0));
		keyboard_logic(&MS_keyboard, &MS_options, cursor_position + (2 * __dev->port), button_action & (1 << 0), MS_grid.time_sec);

		int ele_x = -1;
		int ele_y = -1;
		uint8_t in_grid = 0;

		if(MS_options.focus == 0){
			cursor_on_grid(&MS_grid, &in_grid, &ele_x, &ele_y, button_action, __dev->port, cursor_position);
			if(in_grid){grid_ABX_logic(&MS_grid, &MS_options, ele_x, ele_y, button_action, &start_time, &start_ms_time);}
		}

		//Movement code
		cursor_position[(2 * __dev->port) + 1] += 0.8 * st->dy;
		if(cursor_position[(2 * __dev->port) + 1] > 480){
			cursor_position[(2 * __dev->port) + 1] = 480;
		}
		else if(cursor_position[(2 * __dev->port) + 1] < 0){
			cursor_position[(2 * __dev->port) + 1] = 0;
		}
		cursor_position[2 * __dev->port] += 0.8 * st->dx;
		if(cursor_position[2 * __dev->port] > 640){
			cursor_position[2 * __dev->port] = 640;
		}
		else if(cursor_position[2 * __dev->port] < 0){
			cursor_position[2 * __dev->port] = 0;
		}

		//Might need to change the X/Side button code
		face_logic(&MS_grid, &face, cursor_position + (2 * __dev->port), !!(st->buttons & (MOUSE_LEFTBUTTON)), !!(st->buttons & (MOUSE_SIDEBUTTON)));

		if(MS_options.focus == 0){
			grid_indent_logic(&MS_grid, ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (MOUSE_LEFTBUTTON)),
				!!(st->buttons & (MOUSE_SIDEBUTTON)), !!(st->buttons & (MOUSE_RIGHTBUTTON)), 1);
		}

		previous_buttons[__dev->port] = st->buttons;	//Store the previous button presses

		MAPLE_FOREACH_END()

		//X101 0000 (Where every controller is doing an impure press)
		if((start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			face.frame_coord_keys[0] = 4;
		}

		//XX01 XXXX (Where every controller is doing an impure press)
		if(!(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && (start_primed % (1 << 4))){
			start_primed |= (1 << 5);	//Now an invalid press
			face.frame_coord_keys[0] = 0;
		}

		//X011 0000 (Start is released, but it was invalid)
		if(!(start_primed & (1 << 6)) && (start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			start_primed = 0;
		}

		//X001 0000 (Start is released and it was valid)
		if(!(start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			clear_grid(&MS_grid);
			start_primed = 0;
			face.frame_coord_keys[0] = 0;
		}

		start_primed = start_primed & ((1 << 5) + (1 << 4));	//Clears all bits except active and invalidness

		if(MS_grid.non_mines_left == 0 && MS_grid.game_live && !MS_grid.over_mode){
			MS_grid.game_live = 0;
			MS_grid.over_mode = 1;
		}

		//This code triggers the turn when you win/lose the game (Doesn't trigger while dead/won after that)
		if(!MS_grid.game_live && MS_grid.over_mode == 1){	//This triggers at the start of a new game too...
			if(MS_grid.non_mines_left == 0){	//Only mines left, you win
				MS_grid.over_mode = 3;
				if(MS_options.sound_enabled){ //play "won" sound
					snd_sfx_play(Sound_Win, 192, 128);	//Right now this can play when dead
				}
				face.frame_coord_keys[0] = 3;
			}
			else{	//Tiles are left, you must have been blown up
				MS_grid.over_mode = 2;
				if(MS_options.sound_enabled){	//play bomb/death sound
					if(MS_options.language){	//Italian
						snd_sfx_play(Sound_Death_Italian, 192, 128);
					}
					else{
						snd_sfx_play(Sound_Death, 192, 128);
					}
				}
				face.frame_coord_keys[0] = 2;
			}
		}

		//When releasing a start press (Or it becomes impure), we want to revert back to the gameover faced instead of the normal face
		if(MS_grid.over_mode && face.frame_coord_keys[0] != 4){
			if(MS_grid.non_mines_left == 0){
				face.frame_coord_keys[0] = 3;
			}
			else{
				face.frame_coord_keys[0] = 2;
			}
		}

		//No mines left, game is over and we weren't doing a custom grid (Also not revealed yet)
		if(MS_grid.non_mines_left == 0 && MS_grid.over_mode && MS_grid.difficulty && !MS_grid.revealed){
			if(player_active == (1 << 0) || player_active == (1 << 1) || player_active == (1 << 2) || player_active == (1 << 3)){	//Only one player
				MS_keyboard.record_index = MS_grid.difficulty - 1;
			}
			else{	//Multiplayer
				MS_keyboard.record_index = MS_grid.difficulty + 2;
			}
			//If this is a new record and we have a savefile
			if(MS_grid.time_sec < MS_options.savefile.times[MS_keyboard.record_index] && MS_options.savefile_details.valid_saves){
				strcpy(MS_keyboard.type_buffer, MS_options.savefile.record_names[MS_keyboard.record_index]);	//The keyboard contains the previous master's name
				MS_keyboard.chars_typed = strlen(MS_keyboard.type_buffer);
				MS_options.focus = 1;
			}
		}

		//Right now this is always triggered when a game ends and thanks to "revealed" it only triggers once
		if(!MS_grid.revealed && !MS_grid.game_live && MS_grid.over_mode){
			reveal_map(&MS_grid);
		}

		//Re-create the indent tile list based on inputs
		uint8_t indent_count = 0;
		for(iter = 0; iter < 4; iter++){
			if(press_data[3 * iter]){	//A or X pressed, centre logic first
				uint8_t valids;
				uint16_t ele = press_data[(3 * iter) + 1] + (MS_grid.x * press_data[(3 * iter) + 2]);
				if(press_data[3 * iter] == 2){	//For the X press
					valids = neighbouring_tiles(&MS_grid, press_data[(3 * iter) + 1], press_data[(3 * iter) + 2]);	//Get the tiles that are in bounds
					for(jiter = 0; jiter < 8; jiter++){
						int8_t tile_offset_x;
						int8_t tile_offset_y;
						if(!neightbours_to_adjustments(&tile_offset_x, &tile_offset_y, valids, jiter)){
							uint16_t ele_offset = ele + tile_offset_x + (tile_offset_y * MS_grid.x);
							if(!(MS_grid.logic_grid[ele_offset] & ((1 << 7) + (1 << 6)))){
								indented_tiles.frame_coord_keys[indent_count] = MS_grid.logic_grid[ele_offset] & (1 << 5) ? 0: 1;
								indented_tiles.positions[(2 * indent_count)] = MS_grid.start_x + (16 * (press_data[(3 * iter) + 1] + tile_offset_x));
								indented_tiles.positions[(2 * indent_count) + 1] = MS_grid.start_y + (16 * (press_data[(3 * iter) + 2] + tile_offset_y));
								indent_count++;
							}
						}
					}
				}
				if(!(MS_grid.logic_grid[ele] & ((1 << 7) + (1 << 6)))){	//Is unreleaved and not flagged
					indented_tiles.frame_coord_keys[indent_count] = MS_grid.logic_grid[ele] & (1 << 5) ? 0: 1;
					indented_tiles.positions[(2 * indent_count)] = MS_grid.start_x + (16 * press_data[(3 * iter) + 1]);
					indented_tiles.positions[(2 * indent_count) + 1] = MS_grid.start_y + (16 * press_data[(3 * iter) + 2]);
					indent_count++;
				}
			}
		}
		indented_tiles.num_sprites = indent_count;	//So we don't display "Garbage info"

		timer_ms_gettime(&current_time, &current_ms_time);
		if(MS_grid.game_live && MS_grid.time_sec < 999){	//Prevent timer overflows
			int temp_sec = current_time - start_time + (current_ms_time > start_ms_time); //MS is there to account for the "1st second" inaccuracy
			//(How does this do the "start at 1 sec" thing? I forgot)
			if(MS_options.focus < 1 && MS_options.sound_enabled && temp_sec > MS_grid.time_sec){	//Play the "tick" sound effect (But only when time_sec changes and we're on the game tab)
				snd_sfx_play(Sound_Tick, 192, 128);
			}
			MS_grid.time_sec = temp_sec;
		}

		//Update the two digit displays (Flags and timer)
		digit_set(&digit_display, MS_grid.num_flags, 0);
		digit_set(&digit_display, MS_grid.time_sec, 1);

		time(&os_clock);	//I think this is how I populate it with the current time
		time_struct = localtime(&os_clock);

		#if CRAYON_DEBUG == 1
			pvr_get_stats(&pvr_stats);	//Get the framerate
			last_30_FPS[FPS_array_iter] = pvr_stats.frame_rate;
			FPS_array_iter++;
			FPS_array_iter %= 30;
			fps_ave = 0;
			fps_min = last_30_FPS[0];
			fps_max = 0;
			for(iter = 0; iter < 30; iter++){
				fps_ave += last_30_FPS[iter];
				if(last_30_FPS[iter] > fps_max){
					fps_max = last_30_FPS[iter];
				}
				if(last_30_FPS[iter] < fps_min){
					fps_min = last_30_FPS[iter];
				}
			}
			fps_ave /= 30;
		#endif

		pvr_wait_ready();
		pvr_scene_begin();

		//Setup the main palette
		graphics_setup_palette(&Board_P);		//0
		graphics_setup_palette(&Icons_P);		//1
		graphics_setup_palette(&Controllers_P);	//2
		if(!MS_options.operating_system){	//Since it uses palettes and XP doesn't, we do this
			graphics_setup_palette(&Windows_P);	//1. But since Windows uses 8bpp, this doesn't overlap with "Icons" and "Controllers"
		}
		graphics_setup_palette(&cursor_red);	//8
		graphics_setup_palette(&cursor_yellow);	//9
		graphics_setup_palette(&cursor_green);	//10
		graphics_setup_palette(&cursor_blue);	//11

		graphics_setup_palette(&White_Tahoma_P);//61
		graphics_setup_palette(&Tahoma_P);		//62
		// graphics_setup_palette(&BIOS_P);		//63

		//Transfer more stuff from this list into either the PT or OP lists
		pvr_list_begin(PVR_LIST_TR_POLY);

			//Draw the indented tiles ontop of the grid and the cursors themselves
			uint8_t menus_selected = 0;
			for(iter = 0; iter < 4; iter++){
				if(player_active & (1 << iter)){
					//Passing coords as ints because otherwise we can get case where each pixel contains more than 1 texel
					//ADD A DRAW FOR THE SHADOW
					uint8_t cursor_palette = 8 + iter;	//I should be basing this off the palettes, but I'd need to change to much
					if(player_active == (1 << 0) || player_active == (1 << 1) || player_active == (1 << 2) || player_active == (1 << 3)){
						cursor_palette = 1;
					}
					graphics_draw_sprite(&Icons, &Icons.spritesheet_animation_array[0], (int) cursor_position[2 * iter],
						(int) cursor_position[(2 * iter) + 1], 51, 1, 1, 0, 0, cursor_palette);

					//Add code for checking if cursor is hovering over a button
					menus_selected |= button_hover(cursor_position[(2 * iter)], cursor_position[(2 * iter) + 1], &os, &MS_options);
				}
			}

			//Drawing the highlight boxes (Maybe make a change so they're brighter in XP?)
			for(iter = 0; iter < 5; iter++){
				if(menus_selected & (1 << iter)){
					if(MS_options.operating_system){	//Highlight is more opaque so its more noticable
						graphics_draw_untextured_poly(MS_options.tabs_x[iter] - 4, os.tabs_y - 3, 19,
							(MS_options.tabs_width[iter] + 8), 16, 0x80FFFFFF, 0);	//Font is layer 20
					}
					else{
						graphics_draw_untextured_poly(MS_options.tabs_x[iter] - 4, os.tabs_y - 3, 19,
							(MS_options.tabs_width[iter] + 8), 16, 0x40FFFFFF, 0);	//Font is layer 20
					}
				}
			}

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);

			//DEBUG
			char snum[32];
			sprintf(snum, "Valid: VMUs %d\n", MS_options.savefile_details.valid_vmus);
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 5, 100, 50, 1, 1, 62, snum);
			sprintf(snum, "Screens: %d\n", MS_options.savefile_details.valid_vmu_screens);
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 5, 100 + 12, 50, 1, 1, 62, snum);
			sprintf(snum, "New saves: %d\n", MS_options.savefile_details.valid_saves);
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 5, 100 + 24, 50, 1, 1, 62, snum);
			sprintf(snum, "Old saves: %d\n", old_saves);
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 5, 100 + 36, 50, 1, 1, 62, snum);
			sprintf(snum, "P: %d, S: %d\n", MS_options.savefile_details.port, MS_options.savefile_details.slot);
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 5, 100 + 48, 50, 1, 1, 62, snum);
			sprintf(snum, "Htz: %d\n", MS_options.htz);
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 5, 100 + 60, 50, 1, 1, 62, snum);

			//Draw windows assets
			for(iter = 0; iter < os.num_assets; iter++){
				if(!strcmp(os.assets[iter]->animation->animation_name, "aboutLogo") && MS_options.focus != 5){	//We don't want to draw that unless we're on the about page
					continue;
				}
				crayon_graphics_draw_sprites(os.assets[iter], PVR_LIST_PT_POLY);
			}

			//Draw the sd icon
			if(MS_options.sd_present){
				crayon_graphics_draw_sprites(&os.sd, PVR_LIST_PT_POLY);
			}

			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.tabs_x[0], os.tabs_y, 20, 1, 1, 62, "Game\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.tabs_x[1], os.tabs_y, 20, 1, 1, 62, "Options\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.tabs_x[2], os.tabs_y, 20, 1, 1, 62, "Best Times\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.tabs_x[3], os.tabs_y, 20, 1, 1, 62, "Controls\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.tabs_x[4], os.tabs_y, 20, 1, 1, 62, "About\0");

			//Updating the time in the bottom right
			if(hour != time_struct->tm_hour || minute != time_struct->tm_min){
				hour = time_struct->tm_hour;
				minute = time_struct->tm_min;
				if(time_struct->tm_hour < 13){
					if(time_struct->tm_hour == 0){	//When its midnight, display 12:XX AM instead of 00:XX AM
						time_struct->tm_hour = 12;
					}
					sprintf(time_buffer, "%02d:%02d AM", time_struct->tm_hour, time_struct->tm_min);
				}
				else{
					sprintf(time_buffer, "%02d:%02d PM", time_struct->tm_hour - 12, time_struct->tm_min);
				}
				//The X starting pos varies based on the hour for XP and hour, AM/PM for 2000
				if(MS_options.operating_system){
					if(time_struct->tm_hour % 12 < 10){
						os.clock_x = 583;
					}
					else{
						os.clock_x = 586;
					}
				}
				else{
					uint16_t clock_pos;
					if(time_struct->tm_hour % 12 < 10){
						clock_pos = 581;
					}
					else{
						clock_pos = 585;
					}
					if(time_struct->tm_hour < 13){
						clock_pos--;
					}
					os.clock_x = clock_pos;
				}
			}

			//XP uses another palette (White text version)
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, os.clock_x, os.clock_y, 20, 1, 1, os.clock_palette->palette_id, time_buffer);

			//DEBUG, REMOVE LATER
			#if CRAYON_DEBUG == 1
				char fps_buffer[100];
				sprintf(fps_buffer, "FPS ave: %d, min: %d, max: %d", fps_ave, fps_min, fps_max);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_TR_POLY, 235, 435, 20, 1, 1, 62, fps_buffer);
			#endif

			if(MS_options.focus == 1){

				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 89, keyboard_start_y - 80, 31, 1, 1, Tahoma_P.palette_id, "You have the fastest time for\0");
				if(MS_keyboard.record_index == 0){
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 94, keyboard_start_y - 68, 31, 1, 1, Tahoma_P.palette_id, "Single Player Beginner level\0");
				}
				else if(MS_keyboard.record_index == 1){
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 84, keyboard_start_y - 68, 31, 1, 1, Tahoma_P.palette_id, "Single Player Intermediate level\0");
				}
				else if(MS_keyboard.record_index == 2){
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 99, keyboard_start_y - 68, 31, 1, 1, Tahoma_P.palette_id, "Single Player Expert level\0");
				}
				else if(MS_keyboard.record_index == 3){
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 99, keyboard_start_y - 68, 31, 1, 1, Tahoma_P.palette_id, "Multiplayer Beginner level\0");
				}
				else if(MS_keyboard.record_index == 4){
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 89, keyboard_start_y - 68, 31, 1, 1, Tahoma_P.palette_id, "Multiplayer Intermediate level\0");
				}
				else if(MS_keyboard.record_index == 5){
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 104, keyboard_start_y - 68, 31, 1, 1, Tahoma_P.palette_id, "Multiplayer Expert level\0");
				}
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, keyboard_start_x + 101, keyboard_start_y - 56, 31, 1, 1, Tahoma_P.palette_id, "Please enter your name.\0");

				char key_buffer[3];
				for(iter = 0; iter < MS_keyboard.mini_buttons.num_sprites; iter++){
					if(MS_keyboard.caps){
						key_buffer[0] = MS_keyboard.upper_keys[iter];
					}
					else{
						key_buffer[0] = MS_keyboard.lower_keys[iter];
					}

					if(MS_keyboard.upper_keys[iter] == '<'){
						key_buffer[1] = '-';
						key_buffer[2] = '\0';
					}
					else{
						key_buffer[1] = '\0';
					}

					if(MS_keyboard.upper_keys[iter] == '^'){
						graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_keyboard.mini_buttons.positions[(2 * iter)] + 6, MS_keyboard.mini_buttons.positions[(2 * iter) + 1] + 5, 31, 1, 1, Tahoma_P.palette_id, "_");
					}
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_keyboard.mini_buttons.positions[(2 * iter)] + 6, MS_keyboard.mini_buttons.positions[(2 * iter) + 1] + 5, 31, 1, 1, Tahoma_P.palette_id, key_buffer);
				}

				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_keyboard.key_big_buttons.positions[0] + 24, MS_keyboard.key_big_buttons.positions[1] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Enter\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_keyboard.key_big_buttons.positions[2] + 24, MS_keyboard.key_big_buttons.positions[3] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Space\0");

				//The text the user types
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_keyboard.type_box_x + 6, MS_keyboard.type_box_y + 5, 31, 1, 1, Tahoma_P.palette_id, MS_keyboard.type_buffer);
			}
			else if(MS_options.focus == 2){
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.checkers.positions[0] - 56, MS_options.checkers.positions[1] + 1, 31, 1, 1, Tahoma_P.palette_id, "Sound\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.checkers.positions[2] - 56, MS_options.checkers.positions[3] + 1, 31, 1, 1, Tahoma_P.palette_id, "Marks (?)\0");

				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.buttons.positions[0] + 15, MS_options.buttons.positions[1] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Beginner\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.buttons.positions[2] + 7, MS_options.buttons.positions[3] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Intermediate\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.buttons.positions[4] + 22, MS_options.buttons.positions[5] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Expert\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.buttons.positions[6] + 7, MS_options.buttons.positions[7] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Save to VMU\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.buttons.positions[8] + 9, MS_options.buttons.positions[9] + 5 + MS_options.operating_system, 31, 1, 1, Tahoma_P.palette_id, "Update Grid\0");

				//Draw the numbers for x, y and num_mines displays
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.number_changers.positions[0] - 70, MS_options.number_changers.positions[1] + 5 - (2 * MS_options.operating_system), 31, 1, 1, Tahoma_P.palette_id, "Height:\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.number_changers.positions[0] - 21, MS_options.number_changers.positions[1] + 5 - (2 * MS_options.operating_system), 31, 1, 1, Tahoma_P.palette_id, MS_options.y_buffer);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.number_changers.positions[2] - 70, MS_options.number_changers.positions[3] + 5 - (2 * MS_options.operating_system), 31, 1, 1, Tahoma_P.palette_id, "Width:\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.number_changers.positions[2] - 21, MS_options.number_changers.positions[3] + 5 - (2 * MS_options.operating_system), 31, 1, 1, Tahoma_P.palette_id, MS_options.x_buffer);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.number_changers.positions[4] - 70, MS_options.number_changers.positions[5] + 5 - (2 * MS_options.operating_system), 31, 1, 1, Tahoma_P.palette_id, "Mines:\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, MS_options.number_changers.positions[4] - 21, MS_options.number_changers.positions[5] + 5 - (2 * MS_options.operating_system), 31, 1, 1, Tahoma_P.palette_id, MS_options.m_buffer);
			}
			else if(MS_options.focus == 3){

				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 20, 144, 31, 1, 1, Tahoma_P.palette_id, "Beginner\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 20, 144 + 24, 31, 1, 1, Tahoma_P.palette_id, "Intermediate\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 20, 144 + 48, 31, 1, 1, Tahoma_P.palette_id, "Expert\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 20 + 125, 144 - 24, 31, 1, 1, Tahoma_P.palette_id, "Single Player\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 20 + 345, 144 - 24, 31, 1, 1, Tahoma_P.palette_id, "Multiplayer\0");

				//Display the high scores
				char record_buffer[21];
				uint16_t x_offset = 125;
				for(iter = 0; iter < 6; iter++){
					if(iter == 3){
						x_offset = 345;
					}
					sprintf(record_buffer, "%s: %d", MS_options.savefile.record_names[iter], MS_options.savefile.times[iter]);
					graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 20 + x_offset, 144 + ((iter % 3) * 24), 31, 1, 1, Tahoma_P.palette_id, record_buffer);
				}
			}
			else if(MS_options.focus == 4){
				// The controllers and their swirls
				crayon_graphics_draw_sprites(&devices, PVR_LIST_PT_POLY);
				crayon_graphics_draw_sprites(&cont_swirl, PVR_LIST_PT_POLY);

				//Draw legend dot stuff
				crayon_graphics_draw_sprites(&legend_dot, PVR_LIST_PT_POLY);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[0] + 5, legend_dot.positions[1] + 3, 34, 1, 1, 61, "6\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[2] + 5, legend_dot.positions[3] + 3, 34, 1, 1, 61, "7\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[4] + 5, legend_dot.positions[5] + 3, 34, 1, 1, 61, "8\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[6] + 5, legend_dot.positions[7] + 3, 34, 1, 1, 61, "4\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[8] + 5, legend_dot.positions[9] + 3, 34, 1, 1, 61, "2\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[10] + 5, legend_dot.positions[11] + 3, 34, 1, 1, 61, "1\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[12] + 5, legend_dot.positions[13] + 3, 34, 1, 1, 61, "3\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[14] + 5, legend_dot.positions[15] + 3, 34, 1, 1, 61, "5\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[16] + 5, legend_dot.positions[17] + 3, 34, 1, 1, 61, "9\0");

				//Legend part
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[18] + 5, legend_dot.positions[19] + 3, 34, 1, 1, 61, "1\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[20] - 6, legend_dot.positions[21] + 3, 34, 1, 1, 62, ",\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[20] + 5, legend_dot.positions[21] + 3, 34, 1, 1, 61, "6\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[20] + 18, legend_dot.positions[21] + 3, 34, 1, 1, 62, "Reveal a tile\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[22] + 5, legend_dot.positions[23] + 3, 34, 1, 1, 61, "2\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[24] - 6, legend_dot.positions[25] + 3, 34, 1, 1, 62, ",\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[24] + 5, legend_dot.positions[25] + 3, 34, 1, 1, 61, "7\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[24] + 18, legend_dot.positions[25] + 3, 34, 1, 1, 62, "Mark a tile\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[26] + 5, legend_dot.positions[27] + 3, 34, 1, 1, 61, "3\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[28] - 6, legend_dot.positions[29] + 3, 34, 1, 1, 62, ",\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[28] + 5, legend_dot.positions[29] + 3, 34, 1, 1, 61, "8\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[28] + 17, legend_dot.positions[29] + 3, 34, 1, 1, 62, ", (       +       )\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[30] + 5, legend_dot.positions[31] + 3, 34, 1, 1, 61, "6\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[32] + 5, legend_dot.positions[33] + 3, 34, 1, 1, 61, "7\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[32] + 24, legend_dot.positions[33] + 3, 34, 1, 1, 62, "Reveal neighbouring tiles\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[34] + 5, legend_dot.positions[35] + 3, 34, 1, 1, 61, "4\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[34] + 18, legend_dot.positions[35] + 3, 34, 1, 1, 62, "Swap between thumbstick and D-pad controls\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[36] + 5, legend_dot.positions[37] + 3, 34, 1, 1, 61, "5\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[38] - 6, legend_dot.positions[39] + 3, 34, 1, 1, 62, ",\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[38] + 5, legend_dot.positions[39] + 3, 34, 1, 1, 61, "9\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, legend_dot.positions[38] + 18, legend_dot.positions[39] + 3, 34, 1, 1, 62, "Shortcut to reset the grid\0");

			}
			else if(MS_options.focus == 5){

				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 120, 25, 1, 1, 62, "Microsoft (R) Minesweeper\0");	//Get the proper @R and @c symbols for XP, or not...
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 125 + 12, 25, 1, 1, 62, "Version 1.2.0 (Build 3011: Service Pack 5)\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 130 + 24, 25, 1, 1, 62, "Copyright (C) 1981-2018 Microsoft Corp.\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 135 + 36, 25, 1, 1, 62, "by Robert Donner and Curt Johnson\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 155 + 48, 25, 1, 1, 62, "This Minesweeper re-creation was made by Protofall using KallistiOS,\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 160 + 60, 25, 1, 1, 62, "used texconv textures and powered by my Crayon library. I don't own\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 165 + 72, 25, 1, 1, 62, "the rights to Minesweeper nor do I claim to so don't sue me, k?\0");
				// graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 185 + 84, 25, 1, 1, 62, "Katana logo made by Airofoil\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 185 + 84, 25, 1, 1, 62, "Dreamcast controller and mouse pixel art was made by JamoHTP\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 205 + 96, 25, 1, 1, 62, "A huge thanks to the Dreamcast developers for helping me get started\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 210 + 108, 25, 1, 1, 62, "and answering any questions I had and a huge thanks to the Dreamcast\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 215 + 120, 25, 1, 1, 62, "media for bringing the homebrew scene to my attention.\0");
			}

		pvr_list_finish();

		//None of these need to be transparent, by using the opaque list we are making the program more efficient
		pvr_list_begin(PVR_LIST_OP_POLY);

			//Draw the grid and the indentations
			if(MS_options.focus <= 1){
				crayon_graphics_draw_sprites(&MS_grid.draw_grid, PVR_LIST_OP_POLY);
				crayon_graphics_draw_sprites(&indented_tiles, PVR_LIST_OP_POLY);
				if(MS_options.focus == 1){
					crayon_graphics_draw_sprites(&MS_keyboard.mini_buttons, PVR_LIST_OP_POLY);	//Draw the high score keyboard
					crayon_graphics_draw_sprites(&MS_keyboard.key_big_buttons, PVR_LIST_OP_POLY);	//Draw the high score keyboard
					if(MS_options.operating_system){
						custom_poly_XP_boarder(keyboard_start_x - 8, keyboard_start_y - 88, 19, 318 + 16, 105 + 96);
						graphics_draw_untextured_poly(MS_keyboard.type_box_x, MS_keyboard.type_box_y, 25, 120 + 56, 20, 0xFF7F9DB9, 1);	//Blue outline
						graphics_draw_untextured_poly(MS_keyboard.type_box_x + 1, MS_keyboard.type_box_y + 1, 26, 118 + 56, 18, 0xFFFFFFFF, 1);	//White box

					}
					else{
						custom_poly_2000_boarder(keyboard_start_x - 8, keyboard_start_y - 88, 19, 318 + 16, 105 + 96);
						custom_poly_2000_text_boarder(MS_keyboard.type_box_x, MS_keyboard.type_box_y, 24, 120 + 56, 20);
					}
				}
			}
			else if(MS_options.focus == 2){
				crayon_graphics_draw_sprites(&MS_options.buttons, PVR_LIST_OP_POLY);
				crayon_graphics_draw_sprites(&MS_options.checkers, PVR_LIST_OP_POLY);
				crayon_graphics_draw_sprites(&MS_options.number_changers, PVR_LIST_OP_POLY);
			}
			else if(MS_options.focus == 4){
				custom_draw_line(261, 189, 318, 189, 33);	//Controller 4
				custom_draw_line(283, 210, 318, 210, 33);	//Controller 2
				custom_draw_line(261, 230, 318, 230, 33);	//Controller 1
				custom_draw_line(228, 222, 234, 250, 33);	//Controller 3 diag
				custom_draw_line(234, 250, 318, 250, 33);	//Controller 3
				custom_draw_line(168, 302, 168, 316, 33);	//Controller 5

				custom_draw_line(387, 156, 394, 168, 33);	//Mouse 1 diag
				custom_draw_line(394, 168, 464, 168, 33);	//Mouse 1
				custom_draw_line(595, 156, 588, 168, 33);	//Mouse 2 diag
				custom_draw_line(518, 168, 588, 168, 33);	//Mouse 2
				custom_draw_line(396, 189, 419, 189, 33);	//Mouse 3

				custom_draw_line(334, 77, 342, 77, 33);		//Face 5
			}

			//Draw the flag count and timer digit displays
			crayon_graphics_draw_sprites(&digit_display, PVR_LIST_OP_POLY);

			//Draw the region icon
			crayon_graphics_draw_sprites(&os.region, PVR_LIST_OP_POLY);

			//Draw the reset Face
			crayon_graphics_draw_sprites(&face, PVR_LIST_OP_POLY);

			//Draw the grid's boarder
			if(MS_options.focus <= 1){
				custom_poly_boarder(3, MS_grid.start_x, MS_grid.start_y, 16, MS_grid.x * 16, MS_grid.y * 16, 4286611584u, 4294967295u);
			}
			else{
				if(MS_options.focus >= 2){	//The yellowy background + dividing line
					graphics_draw_untextured_poly(6, 98, 10, 631, 1, 0xFFA0A0A0, 1);
					if(MS_options.operating_system){
						graphics_draw_untextured_poly(6, 99, 10, 631, 350, 0xFFECE9D8, 1);
					}
					else{
						graphics_draw_untextured_poly(6, 99, 10, 631, 350, 0xFFD4D0C8, 1);
					}
				}
				if(MS_options.focus == 2){
					graphics_draw_untextured_array(&Option_polys);	//The boxes the numbers go in
				}
			}

			//Draw the boarders for digit display (I think I should move these calls into digit display itself)
			custom_poly_boarder(1, 20, 65, 16, Board.spritesheet_animation_array[1].animation_frame_width * 3, Board.spritesheet_animation_array[1].animation_frame_height,
				4286611584u, 4294967295u);
			custom_poly_boarder(1, 581, 65, 16, Board.spritesheet_animation_array[1].animation_frame_width * 3, Board.spritesheet_animation_array[1].animation_frame_height,
				4286611584u, 4294967295u);

			//Draw the big indent boarder that encapsulates digit displays and face
			custom_poly_boarder(2, 14, 59, 16, 615, 33, 4286611584u, 4294967295u);

			//Draw the MS background stuff
			graphics_draw_untextured_array(&Bg_polys);

			if(!MS_options.operating_system){
				custom_poly_2000_topbar(3, 3, 15, 634, 18);	//Colour bar for Windows 2000
				custom_poly_2000_boarder(0, 0, 1, 640, 452);	//The Windows 2000 window boarder
			}

		pvr_list_finish();

		pvr_scene_finish();

		//Update the face for the new frame (Unless the game is over)
		if(face.frame_coord_keys[0] != 2 && face.frame_coord_keys[0] != 3){
			face.frame_coord_keys[0] = 0;
		}
	}

	//Confirm everything was unloaded successfully (Should equal zero) This code is never triggered under normal circumstances
	//I'm probs forgetting a few things
	int retVal = 0;
	retVal += crayon_memory_free_sprite_array(&MS_grid.draw_grid, 0, 0);
	retVal += crayon_memory_free_sprite_array(&indented_tiles, 0, 0);
	retVal += crayon_memory_free_sprite_array(&digit_display, 0, 0);
	retVal += crayon_memory_free_sprite_array(&face, 0, 0);
	retVal += crayon_memory_free_sprite_array(&devices, 0, 0);
	retVal += crayon_memory_free_sprite_array(&legend_dot, 0, 0);
	retVal += crayon_memory_free_sprite_array(&cont_swirl, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_keyboard.mini_buttons, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_keyboard.key_big_buttons, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_options.checkers, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_options.buttons, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_options.number_changers, 0, 0);
	retVal += crayon_memory_free_spritesheet(&Board);
	retVal += crayon_memory_free_spritesheet(&Icons);
	retVal += crayon_memory_free_spritesheet(&Windows);
	// retVal += crayon_memory_free_mono_font_sheet(&BIOS_font);
	retVal += crayon_memory_free_prop_font_sheet(&Tahoma_font);
	retVal += crayon_memory_free_palette(&White_Tahoma_P);
	retVal += crayon_memory_free_palette(&cursor_red);
	retVal += crayon_memory_free_palette(&cursor_yellow);
	retVal += crayon_memory_free_palette(&cursor_green);
	retVal += crayon_memory_free_palette(&cursor_blue);
	snd_sfx_unload(Sound_Tick);
	snd_sfx_unload(Sound_Death);
	snd_sfx_unload(Sound_Death_Italian);
	snd_sfx_unload(Sound_Win);
	setup_free_OS_struct(&os);

	crayon_savefile_free_icon(&MS_options.savefile_details);

	error_freeze("Free-ing result %d!\n", retVal);

	return 0;
}

//Add something to be displayed on the VMU screen. But what? Just a static mine/blown up mine
//When choosing an OS, make it boot up with a Dreamcast/Katana legacy BIOS
