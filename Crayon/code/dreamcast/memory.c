#include "memory.h"

extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, uint16_t *dims, uint32_t *format, char *texture_path){
	uint8_t dtex_result = 0;
	dtex_header_t dtex_header;

	#define DTEX_ERROR(n) {dtex_result = n; goto DTEX_cleanup;}

		FILE *texture_file = fopen(texture_path, "rb");
		if(!texture_file){DTEX_ERROR(1);}

		if(fread(&dtex_header, sizeof(dtex_header_t), 1, texture_file) != 1){DTEX_ERROR(2);}

		if(memcmp(dtex_header.magic, "DTEX", 4)){DTEX_ERROR(3);}

		*dtex = pvr_mem_malloc(dtex_header.size);
		if(!*dtex){DTEX_ERROR(4);}

		if(fread(*dtex, dtex_header.size, 1, texture_file) != 1){DTEX_ERROR(5);}

		if(dtex_header.width > dtex_header.height){
			*dims = dtex_header.width;
		}
		else{
			*dims = dtex_header.height;
		}
		*format = dtex_header.type;

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
	uint8_t dtex_result = crayon_memory_load_dtex(&ss->texture, &ss->dimensions, &ss->texture_format, path);

	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = (((1 << 3) - 1) & (ss->texture_format >> (28 - 1)));	//Extract bits 27 - 29, Pixel format

	//Invalid format
	if(texture_format < 0 && texture_format > 6){
		ERROR(6);
	}

	uint8_t bpp = 0;
	if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}
	if(palette_id >= 0 && bpp){	//If we pass in -1, then we skip palettes
		char * palette_path = NULL;
		crayon_assist_append_extension(&palette_path, path, ".pal");

		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, bpp, palette_path);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char * txt_path = NULL;
	if(crayon_assist_change_extension(&txt_path, path, "txt")){ERROR(13);}

	sheet_file = fopen(txt_path, "rb");
	free(txt_path);
	if(!sheet_file){ERROR(14);}

	int uint8_holder;	//Can't really read straight into uint8_t's so this is the work around :(
	fscanf(sheet_file, "%d\n", &uint8_holder);
	ss->animation_count = uint8_holder;

	ss->animation_array = (crayon_animation_t *) malloc(sizeof(crayon_animation_t) * ss->animation_count);
	if(!ss->animation_array){ERROR(15);}
	
	uint16_t i;
	int8_t scanned;
	char anim_name_part;
	int16_t count;
	uint16_t sprite_index = 0;
	for(i = 0; i < ss->animation_count * 2; i++){
		if(i % 2 == 0){
			//Check the length of the name
			anim_name_part = '0';
			count = -1;
			while(anim_name_part != '\n'){
				anim_name_part = getc(sheet_file);
				count++;
			}
			ss->animation_array[sprite_index].name = (char *) malloc((count + 1) * sizeof(char));

			fseek(sheet_file, -count - 1, SEEK_CUR);  //Go back so we can store the name
			fread(ss->animation_array[sprite_index].name, sizeof(uint8_t), count, sheet_file);
			ss->animation_array[sprite_index].name[count] = '\0';	//Add the null-terminator
		}
		else{
			scanned = fscanf(sheet_file, "%hu %hu %hu %hu %hu %hu %d\n",
								&ss->animation_array[sprite_index].x,
								&ss->animation_array[sprite_index].y,
								&ss->animation_array[sprite_index].sheet_width,
								&ss->animation_array[sprite_index].sheet_height,
								&ss->animation_array[sprite_index].frame_width,
								&ss->animation_array[sprite_index].frame_height,
								&uint8_holder);
			ss->animation_array[sprite_index].frame_count = uint8_holder;
			if(scanned != 7){
				free(ss->animation_array);
				ERROR(16);	//Possible Mem-leak place
			}
			sprite_index++;
		}
	}

	#undef ERROR

	cleanup:

	if(sheet_file){fclose(sheet_file);} //May need to enclode this in an if "res >= 12" if statement

	// If a failure occured somewhere
	// This might cause errors if the pointer wasn't initially set to NULL
	if(result && ss->texture){pvr_mem_free(ss->texture);}

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

	uint8_t dtex_result = crayon_memory_load_dtex(&fp->texture, &fp->dimensions, &fp->texture_format, path);
	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = (((1 << 3) - 1) & (fp->texture_format >> (28 - 1)));	//Extract bits 27 - 29, Pixel format

	//Invalid format
	if(texture_format < 0 && texture_format > 6){
		ERROR(6);
	}

	uint8_t bpp = 0;
	if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}
	if(palette_id >= 0 && bpp){	//If we pass in -1, then we skip palettes
		char * palette_path = NULL;
		crayon_assist_append_extension(&palette_path, path, ".pal");

		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, bpp, palette_path);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char * txt_path = NULL;
	if(crayon_assist_change_extension(&txt_path, path, "txt")){ERROR(13);}

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
	if(result && fp->texture){pvr_mem_free(fp->texture);}

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

	uint8_t dtex_result = crayon_memory_load_dtex(&fm->texture, &fm->dimensions, &fm->texture_format, path);
	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = (((1 << 3) - 1) & (fm->texture_format >> (28 - 1)));	//Extract bits 27 - 29, Pixel format

	//Invalid format
	if(texture_format < 0 && texture_format > 6){
		ERROR(6);
	}

	uint8_t bpp = 0;
	if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}
	if(palette_id >= 0 && bpp){	//If we pass in -1, then we skip palettes
		char * palette_path = NULL;
		crayon_assist_append_extension(&palette_path, path, ".pal");
		
		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, bpp, palette_path);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char * txt_path = NULL;
	if(crayon_assist_change_extension(&txt_path, path, "txt")){ERROR(13);}

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

	fm->num_chars = fm->num_columns * fm->num_rows;	//The number of chars *may* be less than this, but it won't be fatal

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
	if(result && fm->texture){pvr_mem_free(fm->texture);}

	//If we allocated memory for the palette and error out
	if(result && cp->palette != NULL){
		free(cp->palette);
	}

	return result;
}

extern uint8_t crayon_memory_load_palette(crayon_palette_t *cp, int8_t bpp, char *path){
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

extern void crayon_memory_init_sprite_array(crayon_sprite_array_t *sprite_array, crayon_spritesheet_t *ss,
	crayon_animation_t *anim, crayon_palette_t *pal, uint16_t list_size, uint8_t frames_used, uint8_t options,
	uint8_t filter){

	sprite_array->spritesheet = ss;
	sprite_array->animation = anim;
	sprite_array->list_size = list_size;
	sprite_array->frames_used = frames_used;
	sprite_array->options = options;
	sprite_array->filter = filter;

	// ((sprite_array->options >> 5) && 1)	//Extract the colour bit
	// ((sprite_array->options >> 4) && 1)	//Extract the rotation bit
	// ((sprite_array->options >> 3) && 1)	//Extract the flip bit
	// ((sprite_array->options >> 2) && 1)	//Extract the scale bit
	// ((sprite_array->options >> 1) && 1)	//Extract the frames_used bit
	// ((sprite_array->options) && 1)		//Extract the layer bit

	sprite_array->pos = (float *) malloc(list_size * 2 * sizeof(float));
	sprite_array->frame_coord_key = (uint8_t *) malloc(((sprite_array->options >> 1) && 1 ? list_size: 1) * sizeof(uint8_t));
	sprite_array->frame_coord_map = (uint16_t *) malloc(frames_used * 2 * sizeof(uint16_t));
	sprite_array->colour = (uint32_t *) malloc(((sprite_array->options >> 5) && 1 ? list_size: 1) * sizeof(uint32_t));
	sprite_array->fade = (uint8_t *) malloc(((sprite_array->options >> 5) && 1 ? list_size: 1) * sizeof(uint8_t));
	sprite_array->scale = (float *) malloc(((sprite_array->options >> 2) && 1 ? list_size: 1) * 2 * sizeof(float));
	sprite_array->flip = (uint8_t *) malloc(((sprite_array->options >> 3) && 1 ? list_size: 1) * sizeof(uint8_t));
	sprite_array->rotation = (float *) malloc(((sprite_array->options >> 4) && 1 ? list_size: 1) * sizeof(float));
	sprite_array->layer = (uint8_t *) malloc(((sprite_array->options) && 1 ? list_size: 1) * sizeof(uint8_t));

	sprite_array->palette = pal;

	return;
}

extern void crayon_memory_init_untextered_array(crayon_untextured_array_t *poly_array, uint16_t list_size, uint8_t options){
	poly_array->list_size = list_size;
	poly_array->options = options;

	poly_array->pos = (float *) malloc(list_size * 2 * sizeof(float));
	poly_array->layer = (uint8_t *) malloc((options & 1 ? list_size: 1) * sizeof(uint8_t));
	poly_array->dimensions = (uint16_t *) malloc(((options >> 2) && 1 ? 2 * list_size: 2) * sizeof(uint16_t));
	poly_array->rotation = (float *) malloc(((options >> 4) && 1 ? list_size: 1) * sizeof(float));
	poly_array->colour = (uint32_t *) malloc(((options >> 5) && 1 ? list_size: 1) * sizeof(uint32_t));

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

//Free Texture and anim array
//Doesn't free the spritesheet struct itself
extern uint8_t crayon_memory_free_spritesheet(crayon_spritesheet_t *ss){
	if(ss){
		uint16_t i;
		for(i = 0; i < ss->animation_count; i++){
			free(ss->animation_array[i].name);
		}
		free(ss->animation_array);

		//name is unused so we don't free it
		//free(ss->name);

		pvr_mem_free(ss->texture);

		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_prop_font_sheet(crayon_font_prop_t *fp){
	if(fp){
		free(fp->char_x_coord);
		free(fp->char_width);
		free(fp->chars_per_row);
		pvr_mem_free(fp->texture);
		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_mono_font_sheet(crayon_font_mono_t *fm){
	if(fm){
		pvr_mem_free(fm->texture);
		return 0;
	}
	return 1;
}

extern uint8_t crayon_memory_free_palette(crayon_palette_t *cp){
	if(cp){
		if(cp->palette){
			free(cp->palette);
			return 0;
		}
	}
	return 1;
}

extern uint8_t crayon_memory_free_sprite_array(crayon_sprite_array_t *sprite_array, uint8_t free_ss, uint8_t free_pal){
	uint8_t ret_val = 0;
	if(free_ss){
		ret_val = crayon_memory_free_spritesheet(sprite_array->spritesheet);
		if(ret_val){return ret_val;}
	}
	if(free_pal){
		ret_val = crayon_memory_free_palette(sprite_array->palette);
		if(ret_val){return ret_val;}
	}

	//Free shouldn't do anything if you try to free a NULL ptr, but just incase...
	if(sprite_array->pos){free(sprite_array->pos);}
	if(sprite_array->frame_coord_key){free(sprite_array->frame_coord_key);}
	if(sprite_array->frame_coord_map){free(sprite_array->frame_coord_map);}
	if(sprite_array->colour){free(sprite_array->colour);}
	if(sprite_array->fade){free(sprite_array->fade);}
	if(sprite_array->scale){free(sprite_array->scale);}
	if(sprite_array->flip){free(sprite_array->flip);}
	if(sprite_array->rotation){free(sprite_array->rotation);}
	if(sprite_array->layer){free(sprite_array->layer);}

	//Set to NULL just incase user accidentally tries to free these arrays again
	sprite_array->pos = NULL;
	sprite_array->frame_coord_key = NULL;
	sprite_array->frame_coord_map = NULL;
	sprite_array->colour = NULL;
	sprite_array->scale = NULL;
	sprite_array->flip = NULL;
	sprite_array->rotation = NULL;
	sprite_array->layer = NULL;

	return 0;
}

extern uint8_t crayon_memory_free_untextured_array(crayon_untextured_array_t *untextured_array){
	if(untextured_array->pos){free(untextured_array->pos);}
	if(untextured_array->layer){free(untextured_array->layer);}
	if(untextured_array->colour){free(untextured_array->colour);}
	if(untextured_array->dimensions){free(untextured_array->dimensions);}
	if(untextured_array->rotation){free(untextured_array->rotation);}

	untextured_array->pos = NULL;
	untextured_array->layer = NULL;
	untextured_array->colour = NULL;
	untextured_array->dimensions = NULL;
	untextured_array->rotation = NULL;

	return 0;
}

extern uint8_t crayon_memory_mount_romdisk(char *filename, char *mountpoint){
	void *buffer;
	ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM

	if(size == -1){
		return 1;
	}
	
	fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
	return 0;
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
