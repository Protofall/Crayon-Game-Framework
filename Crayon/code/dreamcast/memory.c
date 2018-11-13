#include "memory.h"

extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, dtex_header_t *dtex_header, char *texture_path){	
	uint8_t dtex_result = 0;

	#define DTEX_ERROR(n) {dtex_result = n; goto DTEX_cleanup;}

		FILE *texture_file = fopen(texture_path, "rb");
		if(!texture_file){DTEX_ERROR(1);}

		if(fread(dtex_header, sizeof(dtex_header_t), 1, texture_file) != 1){DTEX_ERROR(2);}

		if(memcmp(dtex_header->magic, "DTEX", 4)){DTEX_ERROR(3);}

		*dtex = pvr_mem_malloc(dtex_header->size);
		if(!*dtex){DTEX_ERROR(4);}

		if(fread(*dtex, dtex_header->size, 1, texture_file) != 1){DTEX_ERROR(5);}

	#undef DTEX_ERROR

	DTEX_cleanup:

	if(texture_file){fclose(texture_file);}

	return dtex_result;
}

extern uint8_t crayon_memory_load_spritesheet(crayon_spritesheet_t *ss, crayon_palette_t *cp, int8_t palette_id, char *path){
	uint8_t result = 0;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	dtex_header_t dtex_header;
	uint8_t dtex_result = crayon_memory_load_dtex(&ss->spritesheet_texture, &dtex_header, path);
	if(dtex_result){ERROR(dtex_result);}

	// Write all metadata except palette stuff
	//---------------------------------------------------------------------------

	if(dtex_header.width > dtex_header.height){
		ss->spritesheet_dims = dtex_header.width;
	}
	else{
		ss->spritesheet_dims = dtex_header.height;
	}

	//This assumes no mip-mapping, no stride, twiddled on, its uncompressed and no stride setting (I might edit this later to allow for compressed)
	if(dtex_header.type == 0x00000000){ //ARGB1555
		ss->spritesheet_format = 0;
	}
	if(dtex_header.type == 0x08000000){ //RGB565
		ss->spritesheet_format = 1;
	}
	else if(dtex_header.type == 0x10000000){ //ARGB4444
		ss->spritesheet_format = 2;
	}
	else if(dtex_header.type == 0x28000000){ //PAL4BPP
		ss->spritesheet_format = 5;
	}
	else if(dtex_header.type == 0x30000000){ //PAL8BPP
		ss->spritesheet_format = 6;
	}
	else{    
		ERROR(6);
	}

	/*
	The correct formats are
	bits 27-29 : Pixel format
	0 = ARGB1555
	1 = RGB565
	2 = ARGB4444
	3 = YUV422
	4 = BUMPMAP
	5 = PAL4BPP
	6 = PAL8BPP
	*/

	int path_length = strlen(path);
	if(palette_id >= 0 && (ss->spritesheet_format == 5 || ss->spritesheet_format == 6)){	//If we pass in -1, then we skip palettes
		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, palette_path, (ss->spritesheet_format - 4) * 4);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char *txt_path = (char *) malloc((path_length)*sizeof(char));  //Add a check here to see if it failed
	if(!txt_path){ERROR(13);}

	strncpy(txt_path, path, path_length - 4);
	// strcat(txt_path, "txt");	//At some point comeback and use something like this instead of the 4 commands below
	txt_path[path_length - 4] = 't';
	txt_path[path_length - 3] = 'x';
	txt_path[path_length - 2] = 't';
	txt_path[path_length - 1] = '\0';

	sheet_file = fopen(txt_path, "rb");
	free(txt_path);
	if(!sheet_file){ERROR(14);}

	int uint8_holder;	//Can't really read straight into uint8_t's so this is the work around :(
	fscanf(sheet_file, "%d\n", &uint8_holder);
	ss->spritesheet_animation_count = uint8_holder;

	ss->spritesheet_animation_array = (crayon_animation_t *) malloc(sizeof(crayon_animation_t) * ss->spritesheet_animation_count);
	if(!ss->spritesheet_animation_array){ERROR(15);}
	
	int i;
	for(i = 0; i < ss->spritesheet_animation_count; i++){
		//Check the length of the name
		char anim_name_part = '0';
		int count = -1;
		while(anim_name_part != ' '){
			anim_name_part = getc(sheet_file);
			count++;
		}
		ss->spritesheet_animation_array[i].animation_name = (char *) malloc((count + 1) * sizeof(char));

		fseek(sheet_file, -count - 1, SEEK_CUR);  //Go back so we can store the name
		int scanned = fscanf(sheet_file, "%s %hu %hu %hu %hu %hu %hu %d\n",
								ss->spritesheet_animation_array[i].animation_name,
								&ss->spritesheet_animation_array[i].animation_x,
								&ss->spritesheet_animation_array[i].animation_y,
								&ss->spritesheet_animation_array[i].animation_sheet_width,
								&ss->spritesheet_animation_array[i].animation_sheet_height,
								&ss->spritesheet_animation_array[i].animation_frame_width,
								&ss->spritesheet_animation_array[i].animation_frame_height,
								&uint8_holder);

		ss->spritesheet_animation_array[i].animation_frames = uint8_holder;
		if(scanned != 8){
			free(ss->spritesheet_animation_array);
			ERROR(16);
		}
	}

	#undef ERROR

	cleanup:

	if(sheet_file){fclose(sheet_file);} //May need to enclode this in an if "res >= 12" if statement

	// If a failure occured somewhere
	// This might cause errors if the pointer wasn't initially set to NULL
	if(result && ss->spritesheet_texture){pvr_mem_free(ss->spritesheet_texture);}

	//If we allocated memory for the palette and error out
	if(result && cp->palette != NULL){
		free(cp->palette);
	}

	return result;
}

extern uint8_t crayon_memory_load_prop_font_sheet(crayon_font_prop_t *fp, crayon_palette_t *cp, int8_t palette_id, char *path){
	uint8_t result = 0;
	fp->chars_per_row = NULL;
	fp->char_width = NULL;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	dtex_header_t dtex_header;
	uint8_t dtex_result = crayon_memory_load_dtex(&fp->fontsheet_texture, &dtex_header, path);
	if(dtex_result){ERROR(dtex_result);}

	// Write all metadata except palette stuff
	//---------------------------------------------------------------------------

	if(dtex_header.width > dtex_header.height){
		fp->fontsheet_dim = dtex_header.width;
	}
	else{
		fp->fontsheet_dim = dtex_header.height;
	}

	//This assumes same stuff as spritesheet loader does
	if(dtex_header.type == 0x00000000){ //ARGB1555
		fp->texture_format = 0;
	}
	if(dtex_header.type == 0x08000000){ //RGB565
		fp->texture_format = 1;
	}
	else if(dtex_header.type == 0x10000000){ //ARGB4444
		fp->texture_format = 2;
	}
	else if(dtex_header.type == 0x28000000){ //PAL4BPP
		fp->texture_format = 5;
	}
	else if(dtex_header.type == 0x30000000){ //PAL8BPP
		fp->texture_format = 6;
	}
	else{    
		ERROR(6);
	}

	int path_length = strlen(path);
	if(palette_id >= 0 && (fp->texture_format == 5 || fp->texture_format == 6)){	//If we pass in -1, then we skip palettes
		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, palette_path, (fp->texture_format - 4) * 4);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char *txt_path = (char *) malloc((path_length)*sizeof(char));  //Add a check here to see if it failed
	if(!txt_path){ERROR(13);}

	strncpy(txt_path, path, path_length - 4);
	// strcat(txt_path, "txt");	//At some point comeback and use something like this instead of the 4 commands below
	txt_path[path_length - 4] = 't';
	txt_path[path_length - 3] = 'x';
	txt_path[path_length - 2] = 't';
	txt_path[path_length - 1] = '\0';

	sheet_file = fopen(txt_path, "rb");
	free(txt_path);
	if(!sheet_file){ERROR(14);}

	int uint8_holder;
	if(fscanf(sheet_file, "%d\n", &uint8_holder) != 1){
		ERROR(15);
	}
	fp->char_height = uint8_holder;

	if(fscanf(sheet_file, "%d", &uint8_holder) != 1){
		ERROR(16);
	}

	fp->num_rows = uint8_holder;


	fp->chars_per_row = (uint8_t *) malloc((fp->num_rows)*sizeof(uint8_t));
	if(fp->chars_per_row == NULL){ERROR(17);}

	fp->num_chars = 0;
	int i;
	for(i = 0; i < fp->num_rows; i++){
		int8_t res = fscanf(sheet_file, "%d", &uint8_holder);
		fp->chars_per_row[i] = uint8_holder;
		fp->num_chars += fp->chars_per_row[i];
		if(res != 1){
			ERROR(18);
		}
	}
	fscanf(sheet_file, "\n");	//Getting to the next line

	fp->char_width = (uint8_t *) malloc((fp->num_chars)*sizeof(uint8_t));
	if(fp->char_width == NULL){ERROR(19);}

	i = 0;	//Might be redundant
	for(i = 0; i < fp->num_chars; i++){
		int8_t res = fscanf(sheet_file, "%d", &uint8_holder);
		fp->char_width[i] = uint8_holder;
		if(res != 1){
			ERROR(20);
		}
	}

	//This section geterates the x coordinates for each char
	fp->char_x_coord = (uint8_t *) malloc((fp->num_chars)*sizeof(uint8_t));
	if(fp->char_x_coord == NULL){ERROR(21);}

	i = 0;	//Might be redundant
	int chars_counted_in_row = 0;
	int current_row = 0;
	for(i = 0; i < fp->num_chars; i++){
		if(chars_counted_in_row == 0){	//first element per row = zero
			fp->char_x_coord[i] = 0;
		}
		else{
			fp->char_x_coord[i] = fp->char_x_coord[i - 1] + fp->char_width[i - 1];	//nth element = (n-1)th element's width + x pos
		}
		chars_counted_in_row++;
		if(chars_counted_in_row == fp->chars_per_row[current_row]){	//When we reach the end of the row, reset our counters
			current_row++;
			chars_counted_in_row = 0;
		}
	}

	#undef ERROR

	cleanup:

	if(sheet_file){fclose(sheet_file);}

	// If a failure occured somewhere
	if(result && fp->fontsheet_texture){pvr_mem_free(fp->fontsheet_texture);}

	//If we allocated memory for the palette and error out
	if(result){
		if(result && cp->palette != NULL){
			free(cp->palette);
		}
		if(fp->chars_per_row != NULL){
			free(fp->chars_per_row);
		}

		if(fp->char_width != NULL){
			free(fp->char_width);
		}
		if(fp->char_x_coord != NULL){
			free(fp->char_x_coord);
		}
	}

	return result;
}

extern uint8_t crayon_memory_load_mono_font_sheet(crayon_font_mono_t *fm, crayon_palette_t *cp, int8_t palette_id, char *path){
	uint8_t result = 0;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	dtex_header_t dtex_header;
	uint8_t dtex_result = crayon_memory_load_dtex(&fm->fontsheet_texture, &dtex_header, path);
	if(dtex_result){ERROR(dtex_result);}

	// Write all metadata except palette stuff
	//---------------------------------------------------------------------------

	if(dtex_header.width > dtex_header.height){
		fm->fontsheet_dim = dtex_header.width;
	}
	else{
		fm->fontsheet_dim = dtex_header.height;
	}

	//This assumes same stuff as spritesheet loader does
	if(dtex_header.type == 0x00000000){ //ARGB1555
		fm->texture_format = 0;
	}
	if(dtex_header.type == 0x08000000){ //RGB565
		fm->texture_format = 1;
	}
	else if(dtex_header.type == 0x10000000){ //ARGB4444
		fm->texture_format = 2;
	}
	else if(dtex_header.type == 0x28000000){ //PAL4BPP
		fm->texture_format = 5;
	}
	else if(dtex_header.type == 0x30000000){ //PAL8BPP
		fm->texture_format = 6;
	}
	else{    
		ERROR(6);
	}

	int path_length = strlen(path);
	if(palette_id >= 0 && (fm->texture_format == 5 || fm->texture_format == 6)){	//If we pass in -1, then we skip palettes
		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, palette_path, (fm->texture_format - 4) * 4);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char *txt_path = (char *) malloc((path_length)*sizeof(char));  //Add a check here to see if it failed
	if(!txt_path){ERROR(13);}

	strncpy(txt_path, path, path_length - 4);
	// strcat(txt_path, "txt");	//At some point comeback and use something like this instead of the 4 commands below
	txt_path[path_length - 4] = 't';
	txt_path[path_length - 3] = 'x';
	txt_path[path_length - 2] = 't';
	txt_path[path_length - 1] = '\0';

	sheet_file = fopen(txt_path, "rb");
	free(txt_path);
	if(!sheet_file){ERROR(14);}

	int test1, test2, test3, test4;
	if(fscanf(sheet_file, "%d %d %d %d\n", &test1, &test2, &test3, &test4) != 4){	//Storing them in into since hhu looks for one char...
		ERROR(15);
	}

	fm->char_width = test1;
	fm->char_height = test2;
	fm->num_columns = test3;
	fm->num_rows = test4;

	//Attempt to come back here and read the whole file directly into the vars
	// if(fscanf(sheet_file, "%hhu %hhu %hhu %hhu\n", fm->char_width, fm->char_height, fm->num_columns, fm->num_rows) != 4){
	// if(fscanf(sheet_file, "%hhu %hhu %hhu %hhu\n", &fm->char_width, &fm->char_height, &fm->num_columns, &fm->num_rows) != 4){
	// if(fscanf(sheet_file, "%d %d %d %d\n", &fm->char_width, &fm->char_height, &fm->num_columns, &fm->num_rows) != 4){
	// if(fscanf(sheet_file, "%hhd %hhd %hhd %hhd\n", &fm->char_width, &fm->char_height, &fm->num_columns, &fm->num_rows) != 4){
	// if(fscanf(sheet_file, "%u %u %u %u\n", &fm->char_width, &fm->char_height, &fm->num_columns, &fm->num_rows) != 4){
		// ERROR(15);
	// }

	#undef ERROR

	cleanup:

	if(sheet_file){fclose(sheet_file);}

	// If a failure occured somewhere
	if(result && fm->fontsheet_texture){pvr_mem_free(fm->fontsheet_texture);}

	//If we allocated memory for the palette and error out
	if(result && cp->palette != NULL){
		free(cp->palette);
	}

	return result;
}

extern uint8_t crayon_memory_load_palette(crayon_palette_t *cp, char *path, int8_t bpp){
	uint8_t result = 0;
	#define PAL_ERROR(n) {result = (n); goto PAL_cleanup;}
	
	FILE *palette_file = fopen(path, "rb");
	if(!palette_file){PAL_ERROR(1);}

	dpal_header_t dpal_header;
	if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){PAL_ERROR(2);}

	if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){PAL_ERROR(3);}

	cp->palette = malloc(dpal_header.color_count * sizeof(uint32_t));
	if(!cp->palette){PAL_ERROR(4);}

	if(fread(cp->palette, sizeof(uint32_t), dpal_header.color_count,
		palette_file) != dpal_header.color_count){PAL_ERROR(5);}

	#undef PAL_ERROR

	cp->colour_count = dpal_header.color_count;
	cp->bpp = bpp;
	cp->palette_id = -1;	//In the ss, prop and mono font load functions this doesn't matter
							//But if used on its own I think this should stay

	PAL_cleanup:

	if(palette_file){fclose(palette_file);}

	return result;
}

extern void crayon_memory_clone_palette(crayon_palette_t *original, crayon_palette_t *copy, int8_t palette_id){
	copy->palette = malloc(original->colour_count * sizeof(uint32_t));
	copy->colour_count = original->colour_count;
	copy->bpp = original->bpp;
	copy->palette_id = palette_id;

	uint16_t i;
	for(i = 0; i < copy->colour_count; i++){	//Goes through the palette and adds in all values
		copy->palette[i] = original->palette[i];
	}
	return;
}

//lol 13 params
extern void crayon_memory_set_sprite_array(crayon_sprite_array_t *sprite_array, uint8_t num_sprites,
	uint8_t unique_frames, uint8_t multi_draw_z, uint8_t multi_frames, uint8_t multi_scales,
	uint8_t multi_rotations, uint8_t multi_colours, uint8_t filter, uint8_t palette_num,
	crayon_spritesheet_t *ss, crayon_animation_t *anim, crayon_palette_t *palette){
	
	sprite_array->ss = ss;
	sprite_array->anim = anim;
	sprite_array->num_sprites = num_sprites;
	sprite_array->unique_frames = unique_frames;
	sprite_array->options = 0 + (multi_colours << 4) + (multi_rotations << 3) + (multi_scales << 2) +
		(multi_frames << 1) + (multi_draw_z << 0);
	sprite_array->filter = filter;
	sprite_array->palette_num = palette_num;

	sprite_array->draw_pos = (float *) malloc(num_sprites * 2 * sizeof(float));
	sprite_array->draw_z = (uint8_t *) malloc((multi_draw_z ? num_sprites: 1) * sizeof(uint8_t));
	sprite_array->frame_coords_keys = (uint8_t *) malloc((multi_frames ? num_sprites: 1) * sizeof(uint8_t));
	sprite_array->frame_coords_map = (uint16_t *) malloc(unique_frames * 2 * sizeof(uint16_t));
	sprite_array->scales = (uint8_t *) malloc((multi_scales ? num_sprites: 1) * 2 * sizeof(uint8_t));
	sprite_array->rotations = (float *) malloc((multi_rotations ? num_sprites: 1) * sizeof(float));
	sprite_array->colours = (uint32_t *) malloc((multi_colours ? num_sprites: 1) * sizeof(uint32_t));

	sprite_array->palette = palette;

	return;
}

extern uint16_t crayon_memory_swap_colour(crayon_palette_t *cp, uint32_t colour1, uint32_t colour2, uint8_t _continue){
	uint16_t i;
	uint16_t found = 0;
	for(i = 0; i < cp->colour_count; ++i){
		if(cp->palette[i] == colour1){
			cp->palette[i] = colour2;
			found++;
			if(!_continue){
				break;
			}
		}
	}
	return found;
}

//Free Texture, anim array and palette (Maybe the anim/ss names later on?). Doesn't free the spritesheet struct itself
extern uint8_t crayon_memory_free_spritesheet(struct crayon_spritesheet *ss){
	if(ss){
		pvr_mem_free(ss->spritesheet_texture);

		uint16_t i;
		for(i = 0; i < ss->spritesheet_animation_count; i++){
			free(ss->spritesheet_animation_array[i].animation_name);
		}
		free(ss->spritesheet_animation_array);

		//We don't free the ss because it could be on the stack and we can't confirm if a pointer points to the heap or stack.
		//If it were on the heap then we would free it

		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_prop_font_sheet(struct crayon_font_prop *fp){
	if(fp){
		free(fp->char_x_coord);
		free(fp->char_width);
		free(fp->chars_per_row);
		pvr_mem_free(fp->fontsheet_texture);
		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_mono_font_sheet(struct crayon_font_mono *fm){
	if(fm){
		pvr_mem_free(fm->fontsheet_texture);
		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_palette(crayon_palette_t *cp){
	if(cp->palette != NULL){
		free(cp->palette);
		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_sprite_array(crayon_sprite_array_t *sprite_array, uint8_t free_ss, uint8_t free_pal){
	uint8_t retval = 0;
	if(free_ss << 0){
		retval = crayon_memory_free_spritesheet(sprite_array->ss);
		if(retval){return retval;}
	}

	free(sprite_array->draw_pos);
	free(sprite_array->draw_z);
	free(sprite_array->frame_coords_keys);
	free(sprite_array->frame_coords_map);
	free(sprite_array->scales);
	free(sprite_array->rotations);
	free(sprite_array->colours);

	return 0;
}

extern uint8_t crayon_memory_mount_romdisk(char *filename, char *mountpoint){
	void *buffer;
	ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM

	if(size != -1){
		fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_mount_romdisk_gz(char *filename, char *mountpoint){
	void *buffer;
	int length = zlib_getlength(filename);

	//Later add check to see if theres enough available main ram
	 
	// Check failure
	if(length == 0){
			return 1;
	}
	 
	// Open file
	gzFile file = gzopen(filename, "rb"); //Seems to be the replacement of fs_load() along with gzread()
	if(!file){
			return 1;
	}

	// Allocate memory, read file
	buffer = malloc(length);  //Might need an (if(!buffer) check here)
	gzread(file, buffer, length);
	gzclose(file);

	fs_romdisk_mount(mountpoint, buffer, 1);
	return 0;
}
