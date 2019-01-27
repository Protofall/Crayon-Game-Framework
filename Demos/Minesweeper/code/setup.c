#include "setup.h"

void setup_pos_lookup_table(MinesweeperOS_t *os, uint8_t sys, uint8_t i, uint8_t sd){
	if(!strcmp(os->assets[i]->animation->name, "aboutLogo")){
		os->assets[i]->positions[0] = 100;
		os->assets[i]->positions[1] = 120;
		os->assets[i]->draw_z[0] = 19;
	}
	else if(!strcmp(os->assets[i]->animation->name, "boarderBottom")){
		if(sys){
			os->assets[i]->positions[0] = 3;
			os->assets[i]->positions[1] = 447;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[0] = 211;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "boarderBottomLeft")){
		if(sys){
			os->assets[i]->positions[0] = 0;
			os->assets[i]->positions[1] = 447;
			os->assets[i]->draw_z[0] = 17;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "boarderBottomRight")){
		if(sys){
			os->assets[i]->positions[0] = 636;
			os->assets[i]->positions[1] = 447;
			os->assets[i]->draw_z[0] = 17;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "boarderLeft")){
		if(sys){
			os->assets[i]->positions[0] = 0;
			os->assets[i]->positions[1] = 29;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[1] = 209;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "boarderRight")){
		if(sys){
			os->assets[i]->positions[0] = 637;
			os->assets[i]->positions[1] = 29;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[1] = 209;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "langIcon")){
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
	else if(!strcmp(os->assets[i]->animation->name, "taskbarCurrentTask")){
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
	else if(!strcmp(os->assets[i]->animation->name, "taskbarFiller")){
		if(sys){
			os->assets[i]->positions[0] = 266;
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[0] = 94;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "taskbarStart")){
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
	else if(!strcmp(os->assets[i]->animation->name, "taskbarTimeFiller")){
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
	else if(!strcmp(os->assets[i]->animation->name, "taskbarTimeLeft")){
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
	else if(!strcmp(os->assets[i]->animation->name, "taskbarTimeRight")){
		if(sys){
			os->assets[i]->positions[0] = 612;
			os->assets[i]->positions[1] = 450;
			os->assets[i]->draw_z[0] = 18;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "topbarAdjusts")){
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
	else if(!strcmp(os->assets[i]->animation->name, "topbarFiller")){
		if(sys){
			os->assets[i]->positions[0] = 105;
			os->assets[i]->positions[1] = 0;
			os->assets[i]->draw_z[0] = 16;
			os->assets[i]->scales[0] = 155;
		}
	}
	else if(!strcmp(os->assets[i]->animation->name, "topbarName")){
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

void setup_OS_assets(MinesweeperOS_t *os, crayon_spritesheet_t *ss, crayon_palette_t *pal, uint8_t sys, uint8_t lang, uint8_t sd){
	os->num_assets = ss->animation_count - 6;	//Minus 6 because of removed assets
	os->assets = (crayon_sprite_array_t **) malloc(os->num_assets * sizeof(crayon_sprite_array_t *));	//Allocate space for all OS draw struct pointers

	//Allocate space for all draw structs
	uint8_t i;
	uint8_t id_count = 0;
	//For all of them except the sd and region icons
	for(i = 0; i < os->num_assets; i++){
		os->assets[i] = (crayon_sprite_array_t *) malloc(sizeof(crayon_sprite_array_t));
		while(!strcmp(ss->animation_array[id_count].name, "italianTiles") ||
			!strcmp(ss->animation_array[id_count].name, "numberChanger") ||
			!strcmp(ss->animation_array[id_count].name, "checker") ||
			!strcmp(ss->animation_array[id_count].name, "miniButton") ||
			!strcmp(ss->animation_array[id_count].name, "mediumButton") ||
			!strcmp(ss->animation_array[id_count].name, "button")){
			id_count++;
		}
		if(!strcmp(ss->animation_array[id_count].name, "langIcon")){
			os->lang_id = i;
		}
		uint8_t multi_frames = 0;
		if(ss->animation_array[id_count].frame_count > 1){
			multi_frames = 1;
		}
		crayon_memory_init_sprite_array(os->assets[i], 1, 2 * multi_frames, 0, multi_frames, 0, 0, 0, 0, 0, ss, &ss->animation_array[id_count], pal);
		os->assets[i]->scales[0] = 1;
		os->assets[i]->scales[1] = 1;
		os->assets[i]->flips[0] = 0;
		os->assets[i]->rotations[0] = 0;
		os->assets[i]->colours[0] = 0;
		setup_pos_lookup_table(os, sys, i, sd);
		crayon_graphics_frame_coordinates(os->assets[i]->animation, os->assets[i]->frame_coord_map + 0, os->assets[i]->frame_coord_map + 1, 0);
		if(os->assets[i]->animation->frame_count > 1){
			crayon_graphics_frame_coordinates(os->assets[i]->animation, os->assets[i]->frame_coord_map + 2, os->assets[i]->frame_coord_map + 3, 1);
		}
		if(lang && os->assets[i]->animation->frame_count > 1){
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
	for(i = 0; i < Icons->animation_count; i++){
		if(!strcmp(Icons->animation_array[i].name, "sd")){
			crayon_memory_init_sprite_array(&os->sd, 1, 1, 0, 0, 0, 0, 0, 0, 0, Icons, &Icons->animation_array[i], Icons_P);
			crayon_graphics_frame_coordinates(os->sd.animation, os->sd.frame_coord_map, os->sd.frame_coord_map + 1, 0);
			os->sd.draw_z[0] = 21;
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
		if(!strcmp(Icons->animation_array[i].name, "regionIcons")){
			crayon_memory_init_sprite_array(&os->region, 1, 1, 0, 0, 0, 0, 0, 0, 0, Icons, &Icons->animation_array[i], Icons_P);
			crayon_graphics_frame_coordinates(os->region.animation, os->region.frame_coord_map, os->region.frame_coord_map + 1, region);
			os->region.draw_z[0] = 21;
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
	crayon_memory_free_sprite_array(&os->sd, 0, 0);
	crayon_memory_free_sprite_array(&os->region, 0, 0);
	//Clock palette points to a palette that is already free-d elsewhere
}

void setup_bg_untextured_poly(crayon_untextured_array_t *Bg, uint8_t os, uint8_t sd){
	Bg->num_polys = 3;
	if(!os){
		Bg->num_polys += 4;	//Windows 2000 has a few extra polys
	}
	Bg->positions = (float *) malloc(2 * Bg->num_polys * sizeof(float));
	Bg->draw_z = (uint8_t *) malloc(Bg->num_polys * sizeof(uint8_t));
	Bg->colours = (uint32_t *) malloc(Bg->num_polys * sizeof(uint32_t));
	Bg->draw_dims = (uint16_t *) malloc(2 * Bg->num_polys * sizeof(uint16_t));
	Bg->rotations = (float *) malloc(sizeof(float));
	Bg->rotations[0] = 0;
	Bg->options = (1 << 1) + (1 << 2) + (1 << 3);	//Z, C and D enabled

	//Grey box
	Bg->positions[0] = 6;
	Bg->positions[1] = 52;
	Bg->draw_z[0] = 9;
	Bg->colours[0] = (255 << 24) | (192 << 16) | (192 << 8) | 192;	//4290822336 (lightGrey)
	Bg->draw_dims[0] = 631;
	Bg->draw_dims[1] = 397;

	//White box
	Bg->positions[2] = 3;
	Bg->positions[3] = 48;
	Bg->draw_z[1] = 8;
	Bg->colours[1] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
	Bg->draw_dims[2] = 634;
	Bg->draw_dims[3] = 401;

	//Yellowy box, only for XP
	if(os){
		Bg->positions[4] = 3;
		Bg->positions[5] = 7;
		Bg->draw_z[2] = 8;
		Bg->colours[2] = (255 << 24) | (236 << 16) | (233 << 8) | 216;	//4293716440 (Yellowy)
		Bg->draw_dims[4] = 635;
		Bg->draw_dims[5] = 447;
	}
	else{
		//Place the polys for 2000's taskbar
		Bg->positions[4] = 0;
		Bg->positions[5] = 452;
		Bg->draw_z[2] = 1;
		Bg->colours[2] = (255 << 24) + (212 << 16) + (208 << 8) + 200;	//??? yellowy grey
		Bg->draw_dims[4] = 640;
		Bg->draw_dims[5] = 28;

		Bg->positions[6] = 0;
		Bg->positions[7] = 453;
		Bg->draw_z[3] = 2;
		Bg->colours[3] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
		Bg->draw_dims[6] = 640;
		Bg->draw_dims[7] = 1;

		//For the time and icons box (This changes based on "sd")
		Bg->positions[9] = 456;
		Bg->draw_z[4] = 3;
		Bg->colours[4] = (255 << 24) + (128 << 16) + (128 << 8) + 128;	//??? (Dark Grey)
		Bg->draw_dims[9] = 21;

		Bg->positions[11] = 456;
		Bg->draw_z[5] = 2;
		Bg->colours[5] = (255 << 24) | (255 << 16) | (255 << 8) | 255;	//4294967295 (White)
		Bg->draw_dims[11] = 22;

		Bg->positions[13] = 457;
		Bg->draw_z[6] = 4;
		Bg->colours[6] = (255 << 24) + (212 << 16) + (208 << 8) + 200;	//4290822336 (lightGrey)
		Bg->draw_dims[13] = 20;

		//Setting the x and dim_x values based on if we have an sd card
		if(sd){
			Bg->positions[8] = 507;	//DGrey
			Bg->draw_dims[8] = 130;
			Bg->positions[10] = 507;	//White
			Bg->draw_dims[10] = 131;
			Bg->positions[12] = 508;	//LGrey
			Bg->draw_dims[12] = 129;
		}
		else{	//Difference of 18 pixels to the right
			Bg->positions[8] = 525;
			Bg->draw_dims[8] = 112;
			Bg->positions[10] = 525;
			Bg->draw_dims[10] = 113;
			Bg->positions[12] = 526;
			Bg->draw_dims[12] = 111;
		}
	}
}

void setup_option_untextured_poly(crayon_untextured_array_t *Options, crayon_sprite_array_t * num_changers, uint8_t os){
	if(!os){
		Options->num_polys = 15;	//Windows 2000 has a extra polys
	}
	else{
		Options->num_polys = 6;	//XP mode (XP not drawing all of the polys?)
	}
	Options->positions = (float *) malloc(2 * Options->num_polys * sizeof(float));
	Options->draw_z = (uint8_t *) malloc(Options->num_polys * sizeof(uint8_t));
	Options->colours = (uint32_t *) malloc(Options->num_polys * sizeof(uint32_t));
	Options->draw_dims = (uint16_t *) malloc(2 * Options->num_polys * sizeof(uint16_t));
	Options->rotations = (float *) malloc(sizeof(float));
	Options->rotations[0] = 0;
	Options->options = (1 << 1) + (1 << 2) + (1 << 3);	//Z, C and D enabled

	uint16_t x, y[3];
	x = num_changers->positions[0] - 25; y[0] = num_changers->positions[1] - (2 * os);
	y[1] = num_changers->positions[3] - (2 * os); y[2] = num_changers->positions[5] - (2 * os);
	int i;

	if(!os){
		uint8_t dim_x, dim_y;
		dim_x = 25; dim_y = 21;
		for(i = 0; i < 3; i++){
			//Centre/top white
			Options->positions[(10 * i) + 0] = x + 2;
			Options->positions[(10 * i) + 1] = y[i] + 2;
			Options->draw_z[(5 * i) + 0] = 29;
			Options->colours[(5 * i) + 0] = 0xFFFFFFFF;	//White
			Options->draw_dims[(10 * i) + 0] = dim_x - 4;
			Options->draw_dims[(10 * i) + 1] = dim_y - 4;

			//Middle colours
			Options->positions[(10 * i) + 2] = x + 1;
			Options->positions[(10 * i) + 3] = y[i] + 1;
			Options->draw_z[(5 * i) + 1] = 28;
			Options->colours[(5 * i) + 1] = 0xFF404040;	//Dark grey
			Options->draw_dims[(10 * i) + 2] = dim_x - 3;
			Options->draw_dims[(10 * i) + 3] = dim_y - 3;

			Options->positions[(10 * i) + 4] = x + 1;
			Options->positions[(10 * i) + 5] = y[i] + 1;
			Options->draw_z[(5 * i) + 2] = 27;
			Options->colours[(5 * i) + 2] = 0xFFD4D0C8;	//Light grey
			Options->draw_dims[(10 * i) + 4] = dim_x - 2;
			Options->draw_dims[(10 * i) + 5] = dim_y - 2;

			//Back colours
			Options->positions[(10 * i) + 6] = x;
			Options->positions[(10 * i) + 7] = y[i];
			Options->draw_z[(5 * i) + 3] = 26;
			Options->colours[(5 * i) + 3] = 0xFF808080;	//Medium grey
			Options->draw_dims[(10 * i) + 6] = dim_x - 1;
			Options->draw_dims[(10 * i) + 7] = dim_y - 1;

			Options->positions[(10 * i) + 8] = x;
			Options->positions[(10 * i) + 9] = y[i];
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
			Options->positions[(4 * i) + 0] = x + 1;
			Options->positions[(4 * i) + 1] = y[i] + 1;
			Options->draw_z[(2 * i) + 0] = 26;
			Options->colours[(2 * i) + 0] = 0xFFFFFFFF;	//White
			Options->draw_dims[(4 * i) + 0] = dim_x - 2;
			Options->draw_dims[(4 * i) + 1] = dim_y - 2;

			Options->positions[(4 * i) + 2] = x;
			Options->positions[(4 * i) + 3] = y[i];
			Options->draw_z[(2 * i) + 1] = 25;
			Options->colours[(2 * i) + 1] = 0xFF7F9DB9;	//Murky Light Blue
			Options->draw_dims[(4 * i) + 2] = dim_x;
			Options->draw_dims[(4 * i) + 3] = dim_y;
		}
	}
}

//Made this a function just to reduce the line number in main
void setup_keys(MinesweeperKeyboard_t * keyboard, crayon_font_prop_t * fontsheet){
	//Some defaults for the keyboard struct
	keyboard->type_buffer[0] = '\0';
	keyboard->chars_typed = 0;
	keyboard->caps = 1;
	keyboard->special = 1;	//This will be toggled later in setup
	keyboard->name_length = 0;

	keyboard->keyboard_start_x = 144;
	keyboard->keyboard_start_y = 250;
	keyboard->keyboard_width = 318 + 16 + 35;
	keyboard->keyboard_height = 105 + 96 + 27;	//Toggle by 27

	keyboard->type_box_x = keyboard->keyboard_start_x - 8 + 71 + 25;
	keyboard->type_box_y = keyboard->keyboard_start_y - 88 + 56;
	//Bix box is 334 wide. half is 167. 11 times 10 is 110. + 10 (Each side) is 120. 167 - 60 = 107

	keyboard->fontsheet = fontsheet;

	keyboard->upper_keys = "QWERTYUIOPASDFGHJKLZXCVBNM";
	keyboard->lower_keys = "qwertyuiopasdfghjklzxcvbnm";
	keyboard->special_keys = "0123456789@#$%&-+()*\"':;_/~`|^={}\\[]<>.,?!";

	return;
}

void setup_pre_1_3_0_savefile_conversion(minesweeper_savefile_t * new_savefile, pre_1_3_0_minesweeper_savefile_t * pre_1_3_0_savefile){
	new_savefile->options = pre_1_3_0_savefile->options;
	uint8_t i;
	for(i = 0; i < 6; i++){
		new_savefile->times[i] = pre_1_3_0_savefile->times[i];
		strcpy(new_savefile->record_names[i], pre_1_3_0_savefile->record_names[i]);
	}
	new_savefile->pref_height = pre_1_3_0_savefile->pref_height;
	new_savefile->pref_width = pre_1_3_0_savefile->pref_width;
	new_savefile->pref_mines = pre_1_3_0_savefile->pref_mines;

	new_savefile->bulletsweeper_beaten = 0;

	return;
}

uint8_t setup_check_for_old_savefile(crayon_savefile_details_t * pre_1_2_0_savefile_details, uint8_t savefile_port, uint8_t savefile_slot){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;
	char savename[32];

	sprintf(savename, "/vmu/%c%d/", savefile_port + 97, savefile_slot);
	strcat(savename, "MINESWEEPER.s");

	//File DNE
	if(!(fp = fopen(savename, "rb"))){
		return 1;
	}

	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	pkg_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pkg_out = (uint8 *)malloc(pkg_size);
	fread(pkg_out, pkg_size, 1, fp);
	fclose(fp);

	vmu_pkg_parse(pkg_out, &pkg);

	free(pkg_out);

	//I have to make my own checker since it used all 16 chars without a null terminator
	uint8_t i;
	for(i = 0; i < 16; i++){
		if(pre_1_2_0_savefile_details->app_id[i] != pkg.app_id[i]){	//The names differ, They aren't the same
			return 2;
		}
	}
	return 0;
}

//Note, in testing I noticed I couldn't test VMUs c1 and d1 (Maybe the 2nd slots too)
	//However if I read a1, a2, b1 and b2 first then they work. Also calling the function later worked too.
	//It seems like KOS is still initialising when I call it :/ Thats kinda dodgy
uint8_t setup_update_old_saves(crayon_savefile_details_t * new_savefile_details){
	crayon_savefile_details_t pre_1_2_0_savefile_details;
	crayon_savefile_details_t pre_1_3_0_savefile_details;
	pre_1_3_0_minesweeper_savefile_t pre_1_3_0_savefile;	//Storing old savefata

	//Pre 1.2.0, it used the incorrect length app_id. Savefile format is still the same as pre 1.3.0
	crayon_savefile_init_savefile_details(&pre_1_2_0_savefile_details, (uint8_t *)&pre_1_3_0_savefile,
		sizeof(pre_1_3_0_minesweeper_savefile_t), 1, 0, "Made with Crayon by Protofall\0", "Minesweepe\0",
		"Proto_Minesweepe\0", "MINESWEEPER.s\0");

	//This is pre 1.3.0, it had an insufficient pref_mines var size
	crayon_savefile_init_savefile_details(&pre_1_3_0_savefile_details, (uint8_t *)&pre_1_3_0_savefile,
		sizeof(pre_1_3_0_minesweeper_savefile_t), 1, 0, "Made with Crayon by Protofall\0", "Minesweepe\0",
		"Proto_Minesweep\0", "MINESWEEPER.s\0");

	//Check for saves with old name (Pre 1.2.0)
	uint8_t pre_1_2_0_valid_saves = 0;	//a1a2b1b2c1c2d1d2
	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(crayon_savefile_check_for_device(i, j, MAPLE_FUNC_MEMCARD)){
				continue;
			}

			if(setup_check_for_old_savefile(&pre_1_2_0_savefile_details, i, j)){
				continue;
			}

			crayon_savefile_set_vmu_bit(&pre_1_2_0_valid_saves, i, j);
		}
	}

	//Pre 1.3.0
	uint8_t pre_1_3_0_valid_saves = 0;	//a1a2b1b2c1c2d1d2
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(crayon_savefile_check_for_device(i, j, MAPLE_FUNC_MEMCARD)){
				continue;
			}

			if(crayon_savefile_check_for_save(&pre_1_3_0_savefile_details, i, j)){
				continue;
			}

			crayon_savefile_set_vmu_bit(&pre_1_3_0_valid_saves, i, j);
		}
	}

	//If we have pre_1_2_0 saves
	if(pre_1_2_0_valid_saves){
		//Can't rely on the init version due to app_id being too long. This will get the job done when loading
		pre_1_2_0_savefile_details.valid_memcards = pre_1_2_0_valid_saves;
		new_savefile_details->valid_memcards = pre_1_2_0_valid_saves;

		//If present then it will update them to the new format
		for(i = 0; i <= 3; i++){
			for(j = 1; j <= 2; j++){
				//Check if this device contains an old-format savefile
				if(crayon_savefile_get_vmu_bit(pre_1_2_0_valid_saves, i, j)){
					pre_1_2_0_savefile_details.savefile_port = i;
					pre_1_2_0_savefile_details.savefile_slot = j;
					new_savefile_details->savefile_port = i;
					new_savefile_details->savefile_slot = j;

					//Note: This doesn't check the app_id (Which is good here)
					if(crayon_savefile_load(&pre_1_2_0_savefile_details)){
						continue;
					}

					//Convert from one details struct to another
					setup_pre_1_3_0_savefile_conversion((minesweeper_savefile_t *)new_savefile_details->savefile_data, &pre_1_3_0_savefile);

					crayon_savefile_save(new_savefile_details);
				}
			}
		}
	}
	//If we have some 1.2.0 savefiles
	if(pre_1_3_0_valid_saves){
		//Can't rely on the init version due to app_id being too long. This will get the job done when loading
		pre_1_3_0_savefile_details.valid_memcards = pre_1_3_0_valid_saves;
		new_savefile_details->valid_memcards = pre_1_3_0_valid_saves;

		//If present then it will update them to the new format
		for(i = 0; i <= 3; i++){
			for(j = 1; j <= 2; j++){
				//Check if this device contains an old-format savefile
				if(crayon_savefile_get_vmu_bit(pre_1_3_0_valid_saves, i, j)){
					pre_1_3_0_savefile_details.savefile_port = i;
					pre_1_3_0_savefile_details.savefile_slot = j;
					new_savefile_details->savefile_port = i;
					new_savefile_details->savefile_slot = j;

					//Note: This doesn't check the app_id (Which is good here)
					if(crayon_savefile_load(&pre_1_3_0_savefile_details)){
						continue;
					}

					//Convert from one details struct to another
					setup_pre_1_3_0_savefile_conversion((minesweeper_savefile_t *)new_savefile_details->savefile_data, &pre_1_3_0_savefile);

					crayon_savefile_save(new_savefile_details);
				}
			}
		}
	}

	//Update the savefile details
	if(pre_1_2_0_valid_saves || pre_1_3_0_valid_saves){
		new_savefile_details->valid_memcards = crayon_savefile_get_valid_memcards(new_savefile_details);
		new_savefile_details->valid_saves = crayon_savefile_get_valid_saves(new_savefile_details);	//Update the list
	}

	return pre_1_2_0_valid_saves | pre_1_3_0_valid_saves;
}

//We use a double pointer because we want to modify the pointer itself with malloc
int16_t setup_vmu_icon_load(uint8_t ** vmu_lcd_icon, char * icon_path){
	*vmu_lcd_icon = (uint8_t *) malloc(6 * 32);	//6 * 32 because we have 48/32 1bpp so we need that / 8 bytes
	FILE * file_lcd_icon = fopen(icon_path, "rb");
	if(!file_lcd_icon){return -1;}
	size_t res = fread(*vmu_lcd_icon, 192, 1, file_lcd_icon);	//If the icon is right, it *must* byt 192 bytes
	fclose(file_lcd_icon);

	return res;
}

void setup_vmu_icon_apply(uint8_t * vmu_lcd_icon, uint8_t valid_vmu_screens){
	crayon_vmu_display_icon(valid_vmu_screens, vmu_lcd_icon);
	free(vmu_lcd_icon);

	return;
}

//Its possible you could have a hacked (Or corrupted) savefile. This will fix those vars
uint8_t setup_sanitise_savefile(minesweeper_savefile_t * savefile){
	uint8_t hacker = 0;

	uint8_t i, j;
	for(i = 0; i < 6; i++){
		if(savefile->times[i] > 999){
			savefile->times[i] = 999;
			hacker = 1;
		}
		for(j = 0; j < 16; j++){
			if(savefile->record_names[i][j] != '\n'){	//End of input, we don't care whats after this
				break;
			}
			if(savefile->record_names[i][j] < ' ' || savefile->record_names[i][j] > '~'){	//Outside the fontsheet
				savefile->record_names[i][j] = ' ';
				hacker = 1;
			}
		}
	}

	if(savefile->pref_height < 9){
		savefile->pref_height = 9;
		hacker = 1;
	}
	if(savefile->pref_height > 21){
		savefile->pref_height = 21;
		hacker = 1;
	}
	if(savefile->pref_width < 9){
		savefile->pref_width = 9;
		hacker = 1;
	}
	if(savefile->pref_width > 38){
		savefile->pref_width = 38;
		hacker = 1;
	}
	if(savefile->pref_mines < 10){
		savefile->pref_mines = 10;
		hacker = 1;
	}
	if(savefile->pref_mines > (savefile->pref_width - 1) * (savefile->pref_height - 1)){
		savefile->pref_mines = (savefile->pref_width - 1) * (savefile->pref_height - 1);
		hacker = 1;
	}

	return hacker;
}