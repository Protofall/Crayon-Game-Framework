#include "memory.h"

char *__game_base_path = NULL;
float __CRAYON_MEMORY_DEBUG_VARIABLES[16] = {0};

char *crayon_memory_get_full_path(char *path){
	uint8_t game_base_path_length = strlen(__game_base_path);
	uint8_t path_length = strlen(path);

	char *real_path = malloc(sizeof(char) * (game_base_path_length + path_length + 1));
	memcpy(real_path, __game_base_path, game_base_path_length + 1);
	memcpy(real_path + game_base_path_length, path, path_length + 1);

	return real_path;
}

uint8_t crayon_memory_load_dtex(crayon_txr_ptr_t *dtex, uint16_t *texture_width, uint16_t *texture_height,
	uint32_t *format, char *texture_path){

	if(*dtex != NULL){return 1;}

	uint8_t dtex_result = 0;
	dtex_header_t dtex_header;

	#define DTEX_ERROR(n) {dtex_result = n; goto DTEX_cleanup;}

		FILE *texture_file = fopen(texture_path, "rb");
		if(!texture_file){DTEX_ERROR(2);}

		if(fread(&dtex_header, sizeof(dtex_header_t), 1, texture_file) != 1){DTEX_ERROR(3);}

		if(memcmp(dtex_header.magic, "DTEX", 4)){DTEX_ERROR(4);}

		*dtex = pvr_mem_malloc(dtex_header.size);
		if(!*dtex){DTEX_ERROR(5);}

		if(fread(*dtex, dtex_header.size, 1, texture_file) != 1){DTEX_ERROR(6);}

		*texture_width = dtex_header.width;
		*texture_height = dtex_header.height;
		*format = dtex_header.type;

	#undef DTEX_ERROR

	DTEX_cleanup:

	if(texture_file){fclose(texture_file);}
	if(dtex_result && *dtex != NULL){crayon_memory_free_txr(*dtex);}

	return dtex_result;
}

uint8_t crayon_memory_load_spritesheet(crayon_spritesheet_t *ss, crayon_palette_t *cp, char *path,
		uint8_t use_game_base_path, int8_t palette_id){
	char *real_path = NULL;
	if(use_game_base_path){
		real_path = crayon_memory_get_full_path(path);
	}
	else{
		real_path = path;
	}

	uint8_t result = 0;
	unsigned int i = 0;

	FILE *txt_file = NULL;
	char *palette_path = NULL;
	char *txt_path = NULL;

	ss->texture = NULL;
	ss->animation = NULL;
	if(cp){cp->palette = NULL;}

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	uint8_t dtex_result = crayon_memory_load_dtex(&ss->texture, &ss->texture_width, &ss->texture_height, &ss->texture_format, path);
	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = crayon_misc_extract_bits(ss->texture_format, 3, 27);	// Extract the Pixel format

	uint8_t bpp = 0;
	if(texture_format > 6){	// Invalid format
		ERROR(7);
	}
	else if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}

	if(cp && palette_id >= 0 && bpp){	// If we pass in -1 or no cp, then we skip palettes
		if(crayon_misc_combine_strings(&palette_path, real_path, ".pal")){ERROR(8);}

		int resultPal = crayon_memory_load_palette(cp, palette_path, bpp, palette_id, CRAYON_USE_EXACT_PATH);
		// The function will modify the palette and colour count. Also it sends the BPP through
		if(resultPal){ERROR(9 + resultPal);}
		free(palette_path);
		palette_path = NULL;
	}

	if(crayon_misc_change_extension(&txt_path, real_path, "txt")){ERROR(16);}

	txt_file = fopen(txt_path, "rb");
	if(!txt_file){ERROR(17);}
	free(txt_path);
	txt_path = NULL;

	int number_holder;
	if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(18);}
	ss->animation_count = number_holder;

	ss->animation = malloc(sizeof(crayon_animation_t) * ss->animation_count);
	if(!ss->animation){ERROR(19);}
	
	char anim_name_part;
	int16_t count;
	for(i = 0; i < ss->animation_count; i++){
		ss->animation[i].name = NULL;

		// Check the length of the name
		count = 0;
		while((anim_name_part = getc(txt_file)) != '\n'){
			count++;
		}

		ss->animation[i].name = malloc((count + 1) * sizeof(char));
		if(!ss->animation[i].name){ERROR(20);}

		fseek(txt_file, -count - 1, SEEK_CUR);  // Go back so we can store the name

		// Store the name
		fread(ss->animation[i].name, sizeof(char), count, txt_file);
		ss->animation[i].name[count] = '\0';

		// Store the rest of the info
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(21);}
		ss->animation[i].x = number_holder;
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(22);}
		ss->animation[i].y = number_holder;
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(23);}
		ss->animation[i].sheet_width = number_holder;
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(24);}
		ss->animation[i].sheet_height = number_holder;
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(25);}
		ss->animation[i].frame_width = number_holder;
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(26);}
		ss->animation[i].frame_height = number_holder;
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(27);}
		ss->animation[i].frame_count = number_holder;
	}

	#undef ERROR

	cleanup:

	if(use_game_base_path){
		free(real_path);
	}

	if(txt_file){fclose(txt_file);} // May need to enclode this in an if "res >= 12" if statement

	// If a failure occured somewhere
	if(result){
		if(ss->texture){crayon_memory_free_txr(ss->texture);}
		if(cp->palette){free(cp->palette);}

		// Cleanup any names that were given
		if(ss->animation){
			uint16_t j;
			for(j = 0; j <= i; j++){
				if(ss->animation[j].name){free(ss->animation[j].name);}
			}
			free(ss->animation);
		}
	}

	if(palette_path){free(palette_path);}
	if(txt_path){free(palette_path);}

	return result;
}

uint8_t crayon_memory_load_prop_font_sheet(crayon_font_prop_t *fp, crayon_palette_t *cp, char *path,
		uint8_t use_game_base_path, int8_t palette_id){
	char *real_path = NULL;
	if(use_game_base_path){
		real_path = crayon_memory_get_full_path(path);
	}
	else{
		real_path = path;
	}

	uint8_t result = 0;
	fp->chars_per_row = NULL;
	fp->char_width = NULL;
	fp->char_x_coord = NULL;
	fp->texture = NULL;

	if(cp){cp->palette = NULL;}
	FILE *txt_file = NULL;

	char *palette_path = NULL;
	char *txt_path = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	uint8_t dtex_result = crayon_memory_load_dtex(&fp->texture, &fp->texture_width, &fp->texture_height, &fp->texture_format, path);
	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = crayon_misc_extract_bits(fp->texture_format, 3, 27);	// Extract the Pixel format

	uint8_t bpp = 0;
	if(texture_format > 6){	// Invalid format
		ERROR(7);
	}
	else if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}

	if(palette_id >= 0 && bpp){	// If we pass in -1, then we skip palettes
		if(crayon_misc_combine_strings(&palette_path, real_path, ".pal")){ERROR(8);}

		// The function will load the palette and colour count. With the specified BPP
		int resultPal = crayon_memory_load_palette(cp, palette_path, bpp, palette_id, CRAYON_USE_EXACT_PATH);
		if(resultPal){ERROR(9 + resultPal);}
		free(palette_path);
		palette_path = NULL;
	}

	if(crayon_misc_change_extension(&txt_path, real_path, "txt")){ERROR(16);}

	txt_file = fopen(txt_path, "rb");
	if(!txt_file){ERROR(17);}
	free(txt_path);
	txt_path = NULL;

	int number_holder;
	if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(18);}
	fp->char_height = number_holder;

	if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(19);}
	fp->num_rows = number_holder;

	fp->chars_per_row = malloc(fp->num_rows * sizeof(uint8_t));
	if(!fp->chars_per_row){ERROR(20);}

	fp->num_chars = 0;
	unsigned int i;
	for(i = 0; i < fp->num_rows; i++){
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(21);}
		fp->chars_per_row[i] = number_holder;
		fp->num_chars += fp->chars_per_row[i];
	}

	fp->char_width = malloc((fp->num_chars) * sizeof(uint8_t));
	if(!fp->char_width){ERROR(22);}

	for(i = 0; i < fp->num_chars; i++){
		if(crayon_misc_fget_next_int(txt_file, &number_holder)){ERROR(23);}
		fp->char_width[i] = number_holder;
	}

	// This section geterates the x coordinates for each char
	fp->char_x_coord = malloc((fp->num_chars) * sizeof(uint8_t));
	if(!fp->char_x_coord){ERROR(24);}

	// This for loop generates the x positions in the texture for each character
	int chars_counted_in_row = 0;
	int current_row = 0;
	for(i = 0; i < fp->num_chars; i++){
		if(chars_counted_in_row == 0){	// first element per row = zero
			fp->char_x_coord[i] = 0;
		}
		else{
			fp->char_x_coord[i] = fp->char_x_coord[i - 1] + fp->char_width[i - 1];	// nth element = (n-1)th element's width + x pos
		}
		chars_counted_in_row++;

		if(chars_counted_in_row == fp->chars_per_row[current_row]){	// When we reach the end of the row, reset our counters
			current_row++;
			chars_counted_in_row = 0;
		}
	}

	// Spacing when rendering the text
	fp->char_spacing.x = 0;
	fp->char_spacing.y = 0;

	#undef ERROR

	cleanup:

	if(use_game_base_path){
		free(real_path);
	}

	if(txt_file){fclose(txt_file);}

	// If an error occured, free these things
	if(result){
		if(fp->texture){crayon_memory_free_txr(fp->texture);}
		if(cp->palette){free(cp->palette);}
		if(fp->chars_per_row){free(fp->chars_per_row);}
		if(fp->char_width){free(fp->char_width);}
		if(fp->char_x_coord){free(fp->char_x_coord);}
	}

	if(palette_path){free(palette_path);}
	if(txt_path){free(palette_path);}

	return result;
}

uint8_t crayon_memory_load_mono_font_sheet(crayon_font_mono_t *fm, crayon_palette_t *cp, char *path,
		uint8_t use_game_base_path, int8_t palette_id){
	char *real_path = NULL;
	if(use_game_base_path){
		real_path = crayon_memory_get_full_path(path);
	}
	else{
		real_path = path;
	}

	uint8_t result = 0;
	
	fm->texture = NULL;
	if(cp){cp->palette = NULL;}
	FILE *txt_file = NULL;

	char *palette_path = NULL;
	char *txt_path = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	uint8_t dtex_result = crayon_memory_load_dtex(&fm->texture, &fm->texture_width, &fm->texture_height, &fm->texture_format, path);
	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = crayon_misc_extract_bits(fm->texture_format, 3, 27);	// Extract the Pixel format

	uint8_t bpp = 0;
	if(texture_format > 6){	// Invalid format
		ERROR(7);
	}
	else if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}

	if(cp && palette_id >= 0 && bpp){	// If we pass in -1, then we skip palettes
		if(crayon_misc_combine_strings(&palette_path, real_path, ".pal")){ERROR(8);}
		
		cp->palette = NULL;
		// The function will load the palette and colour count. With the specified BPP
		uint8_t resultPal = crayon_memory_load_palette(cp, palette_path, bpp, palette_id, CRAYON_USE_EXACT_PATH);
		if(resultPal){ERROR(9 + resultPal);}
		free(palette_path);
		palette_path = NULL;
	}

	if(crayon_misc_change_extension(&txt_path, real_path, "txt")){ERROR(16);}

	// Read the info file ( Format "NUM1\sNUM2\sNUM3\sNUM4\n" )
	txt_file = fopen(txt_path, "rb");
	if(!txt_file){ERROR(17);}
	free(txt_path);
	txt_path = NULL;

	int number;
	if(crayon_misc_fget_next_int(txt_file, &number)){ERROR(18);}
	fm->char_width = number;
	if(crayon_misc_fget_next_int(txt_file, &number)){ERROR(19);}
	fm->char_height = number;
	if(crayon_misc_fget_next_int(txt_file, &number)){ERROR(20);}
	fm->num_columns = number;
	if(crayon_misc_fget_next_int(txt_file, &number)){ERROR(21);}
	fm->num_rows = number;

	fm->num_chars = fm->num_columns * fm->num_rows;	// The number of chars *may* be less than this, but it won't be fatal

	fm->char_spacing.x = 0;
	fm->char_spacing.y = 0;

	#undef ERROR

	cleanup:

	if(use_game_base_path){
		free(real_path);
	}

	if(txt_file){fclose(txt_file);}

	// If a failure occured somewhere after loading texture
	if(result && fm->texture){crayon_memory_free_txr(fm->texture);}

	// If we allocated memory for the palette and error out
	if(result && cp->palette){free(cp->palette);}

	if(palette_path){free(palette_path);}
	if(txt_path){free(txt_path);}

	return result;
}

uint8_t crayon_memory_load_palette(crayon_palette_t *cp, char *path, int8_t bpp, int8_t palette_id,
		uint8_t use_game_base_path){
	char *real_path = NULL;
	if(use_game_base_path){
		real_path = crayon_memory_get_full_path(path);
	}
	else{
		real_path = path;
	}

	uint8_t result = 0;
	FILE *palette_file = NULL;

	#define PAL_ERROR(n) {result = (n); goto PAL_cleanup;}

	if(!cp){PAL_ERROR(1);}
	
	palette_file = fopen(real_path, "rb");
	if(!palette_file){PAL_ERROR(2);}

	dpal_header_t dpal_header;
	if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){PAL_ERROR(3);}

	if(memcmp(dpal_header.magic, "DPAL", 4) || dpal_header.color_count == 0){PAL_ERROR(4);}

	cp->palette = malloc(dpal_header.color_count * sizeof(uint32_t));
	if(!cp->palette){PAL_ERROR(5);}

	if(fread(cp->palette, sizeof(uint32_t), dpal_header.color_count, palette_file) !=
		dpal_header.color_count){PAL_ERROR(6);}

	#undef PAL_ERROR

	cp->colour_count = dpal_header.color_count;
	cp->bpp = bpp;
	cp->palette_id = palette_id;

	PAL_cleanup:

	if(use_game_base_path){
		free(real_path);
	}

	if(palette_file){fclose(palette_file);}
	if(result > 5){free(cp->palette); cp->palette = NULL;}

	return result;
}

uint8_t crayon_memory_clone_palette(crayon_palette_t *original, crayon_palette_t *copy, int8_t palette_id){
	copy->palette = malloc(original->colour_count * sizeof(uint32_t));
	if(copy->palette == NULL){return 1;}
	copy->colour_count = original->colour_count;
	copy->bpp = original->bpp;
	copy->palette_id = palette_id;

	uint16_t i;
	for(i = 0; i < copy->colour_count; i++){	// Goes through the palette and adds in all values
		copy->palette[i] = original->palette[i];
	}

	return 0;
}

uint8_t crayon_memory_init_sprite_array(crayon_sprite_array_t *sprite_array, crayon_spritesheet_t *ss,
	int16_t animation_id, crayon_palette_t *pal, uint16_t size, uint8_t frames_used, uint8_t options,
	uint8_t filter, uint8_t set_defaults){

	sprite_array->options = options;
	sprite_array->filter = filter;

	if(ss){
		// For example, crayon_memory_get_animation_id() failed
		if(animation_id < 0){
			return 1;
		}

		sprite_array->spritesheet = ss;
		sprite_array->animation = &ss->animation[animation_id];
		sprite_array->palette = pal;
		sprite_array->frames_used = frames_used;
		sprite_array->options |= CRAYON_HAS_TEXTURE;	// Set the textured bit
	}
	else{
		sprite_array->spritesheet = NULL;	// For safety sake
		sprite_array->palette = NULL;
	}

	sprite_array->coord = NULL;
	sprite_array->layer = NULL;
	sprite_array->scale = NULL;
	sprite_array->colour = NULL;
	sprite_array->rotation = NULL;
	sprite_array->visible = NULL;
	sprite_array->frame_id = NULL;
	sprite_array->frame_uv = NULL;
	sprite_array->fade = NULL;
	sprite_array->flip = NULL;

	sprite_array->head = NULL;

	// Since allocate function doesn't do this one
	if(ss){
		sprite_array->frame_uv = (vec2_u16_t *) malloc(frames_used * sizeof(vec2_u16_t));
		if(sprite_array->frame_uv == NULL){
			crayon_memory_free_sprite_array(sprite_array);
			return 1;
		}
	}

	// First param is there to not throw errors when we init an empty list
	if(crayon_memory_allocate_sprite_array(sprite_array, size, 1)){
		// An allocation error occured. Free the arrays that were allocated
		crayon_memory_free_sprite_array(sprite_array);
		return 1;
	}

	// Sets default values so everything is initialised
	if(set_defaults){
		crayon_memory_set_defaults_sprite_array(sprite_array, 0, sprite_array->size - 1, 1);
	}

	// Add the references if the user asked for them
	if(options & CRAYON_REF_LIST){
		if(crayon_memory_add_sprite_array_refs(sprite_array, 0, size - 1)){
			crayon_memory_free_sprite_array(sprite_array);
			return 1;
		}
	}

	return 0;
}

uint8_t crayon_memory_clone_sprite_array(crayon_sprite_array_t *dest, crayon_sprite_array_t *src){
	dest->options = src->options;
	dest->filter = src->filter;

	if(src->spritesheet != NULL){
		dest->spritesheet = src->spritesheet;
		dest->animation = src->animation;
		dest->palette = src->palette;
		dest->frames_used = src->frames_used;
	}
	else{
		dest->spritesheet = NULL;	// For safety sake
		dest->palette = NULL;
	}

	dest->coord = NULL;
	dest->layer = NULL;
	dest->scale = NULL;
	dest->colour = NULL;
	dest->rotation = NULL;
	dest->visible = NULL;
	dest->frame_id = NULL;
	dest->frame_uv = NULL;
	dest->fade = NULL;
	dest->flip = NULL;

	dest->head = NULL;

	// Since allocate function doesn't do this one
	if(dest->spritesheet){
		dest->frame_uv = (vec2_u16_t *) malloc(dest->frames_used * sizeof(vec2_u16_t));
		if(dest->frame_uv == NULL){
			crayon_memory_free_sprite_array(dest);
			return 1;
		}
	}

	// First param is there to not throw errors when we init an empty list
	if(crayon_memory_allocate_sprite_array(dest, src->size, 1)){
		// An allocation error occured. Free the arrays that were allocated
		crayon_memory_free_sprite_array(dest);
		return 1;
	}

	// Now copy over the actual data
	uint16_t i;
	for(i = 0; i < dest->size; i++){
		dest->coord[i].x = src->coord[i].x;
		dest->coord[i].y = src->coord[i].y;
		dest->layer[i] = src->layer[i];
		dest->visible[i] = src->visible[i];

		if(i == 0 || (dest->options & CRAYON_MULTI_SCALE)){
			dest->scale[i].x = src->scale[i].x;
			dest->scale[i].y = src->scale[i].y;
		}

		if(i == 0 || (dest->options & CRAYON_MULTI_FLIP)){
			dest->flip[i] = src->flip[i];
		}

		if(i == 0 || (dest->options & CRAYON_MULTI_FRAME)){
			dest->frame_id[i] = src->frame_id[i];
		}

		if(i == 0 || (dest->options & CRAYON_MULTI_ROTATE)){
			dest->rotation[i] = src->rotation[i];
		}

		if(i == 0 || (dest->options & CRAYON_MULTI_COLOUR)){
			dest->colour[i] = src->colour[i];
			dest->fade[i] = src->fade[i];
		}
	}
	for(i = 0; i < dest->frames_used; i++){
		dest->frame_uv[i].x = src->frame_uv[i].x;
		dest->frame_uv[i].y = src->frame_uv[i].y;
	}

	// And copy over the sprite_array (For now I'll add all elements if the user has the setting enabled)
	if(dest->options & CRAYON_REF_LIST){
		if(crayon_memory_add_sprite_array_refs(dest, 0, dest->size - 1)){
			crayon_memory_free_sprite_array(dest);
			return 1;
		}
	}

	return 0;
}

void crayon_memory_init_camera(crayon_viewport_t *camera, vec2_f_t world_coord, vec2_u16_t world_dim,
	vec2_u16_t window_coord, vec2_u16_t window_dim, float world_movement_factor){

	camera->world_x = world_coord.x;
	camera->world_y = world_coord.y;
	camera->world_width = world_dim.x;
	camera->world_height = world_dim.y;
	camera->world_movement_factor = world_movement_factor;
	camera->window_x = window_coord.x;
	camera->window_y = window_coord.y;
	camera->window_width = window_dim.x;
	camera->window_height = window_dim.y;

	return;
}

// Example, only 2 elements in list and we give 0, 1.
// We'd expect references with id's {0, 1}. But we're getting {0, 0}
// Indexes length = 2
/*
	while: i = 0. Enters loop:
		Enters while loop
		if: walker->id == 0 which isn't greater than indexes[0] == 0
		if: walker->id == indexes[0]
			// The correct reference is added to list[0]
			// i = 1;
		go to next in list
	while: i = 1. Enters loop:
		if: walker->id == 1, indexes[1] == 1. Fails
		if: walker->id == indexes[1]. True
			// The correct element *should* be added...
			// i++
		go to next in list
	while: i = 2. Fails
	returns list
*/
crayon_sprite_array_reference_t **crayon_memory_get_sprite_array_refs(crayon_sprite_array_t *sprite_array,
	uint16_t *indexes, uint16_t indexes_length){
	if(sprite_array->head == NULL){return NULL;}	// Incase you call on a sprite array with no refs

	crayon_sprite_array_reference_t **list = malloc(sizeof(crayon_sprite_array_reference_t *) * indexes_length);
	if(list == NULL){return NULL;}

	// Set all to null, just incase a reference isn't found
	uint16_t i;
	for(i = 0; i < indexes_length; i++){
		list[i] = NULL;
	}

	i = 0;
	crayon_sprite_array_reference_t *walker = sprite_array->head;
	while(walker != NULL && i < indexes_length){
		if(walker->id > indexes[i]){	// Lets say we ask for 6, 7, 8, 9 but we only have 6, 8, 9 and 10
										// When we get to 8 (In LL), thats > 7 so we check the next element which is 8
										// 8 is a number we are looking for so we add that in
										// element 2 of "list" will be NULL due to the above for-loop
			i++;
			continue;
		}
		if(walker->id == indexes[i]){	// If the elements match, add it to the list
			list[i] = walker;
			i++;
		}
		walker = walker->next;
	}

	return list;
}

// This would have issues if you try to re-add an existing reference or add before the last element
	// However in the intended use, its only called by init for all elements and extend so this doesn't normally occur
uint8_t crayon_memory_add_sprite_array_refs(crayon_sprite_array_t *sprite_array, uint16_t lower, int32_t upper){
	if(upper < 0 || sprite_array->size == 0){return 0;}	// Since this sorta is valid, we return 0
	if(upper >= sprite_array->size || lower > upper){return 1;}

	crayon_sprite_array_reference_t *prev_node = NULL;
	crayon_sprite_array_reference_t *curr_node = sprite_array->head;

	uint16_t i;

	// This should handle an empty reference list case
	if(sprite_array->head == NULL){
		for(i = lower; i <= upper; i++){
			// Setup new node
			curr_node = malloc(sizeof(crayon_sprite_array_reference_t));
			if(!curr_node){return 1;}

			curr_node->next = NULL;
			curr_node->id = i;

			// Update head if needed
			if(sprite_array->head == NULL){
				sprite_array->head = curr_node;
			}
			else{
				prev_node->next = curr_node;	// Because otheriwse prev_node == NULL and that has issues
			}

			// Update previous node
			prev_node = curr_node;
		}
		return 0;
	}

	// This assumes we are always adding to the end
	while(curr_node->next != NULL){
		curr_node = curr_node->next;
	}

	prev_node = curr_node;
	for(i = lower; i <= upper; i++){
		// Setup new node
		curr_node = malloc(sizeof(crayon_sprite_array_reference_t));
		if(!curr_node){return 1;}

		curr_node->next = NULL;
		curr_node->id = i;

		prev_node->next = curr_node;
		prev_node = curr_node;
	}

	return 0;
}

void crayon_memory_remove_sprite_array_refs(crayon_sprite_array_t *sprite_array, uint16_t *indexes,
	uint16_t indexes_length){

	uint16_t i_c = 0;	// The index for the "indexes" array. Also used to adjust the id's of the nodes
	crayon_sprite_array_reference_t *curr_node = sprite_array->head;
	crayon_sprite_array_reference_t *prev_node = NULL;
	crayon_sprite_array_reference_t *delete_node;
	while(curr_node != NULL){
		if(i_c < indexes_length && curr_node->id == indexes[i_c]){	// Element we need to delete
			// Remove this current node from the list
			if(prev_node != NULL){
				prev_node->next = curr_node->next;
			}
			if(sprite_array->head == curr_node){
				sprite_array->head = curr_node->next;
			}

			delete_node = curr_node;
			curr_node = curr_node->next;
			free(delete_node);
			i_c++;
			continue;
		}

		// Else we keep curr_node and adjust its id
		curr_node->id -= i_c;
		prev_node = curr_node;
		curr_node = curr_node->next;
	}

	return;
}

uint16_t crayon_memory_swap_colour(crayon_palette_t *cp, uint32_t old, uint32_t new, uint8_t _continue){
	uint16_t i;
	uint16_t found = 0;
	for(i = 0; i < cp->colour_count; ++i){
		if(cp->palette[i] == old){
			cp->palette[i] = new;
			found++;
			if(!_continue){
				break;
			}
		}
	}
	return found;
}

uint8_t crayon_memory_remove_sprite_array_elements(crayon_sprite_array_t *sprite_array, uint16_t *indexes,
	uint16_t indexes_length){

	if(indexes_length == 0 || sprite_array->size == 0){return 1;}

	uint16_t array_index = indexes[0];	// Start from the first remove
	uint16_t elements_to_shift;

	uint16_t i;
	// Basically what we want to do is find all the gaps in the list and shift them down.
	for(i = 0; i < indexes_length; i++){
		// Incase you try to remove an element beyond the array (size 2, removing {0,1,2})
		if(indexes[i] >= sprite_array->size){
			i--;
			break;
		}

		// Determine how many elements to copy over
		if(i != indexes_length - 1){
			elements_to_shift = indexes[i + 1] - indexes[i] - 1;
		}
		else{	// For final element do that against the end of the list
			elements_to_shift = sprite_array->size - indexes[i] - 1;
		}

		if(elements_to_shift > 0){
			memmove(&sprite_array->coord[array_index], &sprite_array->coord[array_index + i + 1], elements_to_shift * sizeof(vec2_f_t));
			memmove(&sprite_array->layer[array_index], &sprite_array->layer[array_index + i + 1], elements_to_shift * sizeof(uint8_t));
			memmove(&sprite_array->visible[array_index], &sprite_array->visible[array_index + i + 1], elements_to_shift * sizeof(uint8_t));

			if(sprite_array->options & CRAYON_MULTI_COLOUR){
				memmove(&sprite_array->colour[array_index], &sprite_array->colour[array_index + i + 1], elements_to_shift * sizeof(uint32_t));
			}
			if(sprite_array->options & CRAYON_MULTI_SCALE){
				memmove(&sprite_array->scale[array_index], &sprite_array->scale[array_index + i + 1], elements_to_shift * sizeof(vec2_f_t));
			}
			if(sprite_array->options & CRAYON_MULTI_ROTATE){
				memmove(&sprite_array->rotation[array_index], &sprite_array->rotation[array_index + i + 1], elements_to_shift * sizeof(float));
			}

			if(sprite_array->options & CRAYON_HAS_TEXTURE){
				if(sprite_array->options & CRAYON_MULTI_FLIP){
					memmove(&sprite_array->flip[array_index], &sprite_array->flip[array_index + i + 1], elements_to_shift * sizeof(uint8_t));
				}
				if(sprite_array->options & CRAYON_MULTI_COLOUR){
					memmove(&sprite_array->fade[array_index], &sprite_array->fade[array_index + i + 1], elements_to_shift * sizeof(uint8_t));
				}
				if(sprite_array->options & CRAYON_MULTI_FRAME){
					memmove(&sprite_array->frame_id[array_index], &sprite_array->frame_id[array_index + i + 1], elements_to_shift * sizeof(uint8_t));
				}
			}

			// Lastly do this
			array_index += elements_to_shift;
		}
	}

	// Replaced indexes_length with i to protect against removing too much

	// Resize the arrays with realloc (MIGHT BE ABLE TO REUSE array_index HERE)
	if(crayon_memory_allocate_sprite_array(sprite_array, sprite_array->size - i, 0)){return 2;}

	// Handle the references linked list here
	crayon_memory_remove_sprite_array_refs(sprite_array, indexes, i);

	return 0;
}

// Note, even if these pointers point to "NULL", it will instead behave like malloc
	// So if I use this in the init function. Make sure to set all the pointers to NULL before sending them through here
	// Also the "size == 0" checks are there since zero-length sprite-arrays should be allowed to pass
uint8_t crayon_memory_allocate_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t size, uint8_t set_array_globals){
	void *holder;

	// 1 per element (Set these every time)

	holder =  realloc(sprite_array->coord, size * sizeof(vec2_f_t));
	if(size == 0 || holder != NULL){
		sprite_array->coord = holder;
	}
	else{return 1;}

	holder =  realloc(sprite_array->layer, size * sizeof(uint8_t));
	if(size == 0 || holder != NULL){
		sprite_array->layer = holder;
	}
	else{return 1;}

	holder =  realloc(sprite_array->visible, size * sizeof(uint8_t));
	if(size == 0 || holder != NULL){
		sprite_array->visible = holder;
	}
	else{return 1;}

	// MULTIs
		// Cases where we *don't* realloc.
			// Its an array_global and set_array_globals == 0
				// if(!(!(options & MULTI) && set_array_globals == 0))
				// OR we enter if(set_array_globals || its not a global)
		// We only enter it if we change size


		// If its both multi and global, then we allocate size, which isn't right...


	// Allocate to 1 if non-multi and global

	// Set to size if mutli

	// else don't touch it

	// RULEa (3/3 satisfied)
		// We always alloc on set_array_globals (And in that case, we set to 1 if non-Multi or size if Multi)
		// if set_array_globals is 0, we never update the non-Multis
		// if we set size to zero and set_array_globals == 0, then it only enters if its multi and then it sets itself to zero

	if((sprite_array->options & CRAYON_MULTI_SCALE) || set_array_globals){
		holder =  realloc(sprite_array->scale, ((sprite_array->options & CRAYON_MULTI_SCALE) ? size: 1) * sizeof(vec2_f_t));
		if(size == 0 || holder != NULL){
			sprite_array->scale = holder;
		}
		else{return 1;}
	}

	if((sprite_array->options & CRAYON_MULTI_COLOUR) || set_array_globals){
		holder =  realloc(sprite_array->colour, ((sprite_array->options & CRAYON_MULTI_COLOUR) ? size: 1) * sizeof(uint32_t));
		if(size == 0 || holder != NULL){
			sprite_array->colour = holder;
		}
		else{return 1;}
	}

	if((sprite_array->options & CRAYON_MULTI_ROTATE) || set_array_globals){
		holder =  realloc(sprite_array->rotation, ((sprite_array->options & CRAYON_MULTI_ROTATE) ? size: 1) * sizeof(float));
		if(size == 0 || holder != NULL){
			sprite_array->rotation = holder;
		}
		else{return 1;}
	}

	if(sprite_array->options & CRAYON_HAS_TEXTURE){
		if((sprite_array->options & CRAYON_MULTI_FRAME) || set_array_globals){
			holder =  realloc(sprite_array->frame_id, ((sprite_array->options & CRAYON_MULTI_FRAME) ? size: 1) * sizeof(uint8_t));
			if(size == 0 || holder != NULL){
				sprite_array->frame_id = holder;
			}
			else{return 1;}
		}

		if((sprite_array->options & CRAYON_MULTI_COLOUR) || set_array_globals){
			holder =  realloc(sprite_array->fade, ((sprite_array->options & CRAYON_MULTI_COLOUR) ? size: 1) * sizeof(uint8_t));
			if(size == 0 || holder != NULL){
				sprite_array->fade = holder;
			}
			else{return 1;}
		}

		if((sprite_array->options & CRAYON_MULTI_FLIP) || set_array_globals){
			holder =  realloc(sprite_array->flip, ((sprite_array->options & CRAYON_MULTI_FLIP) ? size: 1) * sizeof(uint8_t));
			if(size == 0 || holder != NULL){
				sprite_array->flip = holder;
			}
			else{return 1;}
		}
	}

	sprite_array->size = size;
	
	return 0;
}

uint8_t crayon_memory_extend_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t elements, uint8_t set_defaults){
	elements += sprite_array->size;
	if(elements <= sprite_array->size){return 1;}	// Overflow or adding zero elements

	uint16_t old_size = sprite_array->size;	// Needed for setting defaults
	crayon_memory_allocate_sprite_array(sprite_array, elements, 0);

	if(set_defaults){
		crayon_memory_set_defaults_sprite_array(sprite_array, old_size, sprite_array->size - 1, 0);
	}

	if(sprite_array->options & CRAYON_REF_LIST){
		crayon_memory_add_sprite_array_refs(sprite_array, old_size, elements - 1);
	}

	return 0;
}

void crayon_memory_set_defaults_sprite_array(crayon_sprite_array_t *sprite_array, uint16_t start, int32_t end,
	uint8_t set_array_globals){

	uint16_t i;

	// If length is zero, only set the multis that aren't multi
	if(sprite_array->size == 0 && set_array_globals){
		if(!(sprite_array->options & CRAYON_MULTI_COLOUR)){
			sprite_array->colour[0] = 0xFFFFFFFF;
			sprite_array->fade[0] = 0xFF;
		}
		if(!(sprite_array->options & CRAYON_MULTI_ROTATE)){
			sprite_array->rotation[0] = 0;
		}
		if(!(sprite_array->options & CRAYON_MULTI_FLIP)){
			sprite_array->flip[0] = 0;
		}
		if(!(sprite_array->options & CRAYON_MULTI_SCALE)){
			sprite_array->scale[0].x = 1;
			sprite_array->scale[0].y = 1;
		}
		if(!(sprite_array->options & CRAYON_MULTI_FRAME)){
			sprite_array->frame_id[0] = 0;
		}
	}
	else{	// Add set_array_globals to this
		for(i = start; i <= end; i++){
			// Only set if Multi-and-size > 0 things or first loop
			if((i == 0 && set_array_globals) || ((sprite_array->options & CRAYON_MULTI_COLOUR))){
				sprite_array->colour[i] = 0xFFFFFFFF;
				sprite_array->fade[i] = 0xFF;
			}
			if((i == 0 && set_array_globals) || ((sprite_array->options & CRAYON_MULTI_ROTATE))){
				sprite_array->rotation[i] = 0;
			}
			if((i == 0 && set_array_globals) || ((sprite_array->options & CRAYON_MULTI_FLIP))){
				sprite_array->flip[i] = 0;
			}
			if((i == 0 && set_array_globals) || ((sprite_array->options & CRAYON_MULTI_SCALE))){
				sprite_array->scale[i].x = 1;
				sprite_array->scale[i].y = 1;
			}
			if((i == 0 && set_array_globals) || ((sprite_array->options & CRAYON_MULTI_FRAME))){
				sprite_array->frame_id[i] = 0;
			}

			if(sprite_array->size != 0){	// Set if we have at least one element in list
				sprite_array->coord[i].x = 0;
				sprite_array->coord[i].y = 0;
				sprite_array->layer[i] = 0xFF;
				sprite_array->visible[i] = 1;
			}
		}
	}

	if(set_array_globals){
		for(i = 0; i < sprite_array->frames_used; i++){
			// Later replace this with the UVs for the first frame
			sprite_array->frame_uv[i].x = 0;
			sprite_array->frame_uv[i].y = 0;
		}
	}

	return;
}

static void __crayon_memory_swap(uint16_t *a, uint16_t *b){
	uint16_t t = *a;
	*a = *b;
	*b = t;
} 
  
// This function takes last element as pivot, places 
// the pivot element at its correct position in sorted
// array, and places all smaller (smaller than pivot) 
// to left of pivot and all greater elements to right 
// of pivot
static int __crayon_memory_partition(uint16_t *arr, int low, int high){
	int pivot = arr[high];	// Pivot
	int i = (low - 1);	// Index of smaller element
	int j;

	for(j = low; j <= high- 1; j++){
		// If current element is smaller than the pivot
		if(arr[j] < pivot){
			i++;	// Increment index of smaller element
			__crayon_memory_swap(&arr[i], &arr[j]);
		} 
	} 
	__crayon_memory_swap(&arr[i + 1], &arr[high]);
	return (i + 1);
} 
  
// The main function that implements Quick Sort 
// arr[] --> Array to be sorted,
// low  --> Starting index,
// high  --> Ending index
void crayon_memory_quick_sort(uint16_t *arr, int low, int high){
	if(low < high){
		// pi is partitioning index, arr[p] is now at right place
		int pi = __crayon_memory_partition(arr, low, high);

		// Separately sort elements before
		// partition and after partition
		crayon_memory_quick_sort(arr, low, pi - 1);
		crayon_memory_quick_sort(arr, pi + 1, high);
	}
}


//---------------------------------------------


// Free Texture and anim array
// Doesn't free the spritesheet struct itself
int8_t crayon_memory_free_spritesheet(crayon_spritesheet_t *ss){
	if(ss){
		uint16_t i;
		for(i = 0; i < ss->animation_count; i++){
			free(ss->animation[i].name);
		}
		free(ss->animation);

		// name is unused so we don't free it
		// free(ss->name);

		crayon_memory_free_txr(ss->texture);

		return 0;
	}
	return 1;
}

int8_t crayon_memory_free_prop_font_sheet(crayon_font_prop_t *fp){
	if(fp){
		free(fp->char_x_coord);
		free(fp->char_width);
		free(fp->chars_per_row);
		crayon_memory_free_txr(fp->texture);
		return 0;
	}
	return 1;
}

int8_t crayon_memory_free_mono_font_sheet(crayon_font_mono_t *fm){
	if(fm){
		crayon_memory_free_txr(fm->texture);
		return 0;
	}
	return 1;
}

int8_t crayon_memory_free_palette(crayon_palette_t *cp){
	if(cp && cp->palette){
		free(cp->palette);
		return 0;
	}
	return 1;
}

int8_t crayon_memory_free_sprite_array(crayon_sprite_array_t *sprite_array){
	// Free shouldn't do anything if you try to free a NULL ptr, but just incase...
	if(sprite_array->coord){free(sprite_array->coord);}
	if(sprite_array->frame_id){free(sprite_array->frame_id);}
	if(sprite_array->frame_uv){free(sprite_array->frame_uv);}
	if(sprite_array->colour){free(sprite_array->colour);}
	if(sprite_array->fade){free(sprite_array->fade);}
	if(sprite_array->scale){free(sprite_array->scale);}
	if(sprite_array->flip){free(sprite_array->flip);}
	if(sprite_array->rotation){free(sprite_array->rotation);}
	if(sprite_array->layer){free(sprite_array->layer);}
	if(sprite_array->visible){free(sprite_array->visible);}

	// Set to NULL just incase user accidentally tries to free these arrays again
	sprite_array->coord = NULL;
	sprite_array->frame_id = NULL;
	sprite_array->frame_uv = NULL;
	sprite_array->colour = NULL;
	sprite_array->fade = NULL;
	sprite_array->scale = NULL;
	sprite_array->flip = NULL;
	sprite_array->rotation = NULL;
	sprite_array->layer = NULL;
	sprite_array->visible = NULL;

	// Free the references linked list (This should probably use the remove func)
	crayon_sprite_array_reference_t *curr_node = sprite_array->head;
	crayon_sprite_array_reference_t *prev_node;
	while(curr_node != NULL){
		if(curr_node->next == NULL){
			free(curr_node);
			curr_node = NULL;
		}
		else{
			prev_node = curr_node;
			curr_node = curr_node->next;
			free(prev_node);
		}
	}

	sprite_array->head = NULL;

	return 0;
}

int8_t crayon_memory_mount_romdisk(char *filename, char *mountpoint, uint8_t use_game_base_path){
	char *real_filename;
	if(use_game_base_path){
		real_filename = crayon_memory_get_full_path(filename);
	}
	else{
		real_filename = filename;
	}

	// TODO: Later add check to see if theres enough available main ram
	;

	void *buffer;
	ssize_t size = fs_load(real_filename, &buffer); // Loads the file into RAM

	// Free the temp string since we don't need it anymore
	if(use_game_base_path){
		free(real_filename);
	}

	if(size == -1){
		return 1;
	}
	
	fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted

	return 0;
}

int8_t crayon_memory_mount_romdisk_gz(char *filename, char *mountpoint, uint8_t use_game_base_path){
	char *real_filename;
	if(use_game_base_path){
		real_filename = crayon_memory_get_full_path(filename);
	}
	else{
		real_filename = filename;
	}

	void *buffer;
	int length = zlib_getlength(real_filename);

	// Free the temp string since we don't need it anymore
	if(use_game_base_path){
		free(real_filename);
	}

	// TODO: Later add check to see if theres enough available main ram
	;

	if(length == 0){
		return 1;
	}
	
	gzFile file = gzopen(filename, "rb"); // Seems to be the replacement of fs_load() along with gzread()
	if(!file){
		return 1;
	}

	// Allocate memory, read file
	buffer = malloc(length);
	if(!buffer){
		gzclose(file);
		return 1;
	}

	gzread(file, buffer, length);
	gzclose(file);

	fs_romdisk_mount(mountpoint, buffer, 1);

	return 0;
}

float crayon_memory_get_coord_x(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(index < sprites->size){
		if(error){*error = 0;}
		return sprites->coord[index].x;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

float crayon_memory_get_coord_y(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(index < sprites->size){
		if(error){*error = 0;}
		return sprites->coord[index].y;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

vec2_f_t crayon_memory_get_coords(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(index < sprites->size){
		if(error){*error = 0;}
		return sprites->coord[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return (vec2_f_t){0, 0};
}

uint32_t crayon_memory_get_colour(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_COLOUR) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->colour[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

uint8_t crayon_memory_get_fade(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_COLOUR) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->fade[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

float crayon_memory_get_scale_x(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_SCALE) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->scale[index].x;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

float crayon_memory_get_scale_y(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_SCALE) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->scale[index].y;
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

vec2_f_t crayon_memory_get_scales(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_SCALE) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->scale[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return (vec2_f_t){0, 0};
}

uint8_t crayon_memory_get_flip(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_FLIP) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->flip[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

float crayon_memory_get_rotation(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_ROTATE) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->rotation[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

uint8_t crayon_memory_get_visibility(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(index < sprites->size){
		if(error){*error = 0;}
		return sprites->visible[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

uint8_t crayon_memory_get_layer(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(index < sprites->size){
		if(error){*error = 0;}
		return sprites->layer[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

uint8_t crayon_memory_get_frame_id(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(((sprites->options & CRAYON_MULTI_FRAME) && index == 0) || index < sprites->size){
		if(error){*error = 0;}
		return sprites->frame_id[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return 0;
}

vec2_u16_t crayon_memory_get_frame_uv(crayon_sprite_array_t *sprites, uint16_t index, uint8_t *error){
	if(index < sprites->frames_used){
		if(error){*error = 0;}
		return sprites->frame_uv[index];
	}
	if(error){*error = 1;}
	perror("Your index is outside this array");
	return (vec2_u16_t){0, 0};
}

uint8_t crayon_memory_set_coord_x(crayon_sprite_array_t *sprites, uint16_t index, float value){
	if(index < sprites->size){
		sprites->coord[index].x = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_coord_y(crayon_sprite_array_t *sprites, uint16_t index, float value){
	if(index < sprites->size){
		sprites->coord[index].y = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_coords(crayon_sprite_array_t *sprites, uint16_t index, vec2_f_t value){
	if(index < sprites->size){
		sprites->coord[index].x = value.x;
		sprites->coord[index].y = value.y;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_colour(crayon_sprite_array_t *sprites, uint16_t index, uint32_t value){
	if(index == 0 || ((sprites->options & CRAYON_MULTI_COLOUR) && index < sprites->size)){
		sprites->colour[index] = value;
		return 0;
	}
	return 1;
}

// The extra condition at the beginning it to make sure we don't set fade for untextured polys
uint8_t crayon_memory_set_fade(crayon_sprite_array_t *sprites, uint16_t index, uint8_t value){
	if((sprites->options & CRAYON_HAS_TEXTURE) && (index == 0 ||
	((sprites->options & CRAYON_MULTI_COLOUR) && index < sprites->size))){
		sprites->fade[index] = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_scale_x(crayon_sprite_array_t *sprites, uint16_t index, float value){
	if(index == 0 || ((sprites->options & CRAYON_MULTI_SCALE) && index < sprites->size)){
		sprites->scale[index].x = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_scale_y(crayon_sprite_array_t *sprites, uint16_t index, float value){
	if(index == 0 || ((sprites->options & CRAYON_MULTI_SCALE) && index < sprites->size)){
		sprites->scale[index].y = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_scales(crayon_sprite_array_t *sprites, uint16_t index, vec2_f_t value){
	if(index == 0 || ((sprites->options & CRAYON_MULTI_SCALE) && index < sprites->size)){
		sprites->scale[index].x = value.x;
		sprites->scale[index].y = value.y;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_flip(crayon_sprite_array_t *sprites, uint16_t index, uint8_t value){
	if((sprites->options & CRAYON_HAS_TEXTURE) && (index == 0 ||
	((sprites->options & CRAYON_MULTI_FLIP) && index < sprites->size))){
		sprites->flip[index] = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_rotation(crayon_sprite_array_t *sprites, uint16_t index, float value){
	if(index == 0 || ((sprites->options & CRAYON_MULTI_ROTATE) && index < sprites->size)){
		sprites->rotation[index] = value;
		return 0;
	}
	return 1;
}


uint8_t crayon_memory_set_visibility(crayon_sprite_array_t *sprites, uint16_t index, uint8_t value){
	if(index < sprites->size){
		sprites->visible[index] = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_layer(crayon_sprite_array_t *sprites, uint16_t index, uint8_t value){
	if(index < sprites->size){
		sprites->layer[index] = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_frame_id(crayon_sprite_array_t *sprites, uint16_t index, uint8_t value){
	if((sprites->options & CRAYON_HAS_TEXTURE) && (index == 0 ||
	((sprites->options & CRAYON_MULTI_FRAME) && index < sprites->size))){
		sprites->frame_id[index] = value;
		return 0;
	}
	return 1;
}

uint8_t crayon_memory_set_frame_uv(crayon_sprite_array_t *sprites, uint16_t index, uint8_t frame_id){
	if((sprites->options & CRAYON_HAS_TEXTURE) && index < sprites->frames_used){
		crayon_animation_t *anim = sprites->animation;
		uint8_t frames_per_row = anim->sheet_width / anim->frame_width;
		uint8_t column_number = frame_id % frames_per_row;
		uint8_t row_number = frame_id / frames_per_row;

		sprites->frame_uv[index].x = anim->x + (column_number * anim->frame_width);
		sprites->frame_uv[index].y = anim->y + (row_number * anim->frame_height);
		return 0;
	}
	return 1;
}

int16_t crayon_memory_get_animation_id(char *name, crayon_spritesheet_t *ss){
	unsigned int i;
	for(i = 0; i < ss->animation_count; i++){
		if(!strcmp(ss->animation[i].name, name)){
			return i;
		}
	}
	return -1;
}

void crayon_memory_move_camera_x(crayon_viewport_t *camera, float x){
	camera->world_x += (x * camera->world_movement_factor);
	return;
}

void crayon_memory_move_camera_y(crayon_viewport_t *camera, float y){
	camera->world_y += (y * camera->world_movement_factor);
	return;
}
