#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

#include "png_assist.h"

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

//This function takes an element from the texture array and converts it from whatever bpc combo you set to RGBA8888
void dtex_argb_to_rgba8888(dtex_header_t * dtex_header, uint32_t * bugger,
	uint8_t bpc_a, uint8_t bpc_r, uint8_t bpc_g, uint8_t bpc_b){
	uint16_t extracted;

	for(int i = 0; i < dtex_header->width * dtex_header->height; i++){
		extracted = bugger[i];

		// (extracted value) * (Max for 8bpc) / (Max for that bpc) = value
		if(bpc_a != 0){
			bugger[i] = bit_extracted(extracted, bpc_a, bpc_r + bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_a) - 1);	//Alpha
		}
		else{	//We do this to avoid a "divide by zero" error and we don't care about colour data for transparent pixels
			bugger[i] = (1 << 8) - 1;
		}

		bugger[i] += (bit_extracted(extracted, bpc_r, bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_r) - 1)) << 24;		//R
		bugger[i] += (bit_extracted(extracted, bpc_g, bpc_b) * ((1 << 8) - 1) / ((1 << bpc_g) - 1)) << 16;				//G
		bugger[i] += (bit_extracted(extracted, bpc_b, 0) * ((1 << 8) - 1) / ((1 << bpc_b) - 1)) << 8;					//B
	}
	return;
}

uint32_t argb8888_to_rgba8888(uint32_t argb8888){
	return (argb8888 << 8) + bit_extracted(argb8888, 8, 24);
}

//Given an array of indexes and a dtex.pal file, it will replace all indexes with the correct colour
uint8_t apply_palette(dtex_header_t * dtex_header, uint32_t * bugger, char * texture_path){
	char * palette_path = (char *) malloc(sizeof(char) * (strlen(texture_path) + 5));
	if(palette_path == NULL){return 1;}
	strcpy(palette_path, texture_path);
	palette_path[strlen(texture_path)] = '.';
	palette_path[strlen(texture_path) + 1] = 'p';
	palette_path[strlen(texture_path) + 2] = 'a';
	palette_path[strlen(texture_path) + 3] = 'l';
	palette_path[strlen(texture_path) + 4] = '\0';

	dpal_header_t dpal_header;
	
	FILE * palette_file = fopen(palette_path, "rb");
	free(palette_path);
	if(!palette_file){return 2;}

	if(fread(&dpal_header, sizeof(dpal_header_t), 1, palette_file) != 1){fclose(palette_file); return 3;}

	if(memcmp(dpal_header.magic, "DPAL", 4)){fclose(palette_file); return 4;}

	uint32_t * palette_buffer = (uint32_t *) malloc(sizeof(uint32_t) * dpal_header.color_count);
	if(palette_buffer == NULL){fclose(palette_file); return 5;}

	if(fread(palette_buffer, sizeof(uint32_t) * dpal_header.color_count, 1, palette_file) != 1){fclose(palette_file); return 6;}
	fclose(palette_file);

	for(int i = 0; i < dtex_header->width * dtex_header->height; i++){
		if(bugger[i] < dpal_header.color_count){
			bugger[i] = argb8888_to_rgba8888(palette_buffer[bugger[i]]);
		}
		else{	//Invalid palette index
			return 7;
		}
	}

	return 0;
}

//Based on the cow example, I think the Green channel is (mostly) fine and the R/B channels are defective
uint32_t yuv444_to_rgba8888(uint8_t y, uint8_t u, uint8_t v){
	//Note RGB must be clamped within 0 to 255, the uint8_t does this automatically
	uint8_t R = y + (11/8) * (v - 128);
	uint8_t G = y - 0.25 * (11/8) * (u - 128) - (0.5 * (11/8) * (v - 128));
	uint8_t B = y + 1.25 * (11/8) * (u - 128);
	return (R << 24) + (G << 16) + (B << 8) + 255;	//RGBA with full alpha
}

/*
From wiki
C = y - 16
D = u - 128
E = v - 128
R = ((298 * C + 409 * E + 128) >> 8) % 256
G = ((298 * C - 100 * D - 208 * E + 128) >> 8) % 256
B = ((298 * C + 516 * D + 128) >> 8) % 256

R = ((298 * (y - 16) + 409 * (v - 128) + 128) >> 8) % 256
G = ((298 * (y - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8) % 256
B = ((298 * (y - 16) + 516 * (u - 128) + 128) >> 8) % 256
*/

void yuv422_to_rgba8888(dtex_header_t * dtex_header, uint32_t * bugger){
	//We handle 2 pixels at a time (4 bytes = 2 pixels, u y1, v, y2)
	uint16_t byte_section[2];	//Contains 2 vars
	for(int i = 0; i < dtex_header->width * dtex_header->height; i += 2){
		//Might need to swap order in pairs (u/y1 and v/y2)
		uint8_t u = bit_extracted(bugger[i], 8, 0);
		uint8_t y1 = bit_extracted(bugger[i], 8, 8);
		uint8_t v = bit_extracted(bugger[i + 1], 8, 0);
		uint8_t y2 = bit_extracted(bugger[i + 1], 8, 8);

		bugger[i] = yuv444_to_rgba8888(y1, u, v);
		bugger[i + 1] = yuv444_to_rgba8888(y2, u, v);
	}
	return;
}

//This is the function that JamoHTP did. It boggled my mind too much
uint32_t get_twiddled_index(uint16_t w, uint16_t h, uint32_t p){
	uint32_t ddx = 1, ddy = w;
	uint32_t q = 0;

	for(int i = 0; i < 16; i++){
		if(h >>= 1){
			if(p & 1){q |= ddy;}
			p >>= 1;
		}
		ddy <<= 1;
		if(w >>= 1){
			if(p & 1){q |= ddx;}
			p >>= 1;
		}
		ddx <<= 1;
	}

	return q;
}

uint8_t load_dtex(char * texture_path, uint32_t ** bugger, uint16_t * height, uint16_t * width){
	dtex_header_t dtex_header;

	FILE * texture_file = fopen(texture_path, "rb");
	if(!texture_file){return 1;}

	if(fread(&dtex_header, sizeof(dtex_header_t), 1, texture_file) != 1){fclose(texture_file); return 2;}

	if(memcmp(dtex_header.magic, "DTEX", 4)){fclose(texture_file); return 3;}

	uint8_t stride_setting = bit_extracted(dtex_header.type, 5, 0);
	bool stride = dtex_header.type & (1 << 25);
	bool twiddled = !(dtex_header.type & (1 << 26));	//Note this flag is TRUE if twiddled
	uint8_t pixel_format = bit_extracted(dtex_header.type, 3, 27);
	bool compressed = dtex_header.type & (1 << 30);
	bool mipmapped = dtex_header.type & (1 << 31);

	// printf("Format details: str_set %d, str %d, twid %d, pix %d, comp %d, mip %d, whole %d\n",
		// stride_setting, stride, twiddled, pixel_format, compressed, mipmapped, dtex_header.type);

	//'type' contains the various flags and the pixel format packed together:
	// bits 0-4 : Stride setting.
	// 	The width of stride textures is NOT stored in 'width'. To get the actual
	// 	width, multiply the stride setting by 32. The next power of two size up
	// 	from the stride width will be stored in 'width'.
	// bit 25 : Stride flag
	// 	0 = Non-strided
	// 	1 = Strided
	// bit 26 : Untwiddled flag
	// 	0 = Twiddled
	// 	1 = Untwiddled
	// bits 27-29 : Pixel format
	// 	0 = ARGB1555
	// 	1 = RGB565
	// 	2 = ARGB4444
	// 	3 = YUV422
	// 	4 = BUMPMAP
	// 	5 = PAL4BPP
	// 	6 = PAL8BPP
	// bit 30 : Compressed flag
	// 	0 = Uncompressed
	// 	1 = Compressed
	// bit 31 : Mipmapped flag
	// 	0 = No mipmaps
	// 	1 = Mipmapped

	if(mipmapped || compressed || stride || pixel_format == 4){
		printf("Unsupported dtex parameter detected. Stopping conversion\n");
		fclose(texture_file);
		return 4;
	}

	//Allocate some space
	*bugger = (uint32_t *) malloc(sizeof(uint32_t) * dtex_header.height * dtex_header.width);
	if(*bugger == NULL){
		printf("Malloc failed");
		fclose(texture_file);
		return 5;
	}

	uint8_t bpc[4];
	uint8_t bpp;
	bool rgb = false; bool paletted = false; bool yuv = false; bool bumpmap = false;
	switch(pixel_format){
		case 0:
			bpc[0] = 1; bpc[1] = 5; bpc[2] = 5; bpc[3] = 5; bpp = 16; rgb = true; break;
		case 1:
			bpc[0] = 0; bpc[1] = 5; bpc[2] = 6; bpc[3] = 5; bpp = 16; rgb = true; break;
		case 2:
			bpc[0] = 4; bpc[1] = 4; bpc[2] = 4; bpc[3] = 4; bpp = 16; rgb = true; break;
		case 3:
			bpp = 16; yuv = true; break;
		case 4:	//BUMPMAP
			printf("Pixel format %d isn't supported\n", pixel_format);
			fclose(texture_file);
			return 6;
		case 5:
			bpp = 4; paletted = true; break;
		case 6:
			bpp = 8; paletted = true; break;
		default:
			printf("Pixel format %d doesn't exist\n", pixel_format);
			fclose(texture_file);
			return 6;
	}

	size_t read_size;
	if(bpp == 16){
		read_size = sizeof(uint16_t);
	}
	else{
		read_size = sizeof(uint8_t);
	}

	//Read the whole file into memory
	uint8_t error = 0;
	uint16_t extracted;
	for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
		if(fread(&extracted, read_size, 1, texture_file) != 1){error = 1; break;}
		if(bpp != 4){
			// for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
				// if(fread(&extracted, read_size, 1, texture_file) != 1){error = 1; break;}
				(*bugger)[i] = extracted;
			// }
		}
		else{	//PAL4BPP
			// for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
				uint16_t byte_section[2];	//Extracted must be split into two vars containing the top and bottom 4 bits
				// if(fread(&extracted, read_size, 1, texture_file) != 1){error = 1; break;}
				byte_section[0] = bit_extracted(extracted, 4, 4);
				byte_section[1] = bit_extracted(extracted, 4, 0);
				for(int j = 0; j < 2; j++){
					(*bugger)[i + (1 - j)] = byte_section[j];
					i += j;
				}
				// for(int j = 1; j > 0; j--){
				// 	(*bugger)[i + j] = byte_section[j];
				// 	i += (!j);
				// }

			// }
		}
	}

	fclose(texture_file);
	if(error){return 7;}

	//Convert from whatever format to RGBA8888
	if(rgb){
		dtex_argb_to_rgba8888(&dtex_header, *bugger, bpc[0], bpc[1], bpc[2], bpc[3]);
	}
	else if(paletted){
		error = apply_palette(&dtex_header, *bugger, texture_path);
		if(error){return 7 + error;}	//Error goes up to 7
	}
	else if(yuv){
		yuv422_to_rgba8888(&dtex_header, *bugger);
	}
	else if(bumpmap){
		;
	}
	else{
		printf("Somehow an unsupported texture format got through\n");
		return 15;
	}

	//Untwiddle
	if(twiddled){
		uint32_t * twiddled_index = (uint32_t *) malloc(sizeof(uint32_t) * dtex_header.height * dtex_header.width);
		if(twiddled_index == NULL){
			printf("Malloc failed");
			return 16;
		}
		uint32_t array_index;
		for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
			array_index = get_twiddled_index(dtex_header.width, dtex_header.height, i);	//Given i, it says which element should be there to twiddle it
																						//We then set that index pixel to the value at index i
			twiddled_index[array_index] = (*bugger)[i];
		}
		free(*bugger);
		*bugger = twiddled_index;
		twiddled_index = NULL;
	}

	//Set the dimensions
	*width = dtex_header.width;
	*height = dtex_header.height;

	return 0;
}

//Add argb8888/rgba8888 toggle.
	//rgba8888 is the default
int main(int argC, char *argV[]){
	if(argC != 3 && argC != 4){
		invalidInput:

		printf("\nWrong number of arguments provided. This is the format\n");
		printf("./DtexToRGBA8888 [dtex_filename] [rgba8888_binary_filename] *[--preview]\n");
		printf("Preview is an optional tag that will make a png aswell\n");

		printf("Please not the following stuff isn't supported:\n");
		printf("\t-Stride\n");
		printf("\t-YUV422 and BUMPMAP pixel formats\n");
		printf("\t-Compressed\n");
		printf("\t-Mipmapped\n");
		printf("\t-Non-square textures...dunno how they twiddle\n");

		return 1;
	}

	bool flag_png_preview;
	for(int i = 1; i < argC; i++){
		!strcmp(argV[i], "--preview") ? flag_png_preview = true : flag_png_preview = false;
		if(argC == 3){
			goto invalidInput;
			return 0;
		}
	}

	uint32_t * texture = NULL;
	uint16_t height = 0;
	uint16_t width = 0;

	uint8_t error_code = load_dtex(argV[1], &texture, &height, &width);	//Segfaults
	if(error_code){
		printf("Error %d has occurred. Terminating program\n", error_code);
		free(texture);
		return 1;
	}

	FILE * f_binary = fopen(argV[2], "wb");
	if(f_binary == NULL){
		printf("Error opening file!\n");
		return 1;
	}
	fwrite(texture, sizeof(uint32_t), height * width, f_binary);	//Note this is stored in little endian
	fclose(f_binary);

	//Output a PNG if requested
	if(flag_png_preview){
		png_details_t p_det;
		if(rgba8888_to_png_details(texture, height, width, &p_det)){return 1;}

		char * name = (char *) malloc(strlen(argV[1]) * sizeof(char));
		strcpy(name, argV[1]);
		name[strlen(argV[1]) - 4] = 'p';
		name[strlen(argV[1]) - 3] = 'n';
		name[strlen(argV[1]) - 2] = 'g';
		name[strlen(argV[1]) - 1] = '\0';
		write_png_file(name, &p_det);

		free(name);
	}
	free(texture);

	return 0;
}