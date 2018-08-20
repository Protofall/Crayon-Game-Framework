//Crayon libraries
#include "../../Crayon/code/crayon/dreamcast/memory.h"

#include "extra_structs.h"
#include "setup.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/maple/mouse.h>

//For the timer
#include <arch/timer.h>

//To get region info (Not sure if this is needed)
#include <dc/flashrom.h>

uint16_t non_mines_left;	//When this variable equals zero, the game is won
int num_flags;	//More like "number of flags in the pool"

uint8_t over_mode = 0;	//0 = ready for new game, 1 = loss (ded), 2 = win
uint8_t game_live = 0;	//Is true when the timer is turning
uint8_t revealed;
int time_sec;

uint8_t question_enabled = 1;	//Enable the use of question marking
uint8_t sound_enabled = 0;
uint8_t operating_system = 1;	//0 for 2000, 1 for XP
uint8_t language = 0;	//0 for English, 1 for Italian. This also affects the Minesweeper/Prato fiorito themes

uint8_t grid_x;
uint8_t grid_y;
uint16_t grid_start_x;
uint16_t grid_start_y;
uint16_t num_mines;
uint8_t first_reveal;

uint8_t *logic_grid;
uint16_t *coord_grid;	//Unless changing grid size, this won't need to be changed once set
uint16_t *frame_grid;

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
void clear_grid(animation_t * anim){
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
//0 = ready for new game, 1 = loss (ded), 2 = win
void reveal_map(animation_t * anim){
	int i;
	if(over_mode == 1){
		for(i = 0; i < grid_x * grid_y; i++){
			if(logic_grid[i] == 9 || logic_grid[i] == 41){	//Untouched or question marked
				graphics_frame_coordinates(anim, frame_grid + (2 * i), frame_grid + (2 * i) + 1, 3);
			}
			if(logic_grid[i] != 73 && logic_grid[i] & 1<<6){	//Untouched or question marked
				graphics_frame_coordinates(anim, frame_grid + (2 * i), frame_grid + (2 * i) + 1, 5);
			}
		}
	}
	else if(over_mode == 2){
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

void discover_tile(animation_t * anim, int ele_x, int ele_y){
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

void x_press(animation_t * anim, int ele_x, int ele_y){
	int ele_logic = ele_x + grid_x * ele_y;
	if((logic_grid[ele_logic] & 1<<7)){	//If revealed

		uint8_t valids = neighbouring_tiles(ele_x, ele_y);
		uint8_t flag_sum = 0;	//A tiles value

		if((valids & (1 << 0)) && logic_grid[ele_x - 1 + ((ele_y- 1) * grid_x)] & (1 << 6)){flag_sum++;}		//Top Left
		if((valids & (1 << 1)) && logic_grid[ele_x + ((ele_y - 1) * grid_x)]  & (1 << 6)){flag_sum++;}		//Top centre
		if((valids & (1 << 2)) && logic_grid[ele_x + 1 + ((ele_y - 1) * grid_x)]  & (1 << 6)){flag_sum++;}	//Top right
		if((valids & (1 << 3)) && logic_grid[ele_x - 1 + (ele_y * grid_x)]  & (1 << 6)){flag_sum++;}			//Mid Left
		if((valids & (1 << 4)) && logic_grid[ele_x + 1 + (ele_y * grid_x)]  & (1 << 6)){flag_sum++;}			//Mid Right
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
void b_press(animation_t * anim, uint16_t ele_logic){
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
void digit_display(spritesheet_t * ss, animation_t * anim, int num, uint16_t x, uint16_t y){
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
		graphics_draw_sprite(ss, anim, x + (i * 13), y, 5, 1, 1, frame_x, frame_y, 0);
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
			&& cursor_position[2 * id] >= 307 && cursor_position[(2 * id) + 1] >= 64){	//If hovering over face
			*face_frame_id = 4;
		}
	}
}

//Handles the interaction logic with the grid
void grid_ABX_logic(int ele_x, int ele_y, uint8_t button_action, animation_t *tile_anim, uint32_t *start_time, uint32_t *start_ms_time){
	if(button_action & (1 << 0)){	//For A press
		if(over_mode == 0){
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

uint8_t face_press_logic(uint8_t button_action, int id, float *cursor_position, uint8_t *face_frame_id, animation_t * tile_anim,
	uint32_t *previous_buttons, uint32_t buttons){
	if((button_action & (1 << 0)) && (cursor_position[2 * id] <= 307 + 26) && (cursor_position[(2 * id) + 1] <= 64 + 26)
		&& cursor_position[2 * id] >= 307 && cursor_position[(2 * id) + 1] >= 64){	//If face is released on
		clear_grid(tile_anim);
		previous_buttons[id] = buttons;	//Store the previous button press
		*face_frame_id = 0;	//Is this redundant? The grid has been reset, but if someone is still holding A or X then it should still be the same as before
		return 1;
	}
	return 0;
}

int main(){
	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB and we default to NTSC interlace (Make a 50/60 Hz menu later). This handles composite
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}

	pvr_init_defaults();

	srand(time(0));

	float cursor_position[8];	//Update these to the top (Inline with the face (face is 26x26 and starts at 307, 64 (In Windows)))
	cursor_position[0] = 100;
	cursor_position[1] = 77;
	cursor_position[2] = 200;
	cursor_position[3] = 77;
	cursor_position[4] = 414;
	cursor_position[5] = 77;
	cursor_position[6] = 524;
	cursor_position[7] = 77;

	spritesheet_t Board, Windows;
	crayon_untextured_array_t Grid_polys, Bg_polys;	//Contains most, if not all, of the untextured polys that will be drawn.
	//crayon_untextured_array_t Win2000_polys;

	memory_mount_romdisk("/cd/Minesweeper.img", "/Minesweeper");
	memory_load_crayon_packer_sheet(&Board, "/Minesweeper/Board.dtex");
	fs_romdisk_unmount("/Minesweeper");

	//Load the OS assets
	if(operating_system){
		memory_mount_romdisk("/cd/XP.img", "/XP");
		memory_load_crayon_packer_sheet(&Windows, "/XP/Windows.dtex");
		fs_romdisk_unmount("/XP");
	}
	else{
		memory_mount_romdisk("/cd/2000.img", "/2000");
		memory_load_crayon_packer_sheet(&Windows, "/2000/Windows.dtex");
		fs_romdisk_unmount("/2000");
	}

	//Make the OS struct and populate it
	MinesweeperOS_t os;
	setup_OS_assets(&os, &Windows, operating_system, language);

	//Beginner
	// grid_x = 9;
	// grid_y = 9;
	// num_mines = 10;

	//Intermediate
	// grid_x = 16;
	// grid_y = 16;
	// num_mines = 40;

	//Expert
	grid_x = 30;
	grid_y = 20;
	num_mines = 99;

	//Largest	(Tinker with this one)
	// grid_x = 40;
	// grid_y = 21;
	// num_mines = 130;

	grid_start_x = 80;
	grid_start_y = 104;	//Never changes for XP mode, but might in 2000

	//Setup the untextured poly structs
	setup_grid_untextured_poly(&Grid_polys, grid_x, grid_y, grid_start_x, grid_start_y);	//Note the struct will need to be updated when grid_x and grid_y are updated since it will store grid boarders
	setup_bg_untextured_poly(&Bg_polys);

	uint16_t grid_size = grid_x * grid_y;

	logic_grid = (uint8_t *) malloc(grid_size * sizeof(uint8_t));
	coord_grid = (uint16_t *) malloc(2 * grid_size * sizeof(uint16_t));
	frame_grid = (uint16_t *) malloc(2 * grid_size * sizeof(uint16_t));

	int iter;
	int jiter;

	//These two just allow me to easily change between Minesweeper and Prato fiorito
	spritesheet_t tile_ss;
	animation_t tile_anim;

	uint8_t tile_id = 5;
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

	for(jiter = 0; jiter < grid_y; jiter++){
		for(iter = 0; iter < grid_x; iter++){	//iter is x, jiter is y
			uint16_t ele = (jiter * grid_x * 2) + (2 * iter);
			coord_grid[ele] = grid_start_x + (iter * 16);
			coord_grid[ele + 1] = grid_start_y + (jiter * 16);
		}
	}

	//Set the grid's initial values
	clear_grid(&tile_anim);

	//The face frame coords
	uint16_t face_frame_x;
	uint16_t face_frame_y;
	uint8_t face_frame_id = 0;	//0 normal, 1 suprised, 2 ded, 3 sunny, 4 indented

	//Cursor Player icon frame coords
	uint16_t p_frame_x = 0;
	uint16_t p_frame_y = 0;
	uint8_t player_active = 0;	//Used to confirm if a controller is being used
	graphics_frame_coordinates(&Board.spritesheet_animation_array[3], &p_frame_x, &p_frame_y, 0);

	//For the timer
	uint32_t current_time = 0;
	uint32_t current_ms_time = 0;
	uint32_t start_time = 0;
	uint32_t start_ms_time = 0;

	//For the "start to reset"
	uint8_t start_primed = 0;	//Format -PVA 4321 where the numbers are if the player is holding either nothing or just start,
								//A somewhere start is being pressed. V is invalidated, P is someone pressed combo

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
	graphics_frame_coordinates(&Board.spritesheet_animation_array[4], &region_icon_x, &region_icon_y, region);

	float thumb_x = 0;
	float thumb_y = 0;
	uint8_t thumb_active;

	while(1){		
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		if(st->joyx > 0){	//Converting from -128, 127 to -1, 1
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

		if(face_press_logic(button_action, __dev->port, cursor_position, &face_frame_id, &tile_anim, previous_buttons, st->buttons)){break;}

		//These are only ever set if The cursor is on the grid and A/B/X is/was pressed
		int ele_x = -1;
		int ele_y = -1;
		uint8_t in_grid = 0;

		cursor_on_grid(&in_grid, &ele_x, &ele_y, button_action, __dev->port, cursor_position);
		if(in_grid){grid_ABX_logic(ele_x, ele_y, button_action, &tile_anim, &start_time, &start_ms_time);}

		//Y and Start press code
		if(button_action & (1 << 3)){
			player_movement ^= (1 << __dev->port);
		}
		if(button_action & (1 << 4)){
			start_primed |= (1 << 4);	//A bit
		}

		//Movement code
		if(player_movement & (1 << __dev->port)){	//D-Pad
			if(st->buttons & CONT_DPAD_UP){
				cursor_position[(2 * __dev->port) + 1] -= 2;
				if(cursor_position[(2 * __dev->port) + 1] < 0){
					cursor_position[(2 * __dev->port) + 1] = 0;
				}
			}
			if(st->buttons & CONT_DPAD_DOWN){
				cursor_position[(2 * __dev->port) + 1] += 2;
				if(cursor_position[(2 * __dev->port) + 1] > 480){
					cursor_position[(2 * __dev->port) + 1] = 480;
				}
			}
			if(st->buttons & CONT_DPAD_LEFT){
				cursor_position[2 * __dev->port] -= 2;
				if(cursor_position[2 * __dev->port] < 0){
					cursor_position[2 * __dev->port] = 0;
				}
			}
			if(st->buttons & CONT_DPAD_RIGHT){
				cursor_position[2 * __dev->port] += 2;
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

		//Face logic code and indented blank tiles
		face_logic(&face_frame_id, __dev->port, cursor_position, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)));
		grid_indent_logic(ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (CONT_A)), !!(st->buttons & (CONT_X)), !!(st->buttons & (CONT_B)), 0);

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

		if(face_press_logic(button_action, __dev->port, cursor_position, &face_frame_id, &tile_anim, previous_buttons, st->buttons)){break;}

		int ele_x = -1;
		int ele_y = -1;
		uint8_t in_grid = 0;

		cursor_on_grid(&in_grid, &ele_x, &ele_y, button_action, __dev->port, cursor_position);
		if(in_grid){grid_ABX_logic(ele_x, ele_y, button_action, &tile_anim, &start_time, &start_ms_time);}

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
		grid_indent_logic(ele_x, ele_y, in_grid, __dev->port, press_data, !!(st->buttons & (MOUSE_LEFTBUTTON)), !!(st->buttons & (MOUSE_SIDEBUTTON)), !!(st->buttons & (MOUSE_RIGHTBUTTON)), 1);

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

		//X011 0000
		if(!(start_primed & (1 << 6)) && (start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			start_primed = 0;
			face_frame_id = 0;
		}

		//X001 0000
		if(!(start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && (start_primed & (1 << 4)) && !(start_primed % (1 << 4))){
			clear_grid(&tile_anim);
			start_primed = 0;
			face_frame_id = 0;
		}

		start_primed = start_primed & ((1 << 5) + (1 << 4));	//Clears all bits except active and invalidness

		if(non_mines_left == 0 && game_live && over_mode == 0){
			game_live = 0;
			over_mode = 2;
		}

		//Right now this is always triggered when a game ends and thanks to "revealed" it only triggers once
		if(!revealed && !game_live && over_mode != 0){
			reveal_map(&tile_anim);
		}

		//The face frame id code. If not indented or suprised, then choose a face
		if(!face_frame_id){
			if(over_mode == 1){
				//play bomb/death sound
				face_frame_id = 2;
			}
			else if(over_mode == 2){
				//play "won" sound
				face_frame_id = 3;
			}
		}

		graphics_frame_coordinates(&Board.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, face_frame_id);
		face_frame_id = 0;	//Reset face for new frame

		timer_ms_gettime(&current_time, &current_ms_time);
		if(game_live && time_sec < 999){	//Prevent timer overflows
			//Play the "tick" sound effect
			time_sec = current_time - start_time + (current_ms_time > start_ms_time); //MS is there to account for the "1st second" inaccuracy
		}

		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

		//Setup the main palette
		graphics_setup_palette(0, &Board);
		if(!operating_system){	//Since it uses palettes and XP doesn't, we do this
			graphics_setup_palette(1, &Windows);
		}

		//Draw windows graphics using our MinesweeperOpSys struct
		for(iter = 0; iter < os.sprite_count; iter++){
			if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "aboutLogo")){	//We don't want to draw that here so we skip
				continue;
			}
			graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[os.ids[iter]],
				os.coords_pos[3 * iter], os.coords_pos[(3 * iter) + 1], os.coords_pos[(3 * iter) + 2],
				os.scale[2 * iter], os.scale[(2 * iter) + 1], os.coords_frame[2 * iter], os.coords_frame[(2 *iter) + 1], 1);
			//We choose palette 1 because that's 2000's palette and XP uses RGB565
		}

		//Draw the region icon
		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[4], 553, 456, 5, 1, 1, region_icon_x, region_icon_y, 0);

		//Draw the reset button face
		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[1], 307, 64, 3, 1, 1, face_frame_x, face_frame_y, 0);

		//Draw the flag count and timer
		digit_display(&Board, &Board.spritesheet_animation_array[2], num_flags, 20, 65);
		digit_display(&Board, &Board.spritesheet_animation_array[2], time_sec, 581, 65);

		//Draw the background stuff
		graphics_draw_untextured_array(&Bg_polys);

		//Draw the grid/digit display related untextured polys
		graphics_draw_untextured_array(&Grid_polys);

		//Draw the grid
		graphics_draw_sprites_OLD(&tile_ss, &tile_anim, coord_grid, frame_grid, 2 * grid_size, grid_size, 4, 1, 1, !operating_system && language);

		//Draw the indented tiles ontop of the grid and the cursors themselves
		for(iter = 0; iter < 4; iter++){
			if(player_active & (1 << iter)){
				//Passing coords as ints because otherwise we can get case where each pixel contains more than 1 texel
				graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[0], (int) cursor_position[2 * iter], (int) cursor_position[(2 * iter) + 1], 10, 1, 1, 0, 0, 0);
				graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[3], (int) cursor_position[2 * iter] + 5, (int) cursor_position[(2 * iter) + 1],
					11, 1, 1, p_frame_x, p_frame_y + (iter * 10), 0);
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
				graphics_draw_sprites_OLD(&tile_ss, &tile_anim, indented_neighbours, indented_frames, 2 * liter, liter, 5, 1, 1, !operating_system && language);
			}
		}
		
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Confirm everything was unloaded successfully (Should equal zero) This code is never triggered under normal circumstances
	int retVal = 0;
	retVal += memory_free_crayon_packer_sheet(&Board);
	retVal += memory_free_crayon_packer_sheet(&Windows);
	error_freeze("Free-ing result %d!\n", retVal);

	return 0;
}

//Add something to be displayed on the VMU screen. But what? Just a static mine/blown up mine
//When choosing an OS, make it boot up with a Dreamcast/Katana legacy BIOS
//CHANGE "Board" to "Common" and maybe "Windows" to "OS-Dependent"
