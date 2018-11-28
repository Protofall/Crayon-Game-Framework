#include "memory.h"

extern uint8_t crayon_memory_load_dtex(pvr_ptr_t *dtex, uint16_t *dims, int *format, char *texture_path){
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

extern uint8_t crayon_memory_load_mono_font_sheet(crayon_font_mono_t *fm, crayon_palette_t *cp, int8_t palette_id, char *path){
	uint8_t result = 0;
	FILE *sheet_file = NULL;

	#define ERROR(n) {result = (n); goto cleanup;}

	// Load texture
	//---------------------------------------------------------------------------

	uint8_t dtex_result = crayon_memory_load_dtex(&fm->fontsheet_texture, &fm->fontsheet_dim, &fm->texture_format, path);
	if(dtex_result){ERROR(dtex_result);}

	uint8_t texture_format = (((1 << 3) - 1) & (fm->texture_format >> (28 - 1)));	//Extract bits 27 - 29, Pixel format

	//Unknown/unsupported format
	if(texture_format != 0 && texture_format != 1 && texture_format != 2 && texture_format != 5 && texture_format != 6){
		ERROR(6);
	}

	uint8_t bpp = 0;
	if(texture_format == 5){
		bpp = 4;
	}
	else if(texture_format == 6){
		bpp = 8;
	}
	int path_length = strlen(path);
	if(palette_id >= 0 && bpp){	//If we pass in -1, then we skip palettes
		char *palette_path = (char *) malloc((path_length + 5)*sizeof(char));
		if(!palette_path){ERROR(7);}
		strcpy(palette_path, path);
		palette_path[path_length] = '.';
		palette_path[path_length + 1] = 'p';
		palette_path[path_length + 2] = 'a';
		palette_path[path_length + 3] = 'l';
		palette_path[path_length + 4] = '\0';
		cp->palette = NULL;
		int resultPal = crayon_memory_load_palette(cp, bpp, palette_path);
			//The function will modify the palette and colour count. Also it sends the BPP through
		free(palette_path);
		cp->palette_id = palette_id;
		if(resultPal){ERROR(7 + resultPal);}
	}

	char *txt_path = (char *) malloc((path_length)*sizeof(char));  //Add a check here to see if it failed
	if(!txt_path){ERROR(13);}

	strncpy(txt_path, path, path_length - 4);
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
