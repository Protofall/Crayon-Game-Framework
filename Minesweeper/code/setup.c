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
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 17;
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
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 512;
			os->coords_pos[(iter * 3) + 1] = 459;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarCurrentTask")){	//For some reason the langIcon uses this for drawing?
		if(sys){
			os->coords_pos[iter * 3] = 106;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 69;
			os->coords_pos[(iter * 3) + 1] = 458;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarFiller")){
		if(sys){
			os->coords_pos[iter * 3] = 266;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 16;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 0;
			os->coords_pos[(iter * 3) + 1] = 458;
			os->coords_pos[(iter * 3) + 2] = 16;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarTime")){
		if(sys){
			os->coords_pos[iter * 3] = 547;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 585;
			os->coords_pos[(iter * 3) + 1] = 5;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "topbarFiller")){
		if(sys){
			os->coords_pos[iter * 3] = 105;
			os->coords_pos[(iter * 3) + 1] = 0;
			os->coords_pos[(iter * 3) + 2] = 16;
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
			os->coords_pos[(iter * 3) + 2] = 16;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 6;
			os->coords_pos[(iter * 3) + 1] = 5;
			os->coords_pos[(iter * 3) + 2] = 16;
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

//For XP mode it will have the "yellowy" polygon. 2000 misses this.
//I THINK THIS IS BROKEN
void setup_bg_untextured_poly(crayon_untextured_array_t *Bg, uint8_t os, uint8_t sd){
	Bg->num_polys = 3;
	if(!os){
		Bg->num_polys += 4;	//Windows 2000 has a few extra polys
	}
	Bg->draw_pos = (uint16_t *) malloc(2 * Bg->num_polys * sizeof(uint16_t));
	Bg->draw_z = (uint8_t *) malloc(Bg->num_polys * sizeof(uint8_t));
	Bg->colours = (uint32_t *) malloc(Bg->num_polys * sizeof(uint32_t));
	Bg->draw_dims = (uint16_t *) malloc(2 * Bg->num_polys * sizeof(uint16_t));
	Bg->rotations = (float *) malloc(sizeof(float));
	Bg->rotations[0] = 0;
	Bg->options = (1 << 1) + (1 << 2) + (1 << 3);	//Z, C and D enabled

	//Grey box
	Bg->draw_pos[0] = 6;
	Bg->draw_pos[1] = 52;
	Bg->draw_z[0] = 9;
	Bg->colours[0] = (255 << 24) | (192 << 16) | (192 << 8) | 192;	//4290822336 (lightGrey)
	Bg->draw_dims[0] = 631;
	Bg->draw_dims[1] = 397;

	//White box
	Bg->draw_pos[2] = 3;
	Bg->draw_pos[3] = 48;
	Bg->draw_z[1] = 8;
	Bg->colours[1] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
	Bg->draw_dims[2] = 634;
	Bg->draw_dims[3] = 401;

	//Yellowy box, only for XP
	if(os){
		Bg->draw_pos[4] = 3;
		Bg->draw_pos[5] = 7;
		Bg->draw_z[2] = 8;
		Bg->colours[2] = (255 << 24) | (236 << 16) | (233 << 8) | 216;	//4293716440 (Yellowy)
		Bg->draw_dims[4] = 635;
		Bg->draw_dims[5] = 447;
	}
	else{
		//Place the polys for 2000's taskbar
		Bg->draw_pos[4] = 0;
		Bg->draw_pos[5] = 452;
		Bg->draw_z[2] = 1;
		Bg->colours[2] = (255 << 24) + (212 << 16) + (208 << 8) + 200;	//??? yellowy grey
		Bg->draw_dims[4] = 640;
		Bg->draw_dims[5] = 28;

		Bg->draw_pos[6] = 0;
		Bg->draw_pos[7] = 453;
		Bg->draw_z[3] = 2;
		Bg->colours[3] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
		Bg->draw_dims[6] = 640;
		Bg->draw_dims[7] = 1;

		//For the time and icons box (This changes based on "sd")
		Bg->draw_pos[8] = 508;
		Bg->draw_pos[9] = 456;
		Bg->draw_z[4] = 2;
		Bg->colours[4] = (255 << 24) + (128 << 16) + (128 << 8) + 128;	//??? (Dark Grey)
		Bg->draw_dims[8] = 129;
		Bg->draw_dims[9] = 21;

		Bg->draw_pos[10] = 509;
		Bg->draw_pos[11] = 457;
		Bg->draw_z[5] = 3;
		Bg->colours[5] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
		Bg->draw_dims[10] = 129;
		Bg->draw_dims[11] = 21;

		Bg->draw_pos[12] = 509;
		Bg->draw_pos[13] = 457;
		Bg->draw_z[6] = 4;
		Bg->colours[6] = (255 << 24) + (212 << 16) + (208 << 8) + 200;	//4290822336 (lightGrey)
		Bg->draw_dims[12] = 128;
		Bg->draw_dims[13] = 20;
	}
}
