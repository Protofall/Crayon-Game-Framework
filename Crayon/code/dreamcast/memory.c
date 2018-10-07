#include "memory.h"

typedef struct dtex_header{
	uint8_t magic[4]; //magic number "DTEX"
	uint16_t   width; //texture width in pixels
	uint16_t  height; //texture height in pixels
	uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
	uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
	uint8_t     magic[4]; //magic number "DPAL"
	uint32_t color_count; //number of 32-bit ARGB palette entries
} dpal_header_t;

extern uint8_t memory_load_crayon_spritesheet(struct crayon_spritesheet *ss, char *path){
	uint8_t result = 0;
	pvr_ptr_t texture = NULL;
	ss->palette_data = NULL;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	FILE *texture_file = fopen(path, "rb");
	if(!texture_file){ERROR(1);}

	// Load texture
	//---------------------------------------------------------------------------

	dtex_header_t dtex_header;
	if(fread(&dtex_header, sizeof(dtex_header), 1, texture_file) != 1){ERROR(2);}

	if(memcmp(dtex_header.magic, "DTEX", 4)){ERROR(3);}

	texture = pvr_mem_malloc(dtex_header.size);
	if(!texture){ERROR(4);}

	if(fread(texture, dtex_header.size, 1, texture_file) != 1){ERROR(5);}

	// Write all metadata except palette stuff
	//---------------------------------------------------------------------------

	if(dtex_header.width > dtex_header.height){
		ss->spritesheet_dims = dtex_header.width;
	}
	else{
		ss->spritesheet_dims = dtex_header.height;
	}

	ss->spritesheet_texture = texture;

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
	if(ss->spritesheet_format == 5 || ss->spritesheet_format == 6){
		ss->palette_data = (crayon_palette_t *) malloc(sizeof(crayon_palette_t));
		ss->palette_data->palette = NULL;

		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));  //Add a check here to see if it failed
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		int resultPal = memory_load_palette(ss->palette_data, palette_path); //The function will modify the palette and colour count
		free(palette_path);
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
	fscanf(sheet_file, "%hhu\n", &ss->spritesheet_animation_count);

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
		int scanned = fscanf(sheet_file, "%s %hu %hu %hu %hu %hu %hu %hhu\n",
								ss->spritesheet_animation_array[i].animation_name,
								&ss->spritesheet_animation_array[i].animation_x,
								&ss->spritesheet_animation_array[i].animation_y,
								&ss->spritesheet_animation_array[i].animation_sheet_width,
								&ss->spritesheet_animation_array[i].animation_sheet_height,
								&ss->spritesheet_animation_array[i].animation_frame_width,
								&ss->spritesheet_animation_array[i].animation_frame_height,
								&ss->spritesheet_animation_array[i].animation_frames);
		if(scanned != 8){
			free(ss->spritesheet_animation_array);
			ERROR(16);
		}
	}

	#undef ERROR

	cleanup:

	if(texture_file){fclose(texture_file);}
	if(sheet_file){fclose(sheet_file);} //May need to enclode this in an if "res >= 12" if statement

	// If a failure occured somewhere
	if(result && texture){pvr_mem_free(texture);}

	//If we allocated memory for the palette and error out
	if(result && ss->palette_data != NULL){
		if(ss->palette_data->palette != NULL){ //If we allocated memory for the palette itself, free that
			free(ss->palette_data->palette);
		}
		free(ss->palette_data);
	}

	return result;
}

extern uint8_t memory_load_prop_font_sheet(struct crayon_font_prop *fp, char *path){
	uint8_t result = 0;
	pvr_ptr_t texture = NULL;
	fp->palette_data = NULL;
	fp->chars_per_row = NULL;
	fp->char_width = NULL;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	FILE *texture_file = fopen(path, "rb");
	if(!texture_file){ERROR(1);}

	// Load texture
	//---------------------------------------------------------------------------

	dtex_header_t dtex_header;
	if(fread(&dtex_header, sizeof(dtex_header), 1, texture_file) != 1){ERROR(2);}

	if(memcmp(dtex_header.magic, "DTEX", 4)){ERROR(3);}

	texture = pvr_mem_malloc(dtex_header.size);
	if(!texture){ERROR(4);}

	if(fread(texture, dtex_header.size, 1, texture_file) != 1){ERROR(5);}

	// Write all metadata except palette stuff
	//---------------------------------------------------------------------------

	if(dtex_header.width > dtex_header.height){
		fp->fontsheet_dim = dtex_header.width;
	}
	else{
		fp->fontsheet_dim = dtex_header.height;
	}

	fp->fontsheet_texture = texture;

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
	if(fp->texture_format == 5 || fp->texture_format == 6){
		fp->palette_data = (crayon_palette_t *) malloc(sizeof(crayon_palette_t));
		fp->palette_data->palette = NULL;

		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));  //Add a check here to see if it failed
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		int resultPal = memory_load_palette(fp->palette_data, palette_path); //The function will modify the palette and colour count
		free(palette_path);
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

	if(fscanf(sheet_file, "%hhu\n", &fp->char_height) != 1){
		ERROR(15);
	}

	if(fscanf(sheet_file, "%hhu", &fp->num_rows) != 1){
		ERROR(16);
	}

	fp->chars_per_row = (uint8_t *) malloc((fp->num_rows)*sizeof(uint8_t));
	if(fp->chars_per_row == NULL){ERROR(17);}

	fp->num_chars = 0;
	int i;
	for(i = 0; i < fp->num_rows; i++){
		int8_t res = fscanf(sheet_file, "%hhu", &fp->chars_per_row[i]);
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
		int8_t res = fscanf(sheet_file, "%hhu", &fp->char_width[i]);
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

	if(texture_file){fclose(texture_file);}
	if(sheet_file){fclose(sheet_file);}

	// If a failure occured somewhere
	if(result && texture){pvr_mem_free(texture);}

	//If we allocated memory for the palette and error out
	if(result){
		if(fp->palette_data != NULL){
			if(fp->palette_data->palette != NULL){ //If we allocated memory for the palette itself, free that
				free(fp->palette_data->palette);
			}
			free(fp->palette_data);
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

extern uint8_t memory_load_mono_font_sheet(struct crayon_font_mono *fm, char *path){
	uint8_t result = 0;
	pvr_ptr_t texture = NULL;
	fm->palette_data = NULL;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	FILE *texture_file = fopen(path, "rb");
	if(!texture_file){ERROR(1);}

	// Load texture
	//---------------------------------------------------------------------------

	dtex_header_t dtex_header;
	if(fread(&dtex_header, sizeof(dtex_header), 1, texture_file) != 1){ERROR(2);}

	if(memcmp(dtex_header.magic, "DTEX", 4)){ERROR(3);}

	texture = pvr_mem_malloc(dtex_header.size);
	if(!texture){ERROR(4);}

	if(fread(texture, dtex_header.size, 1, texture_file) != 1){ERROR(5);}

	// Write all metadata except palette stuff
	//---------------------------------------------------------------------------

	if(dtex_header.width > dtex_header.height){
		fm->fontsheet_dim = dtex_header.width;
	}
	else{
		fm->fontsheet_dim = dtex_header.height;
	}

	fm->fontsheet_texture = texture;

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
	if(fm->texture_format == 5 || fm->texture_format == 6){
		fm->palette_data = (crayon_palette_t *) malloc(sizeof(crayon_palette_t));
		fm->palette_data->palette = NULL;

		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));  //Add a check here to see if it failed
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		int resultPal = memory_load_palette(fm->palette_data, palette_path); //The function will modify the palette and colour count
		free(palette_path);
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

	if(fscanf(sheet_file, "%hhu %hhu %hhu %hhu\n", &fm->char_width, &fm->char_height, &fm->num_columns, &fm->num_rows) != 4){
		ERROR(15);
	}

	#undef ERROR

	cleanup:

	if(texture_file){fclose(texture_file);}
	if(sheet_file){fclose(sheet_file);}

	// If a failure occured somewhere
	if(result && texture){pvr_mem_free(texture);}

	//If we allocated memory for the palette and error out
	if(result && fm->palette_data != NULL){
		if(fm->palette_data->palette != NULL){ //If we allocated memory for the palette itself, free that
			free(fm->palette_data->palette);
		}
		free(fm->palette_data);
	}

	return result;
}

extern uint8_t memory_load_palette(crayon_palette_t *cp, char *path){
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

	PAL_cleanup:

	if(palette_file){fclose(palette_file);}

	return result;
}

extern crayon_palette_t * memory_clone_palette(crayon_palette_t *original){
	crayon_palette_t *copy = (crayon_palette_t *) malloc(sizeof(crayon_palette_t));
	copy->palette = malloc(original->colour_count * sizeof(uint32_t));
	copy->colour_count = original->colour_count;

	uint16_t i;
	for(i = 0; i < copy->colour_count; i++){	//Goes through the palette and adds in all values
		copy->palette[i] = original->palette[i];
	}
	return copy;
}

extern uint16_t memory_swap_colour(crayon_palette_t *cp, uint32_t colour1, uint32_t colour2, uint8_t _continue){
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
extern uint8_t memory_free_crayon_spritesheet(struct crayon_spritesheet *ss, uint8_t free_palette){
	if(ss){
		if(free_palette && ss->palette_data != NULL){ //Free the palette
			if(ss->palette_data->palette != NULL){
				free(ss->palette_data->palette);
			}
			free(ss->palette_data);
		}
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

extern uint8_t memory_free_prop_font_sheet(struct crayon_font_prop *fp, uint8_t free_palette){
	if(fp){
		if(free_palette && fp->palette_data != NULL){ //Free the palette
			if(fp->palette_data->palette != NULL){
				free(fp->palette_data->palette);
			}
			free(fp->palette_data);
		}
		free(fp->char_x_coord);
		free(fp->char_width);
		free(fp->chars_per_row);
		pvr_mem_free(fp->fontsheet_texture);

		return 0;
	}
	return 1;
}

extern uint8_t memory_free_mono_font_sheet(struct crayon_font_mono *fm, uint8_t free_palette){
	if(fm){
		if(free_palette && fm->palette_data != NULL){ //Free the palette
			if(fm->palette_data->palette != NULL){
				free(fm->palette_data->palette);
			}
			free(fm->palette_data);
		}
		pvr_mem_free(fm->fontsheet_texture);

		return 0;
	}
	return 1;
}

extern uint8_t memory_free_palette(crayon_palette_t *cp){
	if(cp->palette != NULL){
		free(cp->palette);
		return 0;
	}
	return 1;
}

extern uint8_t memory_mount_romdisk(char *filename, char *mountpoint){
	void *buffer;
	ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM

	if(size != -1){
		fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
		return 0;
	}
	return 1;
}

extern uint8_t memory_mount_romdisk_gz(char *filename, char *mountpoint){
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
