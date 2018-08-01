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
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 3;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 512;
			os->coords_pos[(iter * 3) + 1] = 459;
			os->coords_pos[(iter * 3) + 2] = 3;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarCurrentTask")){	//For some reason the langIcon uses this for drawing?
		if(sys){
			os->coords_pos[iter * 3] = 106;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 3;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 69;
			os->coords_pos[(iter * 3) + 1] = 458;
			os->coords_pos[(iter * 3) + 2] = 2;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarFiller")){
		if(sys){
			os->coords_pos[iter * 3] = 266;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 3;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 458;
			os->coords_pos[(iter * 3) + 2] = 2;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarTime")){
		if(sys){
			os->coords_pos[iter * 3] = 547;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 3;
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
			os->coords_pos[(iter * 3) + 2] = 3;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 588;	//Modify this later?
			os->coords_pos[(iter * 3) + 1] = 2;
			os->coords_pos[(iter * 3) + 2] = 2;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "topbarFiller")){
		if(sys){
			os->coords_pos[iter * 3] = 105;
			os->coords_pos[(iter * 3) + 1] = 0;
			os->coords_pos[(iter * 3) + 2] = 2;
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
			os->coords_pos[(iter * 3) + 2] = 3;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 3;
			os->coords_pos[(iter * 3) + 1] = 2;
			os->coords_pos[(iter * 3) + 2] = 2;
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
		uint8_t langFrame = 0;
		if(lang && ss->spritesheet_animation_array[id_count].animation_frames > 1){
			langFrame = 1;
		}
		if(!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "italianTiles")){
			id_count++;
		}
		graphics_frame_coordinates(&ss->spritesheet_animation_array[id_count],
			os->coords_frame + (2 * iter), os->coords_frame + (2 * iter) + 1, langFrame);
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
