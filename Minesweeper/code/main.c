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

uint16_t non_mines_left;	//When this variable equals zero, the game is won
int num_flags;	//More like "number of flags in the pool"

uint8_t over_mode = 0;	//0 = ready for new game, 1 = game just ended, 2 = loss (ded), 3 = win
uint8_t game_live = 0;	//Is true when the timer is turning
uint8_t revealed;
int time_sec;

uint8_t sd_present = 0;			//If an ext2 formatted SD card is detected, this this becomes true and modifies some textures/coords and allows R to save screenshots
uint8_t question_enabled = 1;	//Enable the use of question marking
uint8_t sound_enabled = 1;		//Toggle the sound
uint8_t operating_system = 0;	//0 for 2000, 1 for XP
uint8_t language = 0;			//0 for English, 1 for Italian. This affects the font language and the Minesweeper/Prato fiorito themes

uint8_t focus = 0;	//Which window/tab is being focused on. 0 is The game, 1 is "New high score", 2 is settings/savefile, 3 is controls and 4 is "About"
uint8_t grid_x;
uint8_t grid_y;
uint16_t grid_start_x;
uint16_t grid_start_y;
uint16_t num_mines;
uint8_t first_reveal;

uint8_t *logic_grid;
uint16_t *coord_grid;
uint16_t *frame_grid;

//For the options page (Apply only affects these 3 and not the checkboxes)
uint8_t disp_grid_x;
uint8_t disp_grid_y;
uint8_t disp_grid_mines;
char x_buffer[4], y_buffer[4], m_buffer[4];

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
uint8_t neighbouring_tiles(int ele_x, int ele_y){
	uint8_t ret_val = 255;
	if(ele_x == 0){
		ret_val &= ~(1 << 0);	//Clearing bits which can't be right
		ret_val &= ~(1 << 3);
		ret_val &= ~(1 << 5);
	}
	else if(ele_x >= grid_x - 1){
		ret_val &= ~(1 << 2);
		ret_val &= ~(1 << 4);
		ret_val &= ~(1 << 7);
	}
	if(ele_y == 0){
		ret_val &= ~(1 << 0);
		ret_val &= ~(1 << 1);
		ret_val &= ~(1 << 2);
	}
	else if(ele_y >= grid_y - 1){
		ret_val &= ~(1 << 5);
		ret_val &= ~(1 << 6);
		ret_val &= ~(1 << 7);
	}

	return ret_val;
}

//Initially called by clear_grid where x and y are always a valid numbers
void populate_logic(int ele_x, int ele_y){
	int ele_logic = ele_x + grid_x * ele_y;
	if(logic_grid[ele_logic] % (1 << 4) == 9){	//Is mine
		return;
	}

	uint8_t valids = neighbouring_tiles(ele_x, ele_y);
	uint8_t sum = 0;	//A tiles value

	if((valids & (1 << 0)) && logic_grid[ele_x - 1 + ((ele_y- 1) * grid_x)] % (1 << 4) == 9){sum++;}	//Top Left
	if((valids & (1 << 1)) && logic_grid[ele_x + ((ele_y - 1) * grid_x)] % (1 << 4) == 9){sum++;}		//Top centre
	if((valids & (1 << 2)) && logic_grid[ele_x + 1 + ((ele_y - 1) * grid_x)] % (1 << 4) == 9){sum++;}	//Top right
	if((valids & (1 << 3)) && logic_grid[ele_x - 1 + (ele_y * grid_x)] % (1 << 4) == 9){sum++;}			//Mid Left
	if((valids & (1 << 4)) && logic_grid[ele_x + 1 + (ele_y * grid_x)] % (1 << 4) == 9){sum++;}			//Mid Right
	if((valids & (1 << 5)) && logic_grid[ele_x - 1 + ((ele_y + 1) * grid_x)] % (1 << 4) == 9){sum++;}	//Bottom left
	if((valids & (1 << 6)) && logic_grid[ele_x + ((ele_y + 1) * grid_x)] % (1 << 4) == 9){sum++;}		//Bottom centre
	if((valids & (1 << 7)) && logic_grid[ele_x + 1 + ((ele_y + 1) * grid_x)] % (1 << 4) == 9){sum++;}	//Bottom right

	logic_grid[ele_logic] += sum;

	return;
}


uint8_t true_prob(double p){
	return rand() < p * (RAND_MAX + 1.0);
}

//Blanks out grid then fills with mines, but doesn't number them
void clear_grid(crayon_animation_t * anim){
	num_flags = 0;
	uint16_t grid_size = grid_x * grid_y;

	int i = 0;
	uint16_t mines_left = num_mines;
	uint16_t tiles_left = grid_size;

	while(i < grid_size){	//Populate board
		double prob = (double)mines_left / (double)tiles_left;	//Can I do better than using a division?
		logic_grid[i] = 9 * true_prob(prob);
		if(logic_grid[i]){
			mines_left--;
			num_flags++;
		}
		tiles_left--;
		i++;
	}

	for(i = 0; i < 2 * grid_size; i = i + 2){
		graphics_frame_coordinates(anim, frame_grid + i, frame_grid + i + 1, 0);	//All tiles are now blank
	}

	game_live = 0;
	over_mode = 0;
	revealed = 0;
	time_sec = 0;
	first_reveal = 0;

	non_mines_left = grid_size - num_flags;

	return;
}

//When called it changes the grid size and calls clear
//Max grid size is 38, 21
void reset_grid(crayon_animation_t * anim, uint8_t x, uint8_t y, uint16_t mine_count){
	grid_x = x;
	grid_y = y;
	num_mines = mine_count;
	uint16_t grid_size = x * y;

	if(logic_grid != NULL){
		free(logic_grid);
	}
	if(coord_grid != NULL){
		free(coord_grid);
	}
	if(frame_grid != NULL){
		free(frame_grid);
	}

	logic_grid = (uint8_t *) malloc(grid_size * sizeof(uint8_t));
	coord_grid = (uint16_t *) malloc(2 * grid_size * sizeof(uint16_t));
	frame_grid = (uint16_t *) malloc(2 * grid_size * sizeof(uint16_t));

	//Calculate some grid_start_x stuff. Default for expert is 80, 104
	grid_start_x = 320 - (grid_x * 8);
	grid_start_y = 104 + 160 - (grid_y * 8);
	if(grid_start_y < 104){
		grid_start_y = 104;
	}

	int i, j;

	for(j = 0; j < grid_y; j++){
		for(i = 0; i < grid_x; i++){   //i is x, j is y
			uint16_t ele = (j * grid_x * 2) + (2 * i);
			coord_grid[ele] = grid_start_x + (i * 16);
			coord_grid[ele + 1] = grid_start_y + (j * 16);
		}
	}

	clear_grid(anim);

	disp_grid_x = grid_x;
	disp_grid_y = grid_y;
	disp_grid_mines = num_mines;

	sprintf(x_buffer, "%d", disp_grid_x);
	sprintf(y_buffer, "%d", disp_grid_y);
	sprintf(m_buffer, "%d", disp_grid_mines);

	return;
}

//Numbers every tile in the grid
void number_grid(){
	int i, j;
	for(j = 0; j < grid_y; j++){
		for(i = 0; i < grid_x; i++){
			populate_logic(i, j);
		}
	}

	return;
}

//If you initially press a tile, move the mines around so you get a "free first press"
void adjust_grid(int ele_logic){
	logic_grid[ele_logic] = 0;
	// non_mines_left++;	//I commented these out because normally they aren't needed, but if you had a grid with all mines (why?) then this matters
	// num_flags--;
	int i = 0;
	for(i = 0; i < grid_x * grid_y; i++){
		if(i != ele_logic && logic_grid[i] % (1 << 4) != 9){
			logic_grid[i] += 9;
			break;
			// non_mines_left--;
			// num_flags++;
		}
	}

	return;
}

//It fills it out differently depending on over_mode
void reveal_map(crayon_animation_t * anim){
	int i;
	if(over_mode == 2){
		for(i = 0; i < grid_x * grid_y; i++){
			if(logic_grid[i] == 9 || logic_grid[i] == 41){	//Untouched or question marked
				graphics_frame_coordinates(anim, frame_grid + (2 * i), frame_grid + (2 * i) + 1, 3);
			}
			if(logic_grid[i] != 73 && logic_grid[i] & 1<<6){	//Untouched or question marked
				graphics_frame_coordinates(anim, frame_grid + (2 * i), frame_grid + (2 * i) + 1, 5);
			}
		}
	}
	else if(over_mode == 3){
		num_flags = 0;
		for(i = 0; i < grid_x * grid_y; i++){
			if(logic_grid[i] % (1 << 5) == 9){
				graphics_frame_coordinates(anim, frame_grid + (2 * i), frame_grid + (2 * i) + 1, 1);
			}
		}
	}
	revealed = 1;
	return;
}

void discover_tile(crayon_animation_t * anim, int ele_x, int ele_y){
	int ele_logic = ele_x + grid_x * ele_y;
	if(!(logic_grid[ele_logic] & 1 << 6)){	//If not flagged
		if(logic_grid[ele_logic] & 1 << 7){	//Already discovered
			return;
		}
		int ele = ele_logic * 2;
		if(logic_grid[ele_logic] & 1 << 5){	//If questioned, remove the question mark and set it to a normal tile
			logic_grid[ele_logic] &= ~(1 << 5);
			graphics_frame_coordinates(anim, frame_grid + ele, frame_grid + ele + 1, 0);
		}
		if(logic_grid[ele_logic] == 9){	//If mine
			graphics_frame_coordinates(anim, frame_grid + ele, frame_grid + ele + 1, 4);
			game_live = 0;
			over_mode = 1;
		}
		else{
			graphics_frame_coordinates(anim, frame_grid + ele, frame_grid + ele + 1, 7 + logic_grid[ele_logic]);
			non_mines_left--;
		}
		logic_grid[ele_logic] |= (1 << 7);
		if(logic_grid[ele_logic] % (1 << 5) == 0){
			uint8_t valids = neighbouring_tiles(ele_x, ele_y);
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
					discover_tile(anim, ele_x + xVarriant, ele_y + yVariant);
				}
			}
		}
	}
	return;
}

void x_press(crayon_animation_t * anim, int ele_x, int ele_y){
	int ele_logic = ele_x + grid_x * ele_y;
	if((logic_grid[ele_logic] & 1<<7)){	//If revealed

		uint8_t valids = neighbouring_tiles(ele_x, ele_y);
		uint8_t flag_sum = 0;	//A tiles value

		if((valids & (1 << 0)) && logic_grid[ele_x - 1 + ((ele_y- 1) * grid_x)] & (1 << 6)){flag_sum++;}	//Top Left
		if((valids & (1 << 1)) && logic_grid[ele_x + ((ele_y - 1) * grid_x)]  & (1 << 6)){flag_sum++;}		//Top centre
		if((valids & (1 << 2)) && logic_grid[ele_x + 1 + ((ele_y - 1) * grid_x)]  & (1 << 6)){flag_sum++;}	//Top right
		if((valids & (1 << 3)) && logic_grid[ele_x - 1 + (ele_y * grid_x)]  & (1 << 6)){flag_sum++;}		//Mid Left
		if((valids & (1 << 4)) && logic_grid[ele_x + 1 + (ele_y * grid_x)]  & (1 << 6)){flag_sum++;}		//Mid Right
		if((valids & (1 << 5)) && logic_grid[ele_x - 1 + ((ele_y + 1) * grid_x)]  & (1 << 6)){flag_sum++;}	//Bottom left
		if((valids & (1 << 6)) && logic_grid[ele_x + ((ele_y + 1) * grid_x)]  & (1 << 6)){flag_sum++;}		//Bottom centre
		if((valids & (1 << 7)) && logic_grid[ele_x + 1 + ((ele_y + 1) * grid_x)]  & (1 << 6)){flag_sum++;}	//Bottom right

		if(logic_grid[ele_logic] % (1 << 4) == flag_sum){
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
					discover_tile(anim, ele_x + xVarriant, ele_y + yVariant);
				}
			}
		}
	}
	return;
}

//If we use a flag that decrements the flag count, leaving flag will increment it
void b_press(crayon_animation_t * anim, uint16_t ele_logic){
	int ele = ele_logic * 2;
	if(!(logic_grid[ele_logic] & (1 << 7))){	//Not discovered
		uint8_t status = 0;	//0 = normal, 1 = flag, 2 = question icons
		if(logic_grid[ele_logic] & (1 << 6)){	//If flagged
			logic_grid[ele_logic] &= ~(1 << 6);	//Clears the flag bit
			if(question_enabled){
				logic_grid[ele_logic] |= (1 << 5);	//Sets the question bit
				status = 2;
			}
			num_flags++;
		}
		else{	//Not flagged, but maybe questioned
			if(logic_grid[ele_logic] & (1 << 5)){	//Normal it
				logic_grid[ele_logic] &= ~(1 << 5);
				status = 0;
			}
			else{	//Flag it
				logic_grid[ele_logic] |= (1 << 6);
				status = 1;
				num_flags--;
			}
		}
		graphics_frame_coordinates(anim, frame_grid + ele, frame_grid + ele + 1, status);
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
void grid_indent_logic(int ele_x, int ele_y, uint8_t in_grid, int id, uint16_t *press_data, uint8_t A_held, uint8_t X_held,
	uint8_t B_held, uint8_t control_type){
	if(over_mode != 0 || (!A_held && !X_held) || !in_grid){
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

void face_logic(uint8_t *face_frame_id, int id, float *cursor_position, uint8_t A_held, uint8_t X_held){
	if(A_held || X_held){
		if(!over_mode && !*face_frame_id){	//Basically face_frame_id != 2, 3 or 4
			*face_frame_id = 1;	//Apply suprised face
		}
		if(A_held && (cursor_position[2 * id] <= 307 + 26) && (cursor_position[(2 * id) + 1] <= 64 + 26)
			&& cursor_position[2 * id] >= 307 && cursor_position[(2 * id) + 1] >= 64){	//If hovering over face and press on it
			*face_frame_id = 4;
		}
	}
}

//Handles the interaction logic with the grid
void grid_ABX_logic(int ele_x, int ele_y, uint8_t button_action, crayon_animation_t *tile_anim, uint32_t *start_time, uint32_t *start_ms_time){
	if(over_mode == 0){
		if(button_action & (1 << 0)){	//For A press
			if(!game_live){
				timer_ms_gettime(start_time, start_ms_time);
				game_live = 1;
			}
			if(first_reveal == 0){
				int ele_logic = ele_x + grid_x * ele_y;
				if(!(logic_grid[ele_logic] & (1 << 6))){
					if(logic_grid[ele_logic] % (1 << 4) == 9){	//If this is the first reveal and its not flagged and its a mine
						adjust_grid(ele_logic);
					}
					number_grid();
					first_reveal = 1;
				}
			}
			discover_tile(tile_anim, ele_x, ele_y);
		}
		if(button_action & (1 << 1)){	//For X press
			if(!game_live){
				timer_ms_gettime(start_time, start_ms_time);
				game_live = 1;
			}
			x_press(tile_anim, ele_x, ele_y);
		}
		if(button_action & (1 << 2)){	//For B press
			b_press(tile_anim, ele_x + grid_x * ele_y);
		}
	}
}

//Its purpose is to check if the player's cursor is on the grid when doing an A/B/X action
void cursor_on_grid(uint8_t *in_grid, int *ele_x, int *ele_y, uint8_t button_action, int id, float *cursor_position){
	if((button_action % (1 << 3)) || !over_mode){
		*in_grid = (cursor_position[2 * id] < grid_start_x + (grid_x * 16)) && (cursor_position[(2 * id) + 1] < grid_start_y + (grid_y * 16))
			&& cursor_position[2 * id] >= grid_start_x && cursor_position[(2 * id) + 1] >= grid_start_y;
		if(*in_grid){
			*ele_x = (cursor_position[2 * id]  - grid_start_x) / 16;
			*ele_y = (cursor_position[(2 * id) + 1]  - grid_start_y) / 16;
		}
	}
}

//Handles focus related things
uint8_t button_press_logic_buttons(MinesweeperOS_t *os, crayon_animation_t *anim, int id, float *cursor_position, uint32_t previous_buttons, uint32_t buttons){
	//CLEAN UP THE MAGIC NUMBERS
	if(focus != 1){	//When we get a new score, we don't want to change focus easily
		//Top 4 options
		if((buttons & CONT_A) && !(previous_buttons & CONT_A)){
			if((cursor_position[2 * id] <= 9 + 27 + 3) && (cursor_position[(2 * id) + 1] <= os->variant_pos[1] + 13)
					&& cursor_position[2 * id] >= 9 - 4 && cursor_position[(2 * id) + 1] >= os->variant_pos[1] - 3){
				focus = 0;
			}
			else if((cursor_position[2 * id] <= 48 + 37 + 3) && (cursor_position[(2 * id) + 1] <= os->variant_pos[1] + 13)
					&& cursor_position[2 * id] >= 48 - 4 && cursor_position[(2 * id) + 1] >= os->variant_pos[1] - 3){
				focus = 2;
			}
			else if((cursor_position[2 * id] <= 97 + 40 + 3) && (cursor_position[(2 * id) + 1] <= os->variant_pos[1] + 13)
					&& cursor_position[2 * id] >= 97 - 4 && cursor_position[(2 * id) + 1] >= os->variant_pos[1] - 3){
				focus = 3;
			}
			else if((cursor_position[2 * id] <= 149 + 29 + 3) && (cursor_position[(2 * id) + 1] <= os->variant_pos[1] + 13)
					&& cursor_position[2 * id] >= 149 - 4 && cursor_position[(2 * id) + 1] >= os->variant_pos[1] - 3){
				focus = 4;
			}
		}
	}

	if(focus == 2){
		if((buttons & CONT_A) && !(previous_buttons & CONT_A)){	//When we get a new score, we don't want to change focus easily
			if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 149)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 140){	//Incrementer X
				if(disp_grid_x < 38){
					disp_grid_x++;
					sprintf(x_buffer, "%d", disp_grid_x);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 159)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 150){	//Decrementer X
				if(disp_grid_x > 9){
					disp_grid_x--;
					sprintf(x_buffer, "%d", disp_grid_x);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 189)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 180){	//Incrementer Y
				if(disp_grid_y < 21){
					disp_grid_y++;
					sprintf(y_buffer, "%d", disp_grid_y);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 199)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 190){	//Decrementer Y
				if(disp_grid_y > 9){
					disp_grid_y--;
					sprintf(y_buffer, "%d", disp_grid_y);
				}
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 229)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 220){	//Incrementer Num Mines
				if(disp_grid_mines < (disp_grid_x - 1) * (disp_grid_y - 1)){
					disp_grid_mines++;
				}
				else{
					disp_grid_mines = (disp_grid_x - 1) * (disp_grid_y - 1);
				}
				sprintf(m_buffer, "%d", disp_grid_mines);
			}
			else if((cursor_position[2 * id] <= 436) && (cursor_position[(2 * id) + 1] <= 239)
					&& cursor_position[2 * id] >= 420 && cursor_position[(2 * id) + 1] >= 230){	//Decrementer Num Mines
				if(disp_grid_mines > 10){
					disp_grid_mines--;
					sprintf(m_buffer, "%d", disp_grid_mines);
				}
			}
			else if((cursor_position[2 * id] <= 189 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 189 && cursor_position[(2 * id) + 1] >= 306){	//Beginner
				disp_grid_x = 9;
				disp_grid_y = 9;
				disp_grid_mines = 10;
				sprintf(x_buffer, "%d", disp_grid_x);
				sprintf(y_buffer, "%d", disp_grid_y);
				sprintf(m_buffer, "%d", disp_grid_mines);
			}
			else if((cursor_position[2 * id] <= 275 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 275 && cursor_position[(2 * id) + 1] >= 306){	//Intermediate
				disp_grid_x = 16;
				disp_grid_y = 16;
				disp_grid_mines = 40;
				sprintf(x_buffer, "%d", disp_grid_x);
				sprintf(y_buffer, "%d", disp_grid_y);
				sprintf(m_buffer, "%d", disp_grid_mines);
			}
			else if((cursor_position[2 * id] <= 361 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 361 && cursor_position[(2 * id) + 1] >= 306){	//Expert
				disp_grid_x = 30;
				disp_grid_y = 20;
				disp_grid_mines = 99;
				sprintf(x_buffer, "%d", disp_grid_x);
				sprintf(y_buffer, "%d", disp_grid_y);
				sprintf(m_buffer, "%d", disp_grid_mines);
			}
			else if((cursor_position[2 * id] <= 361 + 75) && (cursor_position[(2 * id) + 1] <= 252 + 23)
					&& cursor_position[2 * id] >= 361 && cursor_position[(2 * id) + 1] >= 252){	//Apply
				if(disp_grid_mines > (disp_grid_x - 1) * (disp_grid_y - 1)){
					disp_grid_mines = (disp_grid_x - 1) * (disp_grid_y - 1);
					sprintf(m_buffer, "%d", disp_grid_mines);
				}
				reset_grid(anim, disp_grid_x, disp_grid_y, disp_grid_mines);	//Doesn't reset the face...the var is global, but the anim isn't...
			}
			else if((cursor_position[2 * id] <= 361 + 75) && (cursor_position[(2 * id) + 1] <= 306 + 23)
					&& cursor_position[2 * id] >= 361 && cursor_position[(2 * id) + 1] >= 306){	//Save to VMU (UNFINISHED)
				;
			}
			else if((cursor_position[2 * id] <= 245 + 13) && (cursor_position[(2 * id) + 1] <= 144 + 13)
					&& cursor_position[2 * id] >= 245 && cursor_position[(2 * id) + 1] >= 144){	//Sound checker
				sound_enabled = !sound_enabled;
				//ADD SOMETHING TO CHANGE THE FRAME
			}
			else if((cursor_position[2 * id] <= 245 + 13) && (cursor_position[(2 * id) + 1] <= 184 + 13)
					&& cursor_position[2 * id] >= 245 && cursor_position[(2 * id) + 1] >= 184){	//Question mark checker
				question_enabled = !question_enabled;
				//ADD SOMETHING TO CHANGE THE FRAME
			}
		}
	}

	// coord_checker[0] = 245;	//Sound
	// coord_checker[1] = 144;
	// coord_checker[2] = 245;	//Question mark
	// coord_checker[3] = 184;
	//13,13

	return 0;
}

//This handles all the buttons you can press aside from the grid itself. Currently only does the face, your re-add the buttons variant here
uint8_t button_press_logic(uint8_t button_action, int id, float *cursor_position, crayon_animation_t * tile_anim,
	uint32_t *previous_buttons, uint32_t buttons){
	if(focus == 0){
		if((button_action & (1 << 0)) && (cursor_position[2 * id] <= 307 + 26) && (cursor_position[(2 * id) + 1] <= 64 + 26)
			&& cursor_position[2 * id] >= 307 && cursor_position[(2 * id) + 1] >= 64){	//If face is released on
			clear_grid(tile_anim);
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
	if((cursor_x <= 39) && (cursor_y <= os->variant_pos[1] + 13) && cursor_x >= 5 && cursor_y >= os->variant_pos[1] - 3){
		menus_selected += (1 << 0);
	}
	else if((cursor_x <= 88) && (cursor_y <= os->variant_pos[1] + 13) && cursor_x >= 44 && cursor_y >= os->variant_pos[1] - 3){
		menus_selected += (1 << 1);
	}
	else if((cursor_x <= 140) && (cursor_y <= os->variant_pos[1] + 13) && cursor_x >= 93 && cursor_y >= os->variant_pos[1] - 3){
		menus_selected += (1 << 2);
	}
	else if((cursor_x <= 181) && (cursor_y <= os->variant_pos[1] + 13) && cursor_x >= 145 && cursor_y >= os->variant_pos[1] - 3){
		menus_selected += (1 << 3);
	}
	return menus_selected;
}

int main(){
	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes == 0){
			sd_present = 1;
		}
	#endif

	focus = 2;	//Remove this later, its only used for debugging right now

	//Currently this is the only way to access some of the hidden features
	//Later OS will be chosen in BIOS and language through save file name
	MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
	if(st->buttons & (1 << 1)){		//B press
		operating_system = 1;	//Else 0
	}
	if(st->buttons & (1 << 2)){		//A press
		language = 1;	//Else 0
	}
	MAPLE_FOREACH_END()

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB and we default to NTSC interlace (Make a 50/60 Hz menu later). This handles composite
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}

	pvr_init_defaults();

	srand(time(0));	//Set the seed for rand()
	time_t os_clock;	//Stores the current time
	struct tm *readable_time;

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

	crayon_memory_load_mono_font_sheet(&BIOS_font, "/Minesweeper/Fonts/BIOS_font.dtex");
	crayon_memory_load_prop_font_sheet(&Tahoma_font, "/Minesweeper/Fonts/Tahoma_font.dtex");
	crayon_memory_load_spritesheet(&Board, "/Minesweeper/Board.dtex");
	crayon_memory_load_spritesheet(&Icons, "/Minesweeper/Icons.dtex");

	Sound_Tick = snd_sfx_load("/Minesweeper/Sounds/tick.wav");
	Sound_Death = snd_sfx_load("/Minesweeper/Sounds/death.wav");
	Sound_Death_Italian = snd_sfx_load("/Minesweeper/Sounds/deathItalian.wav");
	Sound_Win = snd_sfx_load("/Minesweeper/Sounds/win.wav");
	fs_romdisk_unmount("/Minesweeper");

	//Load the OS assets
	if(operating_system){
		#if CRAYON_SD_MODE == 1
			crayon_memory_mount_romdisk("/sd/XP.img", "/XP");
		#else
			crayon_memory_mount_romdisk("/cd/XP.img", "/XP");
		#endif
		crayon_memory_load_spritesheet(&Windows, "/XP/Windows.dtex");
		fs_romdisk_unmount("/XP");
	}
	else{
		#if CRAYON_SD_MODE == 1
			crayon_memory_mount_romdisk("/sd/2000.img", "/2000");
		#else
			crayon_memory_mount_romdisk("/cd/2000.img", "/2000");
		#endif
		crayon_memory_load_spritesheet(&Windows, "/2000/Windows.dtex");
		fs_romdisk_unmount("/2000");
	}

	//Make the OS struct and populate it
	MinesweeperOS_t os;
	setup_OS_assets(&os, &Windows, operating_system, language, sd_present);

	//Setup the untextured poly structs
	setup_bg_untextured_poly(&Bg_polys, operating_system, sd_present);
	setup_option_untextured_poly(&Option_polys, operating_system);

	int iter;
	int jiter;

	//These two just allow me to easily change between Minesweeper and Prato fiorito
	crayon_spritesheet_t tile_ss;
	crayon_animation_t tile_anim;

	uint8_t tile_id = 2;
	if(!language){
		tile_ss = Board;
	}
	else{
		tile_ss = Windows;
		for(iter = 0; iter < tile_ss.spritesheet_animation_count; iter++){
			if(!strcmp(tile_ss.spritesheet_animation_array[iter].animation_name, "italianTiles")){
				tile_id = iter;
				break;
			}
		}
	}
	tile_anim = tile_ss.spritesheet_animation_array[tile_id];

	//Get the info for num_changer
	uint8_t num_changer_id = 4;
	uint16_t coord_num_changer[6], frame_num_changer[2];
	uint8_t button_id = 4;
	uint16_t coord_button[10], frame_button[2];
	uint8_t checker_id = 4;
	uint16_t coord_checker[4], frame_checker[2];
	for(iter = 0; iter < Windows.spritesheet_animation_count; iter++){
		if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "button")){
			button_id = iter;
			graphics_frame_coordinates(&Windows.spritesheet_animation_array[iter], frame_button, frame_button + 1, 0);
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "checker")){
			checker_id = iter;
			graphics_frame_coordinates(&Windows.spritesheet_animation_array[iter], frame_checker, frame_checker + 1, 0);			
		}
		else if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "numberChanger")){
			num_changer_id = iter;
			graphics_frame_coordinates(&Windows.spritesheet_animation_array[iter], frame_num_changer, frame_num_changer + 1, 0);
			// break;
		}
	}

	coord_num_changer[0] = 420;
	coord_num_changer[1] = 140 + (2 * operating_system);
	coord_num_changer[2] = 420;
	coord_num_changer[3] = 180 + (2 * operating_system);
	coord_num_changer[4] = 420;
	coord_num_changer[5] = 220 + (2 * operating_system);

	coord_button[0] = 189;	//Beginner
	coord_button[1] = 306;
	coord_button[2] = 275;	//Intermediate
	coord_button[3] = 306;
	coord_button[4] = 361;	//Expert
	coord_button[5] = 306;
	coord_button[6] = 275;	//Save to VMU
	coord_button[7] = 252;
	coord_button[8] = 361;	//Apply
	coord_button[9] = 252;

	coord_checker[0] = 245;	//Sound
	coord_checker[1] = 144;
	coord_checker[2] = 245;	//Question mark
	coord_checker[3] = 184;

	logic_grid = NULL;
	coord_grid = NULL;
	frame_grid = NULL;

	reset_grid(&tile_anim, 30, 20, 99);

	uint16_t grid_size = grid_x * grid_start_y;

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
	uint16_t press_data[12] = {0};
	uint8_t player_movement = 0;	//This tells whether a player is using a joystick (0) or D-Pad (1) Only last 4 bits are used
	uint16_t indented_neighbours[18];	//For now I'm doing the centre independently, but later I'll add it here
	uint16_t indented_frames[18];

	//The dreamcast logo to be displayed on the windows taskbar
	uint16_t region_icon_x = 0;
	uint16_t region_icon_y = 0;
	int8_t region = flashrom_get_region() - 1;
	if(region < 0){	//If error we just default to green swirl. Apparently its possible for some DCs to return -1 despite having a region
		region = 3;
	}
	graphics_frame_coordinates(&Icons.spritesheet_animation_array[1], &region_icon_x, &region_icon_y, region);

	float thumb_x = 0;
	float thumb_y = 0;
	uint8_t thumb_active;

	uint16_t sd_x;
	uint16_t sd_y;
	graphics_frame_coordinates(&Icons.spritesheet_animation_array[2], &sd_x, &sd_y, 0);

	//The palette for XP's clock
	crayon_palette_t *White_Tahoma_Font = crayon_memory_clone_palette(Tahoma_font.palette_data);
	crayon_memory_swap_colour(White_Tahoma_Font, 0xFF000000, 0xFFFFFFFF, 0);	//Swapps black for white

	//The cursor colours
	crayon_palette_t *cursor_red, *cursor_yellow, *cursor_green, *cursor_blue;
	cursor_red = crayon_memory_clone_palette(Icons.palette_data);
	cursor_yellow = crayon_memory_clone_palette(Icons.palette_data);
	cursor_green = crayon_memory_clone_palette(Icons.palette_data);
	cursor_blue = crayon_memory_clone_palette(Icons.palette_data);
	crayon_memory_swap_colour(cursor_red, 0xFFFFFFFF, 0xFFFF0000, 0);
	crayon_memory_swap_colour(cursor_yellow, 0xFFFFFFFF, 0xFFFFFF00, 0);
	crayon_memory_swap_colour(cursor_green, 0xFFFFFFFF, 0xFF008000, 0);
	crayon_memory_swap_colour(cursor_blue, 0xFFFFFFFF, 0xFF4D87D0, 0);

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

		if(button_press_logic(button_action, __dev->port, cursor_position, &tile_anim, previous_buttons, st->buttons)){break;}	//Is previous_buttons right?
		button_press_logic_buttons(&os, &tile_anim, __dev->port, cursor_position, previous_buttons[__dev->port], st->buttons);

		//These are only ever set if The cursor is on the grid and A/B/X is/was pressed
		int ele_x = -1;
		int ele_y = -1;
		uint8_t in_grid = 0;

		if(focus == 0){
			cursor_on_grid(&in_grid, &ele_x, &ele_y, button_action, __dev->port, cursor_position);
			if(in_grid){grid_ABX_logic(ele_x, ele_y, button_action, &tile_anim, &start_time, &start_ms_time);}

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

		face_logic(&face_frame_id, __dev->port, cursor_position, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)));

		if(focus == 0){
			grid_indent_logic(ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)), !!(st->buttons & (CONT_B)), 0);
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

		if(button_press_logic(button_action, __dev->port, cursor_position, &tile_anim, previous_buttons, st->buttons)){break;}
		button_press_logic_buttons(&os, &tile_anim, __dev->port, cursor_position, previous_buttons[__dev->port], st->buttons);

		int ele_x = -1;
		int ele_y = -1;
		uint8_t in_grid = 0;

		if(focus == 0){
			cursor_on_grid(&in_grid, &ele_x, &ele_y, button_action, __dev->port, cursor_position);
			if(in_grid){grid_ABX_logic(ele_x, ele_y, button_action, &tile_anim, &start_time, &start_ms_time);}
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

		face_logic(&face_frame_id, __dev->port, cursor_position, !!(st->buttons & (MOUSE_LEFTBUTTON)), !!(st->buttons & (MOUSE_SIDEBUTTON)));	//Might need to change the X/Side button code

		if(focus == 0){
			grid_indent_logic(ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (MOUSE_LEFTBUTTON)),
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
			clear_grid(&tile_anim);
			start_primed = 0;
			face_frame_id = 0;
		}

		start_primed = start_primed & ((1 << 5) + (1 << 4));	//Clears all bits except active and invalidness

		if(non_mines_left == 0 && game_live && !over_mode){
			game_live = 0;
			over_mode = 1;
		}

		//This code triggers the turn when you win/lose the game (Doesn't trigger while dead/won after that)
		if(!game_live && over_mode == 1){	//This triggers at the start of a new game too...
			if(non_mines_left == 0){	//Only mines left, you win
				over_mode = 3;
				if(sound_enabled){ //play "won" sound
					snd_sfx_play(Sound_Win, 192, 128);	//Right now this can play when dead
				}
				face_frame_id = 3;
			}
			else{	//Tiles are left, you must have been blown up
				over_mode = 2;
				if(sound_enabled){	//play bomb/death sound
					if(language){	//Italian
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
		if(over_mode && face_frame_id != 4){
			if(non_mines_left == 0){
				face_frame_id = 3;
			}
			else{
				face_frame_id = 2;
			}
		}

		//Right now this is always triggered when a game ends and thanks to "revealed" it only triggers once
		if(!revealed && !game_live && over_mode){
			reveal_map(&tile_anim);
		}

		graphics_frame_coordinates(&Board.spritesheet_animation_array[0], &face_frame_x, &face_frame_y, face_frame_id);
		if(face_frame_id != 2 && face_frame_id != 3){	//So the ded/sunnies/end sound code will only be triggered on the turn the game ends
			face_frame_id = 0;	//Reset face for new frame
		}

		timer_ms_gettime(&current_time, &current_ms_time);
		if(game_live && time_sec < 999){	//Prevent timer overflows
			int temp_sec = current_time - start_time + (current_ms_time > start_ms_time); //MS is there to account for the "1st second" inaccuracy
			//(How does this do the "start at 1 sec" thing? I forgot)
			if(focus <= 1 && sound_enabled && temp_sec > time_sec){	//Play the "tick" sound effect (But only when time_sec changes and we're on the game tab)
				snd_sfx_play(Sound_Tick, 192, 128);
			}
			time_sec = temp_sec;
		}

		grid_size = grid_x * grid_y;

		time(&os_clock);	//I think this is how I populate it with the current time
		readable_time = localtime(&os_clock);

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
		graphics_setup_palette(Board.palette_data, Board.spritesheet_format, 0);
		graphics_setup_palette(Icons.palette_data, Icons.spritesheet_format, 1);
		if(!operating_system){	//Since it uses palettes and XP doesn't, we do this
			graphics_setup_palette(Windows.palette_data, Windows.spritesheet_format, 1);	//Since Windows uses 8bpp, this doesn't overlap with "icons"
		}
		graphics_setup_palette(cursor_red, Icons.spritesheet_format, 2);
		graphics_setup_palette(cursor_yellow, Icons.spritesheet_format, 3);
		graphics_setup_palette(cursor_green, Icons.spritesheet_format, 4);
		graphics_setup_palette(cursor_blue, Icons.spritesheet_format, 5);

		//I like to put the fonts at the very back of the system (But really, its probably better standard to go first)
		graphics_setup_palette(White_Tahoma_Font, 5, 61);	//Used in XP's clock and controller legend
		graphics_setup_palette(Tahoma_font.palette_data, Tahoma_font.texture_format, 62);
		graphics_setup_palette(BIOS_font.palette_data, BIOS_font.texture_format, 63);


		pvr_list_begin(PVR_LIST_TR_POLY);

		//Draw windows graphics using our MinesweeperOpSys struct
		for(iter = 0; iter < os.sprite_count; iter++){
			if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "aboutLogo") && focus != 4){	//We don't want to draw that unless we're on the about page
				continue;
			}
			graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[os.ids[iter]],
				os.coords_pos[3 * iter], os.coords_pos[(3 * iter) + 1], os.coords_pos[(3 * iter) + 2],
				os.scale[2 * iter], os.scale[(2 * iter) + 1], os.coords_frame[2 * iter], os.coords_frame[(2 *iter) + 1], 1);
				//We choose palette 1 because that's 2000's palette and XP uses RGB565
		}
		graphics_draw_text_prop(&Tahoma_font, 9, os.variant_pos[1], 20, 1, 1, 62, "Game\0");
		graphics_draw_text_prop(&Tahoma_font, 48, os.variant_pos[1], 20, 1, 1, 62, "Options\0");
		graphics_draw_text_prop(&Tahoma_font, 97, os.variant_pos[1], 20, 1, 1, 62, "Controls\0");
		graphics_draw_text_prop(&Tahoma_font, 149, os.variant_pos[1], 20, 1, 1, 62, "About\0");

		//Updating the time in the bottom right
		//CONSIDER ONLY UPDATING IF TIME IS DIFFERENT
		//Will come back to this at a later date to optimise it
		char time_buffer[9];
		if(readable_time->tm_hour < 13){
			if(readable_time->tm_hour == 0){	//When its midnight, display 12:XX AM instead of 00:XX AM
				readable_time->tm_hour = 12;
			}
			sprintf(time_buffer, "%02d:%02d AM", readable_time->tm_hour, readable_time->tm_min);
		}
		else{
			sprintf(time_buffer, "%02d:%02d PM", readable_time->tm_hour - 12, readable_time->tm_min);
		}

		//The X starting pos varies based on the hour for XP and hour, AM/PM for 2000
		if(operating_system){
			if(readable_time->tm_hour % 12 < 10){
				os.variant_pos[2] = 583;
			}
			else{
				os.variant_pos[2] = 586;
			}
		}
		else{
			uint16_t clock_pos;
			if(readable_time->tm_hour % 12 < 10){
				clock_pos = 581;
			}
			else{
				clock_pos = 585;
			}
			if(readable_time->tm_hour < 13){
				clock_pos--;
			}
			os.variant_pos[2] = clock_pos;
		}

		graphics_draw_text_prop(&Tahoma_font, os.variant_pos[2], os.variant_pos[3], 20, 1, 1, os.clock_palette, time_buffer);	//In XP is uses another palette (White text version)

		//DEBUG, REMOVE LATER
		// char focus_buffer[9];
		// sprintf(focus_buffer, "Focus: %d", focus);
		// graphics_draw_text_prop(&Tahoma_font, 580, os.variant_pos[1], 20, 1, 1, 62, focus_buffer);
		#if CRAYON_DEBUG == 1
			char fps_buffer[100];
			sprintf(fps_buffer, "FPS ave: %d, min: %d, max: %d", fps_ave, fps_min, fps_max);
			graphics_draw_text_prop(&Tahoma_font, 235, 435, 20, 1, 1, 62, fps_buffer);
		#endif

		//Some of the options stuff is in opaque list
		if(focus == 2){

			//A draw list for all 3 clicker thingys (Maybe enlarge them?) (MOVE TO OPAQUE LATER)
			graphics_draw_sprites_OLD(&Windows, &Windows.spritesheet_animation_array[num_changer_id], coord_num_changer,
				frame_num_changer, 2, 3, 30, 1, 1, 1);

			//Draw all 5 buttons
			graphics_draw_sprites_OLD(&Windows, &Windows.spritesheet_animation_array[button_id], coord_button,
				frame_button, 2, 5, 30, 1, 1, 1);

			//Draw all 2 check boxes
			graphics_draw_sprites_OLD(&Windows, &Windows.spritesheet_animation_array[checker_id], coord_checker,
				frame_checker, 2, 2, 30, 1, 1, 1);

			graphics_draw_text_prop(&Tahoma_font, 189, 145, 31, 1, 1, 62, "Sound\0");
			graphics_draw_text_prop(&Tahoma_font, 189, 185, 31, 1, 1, 62, "Marks (?)\0");

			graphics_draw_text_prop(&Tahoma_font, 282, 257, 31, 1, 1, 62, "Save to VMU\0");
			graphics_draw_text_prop(&Tahoma_font, 385, 257, 31, 1, 1, 62, "Apply\0");

			//Without these printf's lxdream crashes when trying to print the best times text...
				//I've tested this on Redream, Redream + BIOS, DEMUL and real hardware and it works fine there. This is an lxdream bug
			// printf("Garbage\n");
			// printf("Garbage\n");
			// printf("Garbage\n");
			// printf("Garbage\n");
			// printf("Garbage\n");
			// printf("Garbage\n");
			// printf("Garbage\n");

			// graphics_draw_text_prop(&Tahoma_font, 190, 280, 31, 1, 1, 62, "Best Times...\n(NOT IMPLEMENTED YET)\0");
			graphics_draw_text_prop(&Tahoma_font, 189, 280, 31, 1, 1, 62, "Best Times...\0");

			//Re-position these later
			graphics_draw_text_prop(&Tahoma_font, 204, 311, 31, 1, 1, 62, "Beginner\0");
			graphics_draw_text_prop(&Tahoma_font, 284, 311, 31, 1, 1, 62, "Intemediate\0");
			graphics_draw_text_prop(&Tahoma_font, 383, 311, 31, 1, 1, 62, "Expert\0");

			//Draw the numbers for grid_x, grid_y and num_mines displays
			graphics_draw_text_prop(&Tahoma_font, 399, 145, 31, 1, 1, 62, x_buffer);
			graphics_draw_text_prop(&Tahoma_font, 399, 185, 31, 1, 1, 62, y_buffer);
			graphics_draw_text_prop(&Tahoma_font, 399, 225, 31, 1, 1, 62, m_buffer);

			graphics_draw_text_prop(&Tahoma_font, 350, 145, 31, 1, 1, 62, "Height:\0");
			graphics_draw_text_prop(&Tahoma_font, 350, 185, 31, 1, 1, 62, "Width:\0");
			graphics_draw_text_prop(&Tahoma_font, 350, 225, 31, 1, 1, 62, "Mines:\0");

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
		else if(focus == 4){
			graphics_draw_text_prop(&Tahoma_font, 140, 120, 25, 1, 1, 62, "Microsoft (R) Minesweeper\0");	//Get the proper @R and @c symbols for XP, or not...
			graphics_draw_text_prop(&Tahoma_font, 140, 125 + 12, 25, 1, 1, 62, "Version \"Pre-reveal\" (Build 3011: Service Pack 5)\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 130 + 24, 25, 1, 1, 62, "Copyright (C) 1981-2018 Microsoft Corp.\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 135 + 36, 25, 1, 1, 62, "by Robert Donner and Curt Johnson\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 155 + 48, 25, 1, 1, 62, "This Minesweeper re-creation was made by Protofall using KallistiOS,\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 160 + 60, 25, 1, 1, 62, "used texconv textures and powered by my Crayon library. I don't own\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 165 + 72, 25, 1, 1, 62, "the rights to Minesweeper nor do I claim to so don't sue me, k?\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 185 + 84, 25, 1, 1, 62, "Katana logo made by Airofoil\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 205 + 96, 25, 1, 1, 62, "A huge thanks to the Dreamcast developers for helping me get started\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 210 + 108, 25, 1, 1, 62, "and answering any questions I had and a huge thanks to the Dreamcast\0");
			graphics_draw_text_prop(&Tahoma_font, 140, 215 + 120, 25, 1, 1, 62, "media for bringing the homebrew scene to my attention.\0");
		}

		//Draw the flag count and timer
		digit_display(&Board, &Board.spritesheet_animation_array[1], num_flags, 20, 65, 17);
		digit_display(&Board, &Board.spritesheet_animation_array[1], time_sec, 581, 65, 17);

		//Draw the sd icon
		if(sd_present){
			graphics_draw_sprite(&Icons, &Icons.spritesheet_animation_array[2], os.variant_pos[4], os.variant_pos[5], 21, 1, 1, sd_x, sd_y, 1);
		}

		//Draw the region icon
		graphics_draw_sprite(&Icons, &Icons.spritesheet_animation_array[1], os.variant_pos[6], os.variant_pos[7], 21, 1, 1, region_icon_x, region_icon_y, 1);

		//Draw the reset button face
		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[0], 307, 64, 16, 1, 1, face_frame_x, face_frame_y, 0);

		//Draw the grid
		if(focus <= 1){
			graphics_draw_sprites_OLD(&tile_ss, &tile_anim, coord_grid, frame_grid, 2 * grid_size, grid_size, 17, 1, 1, !operating_system && language);
		}

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
			if(press_data[3 * iter]){
				uint16_t top_left_x = press_data[(3 * iter) + 1] * 16;
				uint16_t top_left_y = press_data[(3 * iter) + 2] * 16;
				uint8_t liter = 0;
				if(!(logic_grid[press_data[(3 * iter) + 1] + grid_x * press_data[(3 * iter) + 2]] & ((1 << 7) + (1 << 6)))){	//If its not revealed/flagged
					indented_neighbours[0] = top_left_x + grid_start_x;
					indented_neighbours[1] = top_left_y + grid_start_y;
					if(logic_grid[press_data[(3 * iter) + 1] + grid_x * press_data[(3 * iter) + 2]] & (1 << 5)){	//If its question marked
						graphics_frame_coordinates(&tile_anim, indented_frames, indented_frames + 1, 6);
					}
					else{
						graphics_frame_coordinates(&tile_anim, indented_frames, indented_frames + 1, 7);
					}
					liter = 1;
				}
				if(press_data[3 * iter] == 2){	//For the X press
					uint8_t valids = neighbouring_tiles(press_data[(3 * iter) + 1], press_data[(3 * iter) + 2]);	//Get the tiles that are in bounds
					for(jiter = 0; jiter < 8; jiter++){
						if(!(valids & (1 << jiter))){	//If out of bounds
							continue;
						}
						int8_t x_variant = 0;
						if(jiter == 0 || jiter == 3 || jiter == 5){
							x_variant = -1;
						}
						else if(jiter == 2 || jiter == 4 || jiter == 7){
							x_variant = 1;
						}
						int8_t y_variant = 0;
						if(jiter < 3){
							y_variant = -1;
						}
						else if(jiter > 4){
							y_variant = 1;
						}
						if(logic_grid[press_data[(3 * iter) + 1] + x_variant + ((press_data[(3 * iter) + 2] + y_variant) * grid_x)] & ((1 << 7) + (1 << 6))){
							continue;
						}
						indented_neighbours[2 * liter] = top_left_x + (16 * x_variant) + grid_start_x;
						indented_neighbours[(2 * liter) + 1] = top_left_y + (16 * y_variant) + grid_start_y;
						if(logic_grid[press_data[(3 * iter) + 1] + x_variant + ((press_data[(3 * iter) + 2] + y_variant) * grid_x)] & (1 << 5)){	//Question mark
							graphics_frame_coordinates(&tile_anim, indented_frames + (2 * liter), indented_frames + (2 * liter) + 1, 6);
						}
						else{
							graphics_frame_coordinates(&tile_anim, indented_frames + (2 * liter), indented_frames + (2 * liter) + 1, 7);	//Blank
						}
						liter++;
					}
				}
				graphics_draw_sprites_OLD(&tile_ss, &tile_anim, indented_neighbours, indented_frames, 2 * liter, liter, 18, 1, 1, !operating_system && language);
			}
		}

		//Drawing the highlight boxes
		if(menus_selected & (1 << 0)){
			graphics_draw_untextured_poly(5, os.variant_pos[1] - 3, 19, 34, 16, 0x40FFFFFF, 0);	//Font is layer 20
		}
		if(menus_selected & (1 << 1)){
			graphics_draw_untextured_poly(44, os.variant_pos[1] - 3, 19, 44, 16, 0x40FFFFFF, 0);
		}
		if(menus_selected & (1 << 2)){
			graphics_draw_untextured_poly(93, os.variant_pos[1] - 3, 19, 47, 16, 0x40FFFFFF, 0);
		}
		if(menus_selected & (1 << 3)){
			graphics_draw_untextured_poly(145, os.variant_pos[1] - 3, 19, 36, 16, 0x40FFFFFF, 0);
		}

		pvr_list_finish();

		//None of these need to be transparent, by using the opaque list we are making the program more efficient
		pvr_list_begin(PVR_LIST_OP_POLY);

		//Draw the grid's boarder
		if(focus <= 1){
			custom_poly_boarder(3, grid_start_x, grid_start_y, 16, grid_x * 16, grid_y * 16, 4286611584u, 4294967295u);
		}
		else{
			if(focus > 1){
				graphics_draw_untextured_poly(6, 98, 10, 631, 1, 0xFFA0A0A0, 1);
				graphics_draw_untextured_poly(6, 99, 10, 631, 350, 0xFFD4D0C8, 1);
			}
			if(focus == 2){	//Add in the number changer textures here too when I can draw SS in Opaque
				//Draw all 3 untextured poly number boxes
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

		if(!operating_system){
			custom_poly_2000_topbar(3, 3, 15, 634, 18);	//Colour bar for Windows 2000
			custom_poly_2000_boarder(0, 0, 1, 640, 452);	//The Windows 2000 window boarder
		}
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Confirm everything was unloaded successfully (Should equal zero) This code is never triggered under normal circumstances
	//I'm probs forgetting a few things such as the cursor palettes
	int retVal = 0;
	retVal += crayon_memory_free_crayon_spritesheet(&Board, 1);
	retVal += crayon_memory_free_crayon_spritesheet(&Icons, 1);
	retVal += crayon_memory_free_crayon_spritesheet(&Windows, 1);
	retVal += crayon_memory_free_mono_font_sheet(&BIOS_font, 1);
	retVal += crayon_memory_free_prop_font_sheet(&Tahoma_font, 1);
	retVal += crayon_memory_free_palette(White_Tahoma_Font);
	free(White_Tahoma_Font);
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
