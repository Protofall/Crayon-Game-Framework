#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <math.h>

#include "png_assist.h"

//32 + 16 + 16 + 32 + 32 = 32 + 32 + 64 = 104 bits = 16 bytes
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
uint8_t dtex_argb_to_rgba8888(dtex_header_t * dtex_header, uint32_t * bugger,
	uint8_t bpc_a, uint8_t bpc_r, uint8_t bpc_g, uint8_t bpc_b){
	uint16_t extracted;

	for(int i = 0; i < dtex_header->width * dtex_header->height; i++){
		extracted = bugger[i];

		if(bpc_a != 0){
			bugger[i] = bit_extracted(extracted, bpc_a, bpc_r + bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_a) - 1);	//Alpha
		}
		else{	//We do this to avoid a "divide by zero" error and we don't care about colour data for transparent pixels
			bugger[i] = (1 << 8) - 1;
		}

		bugger[i] += (bit_extracted(extracted, bpc_r, bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_r) - 1)) << 24;		//R
		bugger[i] += (bit_extracted(extracted, bpc_g, bpc_b) * ((1 << 8) - 1) / ((1 << bpc_g) - 1)) << 16;				//G
		bugger[i] += (bit_extracted(extracted, bpc_b, 0) * ((1 << 8) - 1) / ((1 << bpc_b) - 1)) << 8;					//B

		// (extracted value) * (Max for 8bpc) / (Max for that bpc) = value
	}
	return 0;
}

//0 should be 0
//1 should be 8 (if width == 8)
//2 == 1
//3 == 9

//For 1, y = (y << 1) | 1, then y = (y << 1) (because t >>= 1)
//X is the same
uint32_t get_twiddled_index(uint16_t M, uint16_t N, uint32_t t){
	uint x = 0, y = 0;
	uint i = N, j = M;
	while(i || j){
		if(j >>= 1){
			y = (y << 1) | (t & 1);
			t >>= 1;	//t is the dtex coordinate of the pixel
		}
		if(i >>= 1){
			x = (x << 1) | (t & 1);
			t >>= 1;
		}
	}
	return y * N + x;
}

// uint32_t get_twiddled_index(uint16_t w, uint16_t h, uint32_t p){
// 	uint32_t ddx = 1, ddy = w;
// 	uint32_t q = 0;

// 	for(int i = 0; i < 16; i++){
// 		if((h >>= 1) && (p & 1)){
// 			q |= ddy;
// 			p >>= 1;
// 		}
// 		ddy <<= 1;
// 		if((w >>= 1) && (p & 1)){
// 			q |= ddx;
// 			p >>= 1;
// 		}
// 		ddx <<= 1;
// 	}

// 	return q;
// }

uint32_t argb8888_to_rgba8888(uint32_t argb8888){
	return (argb8888 << 8) + bit_extracted(argb8888, 8, 24);
}

uint8_t apply_palette(char * palette_path, uint32_t * bugger, uint32_t texture_size){
	dpal_header_t dpal_header;
	
	FILE * palette_file = fopen(palette_path, "rb");
	if(!palette_file){return 1;}

	if(fread(&dpal_header, sizeof(dpal_header_t), 1, palette_file) != 1){fclose(palette_file); return 2;}

	if(memcmp(dpal_header.magic, "DPAL", 4)){fclose(palette_file); return 3;}

	uint32_t * palette_buffer = (uint32_t *) malloc(sizeof(uint32_t) * dpal_header.color_count);
	if(palette_buffer == NULL){fclose(palette_file); return 4;}

	if(fread(palette_buffer, sizeof(uint32_t) * dpal_header.color_count, 1, palette_file) != 1){fclose(palette_file); return 5;}
	fclose(palette_file);

	for(int i = 0; i < texture_size; i++){
		if(bugger[i] < dpal_header.color_count){
			bugger[i] = argb8888_to_rgba8888(palette_buffer[bugger[i]]);
			// bugger[i] = palette_buffer[bugger[i]];
		}
		else{	//Invalid index
			return 6;
		}
	}

	return 0;
}

//Segfaults when accessing any bugger address other than 0...
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

	if(mipmapped || compressed || stride || pixel_format == 3 || pixel_format == 4){
		printf("Unsupported dtex format detected. Stopping conversion\n");
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
	bool paletted = false;
	switch(pixel_format){
		case 0:
			bpc[0] = 1; bpc[1] = 5; bpc[2] = 5; bpc[3] = 5; bpp = 16;
			break;
		case 1:
			bpc[0] = 0; bpc[1] = 5; bpc[2] = 6; bpc[3] = 5; bpp = 16;

			break;
		case 2:
			bpc[0] = 4; bpc[1] = 4; bpc[2] = 4; bpc[3] = 4; bpp = 16;

			break;
		case 5:
			bpp = 4;
			paletted = true;
			break;
		case 6:
			bpp = 8;
			paletted = true;
			break;
		default: //Unsupported formats
			fclose(texture_file);
			return 6;
	}

	bool twiddle_enabled = true;
	// bool twiddle_enabled = false;

	size_t read_size;
	if(bpp == 16){
		read_size = sizeof(uint16_t);
	}
	else{
		read_size = sizeof(uint8_t);
	}

	uint32_t array_index;
	uint16_t extracted;
	uint16_t error = 0;
	for(int i = 0; i < dtex_header.width * dtex_header.height; i++){
		if(fread(&extracted, read_size, 1, texture_file) != 1){error = 1; break;}

		//Everything except PAL4BPP
		if(bpp != 4){
			if(twiddled && twiddle_enabled){
				array_index = get_twiddled_index(dtex_header.width, dtex_header.height, i);
			}
			else{
				array_index = i;			
			}
			printf("array_index, %d\n", array_index);
			(*bugger)[array_index] = extracted;
		}
		else{	//Due to two pixels per byte, I need slightly different code here
			uint16_t byte_section[2];	//Extracted must be split into two vars containing the top and bottom 4 bits
			byte_section[0] = bit_extracted(extracted, 4, 4);
			byte_section[1] = bit_extracted(extracted, 4, 0);
			for(int j = 0; j < 2; j++){
				if(twiddled && twiddle_enabled){
					array_index = get_twiddled_index(dtex_header.width, dtex_header.height, i + j);
				}
				else{
					array_index = i + j;			
				}
				printf("PAL4BPP: array_index, %d\n", array_index);
				(*bugger)[array_index] = byte_section[j];
				i += j;
			}
		}
	}
	fclose(texture_file);
	if(error){return 7;}

	if(!paletted){
		error = dtex_argb_to_rgba8888(&dtex_header, *bugger, bpc[0], bpc[1], bpc[2], bpc[3]);
	}
	else{
		char * palette_path = (char *) malloc(sizeof(char) * (strlen(texture_path) + 5));
		strcpy(palette_path, texture_path);
		palette_path[strlen(texture_path)] = '.';
		palette_path[strlen(texture_path) + 1] = 'p';
		palette_path[strlen(texture_path) + 2] = 'a';
		palette_path[strlen(texture_path) + 3] = 'l';
		palette_path[strlen(texture_path) + 4] = '\0';
		// printf("Palette name: %s\n", palette_path);
		error = apply_palette(palette_path, *bugger, dtex_header.width * dtex_header.height);
		free(palette_path);
		
		// printf("Palettes currently unsupported\n");
		// return 8;
	}

	//Arrange the pixels in the correct order using something similar to a Z-order curve,
		//except its "top left, bottom left, top right, bottom right"
	// if(twiddled){
	// 	//Confirm its a texture we can work on (Dimensions are a power of two)
	// 	if(!(dtex_header.width == 0) && !(dtex_header.width & (dtex_header.width - 1))
	// 		&& !(dtex_header.height == 0) && !(dtex_header.height & (dtex_header.height - 1))){

	// 		//Since I dunno how to handle this for now
	// 		if(dtex_header.width != dtex_header.height){
	// 			return 7;
	// 		}

	// 		//These are x where the dims = 2^x.
	// 		uint8_t width_power = log(dtex_header.width) / log(2);
	// 		uint8_t height_power = log(dtex_header.height) / log(2);
	// 		// uint8_t current_power = 0;
	// 		// uint8_t current_max_power = 1;

	// 		// for(int i = 0; i < dtex_header.width && dtex_header.height; i++){
	// 			// bugger_the_2nd[get_2D_array_id(0, 0, dtex_header.height)] = *bugger[i];
	// 		// }

	// 		// printf("Powers: %d %d\n", width_power, height_power);

	// 		//This will become the untwiddled texture and we'll free the old data after and point to this
	// 		uint32_t * bugger_the_2nd = (uint32_t *) malloc(sizeof(uint32_t) * dtex_header.height * dtex_header.width);
	// 		// uint32_t get_2D_array_id(x, y, dtex_header.height);
	// 		free(bugger_the_2nd);
	// 	}
	// 	else{	//Dimensions aren't power of twos and its not square
	// 		return 7;
	// 	}
	// 	;
	// }

	*width = dtex_header.width;
	*height = dtex_header.height;

	return 0;
}

//Add argb8888/rgba8888 toggle.
	//rgba8888 is the default
int main(int argC, char *argV[]){
	if(argC != 3 && argC != 4){
		// invalidInput:

		printf("\nWrong number of arguments provided. This is the format\n");
		printf("./DtexToRGBA8888 [dtex_filename] [rgba8888_binary_filename] *[--preview]\n");
		printf("Preview is an optional tag that will make a png aswell\n");

		printf("Please not the following stuff isn't supported:\n");
		printf("\t-Untwiddled\n");
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
			// goto invalidInput;
			return 0;
		}
	}

	uint32_t * texture = NULL;
	uint16_t height = 0;
	uint16_t width = 0;

	uint8_t error_code = load_dtex(argV[1], &texture, &height, &width);	//Segfaults
	// uint8_t error_code = load_dtex2(&texture);	//Segfaults
	// printf("Success\n");
	// return 1;
	// uint8_t error_code = test2(&texture);								//Doesn't
	// uint8_t error_code = tester(&texture);								//Doesn't
	// return 1;
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