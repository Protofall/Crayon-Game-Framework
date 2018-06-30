//Crayon libraries
#include "../crayon/dreamcast/memory.h"
#include "../crayon/dreamcast/debug.h"
#include "../crayon/dreamcast/graphics.h"
//#include "../crayon/dreamcast/render_structs.h"	//This is included in other crayon files
//#include "../crayon/dreamcast/texture_structs.h"	//This is included in other crayon files

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

uint16_t numMines;
uint16_t minesLeft;
uint8_t alive = 1;
uint8_t gridX;
uint8_t gridY;

uint8_t true_prob(double p){
    return rand() < p * (RAND_MAX + 1.0);
}

uint8_t populate_logic(uint8_t * logicGrid, int x, uint8_t mode){
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
		sum += populate_logic(logicGrid, x - 1, 1);			//Left
		sum += populate_logic(logicGrid, x - gridX - 1, 1);	//Top Left
		sum += populate_logic(logicGrid, x + gridX - 1, 1);	//Bottom left
	}
	if(x % gridX != 29){
		sum += populate_logic(logicGrid, x + 1, 1);			//Right
		sum += populate_logic(logicGrid, x - gridX + 1, 1);	//Top right
		sum += populate_logic(logicGrid, x + gridX + 1, 1);	//Bottom right
	}

	//These two are covered by the original OOB test
	sum += populate_logic(logicGrid, x - gridX, 1);		//Top centre
	sum += populate_logic(logicGrid, x + gridX, 1);		//Bottom centre

	logicGrid[x] = sum;

	return 0;
}

//Call this to reset the grid
void reset_grid(animation_t * anim, uint8_t * logicGrid, uint16_t * coordGrid, uint16_t * frameGrid,
		float mineProbability, uint8_t gridStartX, uint8_t gridStartY){
	numMines = 0;
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
		populate_logic(logicGrid, i, 0);
	}

	alive = 1;
	minesLeft = numMines;

	return;
}

//Extracts k bits from "number" starting at index p from right (So number 25/11001, k 4, p 2 = 1100 (All but the 2^0 bit))
//Note to do the whole number p = 1, k = num bits of number
int bitExtraction(int number, int k, int p){
	return (((1 << k) - 1) & (number >> (p - 1)));
}

void discoverTile(animation_t * anim, uint16_t * frameGrid, uint8_t * logicGrid, uint16_t eleLogic){
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
			alive = 0;
		}
		else{
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 6 + logicGrid[eleLogic]);
		}
		logicGrid[eleLogic] |= (1<<7);
		if(bitExtraction(logicGrid[eleLogic], 4, 1) == 0){
			//Make recursive calls to the neighbours

			// error_freeze("haha!");

			if(eleLogic % gridX != 0){
				discoverTile(anim, frameGrid, logicGrid, eleLogic - 1);			//Left
				discoverTile(anim, frameGrid, logicGrid, eleLogic - gridX - 1);	//Top Left
				discoverTile(anim, frameGrid, logicGrid, eleLogic + gridX - 1);	//Bottom left
			}
			if(eleLogic % gridX != 29){
				discoverTile(anim, frameGrid, logicGrid, eleLogic + 1);			//Right
				discoverTile(anim, frameGrid, logicGrid, eleLogic - gridX + 1);	//Top right
				discoverTile(anim, frameGrid, logicGrid, eleLogic + gridX + 1);	//Bottom right
			}

			//These two are covered by the original OOB test
			discoverTile(anim, frameGrid, logicGrid, eleLogic - gridX);		//Top centre
			discoverTile(anim, frameGrid, logicGrid, eleLogic + gridX);		//Bottom centre
		}
	}
	return;
}

void flag(animation_t * anim, uint16_t * frameGrid, uint8_t * logicGrid, uint16_t eleLogic){
	int ele = eleLogic * 2;
	if(!(logicGrid[eleLogic] & 1<<7)){	//Not discovered
		if(logicGrid[eleLogic] & 1<<6){	//Is flagged
			logicGrid[eleLogic] &= ~(1<<6);	//Clears the 6th bit
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 0);
			if(logicGrid[eleLogic] == 9){	//Why is this triggering?
				minesLeft++;
			}
			else{
				minesLeft--;
			}
		}
		else{
			logicGrid[eleLogic] |= (1<<6);	//Sets the nth bit
			graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 1);
			// error_freeze("Its value is: %d, bitExtracted = %d", logicGrid[eleLogic], bitExtraction(logicGrid[eleLogic], 4, 1));	//zero
			if(bitExtraction(logicGrid[eleLogic], 4, 1) == 9){	//If its a mine (Not triggering)
				minesLeft--;
			}
			else{
				minesLeft++;
			}
		}
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
	for(iter = 0; iter < 8; iter++){
		clickedCursorPos[iter] = -1;
		if(iter < 4){
			heldB[iter] = 0;
		}
	}
	cursorPos[0] = 50;
	cursorPos[1] = 100;
	cursorPos[2] = 50;
	cursorPos[3] = 350;
	cursorPos[4] = 590;
	cursorPos[5] = 100;
	cursorPos[6] = 590;
	cursorPos[7] = 350;

	memory_mount_romdisk("/cd/Minesweeper.img", "/Minesweeper");
	memory_load_crayon_packer_sheet(&Tiles, "/Minesweeper/Tiles.dtex");
	fs_romdisk_unmount("/Minesweeper");


	memory_mount_romdisk("/cd/XP.img", "/XP");
	memory_load_crayon_packer_sheet(&Windows, "/XP/Windows.dtex");
	fs_romdisk_unmount("/XP");

	int done = 0;

	gridX = 30;
	gridY = 20;
	uint16_t gridStartX = 80;
	uint16_t gridStartY = 80;
	uint16_t gridSize = gridX * gridY;
	float mineProbability = 0.2;
	uint8_t *logicGrid = (uint8_t *) malloc(gridSize * sizeof(uint8_t));
	uint16_t *coordGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));	//Once set it doesn't need to be modified
	uint16_t *frameGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));

	for(jiter = 0; jiter < gridY; jiter++){
		for(iter = 0; iter < gridX; iter++){	//i is x, j is y
			uint16_t ele = (jiter * gridX * 2) + (2 * iter);
			coordGrid[ele] = gridStartX + (iter * 16);
			coordGrid[ele + 1] = gridStartY + (jiter * 16);
		}
	}

	//Set the grid's initial values
	reset_grid(&Tiles.spritesheet_animation_array[2], logicGrid, coordGrid, frameGrid, mineProbability, gridStartX, gridStartY);

	uint16_t face_frame_x;
	uint16_t face_frame_y;

  	bfont_set_encoding(BFONT_CODE_ISO8859_1);
  	char mineStr[80];

	while(!done){
	    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)	//Need to figure out how to get the loop increment
	    if(st->buttons & CONT_START){	//Applies until not held?
			reset_grid(&Tiles.spritesheet_animation_array[2], logicGrid, coordGrid, frameGrid, mineProbability, gridStartX, gridStartY);
	    }
	    if(st->buttons & CONT_A){
			if(clickedCursorPos[__i * 2] == -1 && clickedCursorPos[(__i * 2) + 1] == -1){
				clickedCursorPos[__i * 2] = cursorPos[__i * 2];
				clickedCursorPos[(__i * 2) + 1] = cursorPos[(__i * 2) + 1];
			}
	    }
	    else{
	    	//Maths needed to see if the pos-es allign, for now I'll just say they're equal
	    	if(cursorPos[__i * 2] == clickedCursorPos[__i * 2] && cursorPos[(__i * 2) + 1] == clickedCursorPos[(__i * 2) + 1]){
	    		int xPart;
	    		int yPart;

	    		//If the cursor is within the maze
	    		if((cursorPos[__i] <= gridStartX + (gridX * 16)) && (cursorPos[__i + 1] <= gridStartY + (gridY * 16))){
	    			//These two calls are supposed to floor it to the below multiple of 16, they contain the thing coords, not elements
	    			xPart = (cursorPos[__i] - cursorPos[__i] % 16) - gridStartX;
	    			yPart = (cursorPos[__i + 1] - cursorPos[__i + 1] % 16) - gridStartY;

	    			int eleLogic = (xPart / 16) + (gridX * yPart / 16);
	    			if(alive){
	    				discoverTile(&Tiles.spritesheet_animation_array[2], frameGrid, logicGrid, eleLogic);
	    			}
	    		}
	    	}
	    	clickedCursorPos[__i * 2] = -1;
	    	clickedCursorPos[(__i * 2) + 1] = -1;
	    }

	    if(st->buttons & CONT_B){
	    	//Press instantly makes flag
	    	if(!heldB[__i]){
		    	int xPart = (cursorPos[__i] - cursorPos[__i] % 16) - gridStartX;
		    	int yPart = (cursorPos[__i + 1] - cursorPos[__i + 1] % 16) - gridStartY;

		    	int eleLogic = (xPart / 16) + (gridX * yPart / 16);
		    	flag(&Tiles.spritesheet_animation_array[2], frameGrid, logicGrid, eleLogic);
		    	heldB[__i] = 1;
		    }
	    }
	    else{
	    	heldB[__i] = 0;
	    }
	    if(st->buttons & CONT_DPAD_UP){
			cursorPos[(__i * 2) + 1] -= 2;
			if(cursorPos[(__i * 2) + 1] < 0){
				cursorPos[(__i * 2) + 1] = 0;
			}
	    }
	    if(st->buttons & CONT_DPAD_DOWN){
			cursorPos[(__i * 2) + 1] += 2;
			if(cursorPos[(__i * 2) + 1] > 480){
				cursorPos[(__i * 2) + 1] = 480;
			}
	    }
	    if(st->buttons & CONT_DPAD_LEFT){
			cursorPos[__i * 2] -= 2;
			if(cursorPos[__i * 2] < 0){
				cursorPos[__i * 2] = 0;
			}
	    }
	    if(st->buttons & CONT_DPAD_RIGHT){
			cursorPos[__i * 2] += 2;
			if(cursorPos[__i * 2] > 640){
				cursorPos[__i * 2] = 640;
			}
	    }
		
   		MAPLE_FOREACH_END()

   		if(!alive){
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 2);
   		}
   		else{
			graphics_frame_coordinates(&Tiles.spritesheet_animation_array[1], &face_frame_x, &face_frame_y, 0);
   		}

   		pvr_wait_ready();
		pvr_scene_begin();

		if(minesLeft != 0){
			sprintf(mineStr, "%d", numMines);
			bfont_draw_str(vram_s, 640, 1, mineStr);	//Draw mine count
		}
		else{
			sprintf(mineStr, "Team Winning!");
			bfont_draw_str(vram_s, 640, 1, mineStr);	//Draw winning message
		}

		pvr_list_begin(PVR_LIST_TR_POLY);

		//Setup the main palette
		graphics_setup_palette(0, &Tiles);

		//Draw the reset button
		graphics_draw_sprite(&Tiles, &Tiles.spritesheet_animation_array[1], 307, 20, 1, 1, 1, face_frame_x, face_frame_y, 0);

		for(iter = 0; iter < 8; iter = iter + 2){
			graphics_draw_sprite(&Tiles, &Tiles.spritesheet_animation_array[0], cursorPos[iter], cursorPos[iter + 1], 10, 1, 1, 0, 0, 0);
		}

		//Draw the grid
		graphics_draw_sprites(&Tiles, &Tiles.spritesheet_animation_array[2], coordGrid, frameGrid, 2 * gridSize, gridSize, 1, 1, 1, 0);

		//Group the windows bar into a single draw later
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[0], 0, 450, 1, 1, 1, 0, 0, 0);
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[0], 160, 450, 1, 1, 1, 0, 30, 0);
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[0], 320, 450, 1, 1, 1, 0, 60, 0);
		graphics_draw_sprite(&Windows, &Windows.spritesheet_animation_array[0], 480, 450, 1, 1, 1, 0, 90, 0);
    	
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