#include "setup.h"

void setup_pos_lookup_table(MinesweeperOS_t *os, crayon_spritesheet_t *ss, uint8_t sys, uint8_t iter, uint8_t sd){
	uint8_t anim_id = os->ids[iter];
	if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "aboutLogo")){
		if(sys){
			os->coords_pos[iter * 3] = 100;
			os->coords_pos[(iter * 3) + 1] = 120;
			os->coords_pos[(iter * 3) + 2] = 19;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 100;
			os->coords_pos[(iter * 3) + 1] = 120;
			os->coords_pos[(iter * 3) + 2] = 19;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
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
		if(sys){	//Adjust this with sd
			os->coords_pos[(iter * 3) + 1] = 457;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
			if(sd){
				os->coords_pos[iter * 3] = 501;
			}
			else{
				os->coords_pos[iter * 3] = 521;
			}
		}
		else{
			os->coords_pos[(iter * 3) + 1] = 459;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
			if(sd){
				os->coords_pos[iter * 3] = 512;
			}
			else{
				os->coords_pos[iter * 3] = 530;
			}
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarCurrentTask")){
		if(sys){
			os->coords_pos[iter * 3] = 106;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
		else{
			os->coords_pos[iter * 3] = 71;
			os->coords_pos[(iter * 3) + 1] = 456;
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
			os->coords_pos[iter * 3] = 2;
			os->coords_pos[(iter * 3) + 1] = 456;
			os->coords_pos[(iter * 3) + 2] = 16;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarTimeFiller")){
		if(sys){
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[(iter * 2) + 1] = 1;
			if(sd){
				os->coords_pos[iter * 3] = 609 - 20;
				os->scale[iter * 2] = 9;
			}
			else{
				os->coords_pos[iter * 3] = 609;
				os->scale[iter * 2] = 1;
			}
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarTimeLeft")){
		if(sys){
			os->coords_pos[iter * 3] = 547;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 17;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
			if(sd){
				os->coords_pos[iter * 3] = 547 - 20;
			}
			else{
				os->coords_pos[iter * 3] = 547;
			}
		}
	}
	else if(!strcmp(ss->spritesheet_animation_array[anim_id].animation_name, "taskbarTimeRight")){
		if(sys){
			os->coords_pos[iter * 3] = 612;
			os->coords_pos[(iter * 3) + 1] = 450;
			os->coords_pos[(iter * 3) + 2] = 18;
			os->scale[iter * 2] = 1;
			os->scale[(iter * 2) + 1] = 1;
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
void setup_OS_assets(MinesweeperOS_t *os, crayon_spritesheet_t *ss, uint8_t sys, uint8_t lang, uint8_t sd){
	os->sprite_count = ss->spritesheet_animation_count - 4;	//Two anims we won't included. ItalianTiles, buttons and num_changer
	os->ids = (uint8_t *) malloc(os->sprite_count * sizeof(uint8_t));	//Because we don't include the ital-tiles, this makes other code easier
	os->coords_pos = (uint16_t *) malloc(3 * os->sprite_count * sizeof(uint16_t));	//x, y, z
	os->coords_frame = (uint16_t *) malloc(2 * os->sprite_count * sizeof(uint16_t));	//u, v
	os->scale = (uint16_t *) malloc(2 * os->sprite_count * sizeof(uint16_t));	//scale x/y
	os->windows_ss = ss;
	uint8_t id_count = 0;
	uint8_t iter;
	for(iter = 0; iter < os->sprite_count; iter++){
		//Skip over the 4 assets we don't want in this list
		while(!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "italianTiles") ||
			!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "numberChanger") ||
			!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "checker") ||
			!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "button")){
			id_count++;
		}
		graphics_frame_coordinates(&ss->spritesheet_animation_array[id_count],
			os->coords_frame + (2 * iter), os->coords_frame + (2 * iter) + 1,
			lang ? ss->spritesheet_animation_array[id_count].animation_frames - 1: 0);
		//Since all assets we will store here only have 2 frames if the 2nd frame is italian and we're in italian mode, then choose that
		os->ids[iter] = id_count;
		id_count++;
	}
	for(iter = 0; iter < os->sprite_count; iter++){
		setup_pos_lookup_table(os, ss, sys, iter, sd);
	}

	//Confirm all these positions are accurate
	os->variant_pos = (uint16_t *) malloc(10 * sizeof(uint16_t));
	if(sys){
		os->clock_palette = 61;
		// os->variant_pos[0] = 0;	//Unused, just hear so its easy to get the item you want
		os->variant_pos[1] = 33;
		os->variant_pos[2] = 581;	//The X for this and other changes based on time (Right alighned) so have this be 640 - const - sum of width of chars
		os->variant_pos[3] = 460;
		os->variant_pos[4] = 533;
		os->variant_pos[5] = 457;
		os->variant_pos[6] = 553;
		os->variant_pos[7] = 456;
		os->variant_pos[8] = 0;	//The BS mode icon will go here eventually
		os->variant_pos[9] = 0;
	}
	else{
		os->clock_palette = 62;
		// os->variant_pos[0] = 0;	//Unused, just hear so its easy to get the item you want
		os->variant_pos[1] = 30;
		os->variant_pos[2] = 581;	//Same as id 2 from XP
		os->variant_pos[3] = 463;
		os->variant_pos[4] = 530;
		os->variant_pos[5] = 459;
		os->variant_pos[6] = 548;
		os->variant_pos[7] = 458;
		os->variant_pos[8] = 0;	//The BS mode icon will go here eventually
		os->variant_pos[9] = 0;
	}
}

void setup_free_OS_struct(MinesweeperOS_t *os){
	free(os->ids);
	free(os->coords_pos);
	free(os->coords_frame);
	free(os->scale);
}

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
	Bg->options = (1 << 1) + (1 << 2) + (1 << 3) + (1 << 4);	//Z, C, D and O enabled

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
		Bg->draw_pos[9] = 456;
		Bg->draw_z[4] = 3;
		Bg->colours[4] = (255 << 24) + (128 << 16) + (128 << 8) + 128;	//??? (Dark Grey)
		Bg->draw_dims[9] = 21;

		Bg->draw_pos[11] = 456;
		Bg->draw_z[5] = 2;
		Bg->colours[5] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
		Bg->draw_dims[11] = 22;

		Bg->draw_pos[13] = 457;
		Bg->draw_z[6] = 4;
		Bg->colours[6] = (255 << 24) + (212 << 16) + (208 << 8) + 200;	//4290822336 (lightGrey)
		Bg->draw_dims[13] = 20;

		//Setting the x and dim_x values based on if we have an sd card
		if(sd){
			Bg->draw_pos[8] = 507;	//DGrey
			Bg->draw_dims[8] = 130;
			Bg->draw_pos[10] = 507;	//White
			Bg->draw_dims[10] = 131;
			Bg->draw_pos[12] = 508;	//LGrey
			Bg->draw_dims[12] = 129;
		}
		else{	//Difference of 18 pixels to the right
			Bg->draw_pos[8] = 525;
			Bg->draw_dims[8] = 112;
			Bg->draw_pos[10] = 525;
			Bg->draw_dims[10] = 113;
			Bg->draw_pos[12] = 526;
			Bg->draw_dims[12] = 111;
		}
	}
}

void setup_option_untextured_poly(crayon_untextured_array_t *Options, uint8_t os){
	if(!os){
		Options->num_polys = 15;	//Windows 2000 has a extra polys
	}
	else{
		Options->num_polys = 6;	//XP mode (XP not drawing all of the polys?)
	}
	Options->draw_pos = (uint16_t *) malloc(2 * Options->num_polys * sizeof(uint16_t));
	Options->draw_z = (uint8_t *) malloc(Options->num_polys * sizeof(uint8_t));
	Options->colours = (uint32_t *) malloc(Options->num_polys * sizeof(uint32_t));
	Options->draw_dims = (uint16_t *) malloc(2 * Options->num_polys * sizeof(uint16_t));
	Options->rotations = (float *) malloc(sizeof(float));
	Options->rotations[0] = 0;
	Options->options = (1 << 1) + (1 << 2) + (1 << 3) + (1 << 4);	//Z, C, D and O enabled

	uint16_t x, y[3];
	x = 395; y[0] = 140; y[1] = 180; y[2] = 220;
	int i;

	if(!os){
		uint8_t dim_x, dim_y;
		dim_x = 25; dim_y = 21;
		for(i = 0; i < 3; i++){
			//Centre/top white
			Options->draw_pos[(10 * i) + 0] = x + 2;
			Options->draw_pos[(10 * i) + 1] = y[i] + 2;
			Options->draw_z[(5 * i) + 0] = 29;
			Options->colours[(5 * i) + 0] = 0xFFFFFFFF;	//White
			Options->draw_dims[(10 * i) + 0] = dim_x - 4;
			Options->draw_dims[(10 * i) + 1] = dim_y - 4;

			//Middle colours
			Options->draw_pos[(10 * i) + 2] = x + 1;
			Options->draw_pos[(10 * i) + 3] = y[i] + 1;
			Options->draw_z[(5 * i) + 1] = 28;
			Options->colours[(5 * i) + 1] = 0xFF404040;	//Dark grey
			Options->draw_dims[(10 * i) + 2] = dim_x - 3;
			Options->draw_dims[(10 * i) + 3] = dim_y - 3;

			Options->draw_pos[(10 * i) + 4] = x + 1;
			Options->draw_pos[(10 * i) + 5] = y[i] + 1;
			Options->draw_z[(5 * i) + 2] = 27;
			Options->colours[(5 * i) + 2] = 0xFFD4D0C8;	//Light grey
			Options->draw_dims[(10 * i) + 4] = dim_x - 2;
			Options->draw_dims[(10 * i) + 5] = dim_y - 2;

			//Back colours
			Options->draw_pos[(10 * i) + 6] = x;
			Options->draw_pos[(10 * i) + 7] = y[i];
			Options->draw_z[(5 * i) + 3] = 26;
			Options->colours[(5 * i) + 3] = 0xFF808080;	//Medium grey
			Options->draw_dims[(10 * i) + 6] = dim_x - 1;
			Options->draw_dims[(10 * i) + 7] = dim_y - 1;

			Options->draw_pos[(10 * i) + 8] = x;
			Options->draw_pos[(10 * i) + 9] = y[i];
			Options->draw_z[(5 * i) + 4] = 25;
			Options->colours[(5 * i) + 4] = 0xFFFFFFFF;	//White
			Options->draw_dims[(10 * i) + 8] = dim_x;
			Options->draw_dims[(10 * i) + 9] = dim_y;
		}
	}
	else{
		uint8_t dim_x, dim_y;
		dim_x = 25 + 17; dim_y = 20;
		for(i = 0; i < 3; i++){
			Options->draw_pos[(4 * i) + 0] = x + 1;
			Options->draw_pos[(4 * i) + 1] = y[i] + 1;
			Options->draw_z[(2 * i) + 0] = 26;
			Options->colours[(2 * i) + 0] = 0xFFFFFFFF;	//White
			Options->draw_dims[(4 * i) + 0] = dim_x - 2;
			Options->draw_dims[(4 * i) + 1] = dim_y - 2;

			Options->draw_pos[(4 * i) + 2] = x;
			Options->draw_pos[(4 * i) + 3] = y[i];
			Options->draw_z[(2 * i) + 1] = 25;
			Options->colours[(2 * i) + 1] = 0xFF7F9DB9;	//Murky Light Blue
			Options->draw_dims[(4 * i) + 2] = dim_x;
			Options->draw_dims[(4 * i) + 3] = dim_y;
		}
	}
}
