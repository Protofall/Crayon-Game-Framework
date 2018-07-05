//Crayon libraries
#include "../crayon/dreamcast/memory.h"
#include "../crayon/dreamcast/debug.h"
#include "../crayon/dreamcast/graphics.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>

//For the timer
#include <arch/timer.h>

uint16_t minesLeft;	//Redundant

uint16_t numMines;	//Redundant?
uint16_t nonMinesLeft;	//When this variable equals zero, the game is won
int numFlags;	//More like "number of flags in the pool"

uint8_t overMode = 0;	//0 = ready for new game, 1 = loss (ded), 2 = win
// uint8_t alive = 1;	//Redendant due to gameLive. Replace with a "homeNotLive" variable
int timeSec;
uint8_t questionEnabled = 1;	//Enable the use of question marking
uint8_t gameLive = 0;	//Is true when the timer is turning

uint8_t gridX;
uint8_t gridY;
uint16_t gridStartX;
uint16_t gridStartY;

uint8_t *logicGrid;
uint16_t *coordGrid;	//Unless changing grid size, this won't need to be changed once set
uint16_t *frameGrid;

uint8_t true_prob(double p){
    return rand() < p * (RAND_MAX + 1.0);
}

uint8_t populate_logic(int x, uint8_t mode){
	if(x < 0 || x > gridX * gridY){	//Out of bounds
		return 0;
	}
	if(logicGrid[x] == 9){	//Is mine
		return 1;
	}
	else if(mode == 1){
		return 0;
	}

	//Sometimes its accidentally detecting right and left
	uint8_t sum = 0;
	if(x % gridX != 0){
		sum += populate_logic(x - 1, 1);			//Left
		sum += populate_logic(x - gridX - 1, 1);	//Top Left
		sum += populate_logic(x + gridX - 1, 1);	//Bottom left
	}
	if(x % gridX != 29){
		sum += populate_logic(x + 1, 1);			//Right
		sum += populate_logic(x - gridX + 1, 1);	//Top right
		sum += populate_logic(x + gridX + 1, 1);	//Bottom right
	}

	//These two are covered by the original OOB test
	sum += populate_logic(x - gridX, 1);		//Top centre
	sum += populate_logic(x + gridX, 1);		//Bottom centre

	logicGrid[x] = sum;

	return 0;
}

//Call this to reset the grid
void reset_grid(animation_t * anim, float mineProbability){
	gameLive = 0;
	numMines = 0;
	timeSec = 0;
	int i;
	uint16_t gridSize = gridX * gridY;
	for(i = 0; i < gridSize; i++){
		logicGrid[i] = 9 * true_prob(mineProbability);	//I think 0 is safe, 1 is mine?
		if(logicGrid[i] == 9){
			numMines++;
		}
	}

	for(i = 0; i < 2 * gridSize; i = i + 2){
		graphics_frame_coordinates(anim, frameGrid + i, frameGrid + i + 1, 0);
	}

	//Iterates through whole loop
	for(i = 0; i < gridSize; i++){
		populate_logic(i, 0);
	}

	overMode = 0;
	minesLeft = numMines;
	numFlags = numMines;
	nonMinesLeft = gridSize - numMines;

	return;
}

//Extracts k bits from "number" starting at index p from right (So number 25/11001, k 4, p 2 = 1100 (All but the 2^0 bit))
//Note to do the whole number p = 1, k = num bits of number
int bitExtraction(int number, int k, int p){
	return (((1 << k) - 1) & (number >> (p - 1)));
}

void discoverTile(animation_t * anim, uint16_t eleLogic){
	if(eleLogic < 0 || eleLogic > gridX * gridY){	//Out of bounds
		return;
	}
	int ele = eleLogic * 2;
	if(!(logicGrid[eleLogic] & 1<<6)){	//When not flagged
		if(logicGrid[eleLogic] & 1<<7){	//Already discovered
			return;
		}
		if(logicGrid[eleLogic] == 9){	//If mine
			numMines--;
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 4);
			gameLive = 0;
			overMode = 1;
		}
		else{
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 6 + logicGrid[eleLogic]);
			nonMinesLeft--;
		}
		logicGrid[eleLogic] |= (1<<7);
		if(bitExtraction(logicGrid[eleLogic], 4, 1) == 0){
			//Make recursive calls to the neighbours

			if(eleLogic % gridX != 0){
				discoverTile(anim, eleLogic - 1);			//Left
				discoverTile(anim, eleLogic - gridX - 1);	//Top Left
				discoverTile(anim, eleLogic + gridX - 1);	//Bottom left
			}
			if(eleLogic % gridX != 29){
				discoverTile(anim, eleLogic + 1);			//Right
				discoverTile(anim, eleLogic - gridX + 1);	//Top right
				discoverTile(anim, eleLogic + gridX + 1);	//Bottom right
			}

			//These two are covered by the original OOB test
			discoverTile(anim, eleLogic - gridX);		//Top centre
			discoverTile(anim, eleLogic + gridX);		//Bottom centre
		}
	}
	return;
}


//If we use a flag that decrements the flag count, leaving flag will increment it
void bPress(animation_t * anim, uint16_t eleLogic){
	int ele = eleLogic * 2;
	if(!(logicGrid[eleLogic] & 1<<7)){	//Not discovered
		uint8_t status = 0;	//0 = normal, 1 = flag, 2 = question icons
		uint8_t isMine = bitExtraction(logicGrid[eleLogic], 4, 1) == 9;

		if(logicGrid[eleLogic] & (1<<6)){	//If flagged
			logicGrid[eleLogic] &= ~(1<<6);	//Clears the flag bit
			if(questionEnabled){
				logicGrid[eleLogic] |= (1<<5);	//Sets the question bit
				status = 2;
			}
			numFlags++;
			if(isMine){
				minesLeft++;
			}
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
				if(isMine){
					minesLeft--;
				}
			}
		}
		graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, status);
	}
	else{	//Clicking on something that was A pressed before (Number, blank pressed)
		;
	}
	return;
}

//Must be called within a pvr_list_begin(), used for displaying the counter for flags and timer
void digitDisplay(spritesheet_t * ss, animation_t * anim, int num, uint16_t x, uint16_t y){
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
	uint8_t heldB[4];
	int iter;
	int jiter;
	for(iter = 0; iter < 4; iter++){
		clickedCursorPos[2 * iter] = -1;
		clickedCursorPos[(2 * iter) + 1] = -1;
		heldB[iter] = 0;
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
	    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)	//Need to figure out how to get the loop increment
	    if(st->buttons & CONT_START){
	    	// error_freeze("non-mines: %d", nonMinesLeft);
			reset_grid(&Tiles.spritesheet_animation_array[3], mineProbability);
	    }
	    if(st->buttons & CONT_A){
			if(clickedCursorPos[2 * __i] == -1 && clickedCursorPos[(2 * __i) + 1] == -1){
				clickedCursorPos[2 * __i] = cursorPos[2 * __i];
				clickedCursorPos[(2 * __i) + 1] = cursorPos[(2 * __i) + 1];
			}
	    }
	    else{
	    	//Maths needed to see if the pos-es allign, for now I'll just say they're equal
	    	if(cursorPos[2 * __i] == clickedCursorPos[2 * __i] && cursorPos[(2 * __i) + 1] == clickedCursorPos[(2 * __i) + 1]){
	    		int xPart;
	    		int yPart;

	    		//If the cursor is within the maze
	    		if((cursorPos[2 * __i] <= gridStartX + (gridX * 16)) && (cursorPos[(2 * __i) + 1] <= gridStartY + (gridY * 16))
	    			&& cursorPos[2 * __i] >= gridStartX && cursorPos[(2 * __i) + 1] >= gridStartY){
	    			//These two calls are supposed to floor it to the below multiple of 16, they contain the thing coords, not elements
	    			xPart = (cursorPos[2 * __i] - cursorPos[2 * __i] % 16) - gridStartX;
	    			yPart = (cursorPos[(2 * __i) + 1] - cursorPos[(2 * __i) + 1] % 16) - gridStartY;

	    			int eleLogic = (xPart / 16) + (gridX * yPart / 16);
	    			if(overMode == 0){
	    				if(!gameLive){
	    					timer_ms_gettime(&startTime, &startMSTime);
	    					gameLive = 1;
	    				}
	    				discoverTile(&Tiles.spritesheet_animation_array[3], eleLogic);
	    			}
	    		}
	    	}
	    	clickedCursorPos[__i * 2] = -1;
	    	clickedCursorPos[(__i * 2) + 1] = -1;
	    }

	    if((st->buttons & CONT_B) && (overMode == 0)){
	    	//Press instantly makes flag
	    	if(!heldB[__i]){
	    		//If the cursor is within the maze
	    		if((cursorPos[2 * __i] <= gridStartX + (gridX * 16)) && (cursorPos[(2 * __i) + 1] <= gridStartY + (gridY * 16))
	    			&& cursorPos[2 * __i] >= gridStartX && cursorPos[(2 * __i) + 1] >= gridStartY){	//This check isn't complete
			    	int xPart = (cursorPos[2 * __i] - cursorPos[2 * __i] % 16) - gridStartX;
			    	int yPart = (cursorPos[(2 * __i) + 1] - cursorPos[(2 * __i) + 1] % 16) - gridStartY;

			    	int eleLogic = (xPart / 16) + (gridX * yPart / 16);
			    	bPress(&Tiles.spritesheet_animation_array[3], eleLogic);
			    }
			    heldB[__i] = 1;
		    }
	    }
	    else{
	    	heldB[__i] = 0;
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

   		if(nonMinesLeft == 0){
   			gameLive = 0;
   			overMode = 2;
   		}

   		//The face icon, this code needs updating
   		if(overMode == 0){
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 0);
   		}
   		else if(overMode == 1){
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 2);
   		}
   		else{
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 3);
   		}

		timer_ms_gettime(&currentTime, &currentMSTime);
		if(gameLive && timeSec < 999){	//Prevent timer overflows
			timeSec = currentTime - startTime + (currentMSTime > startMSTime); //MS is there to account for the "1st second" inaccuracy
		}

   		pvr_wait_ready();	//With the timer it crashes here or waits indefinately
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

		//Setup the main palette
		graphics_setup_palette(0, &Tiles);

		//Draw top bar
		graphics_draw_sprites(&Windows, &Windows.spritesheet_animation_array[1], coordTopBar, frameTopBar, 8, 4, 1, 1, 1, 0);

		//Draw the flag count and timer
		digitDisplay(&Tiles, &Tiles.spritesheet_animation_array[2], numFlags, 50, 35);
		digitDisplay(&Tiles, &Tiles.spritesheet_animation_array[2], timeSec, 551, 35);

		//Draw the reset button
		graphics_draw_sprite(&Tiles, &Tiles.spritesheet_animation_array[1], 307, 35, 1, 1, 1, face_frame_x, face_frame_y, 0);

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

	- You can Right click or double Left click (maybe?) a number to "left click" all the surrounding numbers that aren't flags/question marks
	- Add proper "reveal map" logic and make the player unable to toggle flags then

	Bugs:
	- Game doesn't lock when you win, will fix this when I fix the win condition (Only mines are unclicked)
		- Game does lock when dead though
		- Also says you win based on if all mines have been flagged, doesn't consider numFlags

	Difficulties:
	- Beginner
	- Intermediate
	- Expert
	- Legacy User (Largest grid)

vahntitrio:
As soon as you've marked enough spaces, clicking both mouse buttons at once on a number uncovers all adjacent unflagged squares.
Thus allowing you to go finish faster.

(Confirm how to activate this feature) (Will probs make it B press only for simplicity sake)





Stuff to implement
- Proper game over mechanics. Pressing a mine = game over. number of non mines = number of flags left = 0 is a win.
	Both should result in "gameLive" being set to false and a flag for win/loss being set
	- When gameLive is false, you can't discover tiles, can't B press, but you can left click (On the face for a reset)
	- When gameLive is false, the "revealMap" function is called and it changed the grid differently based on win/loss
- Do you automatically get a free lake? Probs yes

*/
