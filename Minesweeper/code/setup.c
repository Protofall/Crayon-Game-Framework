#include "setup.h"

void setup_pos_lookup_table(MinesweeperOS_t *os, uint8_t sys, uint8_t i, uint8_t sd){
	if(!strcmp(os->assets[i]->animation->animation_name, "aboutLogo")){
		os->assets[i]->positions[0] = 100;
		os->assets[i]->positions[1] = 120;
		os->assets[i]->draw_z[0] = 19;
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "boarderBottom")){
		if(sys){
			os->assets[i]->positions[0] = 3;
			os->assets[i]->positions[1] = 447;
			os->assets[i]->draw_z[0] = 16;
			// os->assets[i]->scales[0] = 633;	// Umm...the scale only goes up to 255...
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "boarderBottomLeft")){
		if(sys){
			os->assets[i]->positions[0] = 0;
			os->assets[i]->positions[1] = 447;
			os->assets[i]->draw_z[0] = 16;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "boarderBottomRight")){
		if(sys){
			os->assets[i]->positions[0] = 636;
			os->assets[i]->positions[1] = 447;
			os->assets[i]->draw_z[0] = 17;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "boarderLeft")){
		if(sys){
			os->assets[i]->positions[0] = 0;
			os->assets[i]->positions[1] = 29;
			os->assets[i]->draw_z[0] = 16;
			// os->assets[i]->scales[0] = 418;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "boarderRight")){
		if(sys){
			os->assets[i]->positions[0] = 637;
			os->assets[i]->positions[1] = 29;
			os->assets[i]->draw_z[0] = 16;
			// os->assets[i]->scales[1] = 418;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "langIcon")){
		if(sys){
			os->assets[i]->positions[1] = 457;
			os->assets[i]->draw_z[0] = 17;
			if(sd){
				os->assets[i]->positions[0] = 501;
			}
			else{
				os->assets[i]->positions[0] = 521;
			}
		}
		else{
			os->assets[i]->positions[1] = 459;
			os->assets[i]->draw_z[0] = 17;
			if(sd){
				os->assets[i]->positions[0] = 512;
			}
			else{
				os->assets[i]->positions[0] = 530;
			}
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "taskbarCurrentTask")){
		if(sys){
			os->assets[i]->positions[0] = 106;
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 17;
		}
		else{
			os->assets[i]->positions[0] = 71;
			os->assets[i]->positions[1] = 456;
			os->assets[i]->draw_z[0] = 17;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "taskbarFiller")){
		if(sys){
			os->assets[i]->positions[0] = 266;
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[0] = 94;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "taskbarStart")){
		if(sys){
			os->assets[i]->positions[0] = 0;
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 16;
		}
		else{
			os->assets[i]->positions[0] = 2;
			os->assets[i]->positions[1] = 456;
			os->assets[i]->draw_z[0] = 16;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "taskbarTimeFiller")){
		if(sys){
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 17;
			if(sd){
				os->assets[i]->positions[0] = 589;
				os->assets[i]->scales[0] = 9;
			}
			else{
				os->assets[i]->positions[0] = 609;
			}
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "taskbarTimeLeft")){
		if(sys){
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 17;
			if(sd){
				os->assets[i]->positions[0] = 527;
			}
			else{
				os->assets[i]->positions[0] = 547;
			}
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "taskbarTimeRight")){
		if(sys){
			os->assets[i]->positions[0] = 612;
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 18;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "topbarAdjusts")){
		if(sys){
			os->assets[i]->positions[0] = 568;
			os->assets[i]->positions[1] = 0;
			os->assets[i]->draw_z[0] = 17;
		}
		else{
			os->assets[i]->positions[0] = 585;
			os->assets[i]->positions[1] = 5;
			os->assets[i]->draw_z[0] = 17;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "topbarFiller")){
		if(sys){
			os->assets[i]->positions[0] = 105;
			os->assets[i]->positions[1] = 0;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[0] = 155;
		}
	}
	else if(!strcmp(os->assets[i]->animation->animation_name, "topbarName")){
		if(sys){
			os->assets[i]->positions[0] = 0;
			os->assets[i]->positions[1] = 0;
			os->assets[i]->draw_z[0] = 16;
		}
		else{
			os->assets[i]->positions[0] = 6;
			os->assets[i]->positions[1] = 5;
			os->assets[i]->draw_z[0] = 16;
		}
	}
}

// void setup_OS_assets(MinesweeperOS_t *os, crayon_spritesheet_t *ss, uint8_t sys, uint8_t lang, uint8_t sd, uint8_t region){
void setup_OS_assets(MinesweeperOS_t *os, crayon_spritesheet_t *ss, crayon_palette_t *pal, uint8_t sys, uint8_t lang, uint8_t sd){
	os->num_assets = ss->spritesheet_animation_count - 4;	//Minus 4 because of removed assets
	os->assets = (crayon_textured_array_t **) malloc(os->num_assets * sizeof(crayon_textured_array_t *));	//Allocate space for all OS draw struct pointers

	//Allocate space for all draw structs
	uint8_t i;
	uint8_t id_count = 0;
	//For all of them except the sd and region icons
	for(i = 0; i < os->num_assets; i++){
		os->assets[i] = (crayon_textured_array_t *) malloc(sizeof(crayon_textured_array_t));
		while(!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "italianTiles") ||
			!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "numberChanger") ||
			!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "checker") ||
			!strcmp(ss->spritesheet_animation_array[id_count].animation_name, "button")){
			id_count++;
		}
		uint8_t multi_frames = 0;
		if(lang){
			multi_frames = 1;
		}
		crayon_memory_set_sprite_array(os->assets[i], 1, 2 * multi_frames, 0, multi_frames, 0, 0, 0, 0, 0, ss, &ss->spritesheet_animation_array[id_count], pal);
		os->assets[i]->scales[0] = 1;
		os->assets[i]->scales[1] = 1;
		os->assets[i]->flips[0] = 0;
		os->assets[i]->rotations[0] = 0;
		os->assets[i]->colours[0] = 0;
		setup_pos_lookup_table(os, sys, i, sd);
		graphics_frame_coordinates(os->assets[i]->animation, os->assets[i]->frame_coord_map + 0, os->assets[i]->frame_coord_map + 1, 0);
		if(os->assets[i]->animation->animation_frames > 1){
			graphics_frame_coordinates(os->assets[i]->animation, os->assets[i]->frame_coord_map + 2, os->assets[i]->frame_coord_map + 3, 1);
		}
		if(lang && os->assets[i]->animation->animation_frames > 1){
			os->assets[i]->frame_coord_keys[0] = 1;
		}
		else{
			os->assets[i]->frame_coord_keys[0] = 0;
		}
		id_count++;
	}

	//The y position of the tabs
	if(sys){
		os->tabs_y = 33;
		os->clock_x = 581;
		os->clock_y = 460;
	}
	else{
		os->tabs_y = 30;
		os->clock_x = 581;
		os->clock_y = 463;
	}
}

void setup_OS_assets_icons(MinesweeperOS_t *os, crayon_spritesheet_t *Icons, crayon_palette_t *Icons_P, uint8_t sys, uint8_t region){
	uint8_t i;
	for(i = 0; i < Icons->spritesheet_animation_count; i++){
		if(!strcmp(Icons->spritesheet_animation_array[i].animation_name, "sd")){
			crayon_memory_set_sprite_array(&os->sd, 1, 1, 0, 0, 0, 0, 0, 0, 0, Icons, &Icons->spritesheet_animation_array[i], Icons_P);
			graphics_frame_coordinates(os->sd.animation, os->sd.frame_coord_map, os->sd.frame_coord_map + 1, 0);
			os->sd.scales[0] = 1;
			os->sd.scales[1] = 1;
			os->sd.flips[0] = 0;
			os->sd.rotations[0] = 0;
			os->sd.colours[0] = 0;
			os->sd.frame_coord_keys[0] = 0;
			if(sys){
				os->sd.positions[0] = 533;	//sd
				os->sd.positions[1] = 457;
			}
			else{
				os->sd.positions[0] = 530;	//sd
				os->sd.positions[1] = 459;
			}
		}
		if(!strcmp(Icons->spritesheet_animation_array[i].animation_name, "regionIcons")){
			crayon_memory_set_sprite_array(&os->region, 1, 1, 0, 0, 0, 0, 0, 0, 0, Icons, &Icons->spritesheet_animation_array[i], Icons_P);
			graphics_frame_coordinates(os->region.animation, os->region.frame_coord_map, os->region.frame_coord_map + 1, region);
			os->region.scales[0] = 1;
			os->region.scales[1] = 1;
			os->region.flips[0] = 0;
			os->region.rotations[0] = 0;
			os->region.colours[0] = 0;
			os->region.frame_coord_keys[0] = 0;
			if(sys){
				os->region.positions[0] = 553;	//region
				os->region.positions[1] = 456;
			}
			else{
				os->region.positions[0] = 548;	//region
				os->region.positions[1] = 458;
			}
		}
	}
}

void setup_free_OS_struct(MinesweeperOS_t *os){
	uint8_t i;
	for(i = 0; i < os->num_assets; i++){
		crayon_memory_free_sprite_array(os->assets[i], 0, 0);
	}
	free(os->assets);
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
