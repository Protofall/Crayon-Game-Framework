#include "setup.h"

void setup_pos_lookup_table(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t iter){
	uint8_t anim_id = os->ids[iter];
	if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "aboutLogo")){
		if(sys){	//Populate this later
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 0;
			os->coords_pos[(iter * 3) + 2] = 0;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			// os->coords_pos[iter * 3] = ;
			// os->coords_pos[(iter * 3) + 1] = ;
			// os->coords_pos[(iter * 3) + 2] = ;
			// os->scale[iter * 2] = ;
			// os->scale[(iter * 2) + 1] = ;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "boarderBottom")){
		if(sys){
			os->coords_pos[iter * 3] = 3;
			os->coords_pos[(iter * 3) + 1] = 447;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 633;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "boarderBottomLeft")){
		if(sys){
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 447;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "boarderBottomRight")){
		if(sys){
			os->coords_pos[iter * 3] = 636;
			os->coords_pos[(iter * 3) + 1] = 447;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "boarderLeft")){
		if(sys){
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 29;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 418;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "boarderRight")){
		if(sys){
			os->coords_pos[iter * 3] = 637;
			os->coords_pos[(iter * 3) + 1] = 29;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 418;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "langIcon")){
		if(sys){
			os->coords_pos[iter * 3] = 521;
			os->coords_pos[(iter * 3) + 1] = 457;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 512;
			os->coords_pos[(iter * 3) + 1] = 459;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarCurrentTask")){	//For some reason the langIcon uses this for drawing?
		if(sys){
			os->coords_pos[iter * 3] = 106;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 69;
			os->coords_pos[(iter * 3) + 1] = 458;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarFiller")){
		if(sys){
			os->coords_pos[iter * 3] = 266;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 94;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarStart")){
		if(sys){
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 458;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarTime")){
		if(sys){
			os->coords_pos[iter * 3] = 547;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "topbarAdjusts")){
		if(sys){
			os->coords_pos[iter * 3] = 568;
			os->coords_pos[(iter * 3) + 1] = 0;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 588;	//Modify this later?
			os->coords_pos[(iter * 3) + 1] = 2;
			os->coords_pos[(iter * 3) + 2] = 5;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "topbarFiller")){
		if(sys){
			os->coords_pos[iter * 3] = 105;
			os->coords_pos[(iter * 3) + 1] = 0;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 155;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "topbarName")){
		if(sys){
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 0;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 3;
			os->coords_pos[(iter * 3) + 1] = 2;
			os->coords_pos[(iter * 3) + 2] = 4;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else{
		error_freeze("Unlisted file detected. %s", ss->spritesheet_animation_array[os->ids[anim_id]].animation_name);
	}
}

//There seems to be an allocation issue here...
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
		uint8_t lang_frame = 0;
		if(lang && ss->spritesheet_animation_array[id_count].animation_frames > 1){
			lang_frame = 1;
		}
		if(!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "italianTiles")){
			id_count++;
		}
		graphics_frame_coordinates(&ss->spritesheet_animation_array[id_count],
			os->coords_frame + (2 * iter), os->coords_frame + (2 * iter) + 1, lang_frame);
		os->ids[iter] = id_count;
		id_count++;
	}
	for(iter = 0; iter < os->sprite_count; iter++){
		setup_pos_lookup_table(os, ss, sys, iter);
	}
}

void setup_free_OS_struct(MinesweeperOS_t *os){
	free(os->ids);
	free(os->coords_pos);
	free(os->coords_frame);
	free(os->scale);
}

//Since this is only doing boarders, I might just make a boarder function instead...
//Note even though they all share the same z value, polys higher on the list will be drawn ontop (On lxdream, order seems to be reversed for redream)
//NOTE. I might need a seperate draw function for the grid boarder, otherwise I'll be using 9 polys instead of possibly 6
void setup_grid_untextured_poly(crayon_untextured_array_t *Grid_polys, uint8_t grid_x, uint8_t grid_y,
	uint16_t grid_start_x, uint16_t grid_start_y){
	Grid_polys->num_polys = 4;
	Grid_polys->draw_pos = (uint16_t *) malloc(2 * Grid_polys->num_polys * sizeof(uint16_t));
	Grid_polys->draw_z = (uint8_t *) malloc(sizeof(uint8_t));
	Grid_polys->colours = (uint32_t *) malloc(Grid_polys->num_polys * sizeof(uint32_t));
	Grid_polys->draw_dims = (uint16_t *) malloc(2 * Grid_polys->num_polys * sizeof(uint16_t));
	// Grid_polys->rotations = (float *) malloc(sizeof(float));	//We don't really use this yet
	// Grid_polys->rotations[0] = 0;
	Grid_polys->options = (1 << 1) + (1 << 2);	//C and D enabled

	Grid_polys->draw_z[0] = 4;	//The polys never (Visibly) overlap so we can keep the same Z for all of them

	//Left digit dark grey
	Grid_polys->draw_pos[0] = 19;
	Grid_polys->draw_pos[1] = 64;
	Grid_polys->colours[0] = (255 << 24) | (128 << 16) | (128 << 8) | 128;	//4286611584
	Grid_polys->draw_dims[0] = 40;
	Grid_polys->draw_dims[1] = 24;

	//Left digit white
	Grid_polys->draw_pos[2] = 20;
	Grid_polys->draw_pos[3] = 65;
	Grid_polys->colours[1] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295
	Grid_polys->draw_dims[2] = 40;
	Grid_polys->draw_dims[3] = 24;

	//Right digit dark grey
	Grid_polys->draw_pos[4] = 580;
	Grid_polys->draw_pos[5] = 64;
	Grid_polys->colours[2] = (255 << 24) | (128 << 16) | (128 << 8) | 128;	//4286611584
	Grid_polys->draw_dims[4] = 40;
	Grid_polys->draw_dims[5] = 24;

	//Right digit white
	Grid_polys->draw_pos[6] = 581;
	Grid_polys->draw_pos[7] = 65;
	Grid_polys->colours[3] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295
	Grid_polys->draw_dims[6] = 40;
	Grid_polys->draw_dims[7] = 24;

	//Grid boarder (Probs will remove eventually and make a special draw function for it?)
	// Grid_polys->draw_pos[8] = grid_start_x + (16 * grid_x);	//Right side white
	// Grid_polys->draw_pos[9] = grid_start_y - 2;
	// Grid_polys->colours[4] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (white)
	// Grid_polys->draw_dims[8] = 3;
	// Grid_polys->draw_dims[9] = (16 * grid_y) + 5;

	// Grid_polys->draw_pos[10] = grid_start_x - 2;	//Bottom side white
	// Grid_polys->draw_pos[11] = grid_start_y + (16 * grid_y);
	// Grid_polys->colours[5] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (white)
	// Grid_polys->draw_dims[10] = (16 * grid_x) + 5;
	// Grid_polys->draw_dims[11] = 3;

	// Grid_polys->draw_pos[12] = grid_start_x - 3;	//Dark grey back board
	// Grid_polys->draw_pos[13] = grid_start_y - 3;
	// Grid_polys->colours[6] = (255 << 24) | (128 << 16) | (128 << 8) | 128;	//4286611584 (darkGrey)
	// Grid_polys->draw_dims[12] = (16 * grid_x) + 5;
	// Grid_polys->draw_dims[13] = (16 * grid_y) + 5;

	// // Grid_polys->colours[7] = (255 << 24) | (192 << 16) | (192 << 8) | 192;	//4290822336 (lightGrey)

	// //Delete this later, this only exist right now so lxdream doesn't crash -_-
	// Grid_polys->draw_pos[14] = grid_start_x - 3;	//Dark grey back board
	// Grid_polys->draw_pos[15] = grid_start_y - 3;
	// Grid_polys->colours[7] = (255 << 24) | (128 << 16) | (128 << 8) | 128;	//4286611584 (darkGrey)
	// Grid_polys->draw_dims[14] = (16 * grid_x) + 5;
	// Grid_polys->draw_dims[15] = (16 * grid_y) + 5;
}

void setup_bg_untextured_poly(crayon_untextured_array_t *Bg){
	Bg->num_polys = 3;
	Bg->draw_pos = (uint16_t *) malloc(2 * Bg->num_polys * sizeof(uint16_t));
	Bg->draw_z = (uint8_t *) malloc(Bg->num_polys * sizeof(uint8_t));
	Bg->colours = (uint32_t *) malloc(Bg->num_polys * sizeof(uint32_t));
	Bg->draw_dims = (uint16_t *) malloc(2 * Bg->num_polys * sizeof(uint16_t));
	// Bg->rotations = (float *) malloc(sizeof(float));	//We don't really use this yet
	// Bg->rotations[0] = 0;
	Bg->options = (1 << 1) + (1 << 2) + (1 << 3);	//Z, C and D enabled

	//Grey box
	Bg->draw_pos[0] = 6;
	Bg->draw_pos[1] = 52;
	Bg->draw_z[0] = 3;
	Bg->colours[0] = (255 << 24) | (192 << 16) | (192 << 8) | 192;	//4290822336 (lightGrey)
	Bg->draw_dims[0] = 634;
	Bg->draw_dims[1] = 428;

	//White box
	Bg->draw_pos[2] = 3;
	Bg->draw_pos[3] = 48;
	Bg->draw_z[1] = 2;
	Bg->colours[1] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
	Bg->draw_dims[2] = 637;
	Bg->draw_dims[3] = 431;

	//Yellowy box
	Bg->draw_pos[4] = 0;
	Bg->draw_pos[5] = 0;
	Bg->draw_z[2] = 1;
	Bg->colours[2] = (255 << 24) | (236 << 16) | (233 << 8) | 216;	//4293716440 (Yellowy)
	Bg->draw_dims[4] = 640;
	Bg->draw_dims[5] = 49;
}
