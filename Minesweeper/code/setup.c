#include "setup.h"

void setup_pos_lookup_table(MinesweeperOS_t *os, uint8_t sys, uint8_t anim_id){
	// printf("Anims: %d\n", anim_id);
	if(sys){
		switch(anim_id){	//I should change to use name. Can switches do strings?
			case 0:
				break;
			case 1:
				break;
			default:
				break;
		}
	}
	else{
		;
	}
}

void setup_OS_assets(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t lang){
	os->sprite_count = ss->spritesheet_animation_count - 1;
	os->ids = (uint8_t *) malloc(os->sprite_count * sizeof(uint8_t));	//Because we don't include the ital-tiles, this makes other code easier
	os->coords_pos = (uint16_t *) malloc(3 * os->sprite_count * sizeof(uint16_t));	//x, y, z
	os->coords_frame = (uint16_t *) malloc(2 * os->sprite_count * sizeof(uint16_t));	//u, v
	os->scale = (uint16_t *) malloc(2 * os->sprite_count * sizeof(uint16_t));	//scale x/y
	os->windows_ss = ss;
	uint8_t id_count = 0;
	uint8_t iter;
	for(iter = 0; iter < os->sprite_count; iter++){
		uint8_t langFrame = 0;
		if(lang && ss->spritesheet_animation_array[id_count].animation_frames > 1){
			langFrame = 1;
		}
		if(!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "italianTiles")){	//Figure out which library I need for this
			id_count++;
		}
		graphics_frame_coordinates(&ss->spritesheet_animation_array[id_count],
			os->coords_frame + (2 * iter), os->coords_frame + (2 * iter) + 1, langFrame);
		os->ids[iter] = id_count;
		id_count++;
	}
	if(sys){	//XP
		for(iter = 0; iter < os->sprite_count; iter++){
			setup_pos_lookup_table(os, sys, os->ids[iter]);
		}
		//Change this code to work with names incase more assets are added

		//0	(Currently not using, but will use later)
		// os->coords_pos[0] = ;
		// os->coords_pos[1] = ;
		// os->coords_pos[2] = ;
		// os->scale[0] = ;
		// os->scale[1] = ;

		//1
		os->coords_pos[3] = 3;
		os->coords_pos[4] = 447;
		os->coords_pos[5] = 2;
		os->scale[2] = 633;
		os->scale[3] = 1;

		//2
		os->coords_pos[6] = 0;
		os->coords_pos[7] = 447;
		os->coords_pos[8] = 2;
		os->scale[4] = 1;
		os->scale[5] = 1;

		//3
		os->coords_pos[9] = 636;
		os->coords_pos[10] = 447;
		os->coords_pos[11] = 2;
		os->scale[6] = 1;
		os->scale[7] = 1;

		//4
		os->coords_pos[12] = 0;
		os->coords_pos[13] = 29;
		os->coords_pos[14] = 2;
		os->scale[8] = 1;
		os->scale[9] = 418;

		//5
		os->coords_pos[15] = 637;
		os->coords_pos[16] = 29;
		os->coords_pos[17] = 2;
		os->scale[10] = 1;
		os->scale[11] = 418;

		//7
		os->coords_pos[18] = 521;
		os->coords_pos[19] = 457;
		os->coords_pos[20] = 3;
		os->scale[12] = 1;
		os->scale[13] = 1;

		//8
		os->coords_pos[21] = 106;
		os->coords_pos[22] = 450;
		os->coords_pos[23] = 3;
		os->scale[14] = 1;
		os->scale[15] = 1;

		//9
		os->coords_pos[24] = 266;
		os->coords_pos[25] = 450;
		os->coords_pos[26] = 2;
		os->scale[16] = 94;
		os->scale[17] = 1;

		//10
		os->coords_pos[27] = 0;
		os->coords_pos[28] = 450;
		os->coords_pos[29] = 3;
		os->scale[18] = 1;
		os->scale[19] = 1;

		//11
		os->coords_pos[30] = 547;
		os->coords_pos[31] = 450;
		os->coords_pos[32] = 1;
		os->scale[20] = 1;
		os->scale[21] = 1;

		//12
		os->coords_pos[33] = 568;
		os->coords_pos[34] = 0;
		os->coords_pos[35] = 3;
		os->scale[22] = 1;
		os->scale[23] = 1;

		//13
		os->coords_pos[36] = 105;
		os->coords_pos[37] = 0;
		os->coords_pos[38] = 2;
		os->scale[24] = 155;
		os->scale[25] = 1;

		//14
		os->coords_pos[39] = 0;
		os->coords_pos[40] = 0;
		os->coords_pos[41] = 3;
		os->scale[26] = 1;
		os->scale[27] = 1;
	}
	else{	//2000
		;
	}
}

void setup_free_OS_struct(MinesweeperOS_t *os){
	free(os->ids);
	free(os->coords_pos);
	free(os->coords_frame);
	free(os->scale);
}
