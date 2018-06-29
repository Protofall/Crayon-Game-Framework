//Crayon libraries
#include "../crayon/dreamcast/memory.h"
#include "../crayon/dreamcast/debug.h"
#include "../crayon/dreamcast/graphics.h"
//#include "../crayon/dreamcast/render_structs.h"	//This is included in other crayon files
//#include "../crayon/dreamcast/texture_structs.h"	//This is included in other crayon files

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

uint8_t true_prob(double p){
    return rand() < p * (RAND_MAX + 1.0);
}

//Call this to reset the grid
void reset_grid(animation_t * anim, uint8_t * logicGrid, uint16_t * coordGrid, uint16_t * frameGrid, float mineProbability, uint8_t gridX, uint8_t gridY){
	int i;
	int j;
	for(i = 0; i < gridX * gridY; i++){
		logicGrid[i] = true_prob(mineProbability);
	}

	//This is missing some positions
	for(j = 0; j < gridY; j++){
		for(i = 0; i < gridX; i++){	//i is x, j is y
			uint16_t ele = (j * gridX * 2) + (2 * i);
			coordGrid[ele] = 80 + (i * 16);
			coordGrid[ele + 1] = 65 + (j * 16);
			//if(logicGrid[(gridX * j) + i] == 0){
				graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 0);
			//}
			//else{
			//	graphics_frame_coordinates(anim, frameGrid + ele, frameGrid + ele + 1, 4);
			//}
		}
	}
	return;
}

int main(){
	vid_set_mode(DM_640x480_VGA, PM_RGB565);

	pvr_init_defaults();

	srand(time(0));

	spritesheet_t Tiles, Windows;

	memory_mount_romdisk("/cd/Minesweeper.img", "/Minesweeper");
	memory_load_crayon_packer_sheet(&Tiles, "/Minesweeper/Tiles.dtex");
	fs_romdisk_unmount("/Minesweeper");


	memory_mount_romdisk("/cd/XP.img", "/XP");
	memory_load_crayon_packer_sheet(&Windows, "/XP/Windows.dtex");
	fs_romdisk_unmount("/XP");

	int done = 0;

	uint8_t gridX = 30;
	uint8_t gridY = 20;
	uint16_t gridSize = gridX * gridY;
	float mineProbability = 0.2;
	uint8_t *logicGrid = (uint8_t *) malloc(gridSize * sizeof(uint8_t));
	uint16_t *coordGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));
	uint16_t *frameGrid = (uint16_t *) malloc(2 * gridSize * sizeof(uint16_t));

	//Set the grid's initial values
	reset_grid(&Tiles.spritesheet_animation_array[2], logicGrid, coordGrid, frameGrid, mineProbability, gridX, gridY);

	uint16_t numMines = 0;

	while(!done){
	    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
	    if(st->buttons & CONT_START){ // Quits if start is pressed. Screen goes black (This code behaves weirdly, I don't get it)
			//reset_grid(&Tiles.spritesheet_animation_array[2], logicGrid, coordGrid, frameGrid, mineProbability, gridX, gridY);
	    }
   		MAPLE_FOREACH_END()

   		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

		//Setup the main palette
		graphics_setup_palette(0, &Tiles);

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