//Crayon libraries
#include "../../Crayon/code/crayon/dreamcast/memory.h"
#include "../../Crayon/code/crayon/dreamcast/debug.h"
#include "../../Crayon/code/crayon/dreamcast/graphics.h"

#include "extra_structs.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>

//For the timer
#include <arch/timer.h>

//To get region info (Not sure if this is needed)
#include <dc/flashrom.h>

uint16_t nonMinesLeft;	//When this variable equals zero, the game is won
int numFlags;	//More like "number of flags in the pool"

uint8_t overMode = 0;	//0 = ready for new game, 1 = loss (ded), 2 = win
uint8_t revealed;
int timeSec;
uint8_t gameLive = 0;	//Is true when the timer is turning

uint8_t questionEnabled = 1;	//Enable the use of question marking
uint8_t soundEnabled = 0;
uint8_t operatingSystem = 1;	//0 for 2000, 1 for XP
uint8_t language = 0;	//0 for English, 1 for Italian. This also affects the Minesweeper/Prato fiorito themes
uint8_t freeLake = 0;	//Initial click generates free lake

uint8_t gridX;
uint8_t gridY;
uint16_t gridStartX;
uint16_t gridStartY;

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
	uint8_t retVal = 255;
	if(xEle == 0){
		retVal &= ~(1 << 0);	//Clearing bits which can't be right
		retVal &= ~(1 << 3);
		retVal &= ~(1 << 5);
	}
	else if(xEle >= gridX - 1){
		retVal &= ~(1 << 2);
		retVal &= ~(1 << 4);
		retVal &= ~(1 << 7);
	}
	if(yEle == 0){
		retVal &= ~(1 << 0);
		retVal &= ~(1 << 1);
		retVal &= ~(1 << 2);
	}
	else if(yEle >= gridY - 1){
		retVal &= ~(1 << 5);
		retVal &= ~(1 << 6);
		retVal &= ~(1 << 7);
	}

	return retVal;
}

//Initially called by reset_grid where x is always a valid number
void populate_logic(int xEle, int yEle){
	int eleLogic = xEle + gridX * yEle;
	if(logicGrid[eleLogic] == 9){	//Is mine
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

	logicGrid[eleLogic] = sum;

	return;
}


uint8_t true_prob(double p){
	return rand() < p * (RAND_MAX + 1.0);
}

//Call this to reset the grid
void reset_grid(animation_t * anim, float mineProbability){
	numFlags = 0;
	int i;
	int j;
	uint16_t gridSize = gridX * gridY;
	for(i = 0; i < gridSize; i++){
		logicGrid[i] = 9 * true_prob(mineProbability);	//I think 0 is safe, 9 is mine
		if(logicGrid[i] == 9){
			numFlags++;
		}
	}

	for(i = 0; i < 2 * gridSize; i = i + 2){
		graphics_frame_coordinates(anim, frameGrid + i, frameGrid + i + 1, 0);
	}

	//Iterates through whole loop
	for(j = 0; j < gridY; j++){
		for(i = 0; i < gridX; i++){
			populate_logic(i, j);
		}
	}

	gameLive = 0;
	overMode = 0;
	revealed = 0;

	timeSec = 0;
	nonMinesLeft = gridSize - numFlags;

	return;
}

//It fills it out differently depending on overMode
//0 = ready for new game, 1 = loss (ded), 2 = win
void reveal_map(animation_t * anim){
	int i;
	if(overMode == 1){
		for(i = 0; i < gridX * gridY; i++){
			if(logicGrid[i] == 9 || logicGrid[i] == 41){	//Untouched or question marked
				graphics_frame_coordinates(anim, frameGrid + (2 * i), frameGrid + (2 * i) + 1, 3);
			}
			if(logicGrid[i] != 73 && logicGrid[i] & 1<<6){	//Untouched or question marked
				graphics_frame_coordinates(anim, frameGrid + (2 * i), frameGrid + (2 * i) + 1, 5);
			}
		}
	}
	else if(overMode == 2){
		numFlags = 0;
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
	int eleLogic = xEle + gridX * yEle;
	if(!(logicGrid[eleLogic] & 1 << 6)){	//When not flagged
		if(logicGrid[eleLogic] & 1 << 7){	//Already discovered
			return;
		}
		int ele = eleLogic * 2;
		if(logicGrid[eleLogic] & 1 << 5){	//If questioned, remove the question mark and set it to a normal tile
			logicGrid[eleLogic] &= ~(1 << 5);
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 0);
		}
		if(logicGrid[eleLogic] == 9){	//If mine
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 4);
			gameLive = 0;
			overMode = 1;
		}
		else{
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 7 + logicGrid[eleLogic]);
			nonMinesLeft--;
		}
		logicGrid[eleLogic] |= (1 << 7);
		if(logicGrid[eleLogic] % (1 << 5) == 0){
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
	int eleLogic = xEle + gridX * yEle;
	if((logicGrid[eleLogic] & 1<<7)){	//If revealed

		uint8_t valids = neighbouring_tiles(xEle, yEle);
		uint8_t flagSum = 0;	//A tiles value

		if((valids & (1 << 0)) && logicGrid[xEle - 1 + ((yEle- 1) * gridX)] & (1 << 6)){flagSum++;}		//Top Left
		if((valids & (1 << 1)) && logicGrid[xEle + ((yEle - 1) * gridX)]  & (1 << 6)){flagSum++;}		//Top centre
		if((valids & (1 << 2)) && logicGrid[xEle + 1 + ((yEle - 1) * gridX)]  & (1 << 6)){flagSum++;}	//Top right
		if((valids & (1 << 3)) && logicGrid[xEle - 1 + (yEle * gridX)]  & (1 << 6)){flagSum++;}			//Mid Left
		if((valids & (1 << 4)) && logicGrid[xEle + 1 + (yEle * gridX)]  & (1 << 6)){flagSum++;}			//Mid Right
		if((valids & (1 << 5)) && logicGrid[xEle - 1 + ((yEle + 1) * gridX)]  & (1 << 6)){flagSum++;}	//Bottom left
		if((valids & (1 << 6)) && logicGrid[xEle + ((yEle + 1) * gridX)]  & (1 << 6)){flagSum++;}		//Bottom centre
		if((valids & (1 << 7)) && logicGrid[xEle + 1 + ((yEle + 1) * gridX)]  & (1 << 6)){flagSum++;}	//Bottom right

		if(logicGrid[eleLogic] % (1 << 4) == flagSum){
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
void b_press(animation_t * anim, uint16_t eleLogic){
	int ele = eleLogic * 2;
	if(!(logicGrid[eleLogic] & 1<<7)){	//Not discovered
		uint8_t status = 0;	//0 = normal, 1 = flag, 2 = question icons
		if(logicGrid[eleLogic] & (1<<6)){	//If flagged
			logicGrid[eleLogic] &= ~(1<<6);	//Clears the flag bit
			if(questionEnabled){
				logicGrid[eleLogic] |= (1<<5);	//Sets the question bit
				status = 2;
			}
			numFlags++;
		}
		else{	//Not flagged, but maybe questioned
			if(logicGrid[eleLogic] & (1<<5)){	//Normal it
				logicGrid[eleLogic] &= ~(1<<5);
				status = 0;
			}
			else{	//Flag it
				logicGrid[eleLogic] |= (1<<6);
				status = 1;
				numFlags--;
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

	char dispNum[3];
	sprintf(dispNum, "%03d", num);

	int i;
	uint16_t frame_x;
	uint16_t frame_y;
	for(i = 0; i < 3; i++){
		if(dispNum[i] == '-'){
			graphics_frame_coordinates(anim, &frame_x, &frame_y, 10);
		}
		else{
			graphics_frame_coordinates(anim, &frame_x, &frame_y, (int)dispNum[i] - 48);
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
	if(operatingSystem){
		memory_mount_romdisk("/cd/XP.img", "/XP");
		memory_load_crayon_packer_sheet(&Windows, "/XP/Windows.dtex");
		fs_romdisk_unmount("/XP");
	}
	else{
		memory_mount_romdisk("/cd/2000.img", "/2000");
		memory_load_crayon_packer_sheet(&Windows, "/2000/Windows.dtex");
		fs_romdisk_unmount("/2000");
	}

	//These two just allow me to easily change between Minesweeper and Prato fiorito
	spritesheet_t TileSS;
	animation_t TileANIM;

	if(!language){
		TileSS = Board;
		TileANIM = Board.spritesheet_animation_array[5];
	}
	else{
		TileSS = Windows;
		TileANIM = Windows.spritesheet_animation_array[6];
	}

	//Setting up the draw arrays for the top, bottom bars and other windows related assets
	uint16_t windowsFrames[30];

	/*
	As of now:
	0	0,1 = aboutLogo
	1	2,3 = boarderBottom
	2	4,5 = boarderBottomLeft
	3	6,7 = boarderBottomRight
	4	8,9 = boarderLeft
	5	10,11 = boarderRight
	6	12,13 = italianIcon
	//WE SKIP THE TILES ANIM
	8	14,15 = taskbarCurrentTask
	9	16,17 = taskbarFiller
	10	18,19 = taskbarStart
	11	20,21 = taskbarTime
	12	22,23 = topbarAdjust
	13	24,25 = topbarFiller
	14	26,27 = topbarName
	*/

	//0, 8, 14

	int iter;
	int jiter;

	for(iter = 0; iter < 6; iter++){
		uint8_t langFrame = 0;
		if(iter == 0 && language){
			langFrame = 1;
		}
		graphics_frame_coordinates(&Windows.spritesheet_animation_array[iter], windowsFrames + (2 * iter), windowsFrames + 1 + (2 * iter), langFrame);
	}

	for(iter = 6; iter < 14; iter++){
		uint8_t langFrame = 0;
		if((iter + 1 == 7 || iter + 1 == 8 || iter + 1 == 14) && language){	//iter + 1 because thats the sprite we are targetting
			langFrame = 1;
		}
		graphics_frame_coordinates(&Windows.spritesheet_animation_array[iter + 1], windowsFrames + (2 * iter), windowsFrames + 1 + (2 * iter), langFrame);
	}

	gridX = 30;
	gridY = 20;
	gridStartX = 80;
	gridStartY = 104;	//Never changes

	uint16_t gridSize = gridX * gridY;
	float mineProbability = 0.175;

	logicGrid = (uint8_t *) malloc(gridSize * sizeof(uint8_t));
	coordGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));
	frameGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));

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

	//Stores the button combination from the previous button press
	uint32_t prevButtons[4] = {0};
	uint8_t buttonAction = 0;	// ---- YBXA format, each maple loop the variable is overritten with released statuses

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
	uint32_t mainBackground = (255 << 24) + (192 << 16) + (192 << 8) + 192;
	uint32_t lightGrey = (255 << 24) + (128 << 16) + (128 << 8) + 128;
	uint32_t white = (255 << 24) + (255 << 16) + (255 << 8) + 255;

	while(1){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
		if(!(playerActive & (1 << __dev->port)) && st->buttons != 0){	//Player is there, but hasn't been activated yet
			playerActive |= (1 << __dev->port);
			continue;
		}

		//Use the buttons previously pressed to control what happens here
		buttonAction = 0;
		buttonAction |= ((prevButtons[__dev->port] & CONT_A) && !(st->buttons & CONT_A)) << 0;
		buttonAction |= ((prevButtons[__dev->port] & CONT_X) && !(st->buttons & CONT_X) && !overMode && gameLive) << 1;
		buttonAction |= (!(prevButtons[__dev->port] & CONT_B) && (st->buttons & CONT_B) && !overMode) << 2;
		buttonAction |= (!(prevButtons[__dev->port] & CONT_Y) && (st->buttons & CONT_Y) && !overMode) << 3;

		//Do something for Start press like "startPrimed" thats true only when start is the only button pressed?
			//Why do I need a seperate var for that? I can just compare previous and current button presses

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
		if(buttonAction){
			if((buttonAction & (1 << 0)) && (cursorPos[2 * __dev->port] <= 307 + 26) && (cursorPos[(2 * __dev->port) + 1] <= 64 + 26)
				&& cursorPos[2 * __dev->port] >= 307 && cursorPos[(2 * __dev->port) + 1] >= 64){	//If face is pressed
				reset_grid(&TileANIM, mineProbability);
				prevButtons[__dev->port] = st->buttons;	//Store the previous button press
				break;	//Since we don't want these old presses interacting with the new board
			}
			if(xEle != -1 && yEle != -1){	//If on the grid
				if(buttonAction & (1 << 0)){	//For A press
					if(overMode == 0){
						if(!gameLive){
							timer_ms_gettime(&startTime, &startMSTime);
							gameLive = 1;
						}
						discover_tile(&TileANIM, xEle, yEle);
					}
				}
				if(buttonAction & (1 << 1)){	//For X press
					x_press(&TileANIM, xEle, yEle);
				}
				if(buttonAction & (1 << 2)){	//For B press
					b_press(&TileANIM, xEle + gridX * yEle);
				}
			}
			if(buttonAction & (1 << 3)){	//For Y press
				;
			}
			if(buttonAction & (1 << 4)){	//For START press, might delete this later...
				;
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
			if(!overMode && !face_frame_id){
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
			if(!gameLive){
				pressData[3 * __dev->port] = 0;
			}
		}
		else{
			pressData[3 * __dev->port] = 0;
		}

		prevButtons[__dev->port] = st->buttons;	//Store the previous button press
		
   		MAPLE_FOREACH_END()

   		if(nonMinesLeft == 0 && gameLive && overMode == 0){
   			gameLive = 0;
   			overMode = 2;
   		}

   		//Right now this is always triggered when a game ends and thanks to "revealed" it only triggers once
   		if(!revealed && !gameLive && overMode != 0){
   			reveal_map(&TileANIM);
   		}

   		//The face frame id code. If not indented or suprised, then choose a face
   		if(!face_frame_id){
	   		if(overMode == 1){
	   			//play bomb/death sound
				face_frame_id = 2;
	   		}
	   		else if(overMode == 2){
	   			//play "won" sound
				face_frame_id = 3;
	   		}
	   	}

		graphics_frame_coordinates(&Board.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, face_frame_id);
		face_frame_id = 0;	//Reset face

		timer_ms_gettime(&currentTime, &currentMSTime);
		if(gameLive && timeSec < 999){	//Prevent timer overflows
			//Play the "tick" sound effect
			timeSec = currentTime - startTime + (currentMSTime > startMSTime); //MS is there to account for the "1st second" inaccuracy
		}

   		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

		//Setup the main palette
		graphics_setup_palette(0, &Board);

		//Draw windows graphics
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[10], 0, 450, 3, 1, 1, windowsFrames[18], windowsFrames[19], 0);		//Task bar (Start)
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[8], 106, 450, 3, 1, 1, windowsFrames[14], windowsFrames[15], 0);	//Task bar (CurrentTask)
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[9], 266, 450, 2, 94, 1, windowsFrames[16], windowsFrames[17], 0);	//Task bar (Filler)
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[11], 547, 450, 1, 1, 1, windowsFrames[20], windowsFrames[21], 0);	//Task bar (Time)

		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[14], 0, 0, 3, 1, 1, windowsFrames[26], windowsFrames[27], 0);		//Top bar (App name)
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[13], 105, 0, 2, 155, 1, windowsFrames[24], windowsFrames[25], 0);	//Top bar (Fillter)
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[12], 568, 0, 3, 1, 1, windowsFrames[22], windowsFrames[23], 0);		//Top bar (Adjusts)


		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[4], 0, 29, 2, 1, 418, windowsFrames[8], windowsFrames[9], 0);		//Draw left bar
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[2], 0, 447, 2, 1, 1, windowsFrames[4], windowsFrames[5], 0);		//Draw bottom left bar
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[3], 636, 447, 2, 1, 1, windowsFrames[6], windowsFrames[7], 0);		//Draw bottom right bar
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[1], 3, 447, 2, 633, 1, windowsFrames[2], windowsFrames[3], 0);		//Draw bottom bar
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[5], 637, 29, 2, 1, 418, windowsFrames[10], windowsFrames[11], 0);	//Draw right bar

		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[7], 521, 457, 3, 1, 1, windowsFrames[12], windowsFrames[13], 0);	//Draw EN/IL icon
		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[4], 553, 455, 4, 1, 1, region_icon_x, region_icon_y, 0);				//Region icon

		//Draw the main background colour
		graphics_draw_colour_poly(0, 0, 1, 640, 480, mainBackground);

		//Draw the reset button face
		graphics_draw_sprite(&Board, &Board.spritesheet_animation_array[1], 307, 64, 2, 1, 1, face_frame_x, face_frame_y, 0);

		//Draw the flag count and timer
		digit_display(&Board, &Board.spritesheet_animation_array[2], numFlags, 20, 65);
		digit_display(&Board, &Board.spritesheet_animation_array[2], timeSec, 581, 65);

		//Depth for digit displays
		graphics_draw_colour_poly(19, 64, 4, 40, 24, lightGrey);
		graphics_draw_colour_poly(21, 66, 4, 40, 24, white);
		graphics_draw_colour_poly(580, 64, 4, 40, 24, lightGrey);
		graphics_draw_colour_poly(582, 66, 4, 40, 24, white);

		//Draw the grid
		graphics_draw_sprites_OLD(&TileSS, &TileANIM, coordGrid, frameGrid, 2 * gridSize, gridSize, 2, 1, 1, 0);

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
				graphics_draw_sprites_OLD(&TileSS, &TileANIM, indented_neighbours, indented_frames, liter, liter, 2, 1, 1, 0);
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
//Bug: Can still A/X press when dead
