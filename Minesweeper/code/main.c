//Crayon libraries
#include "../../Crayon/code/crayon/dreamcast/memory.h"

#include "extra_structs.h"
#include "setup.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>

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
uint8_t operating_system = 0;	//0 for 2000, 1 for XP
uint8_t language = 1;	//0 for English, 1 for Italian. This also affects the Minesweeper/Prato fiorito themes

uint8_t gridX;
uint8_t gridY;
uint16_t gridStartX;
uint16_t gridStartY;
uint16_t num_mines;

uint8_t *logicGrid;
uint16_t *coordGrid;	//Unless changing grid size, this won't need to be changed once set
uint16_t *frameGrid;

// N0 N1 N2
// N3 S  N4
// N5 N6 N7
// N are neighbours, S is source (xEle, yEle)
// The returned value is in the format
// N7N6N5N4N3N2N1N0 where each bit is true if neighbour is valid
uint8_t neighbouring_tiles(int xEle, int yEle){
	uint8_t ret_val = 255;
	if(xEle == 0){
		ret_val &= ~(1 << 0);	//Clearing bits which can't be right
		ret_val &= ~(1 << 3);
		ret_val &= ~(1 << 5);
	}
	else if(xEle >= gridX - 1){
		ret_val &= ~(1 << 2);
		ret_val &= ~(1 << 4);
		ret_val &= ~(1 << 7);
	}
	if(yEle == 0){
		ret_val &= ~(1 << 0);
		ret_val &= ~(1 << 1);
		ret_val &= ~(1 << 2);
	}
	else if(yEle >= gridY - 1){
		ret_val &= ~(1 << 5);
		ret_val &= ~(1 << 6);
		ret_val &= ~(1 << 7);
	}

	return ret_val;
}

//Initially called by reset_grid where x is always a valid number
void populate_logic(int xEle, int yEle){
	int ele_logic = xEle + gridX * yEle;
	if(logicGrid[ele_logic] == 9){	//Is mine
		return;
	}

	uint8_t valids = neighbouring_tiles(xEle, yEle);
	uint8_t sum = 0;	//A tiles value

	if((valids & (1 << 0)) && logicGrid[xEle - 1 + ((yEle- 1) * gridX)] == 9){sum++;}	//Top Left
	if((valids & (1 << 1)) && logicGrid[xEle + ((yEle - 1) * gridX)] == 9){sum++;}		//Top centre
	if((valids & (1 << 2)) && logicGrid[xEle + 1 + ((yEle - 1) * gridX)] == 9){sum++;}	//Top right
	if((valids & (1 << 3)) && logicGrid[xEle - 1 + (yEle * gridX)] == 9){sum++;}		//Mid Left
	if((valids & (1 << 4)) && logicGrid[xEle + 1 + (yEle * gridX)] == 9){sum++;}		//Mid Right
	if((valids & (1 << 5)) && logicGrid[xEle - 1 + ((yEle + 1) * gridX)] == 9){sum++;}	//Bottom left
	if((valids & (1 << 6)) && logicGrid[xEle + ((yEle + 1) * gridX)] == 9){sum++;}		//Bottom centre
	if((valids & (1 << 7)) && logicGrid[xEle + 1 + ((yEle + 1) * gridX)] == 9){sum++;}	//Bottom right

	logicGrid[ele_logic] = sum;

	return;
}


uint8_t true_prob(double p){
	return rand() < p * (RAND_MAX + 1.0);
}

//Call this to reset the grid
void reset_grid(animation_t * anim, float mineProbability){
	num_flags = 0;
	uint16_t grid_size = gridX * gridY;

	int i = 0;
	int j = 0;
	uint16_t mines_left = num_mines;
	uint16_t tiles_left = grid_size;

	while(i < grid_size){
		double prob = (double)mines_left / (double)tiles_left;
		logicGrid[i] = 9 * true_prob(prob);
		// logicGrid[i] = 9 * new_prob(mines_left, tiles_left);
		if(logicGrid[i]){
			mines_left--;
			num_flags++;	//Is being set right
		}
		tiles_left--;
		i++;
	}

	for(i = 0; i < 2 * grid_size; i = i + 2){
		graphics_frame_coordinates(anim, frameGrid + i, frameGrid + i + 1, 0);
	}

	//Iterates through whole loop
	for(j = 0; j < gridY; j++){
		for(i = 0; i < gridX; i++){
			populate_logic(i, j);
		}
	}

	game_live = 0;
	over_mode = 0;
	revealed = 0;
	time_sec = 0;

	non_mines_left = grid_size - num_flags;

	return;
}

//It fills it out differently depending on over_mode
//0 = ready for new game, 1 = loss (ded), 2 = win
void reveal_map(animation_t * anim){
	int i;
	if(over_mode == 1){
		for(i = 0; i < gridX * gridY; i++){
			if(logicGrid[i] == 9 || logicGrid[i] == 41){	//Untouched or question marked
				graphics_frame_coordinates(anim, frameGrid + (2 * i), frameGrid + (2 * i) + 1, 3);
			}
			if(logicGrid[i] != 73 && logicGrid[i] & 1<<6){	//Untouched or question marked
				graphics_frame_coordinates(anim, frameGrid + (2 * i), frameGrid + (2 * i) + 1, 5);
			}
		}
	}
	else if(over_mode == 2){
		num_flags = 0;
		for(i = 0; i < gridX * gridY; i++){
			if(logicGrid[i] % (1 << 5) == 9){
				graphics_frame_coordinates(anim, frameGrid + (2 * i), frameGrid + (2 * i) + 1, 1);
			}
		}
	}
	revealed = 1;
	return;
}

void discover_tile(animation_t * anim, int xEle, int yEle){
	int ele_logic = xEle + gridX * yEle;
	if(!(logicGrid[ele_logic] & 1 << 6)){	//When not flagged
		if(logicGrid[ele_logic] & 1 << 7){	//Already discovered
			return;
		}
		int ele = ele_logic * 2;
		if(logicGrid[ele_logic] & 1 << 5){	//If questioned, remove the question mark and set it to a normal tile
			logicGrid[ele_logic] &= ~(1 << 5);
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 0);
		}
		if(logicGrid[ele_logic] == 9){	//If mine
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 4);
			game_live = 0;
			over_mode = 1;
		}
		else{
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 7 + logicGrid[ele_logic]);
			non_mines_left--;
		}
		logicGrid[ele_logic] |= (1 << 7);
		if(logicGrid[ele_logic] % (1 << 5) == 0){
			uint8_t valids = neighbouring_tiles(xEle, yEle);
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
					discover_tile(anim, xEle + xVarriant, yEle + yVariant);
				}
			}
		}
	}
	return;
}

void x_press(animation_t * anim, int xEle, int yEle){
	int ele_logic = xEle + gridX * yEle;
	if((logicGrid[ele_logic] & 1<<7)){	//If revealed

		uint8_t valids = neighbouring_tiles(xEle, yEle);
		uint8_t flag_sum = 0;	//A tiles value

		if((valids & (1 << 0)) && logicGrid[xEle - 1 + ((yEle- 1) * gridX)] & (1 << 6)){flag_sum++;}		//Top Left
		if((valids & (1 << 1)) && logicGrid[xEle + ((yEle - 1) * gridX)]  & (1 << 6)){flag_sum++;}		//Top centre
		if((valids & (1 << 2)) && logicGrid[xEle + 1 + ((yEle - 1) * gridX)]  & (1 << 6)){flag_sum++;}	//Top right
		if((valids & (1 << 3)) && logicGrid[xEle - 1 + (yEle * gridX)]  & (1 << 6)){flag_sum++;}			//Mid Left
		if((valids & (1 << 4)) && logicGrid[xEle + 1 + (yEle * gridX)]  & (1 << 6)){flag_sum++;}			//Mid Right
		if((valids & (1 << 5)) && logicGrid[xEle - 1 + ((yEle + 1) * gridX)]  & (1 << 6)){flag_sum++;}	//Bottom left
		if((valids & (1 << 6)) && logicGrid[xEle + ((yEle + 1) * gridX)]  & (1 << 6)){flag_sum++;}		//Bottom centre
		if((valids & (1 << 7)) && logicGrid[xEle + 1 + ((yEle + 1) * gridX)]  & (1 << 6)){flag_sum++;}	//Bottom right

		if(logicGrid[ele_logic] % (1 << 4) == flag_sum){
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
					discover_tile(anim, xEle + xVarriant, yEle + yVariant);
				}
			}
		}
	}
	return;
}

//If we use a flag that decrements the flag count, leaving flag will increment it
void b_press(animation_t * anim, uint16_t ele_logic){
	int ele = ele_logic * 2;
	if(!(logicGrid[ele_logic] & (1 << 7))){	//Not discovered
		uint8_t status = 0;	//0 = normal, 1 = flag, 2 = question icons
		if(logicGrid[ele_logic] & (1 << 6)){	//If flagged
			logicGrid[ele_logic] &= ~(1 << 6);	//Clears the flag bit
			if(question_enabled){
				logicGrid[ele_logic] |= (1 << 5);	//Sets the question bit
				status = 2;
			}
			num_flags++;
		}
		else{	//Not flagged, but maybe questioned
			if(logicGrid[ele_logic] & (1 << 5)){	//Normal it
				logicGrid[ele_logic] &= ~(1 << 5);
				status = 0;
			}
			else{	//Flag it
				logicGrid[ele_logic] |= (1 << 6);
				status = 1;
				num_flags--;
			}
		}
		graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, status);
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

int main(){
	vid_set_mode(DM_640x480_VGA, PM_RGB565);

	pvr_init_defaults();

	srand(time(0));

	int cursorPos[8];
	cursorPos[0] = 50;
	cursorPos[1] = 100;
	cursorPos[2] = 50;
	cursorPos[3] = 350;
	cursorPos[4] = 590;
	cursorPos[5] = 100;
	cursorPos[6] = 590;
	cursorPos[7] = 350;

	spritesheet_t Board, Windows;

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

	gridX = 30;
	gridY = 20;
	num_mines = 99;

	gridStartX = 80;
	gridStartY = 104;	//Never changes for XP mode, but might in 2000

	uint16_t gridSize = gridX * gridY;
	float mineProbability = 0.175;

	logicGrid = (uint8_t *) malloc(gridSize * sizeof(uint8_t));
	coordGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));
	frameGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));

	int iter;
	int jiter;

	//These two just allow me to easily change between Minesweeper and Prato fiorito
	spritesheet_t TileSS;
	animation_t TileANIM;

	//The commented code below is doing something very wrong...
	uint8_t tileID = 5;
	if(!language){
		TileSS = Board;
	}
	else{
		TileSS = Windows;
		for(iter = 0; iter < TileSS.spritesheet_animation_count; iter++){
			if(!strcmp(TileSS.spritesheet_animation_array[iter].animation_name, "italianTiles")){
				tileID = iter;
				break;
			}
		}
	}
	TileANIM = TileSS.spritesheet_animation_array[tileID];

	for(jiter = 0; jiter < gridY; jiter++){
		for(iter = 0; iter < gridX; iter++){	//iter is x, jiter is y
			uint16_t ele = (jiter * gridX * 2) + (2 * iter);
			coordGrid[ele] = gridStartX + (iter * 16);
			coordGrid[ele + 1] = gridStartY + (jiter * 16);
		}
	}

	//Set the grid's initial values
	reset_grid(&TileANIM, mineProbability);

	//The face frame coords
	uint16_t face_frame_x;
	uint16_t face_frame_y;
	uint8_t face_frame_id = 0;	//0 normal, 1 suprised, 2 ded, 3 sunny, 4 indented

	//Cursor Player icon frame coords
	uint16_t p_frame_x = 0;
	uint16_t p_frame_y = 0;
	uint8_t playerActive = 0;	//Used to confirm if a controller is being used
	graphics_frame_coordinates(&Board.spritesheet_animation_array[3], &p_frame_x, &p_frame_y, 0);

	//For the timer
	uint32_t currentTime = 0;
	uint32_t currentMSTime = 0;
	uint32_t startTime = 0;
	uint32_t startMSTime = 0;

	//For the "start to reset"
	uint32_t sButtonTime = 0;
	uint32_t sButtonMSTime = 0;
	uint8_t start_primed = 0;	//Format -PVA 4321 where the numbers are if the player is holding either nothing or just start,
								//A is active. V is invalidated, P is someone pressed combo

	//Stores the button combination from the previous button press
	uint32_t prevButtons[4] = {0};
	uint8_t buttonAction = 0;	// ---S YBXA format, each maple loop the variable is overritten with released statuses

	//1st element is type of click, 2nd element is x, 3rd element is y and repeat for all 4 controllers
	//1st, 0 = none, 1 = A, 2 = X, X press has priority over A press
	uint16_t pressData[12] = {0};
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

	//Set colours
	uint32_t mainBackground = (255 << 24) | (192 << 16) | (192 << 8) | 192;	//4290822336
	uint32_t lightGrey = (255 << 24) | (128 << 16) | (128 << 8) | 128;	//4286611584
	uint32_t white = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295

	/*
	Depth plan:

	Solution 1:
		- 6 boxes, 2 big boxes like the digit displays, 4 "single pixel" boxes for overlap

	Solution 2:
		- 5 boxes, but 4 of them are partially transparent. their colour + their alpha + the bg colour equals what they should appear as
		- The white box is the same, but there are 4 semi-transparent grey boxes...Is this worth it?

	Might be simpler and easier to read by going with Solution 1 (Also make a function that given a box and z value and width,
		it can create the boarders?)

	*/

	while(1){		
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)


		if(start_primed && st->buttons != CONT_START && (st->buttons & CONT_START)){	//Player is pressing inpure start
			start_primed |= (1 << (__dev->port));	//Number bits
		}

		if(st->buttons == CONT_START){	//Atleast someone is only pressing start
			start_primed |= (1 << 6);	//P bit
		}

		if(!(playerActive & (1 << __dev->port))){	//Player is there, but hasn't been activated yet
			if(st->buttons != 0){	//Input detected
				playerActive |= (1 << __dev->port);
			}
			else{
				continue;
			}
		}

		//Use the buttons previously pressed to control what happens here
		buttonAction = 0;
		buttonAction |= ((prevButtons[__dev->port] & CONT_A) && !(st->buttons & CONT_A)) << 0;
		buttonAction |= ((prevButtons[__dev->port] & CONT_X) && !(st->buttons & CONT_X) && !over_mode) << 1;
		buttonAction |= (!(prevButtons[__dev->port] & CONT_B) && (st->buttons & CONT_B) && !over_mode) << 2;
		buttonAction |= (!(prevButtons[__dev->port] & CONT_Y) && (st->buttons & CONT_Y) && !over_mode) << 3;
		buttonAction |= ((start_primed & (1 << 6)) && !(start_primed & (1 << 5)) && !(start_primed & (1 << 4))) << 4;	//If we press start, but we haven't done active prime yet and we aren't invalidated

		//These two are only ever set if The cursor is in the grid and A/B/X is/was pressed
		int xEle = -1;
		int yEle = -1;
		uint8_t inGrid = 0;
		//Need to know if in the grid when releasing A/X, pressing B and when holding A/X
		if((buttonAction % (1 << 3)) || (st->buttons & (CONT_A + CONT_X))){
			inGrid = (cursorPos[2 * __dev->port] < gridStartX + (gridX * 16)) && (cursorPos[(2 * __dev->port) + 1] < gridStartY + (gridY * 16))
				&& cursorPos[2 * __dev->port] >= gridStartX && cursorPos[(2 * __dev->port) + 1] >= gridStartY;
			if(inGrid){
				xEle = (cursorPos[2 * __dev->port]  - gridStartX) / 16;
				yEle = (cursorPos[(2 * __dev->port) + 1]  - gridStartY) / 16;
			}
		}

		//Note that I think the press logic might be off. Going on a diagonal rapidly pressing B sometimes does nothing
		//This block does the A, B, X and Y press/release stuff
		if(buttonAction % (1 << 3) != 0){
			if((buttonAction & (1 << 0)) && (cursorPos[2 * __dev->port] <= 307 + 26) && (cursorPos[(2 * __dev->port) + 1] <= 64 + 26)
				&& cursorPos[2 * __dev->port] >= 307 && cursorPos[(2 * __dev->port) + 1] >= 64){	//If face is released on
				reset_grid(&TileANIM, mineProbability);
				prevButtons[__dev->port] = st->buttons;	//Store the previous button press
				face_frame_id = 0;
				break;	//Since we don't want these old presses interacting with the new board
			}
			if(buttonAction % (1 << 3) && xEle != -1 && yEle != -1){	//If on the grid with an A/B/X press
				if(buttonAction & (1 << 0)){	//For A press
					if(over_mode == 0){
						if(!game_live){
							timer_ms_gettime(&startTime, &startMSTime);
							game_live = 1;
						}
						discover_tile(&TileANIM, xEle, yEle);
					}
				}
				if(buttonAction & (1 << 1)){	//For X press
					if(!game_live){
						timer_ms_gettime(&startTime, &startMSTime);
						game_live = 1;
					}
					x_press(&TileANIM, xEle, yEle);
				}
				if(buttonAction & (1 << 2)){	//For B press
					b_press(&TileANIM, xEle + gridX * yEle);
				}
			}
		}
		else if(buttonAction){
			if(buttonAction & (1 << 3)){	//For Y press
				;
			}
			if(buttonAction & (1 << 4)){	//Valid start press starts a mini-timer for the reset
				start_primed |= (1 << 4);	//A bit
				timer_ms_gettime(&sButtonTime, &sButtonMSTime);
			}
		}

		//Movement code
		if(st->buttons & CONT_DPAD_UP){
			cursorPos[(2 * __dev->port) + 1] -= 2;
			if(cursorPos[(2 * __dev->port) + 1] < 0){
				cursorPos[(2 * __dev->port) + 1] = 0;
			}
		}
		if(st->buttons & CONT_DPAD_DOWN){
			cursorPos[(2 * __dev->port) + 1] += 2;
			if(cursorPos[(2 * __dev->port) + 1] > 480){
				cursorPos[(2 * __dev->port) + 1] = 480;
			}
		}
		if(st->buttons & CONT_DPAD_LEFT){
			cursorPos[2 * __dev->port] -= 2;
			if(cursorPos[2 * __dev->port] < 0){
				cursorPos[2 * __dev->port] = 0;
			}
		}
		if(st->buttons & CONT_DPAD_RIGHT){
			cursorPos[2 * __dev->port] += 2;
			if(cursorPos[2 * __dev->port] > 640){
				cursorPos[2 * __dev->port] = 640;
			}
		}

		//Face logic code and indented blank tiles
		if((st->buttons & (CONT_A + CONT_X))){
			if(!over_mode && !face_frame_id){
				face_frame_id = 1;	//Apply suprised face
			}
			if((st->buttons & CONT_A) && (cursorPos[2 * __dev->port] <= 307 + 26) && (cursorPos[(2 * __dev->port) + 1] <= 64 + 26)
				&& cursorPos[2 * __dev->port] >= 307 && cursorPos[(2 * __dev->port) + 1] >= 64){	//If hovering over face
				face_frame_id = 4;
			}
			if(xEle >= 0 && yEle >= 0 && inGrid){	//This code is only supposed to trigger if A/X is pressed and its over the grid
				if(st->buttons & (CONT_X)){	//X press has priority
					pressData[3 * __dev->port] = 2;
				}
				else{
					pressData[3 * __dev->port] = 1;
				}
				pressData[(3 * __dev->port) + 1] = xEle;
				pressData[(3 * __dev->port) + 2] = yEle;
			}
			else{
				pressData[(3 * __dev->port)] = 0;
			}
			if(over_mode != 0){
				pressData[3 * __dev->port] = 0;
			}
		}
		else{
			pressData[3 * __dev->port] = 0;
		}

		prevButtons[__dev->port] = st->buttons;	//Store the previous button press
		
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
			reset_grid(&TileANIM, mineProbability);
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
			reveal_map(&TileANIM);
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

		timer_ms_gettime(&currentTime, &currentMSTime);
		if(game_live && time_sec < 999){	//Prevent timer overflows
			//Play the "tick" sound effect
			time_sec = currentTime - startTime + (currentMSTime > startMSTime); //MS is there to account for the "1st second" inaccuracy
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
			// if(!strcmp(Windows.spritesheet_animation_array[iter].animation_name, "langIcon")){	//We don't want to draw that here so we skip
			// 	error_freeze("Data: %d, %d, %d", os.coords_pos[3 * iter], os.coords_pos[(3 * iter) + 1], os.coords_pos[(3 * iter) + 2]);
			// }
			graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[os.ids[iter]],
				os.coords_pos[3 * iter], os.coords_pos[(3 * iter) + 1], os.coords_pos[(3 * iter) + 2],
				os.scale[2 * iter], os.scale[(2 * iter) + 1], os.coords_frame[2 * iter], os.coords_frame[(2 *iter) + 1], 1);
			//We choose palette 1 because that's 2000's palette and XP uses RGB565
		}

		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[4], 553, 455, 4, 1, 1, region_icon_x, region_icon_y, 0);	//Region icon

		//Draw the reset button face
		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[1], 307, 64, 2, 1, 1, face_frame_x, face_frame_y, 0);

		//Draw the flag count and timer
		digit_display(&Board, &Board.spritesheet_animation_array[2], num_flags, 20, 65);
		digit_display(&Board, &Board.spritesheet_animation_array[2], time_sec, 581, 65);

		//Draw the main background colour
		graphics_draw_colour_poly(0, 0, 1, 640, 480, mainBackground);

		//Depth for digit displays
		graphics_draw_colour_poly(19, 64, 4, 40, 24, lightGrey);
		graphics_draw_colour_poly(21, 66, 4, 40, 24, white);
		graphics_draw_colour_poly(580, 64, 4, 40, 24, lightGrey);
		graphics_draw_colour_poly(582, 66, 4, 40, 24, white);

		//Draw the grid
		graphics_draw_sprites_OLD(&TileSS, &TileANIM, coordGrid, frameGrid, 2 * gridSize, gridSize, 2, 1, 1, !operating_system && language);

		//Draw the indented tiles ontop of the grid and the cursors themselves
		for(iter = 0; iter < 4; iter++){
			if(playerActive & (1 << iter)){
				graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[0], cursorPos[2 * iter], cursorPos[(2 * iter) + 1], 10, 1, 1, 0, 0, 0);
				graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[3], cursorPos[2 * iter] + 5,
					cursorPos[(2 * iter) + 1], 11, 1, 1, p_frame_x, p_frame_y + (iter * 10), 0);
			}
			if(pressData[3 * iter]){
				uint16_t top_left_x = pressData[(3 * iter) + 1] * 16;
				uint16_t top_left_y = pressData[(3 * iter) + 2] * 16;
				uint8_t liter = 0;
				if(!(logicGrid[pressData[(3 * iter) + 1] + gridX * pressData[(3 * iter) + 2]] & ((1 << 7) + (1 << 6)))){	//If its not revealed/flagged
					indented_neighbours[0] = top_left_x + gridStartX;
					indented_neighbours[1] = top_left_y + gridStartY;
					if(logicGrid[pressData[(3 * iter) + 1] + gridX * pressData[(3 * iter) + 2]] & (1 << 5)){	//If its question marked
						graphics_frame_coordinates(&TileANIM, indented_frames, indented_frames + 1, 6);
					}
					else{
						graphics_frame_coordinates(&TileANIM, indented_frames, indented_frames + 1, 7);
					}
					liter = 1;
				}
				if(pressData[3 * iter] == 2){	//For the X press
					uint8_t valids = neighbouring_tiles(pressData[(3 * iter) + 1], pressData[(3 * iter) + 2]);
					for(jiter = 0; jiter < 8; jiter++){
						if(!(valids & (1 << jiter))){	//If out of bounds
							continue;
						}
						int8_t xVariant = 0;
						if(jiter == 0 || jiter == 3 || jiter == 5){
							xVariant = -1;
						}
						else if(jiter == 2 || jiter == 4 || jiter == 7){
							xVariant = 1;
						}
						int8_t yVariant = 0;
						if(jiter < 3){
							yVariant = -1;
						}
						else if(jiter > 4){
							yVariant = 1;
						}
						if(logicGrid[pressData[(3 * iter) + 1] + xVariant + ((pressData[(3 * iter) + 2] + yVariant) * gridX)] & ((1 << 7) + (1 << 6))){
							continue;
						}
						indented_neighbours[2 * liter] = top_left_x + (16 * xVariant) + gridStartX;
						indented_neighbours[(2 * liter) + 1] = top_left_y + (16 * yVariant) + gridStartY;
						if(logicGrid[pressData[(3 * iter) + 1] + xVariant + ((pressData[(3 * iter) + 2] + yVariant) * gridX)] & (1 << 5)){
							graphics_frame_coordinates(&TileANIM, indented_frames + (2 * liter), indented_frames + (2 * liter) + 1, 6);
						}
						else{
							graphics_frame_coordinates(&TileANIM, indented_frames + (2 * liter), indented_frames + (2 * liter) + 1, 7);
						}
						liter++;
					}
				}
				graphics_draw_sprites_OLD(&TileSS, &TileANIM, indented_neighbours, indented_frames, liter, liter, 3, 1, 1, !operating_system && language);
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

/*

Things about Minesweeper:

	Difficulties:
	- Beginner
	- Intermediate
	- Expert
	- Legacy User (Largest grid)



Stuff to implement
- Do you automatically get a free lake? In XP no, but I'll have a setting for free lake

*/

//Add something to be displayed on the VMU screen. But what? Just a static mine/blown up mine




//Set first n tiles in logic array as mines, then "Shuffle" it to "populate" it "nicely"

//Ideas: When choosing an OS, make it boot up with a Dreamcast/Katana legacy BIOS

//CHANGE "Board" to "Common" and maybe "Windows" to "OS-Dependent"


//Can you B-press a number to flag all neighbour tiles if the num of unrevealed neighbours = number?
