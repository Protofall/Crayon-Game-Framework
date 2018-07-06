//Crayon libraries
#include "../crayon/dreamcast/memory.h"
#include "../crayon/dreamcast/debug.h"
#include "../crayon/dreamcast/graphics.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>

//For the timer
#include <arch/timer.h>

uint16_t nonMinesLeft;	//When this variable equals zero, the game is won
int numFlags;	//More like "number of flags in the pool"

uint8_t overMode = 0;	//0 = ready for new game, 1 = loss (ded), 2 = win
uint8_t revealed;
int timeSec;
uint8_t gameLive = 0;	//Is true when the timer is turning
uint8_t questionEnabled = 1;	//Enable the use of question marking

uint8_t gridX;
uint8_t gridY;
uint16_t gridStartX;
uint16_t gridStartY;

uint8_t *logicGrid;
uint16_t *coordGrid;	//Unless changing grid size, this won't need to be changed once set
uint16_t *frameGrid;

//Only correctly works when gridX = gridY
uint8_t neighbouring_tile(int origin, int diverge){
	//Check if y is OOB (-ve or over positive element value)
	if(diverge < 0 || diverge > (gridX * gridY) - 1){
		return 0;
	}

	//Convert x and y to 2D ids
	int8_t diffX = (origin / gridX) - (diverge / gridX);	//x1 - x2
	int8_t diffY = (origin % gridX) - (diverge % gridX);	//y1 - y2	(gridX was formerly gridY here)

	if(diffX > -2 && diffX < 2 && diffY > -2 && diffY < 2){
		return 1;
	}
	return 0;
}

//Initially called by reset_grid where x is always a valid number
uint8_t populate_logic(int x, uint8_t mode){
	if(logicGrid[x] == 9){	//Is mine
		return 1;
	}
	else if(mode == 1){
		return 0;
	}

	uint8_t sum = 0;	//A tiles value

	if(neighbouring_tile(x, x - 1 - gridX)){sum += populate_logic(x - 1 - gridX, 1);}	//Top Left
	if(neighbouring_tile(x, x - 1)){sum += populate_logic(x - 1, 1);}					//Mid Left
	if(neighbouring_tile(x, x - 1 + gridX)){sum += populate_logic(x - 1 + gridX, 1);}	//Bottom left

	if(neighbouring_tile(x, x + 1 - gridX)){sum += populate_logic(x + 1 - gridX, 1);}	//Top right
	if(neighbouring_tile(x, x + 1)){sum += populate_logic(x + 1, 1);}					//Mid Right
	if(neighbouring_tile(x, x + 1 + gridX)){sum += populate_logic(x + 1 + gridX, 1);}	//Bottom right

	if(neighbouring_tile(x, x - gridX)){sum += populate_logic(x - gridX, 1);}		//Top centre
	if(neighbouring_tile(x, x + gridX)){sum += populate_logic(x + gridX, 1);}		//Bottom centre
	
	logicGrid[x] = sum;

	return 0;
}

uint8_t true_prob(double p){
    return rand() < p * (RAND_MAX + 1.0);
}

//Call this to reset the grid
void reset_grid(animation_t * anim, float mineProbability){
	numFlags = 0;
	int i;
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
	for(i = 0; i < gridSize; i++){
		populate_logic(i, 0);	//This is generating values larger than 10 when it shouldn't
	}

	gameLive = 0;
	overMode = 0;
	revealed = 0;

	timeSec = 0;
	nonMinesLeft = gridSize - numFlags;

	return;
}

//Extracts k bits from "number" starting at index p from right (So number 25/11001, k 4, p 2 = 1100 (All but the 2^0 bit))
//Note to do the whole number p = 1, k = num bits of number
int bit_extraction(int number, int k, int p){
	return (((1 << k) - 1) & (number >> (p - 1)));
}

void reveal_map(animation_t * anim){
	//It fills it out differently depending on overMode
	//0 = ready for new game, 1 = loss (ded), 2 = win
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
		for(i = 0; i < gridX * gridY; i++){
			if(bit_extraction(logicGrid[i], 4, 1) == 9){
				graphics_frame_coordinates(anim, frameGrid + (2 * i), frameGrid + (2 * i) + 1, 1);
			}
		}
	}
	revealed = 1;
	return;
}

//Doesn't handle questions correctly
void discover_tile(animation_t * anim, uint16_t eleLogic){
	if(!(logicGrid[eleLogic] & 1<<6)){	//When not flagged
		if(logicGrid[eleLogic] & 1<<7){	//Already discovered
			return;
		}
		int ele = eleLogic * 2;
		if(logicGrid[eleLogic] & 1<<5){	//If questioned, remove the question mark and set it to a normal tile
			logicGrid[eleLogic] &= ~(1<<5);
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 0);
		}
		if(logicGrid[eleLogic] == 9){	//If mine
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 4);
			gameLive = 0;
			overMode = 1;
		}
		else{
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 6 + logicGrid[eleLogic]);
			nonMinesLeft--;
		}
		logicGrid[eleLogic] |= (1<<7);
		if(bit_extraction(logicGrid[eleLogic], 4, 1) == 0){
			if(neighbouring_tile(eleLogic, eleLogic - 1)){discover_tile(anim, eleLogic - 1);}					//Left
			if(neighbouring_tile(eleLogic, eleLogic - 1 - gridX)){discover_tile(anim, eleLogic - 1 - gridX);}	//Top Left
			if(neighbouring_tile(eleLogic, eleLogic - 1 + gridX)){discover_tile(anim, eleLogic - 1 + gridX);}	//Bottom Left
			if(neighbouring_tile(eleLogic, eleLogic + 1)){discover_tile(anim, eleLogic + 1);}					//Right
			if(neighbouring_tile(eleLogic, eleLogic + 1 - gridX)){discover_tile(anim, eleLogic + 1 - gridX);}	//Top Right
			if(neighbouring_tile(eleLogic, eleLogic + 1 + gridX)){discover_tile(anim, eleLogic + 1 + gridX);}	//Bottom Right
			if(neighbouring_tile(eleLogic, eleLogic - gridX)){discover_tile(anim, eleLogic - gridX);}			//Top centre
			if(neighbouring_tile(eleLogic, eleLogic + gridX)){discover_tile(anim, eleLogic + gridX);}			//Bottom centre
		}
	}
	return;
}

//Right clicking next to a question marked mine updates its sprite to a glitchy one

/*

1 2 (Blank)
1 question(isMine) Flag(incorrect)
1 1 2
X clicking the centre bottom 1 results in glitchy question and no gameover

*/

void x_press(animation_t * anim, uint16_t eleLogic){
	if((logicGrid[eleLogic] & 1<<7) || (logicGrid[eleLogic] & 1<<6) || (logicGrid[eleLogic] & 1<<5)){	//If revealed or flagged or questioned
		int eles[8];
		uint8_t successfuls = 0;
		uint8_t invalidness = 0;
		eles[0] = eleLogic - 1;
		eles[1] = eleLogic - 1 - gridX;
		eles[2] = eleLogic - 1 + gridX;
		eles[3] = eleLogic + 1;
		eles[4] = eleLogic + 1 - gridX;
		eles[5] = eleLogic + 1 + gridX;
		eles[6] = eleLogic - gridX;
		eles[7] = eleLogic + gridX;
		int i;
		for(i = 0; i < 8; i++){
			if(neighbouring_tile(eleLogic, eles[i])){	//If it is a neighbour
				successfuls |= (1<<i);	//We'll traverse it later
				if(logicGrid[eles[i]] == 9){	//Don't proceed if an unmarked mine is nearby)
					invalidness |= (1<<0);
				}
				if(logicGrid[eles[i]] & (1<<6) && logicGrid[eles[i]] != 73){	//Do proceed if theres a false question mark within range
					invalidness |= (1<<1);
				}
			}
		}
		if(invalidness == 1){
			return;
		}

		//Discover the valid tiles
		for(i = 0; i < 8; i++){
			if(successfuls & (1<<i)){
				discover_tile(anim, eles[i]);
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
		graphics_draw_sprite(ss, anim, x + (i * 13), y, 11, 1, 1, frame_x, frame_y, 0);
	}

	return;
}

int main(){
	vid_set_mode(DM_640x480_VGA, PM_RGB565);

	pvr_init_defaults();

	srand(time(0));

	spritesheet_t Tiles, Windows;
	int cursorPos[8];
	int clickedCursorPos[8];
	uint8_t held[12];	//B's, START's, A/X's (0, 1, 2) (And Y later)
	int iter;
	int jiter;
	for(iter = 0; iter < 8; iter++){
		clickedCursorPos[iter] = -1;
		held[iter] = 0;
	}
	for(iter = 8; iter < 12; iter++){
		held[iter] = 0;
	}

	cursorPos[0] = 50;
	cursorPos[1] = 100;
	cursorPos[2] = 50;
	cursorPos[3] = 350;
	cursorPos[4] = 590;
	cursorPos[5] = 100;
	cursorPos[6] = 590;
	cursorPos[7] = 350;

	//Setting up the draw arrays for the top and bottom bars
	uint16_t *coordTopBar = (uint16_t *) malloc(8 * sizeof(uint16_t));
	uint16_t *frameTopBar = (uint16_t *) malloc(8 * sizeof(uint16_t));
	uint16_t *coordTaskBar = (uint16_t *) malloc(8 * sizeof(uint16_t));
	uint16_t *frameTaskBar = (uint16_t *) malloc(8 * sizeof(uint16_t));

	for(iter = 0; iter < 4; iter++){
		coordTaskBar[iter * 2] = 0 + (160 * iter);
		coordTopBar[iter * 2] = 0 + (160 * iter);

		coordTaskBar[(iter * 2) + 1] = 450;
		coordTopBar[(iter * 2) + 1] = 0;
	}

	memory_mount_romdisk("/cd/Minesweeper.img", "/Minesweeper");
	memory_load_crayon_packer_sheet(&Tiles, "/Minesweeper/Tiles.dtex");
	fs_romdisk_unmount("/Minesweeper");

	memory_mount_romdisk("/cd/XP.img", "/XP");
	memory_load_crayon_packer_sheet(&Windows, "/XP/Windows.dtex");
	fs_romdisk_unmount("/XP");

	graphics_frame_coordinates(&Windows.spritesheet_animation_array[0], frameTaskBar, frameTaskBar + 1, 0);
	graphics_frame_coordinates(&Windows.spritesheet_animation_array[0], frameTaskBar + 2, frameTaskBar + 3, 1);
	graphics_frame_coordinates(&Windows.spritesheet_animation_array[0], frameTaskBar + 4, frameTaskBar + 5, 2);
	graphics_frame_coordinates(&Windows.spritesheet_animation_array[0], frameTaskBar + 6, frameTaskBar + 7, 3);

	graphics_frame_coordinates(&Windows.spritesheet_animation_array[1], frameTopBar, frameTopBar + 1, 0);
	graphics_frame_coordinates(&Windows.spritesheet_animation_array[1], frameTopBar + 2, frameTopBar + 3, 1);
	graphics_frame_coordinates(&Windows.spritesheet_animation_array[1], frameTopBar + 4, frameTopBar + 5, 1);
	graphics_frame_coordinates(&Windows.spritesheet_animation_array[1], frameTopBar + 6, frameTopBar + 7, 2);


	gridX = 30;
	gridY = 20;

	// gridX = 10;
	// gridY = 10;

	gridStartX = 80;
	gridStartY = 80;
	uint16_t gridSize = gridX * gridY;
	float mineProbability;
	mineProbability = 0.175;

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
	reset_grid(&Tiles.spritesheet_animation_array[3], mineProbability);

	//The face frame coords
	uint16_t face_frame_x;
	uint16_t face_frame_y;

	//For the timer
	uint32_t currentTime = 0;
	uint32_t currentMSTime = 0;
	uint32_t startTime = 0;
	uint32_t startMSTime = 0;

	while(1){
	    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
	    if(st->buttons & CONT_START){
	    	if(!held[__i + 4]){
				reset_grid(&Tiles.spritesheet_animation_array[3], mineProbability);
			    held[__i + 4] = 1;
		    }
	    }
	    else{
	    	held[__i + 4] = 0;
	    }
	    if(st->buttons & CONT_A){
			if(clickedCursorPos[2 * __i] == -1 && clickedCursorPos[(2 * __i) + 1] == -1){
				clickedCursorPos[2 * __i] = cursorPos[2 * __i];
				clickedCursorPos[(2 * __i) + 1] = cursorPos[(2 * __i) + 1];
				held[8 + __i] = 1;
			}
	    }
	    else{
	    	if(clickedCursorPos[__i * 2] != -1 && clickedCursorPos[(__i * 2) + 1] != -1 && held[8 + __i] == 1){
		    	int xPart = (cursorPos[2 * __i] - cursorPos[2 * __i] % 16) - gridStartX;
		    	int yPart = (cursorPos[(2 * __i) + 1] - cursorPos[(2 * __i) + 1] % 16) - gridStartY;

		    	//Check that you're looking at the same tile you pressed on
		    	if(xPart == (clickedCursorPos[2 * __i] - clickedCursorPos[2 * __i] % 16) - gridStartX
		    		&& yPart == (clickedCursorPos[(2 * __i) + 1] - clickedCursorPos[(2 * __i) + 1] % 16) - gridStartY){

		    		//If the cursor is within the maze
		    		if((cursorPos[2 * __i] <= gridStartX + (gridX * 16)) && (cursorPos[(2 * __i) + 1] <= gridStartY + (gridY * 16))
		    			&& cursorPos[2 * __i] >= gridStartX && cursorPos[(2 * __i) + 1] >= gridStartY){
		    			int eleLogic = (xPart / 16) + (gridX * yPart / 16);
		    			if(overMode == 0){
		    				if(!gameLive){
		    					timer_ms_gettime(&startTime, &startMSTime);
		    					gameLive = 1;
		    				}
		    				discover_tile(&Tiles.spritesheet_animation_array[3], eleLogic);
		    			}
		    		}
		    	}
		    	clickedCursorPos[__i * 2] = -1;
		    	clickedCursorPos[(__i * 2) + 1] = -1;
		    	held[8 + __i] = 0;
	    	}
	    }
	    if(st->buttons & CONT_X){
			if(clickedCursorPos[2 * __i] == -1 && clickedCursorPos[(2 * __i) + 1] == -1){
				clickedCursorPos[2 * __i] = cursorPos[2 * __i];
				clickedCursorPos[(2 * __i) + 1] = cursorPos[(2 * __i) + 1];
				held[8 + __i] = 2;
			}
	    }
	    else{
	    	if(clickedCursorPos[__i * 2] != -1 && clickedCursorPos[(__i * 2) + 1] != -1 && held[8 + __i] == 2){
		    	int xPart = (cursorPos[2 * __i] - cursorPos[2 * __i] % 16) - gridStartX;
		    	int yPart = (cursorPos[(2 * __i) + 1] - cursorPos[(2 * __i) + 1] % 16) - gridStartY;

		    	//Check that you're looking at the same tile you pressed on
		    	if(xPart == (clickedCursorPos[2 * __i] - clickedCursorPos[2 * __i] % 16) - gridStartX
		    		&& yPart == (clickedCursorPos[(2 * __i) + 1] - clickedCursorPos[(2 * __i) + 1] % 16) - gridStartY){

		    		//If the cursor is within the maze
		    		if((cursorPos[2 * __i] <= gridStartX + (gridX * 16)) && (cursorPos[(2 * __i) + 1] <= gridStartY + (gridY * 16))
		    			&& cursorPos[2 * __i] >= gridStartX && cursorPos[(2 * __i) + 1] >= gridStartY){
		    			int eleLogic = (xPart / 16) + (gridX * yPart / 16);
		    			if(overMode == 0 && gameLive){
		    				x_press(&Tiles.spritesheet_animation_array[3], eleLogic);
		    			}
		    		}
		    	}
		    	clickedCursorPos[__i * 2] = -1;
		    	clickedCursorPos[(__i * 2) + 1] = -1;
		    	held[8 + __i] = 0;
	    	}
	    }

	    if((st->buttons & CONT_B) && (overMode == 0)){
	    	//Press instantly makes flag
	    	if(!held[__i]){
	    		//If the cursor is within the maze
	    		if((cursorPos[2 * __i] <= gridStartX + (gridX * 16)) && (cursorPos[(2 * __i) + 1] <= gridStartY + (gridY * 16))
	    			&& cursorPos[2 * __i] >= gridStartX && cursorPos[(2 * __i) + 1] >= gridStartY){	//This check isn't complete
			    	int xPart = (cursorPos[2 * __i] - cursorPos[2 * __i] % 16) - gridStartX;
			    	int yPart = (cursorPos[(2 * __i) + 1] - cursorPos[(2 * __i) + 1] % 16) - gridStartY;

			    	int eleLogic = (xPart / 16) + (gridX * yPart / 16);
			    	b_press(&Tiles.spritesheet_animation_array[3], eleLogic);
			    }
			    held[__i] = 1;
		    }
	    }
	    else{
	    	held[__i] = 0;
	    }
	    if(st->buttons & CONT_DPAD_UP){
			cursorPos[(2 * __i) + 1] -= 2;
			if(cursorPos[(2 * __i) + 1] < 0){
				cursorPos[(2 * __i) + 1] = 0;
			}
	    }
	    if(st->buttons & CONT_DPAD_DOWN){
			cursorPos[(2 * __i) + 1] += 2;
			if(cursorPos[(2 * __i) + 1] > 480){
				cursorPos[(2 * __i) + 1] = 480;
			}
	    }
	    if(st->buttons & CONT_DPAD_LEFT){
			cursorPos[2 * __i] -= 2;
			if(cursorPos[2 * __i] < 0){
				cursorPos[2 * __i] = 0;
			}
	    }
	    if(st->buttons & CONT_DPAD_RIGHT){
			cursorPos[2 * __i] += 2;
			if(cursorPos[2 * __i] > 640){
				cursorPos[2 * __i] = 640;
			}
	    }
		
   		MAPLE_FOREACH_END()

   		if(nonMinesLeft == 0 && gameLive && overMode == 0){
   			gameLive = 0;
   			overMode = 2;
   		}

   		//Right now this is always triggered when a game ends, I should do something so it only calls this once
   		if(!revealed && !gameLive && overMode != 0){
   			reveal_map(&Tiles.spritesheet_animation_array[3]);
   		}

   		//The face icon, this code needs updating
   		if(overMode == 0){
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 0);
   		}
   		else if(overMode == 1){
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 2);
   		}
   		else if(overMode == 2){
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 3);
   		}

		timer_ms_gettime(&currentTime, &currentMSTime);
		if(gameLive && timeSec < 999){	//Prevent timer overflows
			timeSec = currentTime - startTime + (currentMSTime > startMSTime); //MS is there to account for the "1st second" inaccuracy
		}

   		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

		//Setup the main palette
		graphics_setup_palette(0, &Tiles);

		//Draw top bar
		graphics_draw_sprites(&Windows, &Windows.spritesheet_animation_array[1], coordTopBar, frameTopBar, 8, 4, 1, 1, 1, 0);

		//Draw the flag count and timer
		digit_display(&Tiles, &Tiles.spritesheet_animation_array[2], numFlags, 50, 35);
		digit_display(&Tiles, &Tiles.spritesheet_animation_array[2], timeSec, 551, 35);

		//Draw the reset button face
		graphics_draw_sprite(&Tiles, &Tiles.spritesheet_animation_array[1], 307, 35, 1, 1, 1, face_frame_x, face_frame_y, 0);

		//Draw the cursors
		for(iter = 0; iter < 8; iter = iter + 2){
			graphics_draw_sprite(&Tiles, &Tiles.spritesheet_animation_array[0], cursorPos[iter], cursorPos[iter + 1], 10, 1, 1, 0, 0, 0);
		}

		//Draw the grid
		graphics_draw_sprites(&Tiles, &Tiles.spritesheet_animation_array[3], coordGrid, frameGrid, 2 * gridSize, gridSize, 1, 1, 1, 0);

		//Group the windows bar into a single draw later
		graphics_draw_sprites(&Windows, &Windows.spritesheet_animation_array[0], coordTaskBar, frameTaskBar, 8, 4, 1, 1, 1, 0);
    	
		pvr_list_finish();

		pvr_scene_finish();
   	}

   	//Confirm everything was unloaded successfully (Should equal zero)
   	int retVal = 0;
   	retVal += memory_free_crayon_packer_sheet(&Tiles);
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

vahntitrio:
As soon as you've marked enough spaces, clicking both mouse buttons at once on a number uncovers all adjacent unflagged squares.
Thus allowing you to go finish faster.

(Confirm how to activate this feature) (Will probs make it X press for simplicity sake)





Stuff to implement
- Do you automatically get a free lake? Probs yes

*/

//Add something to be displayed on the VMU screen






//Note: You CAN A/X press on a question mark. The question mark is only there for the user, doesn't have extra behaviour
//BUG sometimes x press fails when I don't think it should...maybe a screen wrap thing?
	//Seems to be iffy when pressing mines too...need to check execution logic
