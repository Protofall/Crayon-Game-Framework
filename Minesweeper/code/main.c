//Crayon libraries
#include "../../Crayon/code/dreamcast/memory.h"

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

//For the sound effects
#include <dc/sound/stream.h>
#include <dc/sound/sfxmgr.h>

//To get region info
#include <dc/flashrom.h>

#define CRAYON_SD_MODE 0
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

//Must be called within a pvr_list_begin(), used for displaying the counter for flags and timer
void digit_display(crayon_spritesheet_t * ss, crayon_animation_t * anim, int num, uint16_t x, uint16_t y, uint8_t z){
	if(num < -99){
		num = -99;
	}
	else if(num > 999){
		num = 999;
	}

	char disp_num[3];
	sprintf(disp_num, "%03d", num);

	int i;
	uint16_t frame_x;
	uint16_t frame_y;
	for(i = 0; i < 3; i++){
		if(disp_num[i] == '-'){
			graphics_frame_coordinates(anim, &frame_x, &frame_y, 10);
		}
		else{
			graphics_frame_coordinates(anim, &frame_x, &frame_y, (int)disp_num[i] - 48);
		}
		graphics_draw_sprite(ss, anim, x + (i * 13), y, z, 1, 1, frame_x, frame_y, 0);
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

void face_logic(MinesweeperGrid_t * grid, uint8_t *face_frame_id, int id, float *cursor_position, uint8_t A_held, uint8_t X_held){
	if(A_held || X_held){
		if(!grid->over_mode && !*face_frame_id){	//Basically face_frame_id != 2, 3 or 4
			*face_frame_id = 1;	//Apply suprised face
		}
		if(A_held && (cursor_position[2 * id] <= 307 + 26) && (cursor_position[(2 * id) + 1] <= 64 + 26)
			&& cursor_position[2 * id] >= 307 && cursor_position[(2 * id) + 1] >= 64){	//If hovering over face and press on it
			*face_frame_id = 4;
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

//Handles focus related things
	//Could probably clean up some of this stuff with the draw arrays within the OS struct
uint8_t button_press_logic_buttons(MinesweeperGrid_t * grid, MinesweeperOptions_t * options, MinesweeperOS_t *os, int id, float *cursor_position, uint32_t previous_buttons, uint32_t buttons){
	//CLEAN UP THE MAGIC NUMBERS
	if(options->focus != 1){	//When we get a new score, we don't want to change focus easily
		//Top 4 options
		if((buttons & CONT_A) && !(previous_buttons & CONT_A)){
			if((cursor_position[2 * id] <= 9 + 27 + 3) && (cursor_position[(2 * id) + 1] <= os->tabs_y + 13)
					&& cursor_position[2 * id] >= 9 - 4 && cursor_position[(2 * id) + 1] >= os->tabs_y - 3){
				options->focus = 0;
			}
			else if((cursor_position[2 * id] <= 48 + 37 + 3) && (cursor_position[(2 * id) + 1] <= os->tabs_y + 13)
					&& cursor_position[2 * id] >= 48 - 4 && cursor_position[(2 * id) + 1] >= os->tabs_y - 3){
				options->focus = 2;
			}
			else if((cursor_position[2 * id] <= 97 + 40 + 3) && (cursor_position[(2 * id) + 1] <= os->tabs_y + 13)
					&& cursor_position[2 * id] >= 97 - 4 && cursor_position[(2 * id) + 1] >= os->tabs_y - 3){
				options->focus = 3;
			}
			else if((cursor_position[2 * id] <= 149 + 29 + 3) && (cursor_position[(2 * id) + 1] <= os->tabs_y + 13)
					&& cursor_position[2 * id] >= 149 - 4 && cursor_position[(2 * id) + 1] >= os->tabs_y - 3){
				options->focus = 4;
			}
		}
	}

	if(options->focus == 2){
		if((buttons & CONT_A) && !(previous_buttons & CONT_A)){	//When we get a new score, we don't want to change focus easily
			if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 149)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 140){	//Incrementer Y
				if(options->disp_y < 21){
					options->disp_y++;
					sprintf(options->y_buffer, "%d", options->disp_y);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 159)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 150){	//Decrementer Y
				if(options->disp_y > 9){
					options->disp_y--;
					sprintf(options->y_buffer, "%d", options->disp_y);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 189)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 180){	//Incrementer X
				if(options->disp_x < 38){
					options->disp_x++;
					sprintf(options->x_buffer, "%d", options->disp_x);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 199)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 190){	//Decrementer X
				if(options->disp_x > 9){
					options->disp_x--;
					sprintf(options->x_buffer, "%d", options->disp_x);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 229)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 220){	//Incrementer Num Mines
				if(options->disp_mines < (options->disp_x - 1) * (options->disp_y - 1)){
					options->disp_mines++;
				}
				else{
					options->disp_mines = (options->disp_x - 1) * (options->disp_y - 1);
				}
				sprintf(options->m_buffer, "%d", options->disp_mines);
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 239)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 230){	//Decrementer Num Mines
				if(options->disp_mines > 10){
					options->disp_mines--;
					sprintf(options->m_buffer, "%d", options->disp_mines);
				}
			}
			else if((cursor_position[2 * id] <= 189 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 189 && cursor_position[(2 * id) + 1] >= 306){	//Beginner
				options->disp_x = 9;
				options->disp_y = 9;
				options->disp_mines = 10;
				sprintf(options->x_buffer, "%d", options->disp_x);
				sprintf(options->y_buffer, "%d", options->disp_y);
				sprintf(options->m_buffer, "%d", options->disp_mines);
			}
			else if((cursor_position[2 * id] <= 275 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 275 && cursor_position[(2 * id) + 1] >= 306){	//Intermediate
				options->disp_x = 16;
				options->disp_y = 16;
				options->disp_mines = 40;
				sprintf(options->x_buffer, "%d", options->disp_x);
				sprintf(options->y_buffer, "%d", options->disp_y);
				sprintf(options->m_buffer, "%d", options->disp_mines);
			}
			else if((cursor_position[2 * id] <= 361 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 361 && cursor_position[(2 * id) + 1] >= 306){	//Expert
				options->disp_x = 30;
				options->disp_y = 20;
				options->disp_mines = 99;
				sprintf(options->x_buffer, "%d", options->disp_x);
				sprintf(options->y_buffer, "%d", options->disp_y);
				sprintf(options->m_buffer, "%d", options->disp_mines);
			}
			else if((cursor_position[2 * id] <= 275 + 75) && (cursor_position[(2 * id) + 1] <= 252 + 23)
					&& cursor_position[2 * id] >= 275 && cursor_position[(2 * id) + 1] >= 252){	//Save to VMU (UNFINISHED)
				error_freeze("UNIMPLEMENTED");
			}
			else if((cursor_position[2 * id] <= 361 + 75) && (cursor_position[(2 * id) + 1] <= 252 + 23)
					&& cursor_position[2 * id] >= 361 && cursor_position[(2 * id) + 1] >= 252){	//Apply
				if(options->disp_mines > (options->disp_x - 1) * (options->disp_y - 1)){
					options->disp_mines = (options->disp_x - 1) * (options->disp_y - 1);
					sprintf(options->m_buffer, "%d", options->disp_mines);
				}
				reset_grid(grid, options, options->disp_x, options->disp_y, options->disp_mines);	//Doesn't reset the face...
			}
			else if((cursor_position[2 * id] <= 245 + 13) && (cursor_position[(2 * id) + 1] <= 144 + 13)
					&& cursor_position[2 * id] >= 245 && cursor_position[(2 * id) + 1] >= 144){	//Sound checker
				options->sound_enabled = !options->sound_enabled;
				options->checkers.frame_coord_keys[0] = options->sound_enabled;	//Update to show the right frame
			}
			else if((cursor_position[2 * id] <= 245 + 13) && (cursor_position[(2 * id) + 1] <= 184 + 13)
					&& cursor_position[2 * id] >= 245 && cursor_position[(2 * id) + 1] >= 184){	//Question mark checker
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

//Called in draw function, if cursor hovers over a button then return the button map (Either no bits or 1 bit will be set)
uint8_t button_hover(float cursor_x, float cursor_y, MinesweeperOS_t * os){
	uint8_t menus_selected = 0;
	if((cursor_x <= 39) && (cursor_y <= os->tabs_y + 13) && cursor_x >= 5 && cursor_y >= os->tabs_y - 3){
		menus_selected += (1 << 0);
	}
	else if((cursor_x <= 88) && (cursor_y <= os->tabs_y + 13) && cursor_x >= 44 && cursor_y >= os->tabs_y - 3){
		menus_selected += (1 << 1);
	}
	else if((cursor_x <= 140) && (cursor_y <= os->tabs_y + 13) && cursor_x >= 93 && cursor_y >= os->tabs_y - 3){
		menus_selected += (1 << 2);
	}
	else if((cursor_x <= 181) && (cursor_y <= os->tabs_y + 13) && cursor_x >= 145 && cursor_y >= os->tabs_y - 3){
		menus_selected += (1 << 3);
	}
	return menus_selected;
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

	//Load a save file here

	//Setting some default variables
	MS_grid.over_mode = 0;
	MS_grid.game_live = 0;
	MS_options.sd_present = 0;
	MS_options.question_enabled = 1;
	MS_options.sound_enabled = 1;
	MS_options.operating_system = 0;
	MS_options.language = 0;
	MS_options.focus = 0;

	// MS_options.focus = 2;	//DEBUG

	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes == 0){
			MS_options.sd_present = 1;
		}
	#endif

	//Currently this is the only way to access some of the hidden features
	//Later OS will be chosen in BIOS and language through save file name
	MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
	if(st->buttons & (1 << 1)){		//B press
		MS_options.operating_system = 1;
	}
	MAPLE_FOREACH_END()

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB and we default to NTSC interlace (Make a 50/60 Hz menu later). This handles composite
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}

	pvr_init(&pvr_params);

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

	crayon_spritesheet_t Board, Icons, Windows;
	crayon_palette_t Board_P, Icons_P, Windows_P, BIOS_P, Tahoma_P, White_Tahoma_P,
		cursor_red, cursor_yellow, cursor_green, cursor_blue;
	crayon_textured_array_t indented_tiles;	//When doing an A or X press. Currently unused
	crayon_font_mono_t BIOS_font;
	crayon_font_prop_t Tahoma_font;
	Board.spritesheet_texture = NULL;
	Icons.spritesheet_texture = NULL;
	Windows.spritesheet_texture = NULL;
	BIOS_font.fontsheet_texture = NULL;
	Tahoma_font.fontsheet_texture = NULL;

	crayon_untextured_array_t Bg_polys, Option_polys;	//Contains some of the untextured polys that will be drawn.
	sfxhnd_t Sound_Tick, Sound_Death, Sound_Death_Italian, Sound_Win;	//Sound effect handles. Might add more later for startup sounds or maybe put them in cdda? (Note this is a uint32_t)
	snd_stream_init();	//Needed otherwise snd_sfx calls crash

	#if CRAYON_SD_MODE == 1
		crayon_memory_mount_romdisk("/sd/Minesweeper.img", "/Minesweeper");
	#else
		crayon_memory_mount_romdisk("/cd/Minesweeper.img", "/Minesweeper");
	#endif

	//I like to put the font's paletets at the very back of the system (But really, its probably better standard to go first)
	crayon_memory_load_mono_font_sheet(&BIOS_font, &BIOS_P, 63, "/Minesweeper/Fonts/BIOS_font.dtex");
	crayon_memory_load_prop_font_sheet(&Tahoma_font, &Tahoma_P, 62, "/Minesweeper/Fonts/Tahoma_font.dtex");
	crayon_memory_load_spritesheet(&Board, &Board_P, 0, "/Minesweeper/Board.dtex");
	crayon_memory_load_spritesheet(&Icons, &Icons_P, 1, "/Minesweeper/Icons.dtex");	

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

	//Make the OS struct and populate it
	MinesweeperOS_t os;

	//The dreamcast logo to be displayed on the windows taskbar
	uint8_t region = flashrom_get_region();
	if(region < 0){	//If error we just default to green swirl. Apparently its possible for some DCs to return -1 despite having a region
		region = 0;
	}

	//Populate OS struct
	setup_OS_assets(&os, &Windows, &Windows_P, MS_options.operating_system, MS_options.language, MS_options.sd_present);
	setup_OS_assets_icons(&os, &Icons, &Icons_P, MS_options.operating_system, region);

	//Add the clock palette
	if(MS_options.operating_system){
		os.clock_palette = &White_Tahoma_P;
	}
	else{
		os.clock_palette = &Tahoma_P;
	}

	//Setup the untextured poly structs
	setup_bg_untextured_poly(&Bg_polys, MS_options.operating_system, MS_options.sd_present);
	setup_option_untextured_poly(&Option_polys, MS_options.operating_system);

	//Setting size to 1 since reset_grid will reset it soon anyways
	//Also due to lang thing we don't know the spritesheet, animation or palette
	// crayon_memory_set_sprite_array(&MS_grid.draw_grid, 1, 16, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL);
	crayon_memory_set_sprite_array(&MS_grid.draw_grid, 38 * 21, 16, 0, 1, 0, 0, 0, 0, 0, NULL, NULL, NULL);	//Technically there's no need to make change the size after this

	int iter;
	int jiter;

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
	uint8_t checker_id = 4;
	for(iter = 0; iter < Windows.spritesheet_animation_count; iter++){
		if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "button")){
			button_id = iter;
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "checker")){
			checker_id = iter;
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "numberChanger")){
			num_changer_id = iter;
		}
	}

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
	MS_options.buttons.positions[0] = 189;
	MS_options.buttons.positions[1] = 306;
	MS_options.buttons.positions[2] = 275;
	MS_options.buttons.positions[3] = 306;
	MS_options.buttons.positions[4] = 361;
	MS_options.buttons.positions[5] = 306;
	MS_options.buttons.positions[6] = 275;
	MS_options.buttons.positions[7] = 252;
	MS_options.buttons.positions[8] = 361;
	MS_options.buttons.positions[9] = 252;
	MS_options.buttons.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(MS_options.buttons.animation, MS_options.buttons.frame_coord_map, MS_options.buttons.frame_coord_map + 1, 0);

	//Checkers
	MS_options.checkers.draw_z[0] = 30;
	MS_options.checkers.scales[0] = 1;
	MS_options.checkers.scales[1] = 1;
	MS_options.checkers.flips[0] = 0;
	MS_options.checkers.rotations[0] = 0;
	MS_options.checkers.colours[0] = 0;
	MS_options.checkers.positions[0] = 245;
	MS_options.checkers.positions[1] = 144;
	MS_options.checkers.positions[2] = 245;
	MS_options.checkers.positions[3] = 184;
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
	MS_options.number_changers.positions[0] = 420;
	MS_options.number_changers.positions[1] = 140 + (2 * MS_options.operating_system);
	MS_options.number_changers.positions[2] = 420;
	MS_options.number_changers.positions[3] = 180 + (2 * MS_options.operating_system);
	MS_options.number_changers.positions[4] = 420;
	MS_options.number_changers.positions[5] = 220 + (2 * MS_options.operating_system);
	MS_options.number_changers.frame_coord_keys[0] = 0;
	graphics_frame_coordinates(MS_options.number_changers.animation, MS_options.number_changers.frame_coord_map, MS_options.number_changers.frame_coord_map + 1, 0);

	MS_grid.logic_grid = NULL;
	reset_grid(&MS_grid, &MS_options, 30, 20, 99);

	//The face frame coords
	uint16_t face_frame_x;
	uint16_t face_frame_y;
	uint8_t face_frame_id = 0;	//0 normal, 1 suprised, 2 ded, 3 sunny, 4 indented

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
	crayon_memory_clone_palette(&Icons_P, &cursor_red, 2);
	crayon_memory_clone_palette(&Icons_P, &cursor_yellow, 3);
	crayon_memory_clone_palette(&Icons_P, &cursor_green, 4);
	crayon_memory_clone_palette(&Icons_P, &cursor_blue, 5);
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

	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
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
		button_action |= ((previous_buttons[__dev->port] & CONT_A) && !(st->buttons & CONT_A)) << 0;	//A released
		button_action |= ((previous_buttons[__dev->port] & CONT_X) && !(st->buttons & CONT_X)) << 1;	//X released
		button_action |= (!(previous_buttons[__dev->port] & CONT_B) && (st->buttons & CONT_B)) << 2;	//B pressed
		button_action |= (!(previous_buttons[__dev->port] & CONT_Y) && (st->buttons & CONT_Y)) << 3;	//Y pressed
		button_action |= ((start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && !(start_primed & (1 << 4))) << 4;	//If we press start, but we haven't done active prime yet and we aren't invalidated

		if(button_press_logic(&MS_grid, &MS_options, button_action, __dev->port, cursor_position, previous_buttons, st->buttons)){break;}	//Is previous_buttons right?
		button_press_logic_buttons(&MS_grid, &MS_options, &os, __dev->port, cursor_position, previous_buttons[__dev->port], st->buttons);
		language_swap(&MS_grid, &MS_options, &os, cursor_position + (2 * __dev->port), button_action & (1 << 0));

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
				cursor_position[(2 * __dev->port) + 1] -= 2 * movement_speed;
				if(cursor_position[(2 * __dev->port) + 1] < 0){
					cursor_position[(2 * __dev->port) + 1] = 0;
				}
			}
			if(st->buttons & CONT_DPAD_DOWN){
				cursor_position[(2 * __dev->port) + 1] += 2 * movement_speed;
				if(cursor_position[(2 * __dev->port) + 1] > 480){
					cursor_position[(2 * __dev->port) + 1] = 480;
				}
			}
			if(st->buttons & CONT_DPAD_LEFT){
				cursor_position[2 * __dev->port] -= 2 * movement_speed;
				if(cursor_position[2 * __dev->port] < 0){
					cursor_position[2 * __dev->port] = 0;
				}
			}
			if(st->buttons & CONT_DPAD_RIGHT){
				cursor_position[2 * __dev->port] += 2 * movement_speed;
				if(cursor_position[2 * __dev->port] > 640){
					cursor_position[2 * __dev->port] = 640;
				}
			}
		}
		else{	//Thumbstick
			if(thumb_active){	//The thumbstick is outside of the 40% radius
				cursor_position[2 * __dev->port] += 2 * thumb_x;
				if(cursor_position[2 * __dev->port] < 0){
					cursor_position[2 * __dev->port] = 0;
				}
				else if(cursor_position[2 * __dev->port] > 640){
					cursor_position[2 * __dev->port] = 640;
				}
				cursor_position[(2 * __dev->port) + 1] += 2 * thumb_y;
				if(cursor_position[(2 * __dev->port) + 1] < 0){
					cursor_position[(2 * __dev->port) + 1] = 0;
				}
				else if(cursor_position[(2 * __dev->port) + 1] > 480){
					cursor_position[(2 * __dev->port) + 1] = 480;
				}
			}
		}

		face_logic(&MS_grid, &face_frame_id, __dev->port, cursor_position, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)));

		if(MS_options.focus == 0){
			grid_indent_logic(&MS_grid, ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)), !!(st->buttons & (CONT_B)), 0);
		}

		previous_buttons[__dev->port] = st->buttons;	//Store the previous button presses

		MAPLE_FOREACH_END()

		//NOTE TO SELF: Break up some of the controller code into functions since its shared between the controller and the mouse
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_MOUSE, mouse_state_t, st)	//Mouse support (Currently incomplete)

		//In emulators like lxdream this will usually trigger instantly since it doesn't do mouse buttons that well
		if(!(player_active & (1 << __dev->port))){	//Player is there, but hasn't been activated yet
			if(st->buttons != 0 || st->dx != 0 || st->dy != 0){	//Input detected
				player_active |= (1 << __dev->port);
			}
			else{
				continue;
			}
		}

		//Use the buttons previously pressed to control what happens here (Mouse logic needs improvements (Indent code is wrong for Left/Right middle press))
		button_action = 0;
		button_action |= ((previous_buttons[__dev->port] & MOUSE_LEFTBUTTON) && !(st->buttons & MOUSE_LEFTBUTTON)
			&& !(previous_buttons[__dev->port] & MOUSE_RIGHTBUTTON)) << 0;	//Left released and Right wasn't held down
		button_action |= ((((previous_buttons[__dev->port] & MOUSE_SIDEBUTTON) && !(st->buttons & MOUSE_SIDEBUTTON)) ||
			(previous_buttons[__dev->port] & MOUSE_LEFTBUTTON && previous_buttons[__dev->port] & MOUSE_RIGHTBUTTON &&
			(!(st->buttons & MOUSE_LEFTBUTTON) != !(st->buttons & MOUSE_RIGHTBUTTON))))) << 1;	//Side released or Left/Right released while other is still held down and the game isn't over
		button_action |= (!(previous_buttons[__dev->port] & MOUSE_RIGHTBUTTON) && (st->buttons & MOUSE_RIGHTBUTTON)
			&& !(st->buttons & MOUSE_LEFTBUTTON)) << 2;	//Right pressed (Without Left) and the game isn't over

		if(button_press_logic(&MS_grid, &MS_options, button_action, __dev->port, cursor_position, previous_buttons, st->buttons)){break;}
		button_press_logic_buttons(&MS_grid, &MS_options, &os, __dev->port, cursor_position, previous_buttons[__dev->port], st->buttons);
		language_swap(&MS_grid, &MS_options, &os, cursor_position + (2 * __dev->port), button_action & (1 << 0));

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

		face_logic(&MS_grid, &face_frame_id, __dev->port, cursor_position, !!(st->buttons & (MOUSE_LEFTBUTTON)), !!(st->buttons & (MOUSE_SIDEBUTTON)));	//Might need to change the X/Side button code

		if(MS_options.focus == 0){
			grid_indent_logic(&MS_grid, ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (MOUSE_LEFTBUTTON)),
				!!(st->buttons & (MOUSE_SIDEBUTTON)), !!(st->buttons & (MOUSE_RIGHTBUTTON)), 1);
		}

		previous_buttons[__dev->port] = st->buttons;	//Store the previous button presses

		MAPLE_FOREACH_END()

		//X101 0000 (Where every controller is doing an impure press)
		if((start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			face_frame_id = 4;
		}


		//XX01 XXXX (Where every controller is doing an impure press)
		if(!(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && (start_primed % (1 << 4))){
			start_primed |= (1 << 5);	//Now an invalid press
			face_frame_id = 0;
		}

		//X011 0000 (Start is released, but it was invalid)
		if(!(start_primed & (1 << 6)) && (start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			start_primed = 0;
		}

		//X001 0000 (Start is released and it was valid)
		if(!(start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			clear_grid(&MS_grid);
			start_primed = 0;
			face_frame_id = 0;
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
				face_frame_id = 3;
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
				face_frame_id = 2;
			}
		}

		//When releasing a start press (Or it becomes impure), we want to revert back to the gameover faced instead of the normal face
		if(MS_grid.over_mode && face_frame_id != 4){
			if(MS_grid.non_mines_left == 0){
				face_frame_id = 3;
			}
			else{
				face_frame_id = 2;
			}
		}

		//Right now this is always triggered when a game ends and thanks to "revealed" it only triggers once
		if(!MS_grid.revealed && !MS_grid.game_live && MS_grid.over_mode){
			reveal_map(&MS_grid);
		}

		graphics_frame_coordinates(&Board.spritesheet_animation_array[0], &face_frame_x, &face_frame_y, face_frame_id);
		if(face_frame_id != 2 && face_frame_id != 3){	//So the ded/sunnies/end sound code will only be triggered on the turn the game ends
			face_frame_id = 0;	//Reset face for new frame
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
			if(MS_options.focus <= 1 && MS_options.sound_enabled && temp_sec > MS_grid.time_sec){	//Play the "tick" sound effect (But only when time_sec changes and we're on the game tab)
				snd_sfx_play(Sound_Tick, 192, 128);
			}
			MS_grid.time_sec = temp_sec;
		}

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
		graphics_setup_palette(&Board_P);
		graphics_setup_palette(&Icons_P);
		if(!MS_options.operating_system){	//Since it uses palettes and XP doesn't, we do this
			graphics_setup_palette(&Windows_P);	//Since Windows uses 8bpp, this doesn't overlap with "icons"
		}
		graphics_setup_palette(&cursor_red);
		graphics_setup_palette(&cursor_yellow);
		graphics_setup_palette(&cursor_green);
		graphics_setup_palette(&cursor_blue);

		graphics_setup_palette(&White_Tahoma_P);	//Used in XP's clock and controller legend
		graphics_setup_palette(&Tahoma_P);
		graphics_setup_palette(&BIOS_P);

		//Transfer more stuff from this list into either the PT or OP lists
		pvr_list_begin(PVR_LIST_TR_POLY);

			//Draw the flag count and timer (Modify function to draw them as opaque or just redo this part)
			digit_display(&Board, &Board.spritesheet_animation_array[1], MS_grid.num_flags, 20, 65, 17);
			digit_display(&Board, &Board.spritesheet_animation_array[1], MS_grid.time_sec, 581, 65, 17);

			//Draw the reset button face
			graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[0], 307, 64, 16, 1, 1, face_frame_x, face_frame_y, 0);

			//Draw the indented tiles ontop of the grid and the cursors themselves
			uint8_t menus_selected = 0;
			for(iter = 0; iter < 4; iter++){
				if(player_active & (1 << iter)){
					//Passing coords as ints because otherwise we can get case where each pixel contains more than 1 texel
					//ADD A DRAW FOR THE SHADOW
					uint8_t cursor_palette = 2 + iter;
					if(player_active == (1 << 0) || player_active == (1 << 1) || player_active == (1 << 2) || player_active == (1 << 3)){
						cursor_palette = 1;
					}
					graphics_draw_sprite(&Icons, &Icons.spritesheet_animation_array[0], (int) cursor_position[2 * iter],
						(int) cursor_position[(2 * iter) + 1], 51, 1, 1, 0, 0, cursor_palette);

					//Add code for checking if cursor is hovering over a button
					menus_selected |= button_hover(cursor_position[(2 * iter)], cursor_position[(2 * iter) + 1], &os);
				}
			}

			//Drawing the highlight boxes (Maybe make a change so they're brighter in XP?)
			if(menus_selected & (1 << 0)){
				graphics_draw_untextured_poly(5, os.tabs_y - 3, 19, 34, 16, 0x40FFFFFF, 0);	//Font is layer 20
			}
			if(menus_selected & (1 << 1)){
				graphics_draw_untextured_poly(44, os.tabs_y - 3, 19, 44, 16, 0x40FFFFFF, 0);
			}
			if(menus_selected & (1 << 2)){
				graphics_draw_untextured_poly(93, os.tabs_y - 3, 19, 47, 16, 0x40FFFFFF, 0);
			}
			if(menus_selected & (1 << 3)){
				graphics_draw_untextured_poly(145, os.tabs_y - 3, 19, 36, 16, 0x40FFFFFF, 0);
			}

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);

			for(iter = 0; iter < os.num_assets; iter++){
				if(!strcmp(os.assets[iter]->animation->animation_name, "aboutLogo") && MS_options.focus != 4){	//We don't want to draw that unless we're on the about page
					continue;
				}
				crayon_graphics_draw_sprites(os.assets[iter], PVR_LIST_PT_POLY);
			}

			//Draw the sd icon
			if(MS_options.sd_present){
				crayon_graphics_draw_sprites(&os.sd, PVR_LIST_OP_POLY);
			}

			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 9, os.tabs_y, 20, 1, 1, 62, "Game\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 48, os.tabs_y, 20, 1, 1, 62, "Options\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 97, os.tabs_y, 20, 1, 1, 62, "Controls\0");
			graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 149, os.tabs_y, 20, 1, 1, 62, "About\0");

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

			// Move alot of the TR stuff here or into the OP list
			if(MS_options.focus == 2){
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 189, 145, 31, 1, 1, 62, "Sound\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 189, 185, 31, 1, 1, 62, "Marks (?)\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 282, 257, 31, 1, 1, 62, "Save to VMU\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 385, 257, 31, 1, 1, 62, "Apply\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 190, 280, 31, 1, 1, 62, "Best Times...\n(NOT IMPLEMENTED YET)\0");
				// graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 189, 280, 31, 1, 1, 62, "Best Times...\0");

				//Re-position these later (Do I still need to?)
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 204, 311, 31, 1, 1, 62, "Beginner\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 284, 311, 31, 1, 1, 62, "Intemediate\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 383, 311, 31, 1, 1, 62, "Expert\0");

				//Draw the numbers for x, y and num_mines displays
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 399, 145, 31, 1, 1, 62, MS_options.y_buffer);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 399, 185, 31, 1, 1, 62, MS_options.x_buffer);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 399, 225, 31, 1, 1, 62, MS_options.m_buffer);
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 350, 145, 31, 1, 1, 62, "Height:\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 350, 185, 31, 1, 1, 62, "Width:\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 350, 225, 31, 1, 1, 62, "Mines:\0");

				/*

				Options page will contain these:
				(DONE) - X dim toggle (Display box with Up/Down buttons)
				(DONE) - Y dim toggle
				(DONE) - Num mines togle
				(DONE) - Beginner, Intermediate, Expert shortcuts (This changes the X, Y and Num_mines toggles)
				(DONE) - An "Apply" button

				(KINDA DONE) - Sound checkbox
				(KINDA DONE)- Question checkbox
				- Italian checkbox? For debugging, maybe

				*/

				//Display high scores here
			}
			else if(MS_options.focus == 4){
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 120, 25, 1, 1, 62, "Microsoft (R) Minesweeper\0");	//Get the proper @R and @c symbols for XP, or not...
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 125 + 12, 25, 1, 1, 62, "Version \"Pre-reveal\" (Build 3011: Service Pack 5)\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 130 + 24, 25, 1, 1, 62, "Copyright (C) 1981-2018 Microsoft Corp.\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 135 + 36, 25, 1, 1, 62, "by Robert Donner and Curt Johnson\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 155 + 48, 25, 1, 1, 62, "This Minesweeper re-creation was made by Protofall using KallistiOS,\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 160 + 60, 25, 1, 1, 62, "used texconv textures and powered by my Crayon library. I don't own\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 165 + 72, 25, 1, 1, 62, "the rights to Minesweeper nor do I claim to so don't sue me, k?\0");
				graphics_draw_text_prop(&Tahoma_font, PVR_LIST_PT_POLY, 140, 185 + 84, 25, 1, 1, 62, "Katana logo made by Airofoil\0");
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
			}
			if(MS_options.focus == 2){
				crayon_graphics_draw_sprites(&MS_options.buttons, PVR_LIST_OP_POLY);
				crayon_graphics_draw_sprites(&MS_options.checkers, PVR_LIST_OP_POLY);
				crayon_graphics_draw_sprites(&MS_options.number_changers, PVR_LIST_OP_POLY);
			}

			//Draw the region icon
			crayon_graphics_draw_sprites(&os.region, PVR_LIST_OP_POLY);

			//Draw the grid's boarder
			if(MS_options.focus <= 1){
				custom_poly_boarder(3, MS_grid.start_x, MS_grid.start_y, 16, MS_grid.x * 16, MS_grid.y * 16, 4286611584u, 4294967295u);
			}
			else{
				if(MS_options.focus > 1){
					graphics_draw_untextured_poly(6, 98, 10, 631, 1, 0xFFA0A0A0, 1);
					graphics_draw_untextured_poly(6, 99, 10, 631, 350, 0xFFD4D0C8, 1);
				}
				if(MS_options.focus == 2){
					graphics_draw_untextured_array(&Option_polys);
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
	}

	//Confirm everything was unloaded successfully (Should equal zero) This code is never triggered under normal circumstances
	//I'm probs forgetting a few things
	int retVal = 0;
	retVal += crayon_memory_free_sprite_array(&MS_grid.draw_grid, 0, 0);
	retVal += crayon_memory_free_sprite_array(&indented_tiles, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_options.checkers, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_options.buttons, 0, 0);
	retVal += crayon_memory_free_sprite_array(&MS_options.number_changers, 0, 0);
	retVal += crayon_memory_free_spritesheet(&Board);
	retVal += crayon_memory_free_spritesheet(&Icons);
	retVal += crayon_memory_free_spritesheet(&Windows);
	retVal += crayon_memory_free_mono_font_sheet(&BIOS_font);
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
	error_freeze("Free-ing result %d!\n", retVal);

	return 0;
}

//Add something to be displayed on the VMU screen. But what? Just a static mine/blown up mine
//When choosing an OS, make it boot up with a Dreamcast/Katana legacy BIOS
