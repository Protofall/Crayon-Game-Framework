#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

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

//These three functions grab a file pointer to a texture of that format and convert and place it in the bugger
uint8_t dtex_to_rgba8888(FILE * texture_file, dtex_header_t * dtex_header, uint32_t * bugger,
	uint8_t bpc_a, uint8_t bpc_r, uint8_t bpc_g, uint8_t bpc_b){

	uint16_t argb1555;

	// bool first = true;
	for(int i = 0; i < dtex_header->width * dtex_header->height; i++){
		if(fread(&argb1555, sizeof(uint16_t), 1, texture_file) != 1){return 1;}
		// if(first){
			// printf("\nargb1555 %x ", argb1555);

			// argb[0] = bit_extracted(argb1555, 1, 15);
			// argb[1] = (((1 << 5) - 1) & (argb1555 >> 10));
			// argb[2] = bit_extracted(argb1555, 5, 5);
			// argb[3] = bit_extracted(argb1555, 5, 0);
		// }

		// bugger[i] = (((argb1555 & (1 << 15)) * 255) << 24);	//Alpha

		//But what happens when a bpc is zero? We divide by zero...
		bugger[i] = (bit_extracted(argb1555, bpc_r, bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_r) - 1)) << 24;					//R
		bugger[i] += (bit_extracted(argb1555, bpc_g, bpc_b) * ((1 << 8) - 1) / ((1 << bpc_g) - 1)) << 16;							//G
		bugger[i] += (bit_extracted(argb1555, bpc_b, 0) * ((1 << 8) - 1) / ((1 << bpc_b) - 1)) << 8;								//B
		if(bpc_a != 0){
			bugger[i] += bit_extracted(argb1555, bpc_a, bpc_r + bpc_g + bpc_b) * ((1 << 8) - 1) / ((1 << bpc_a) - 1);				//Alpha
		}
		else{	//We do this to avoid a "divide by zero" error
			bugger[i] += (1 << 8) - 1;
		}

		// if(first){
			// printf("%x, %x, %x\n", ((1 << 5) - 1), (argb1555 >> 10), ((1 << 5) - 1) & (argb1555 >> 10));	//1F, 3F == 1 1111, 11 1111, 1F
			// printf("argb8888 %x\n", bugger[i]);
			// printf("a %x r %x g %x b %x\n", argb[0], argb[1], argb[2], argb[3]);	//These seem to be all correct
			// printf("New: a %x r %x g %x b %x\n\n", argb[0] * ((1 << 8) - 1) / ((1 << 1) - 1),
												// argb[1] * ((1 << 8) - 1) / ((1 << 4) - 1),
												// argb[2] * ((1 << 8) - 1) / ((1 << 4) - 1),
												// argb[3] * ((1 << 8) - 1) / ((1 << 4) - 1));
			// first = false;

			//20F == 0010 0000 1111...It should be 11111...

			//Original is fc00 == 1 111 11 00000 00000
			//New should be ???
			//First pixel comes out as 010f0000 == 0001 0000 1111 0000 0000 0000.
			//Extracted is correct. New is a ff r 20f g 0 b 0
		// }

		// (extracted value) * (Max for 8bpc) / (Max for that bpc)
	}
	return 0;
}

uint8_t rgb565_to_argb8888(FILE * texture_file, dtex_header_t * dtex_header, uint32_t * bugger){
	return 0;
}

uint8_t argb4444_to_argb8888(FILE * texture_file, dtex_header_t * dtex_header, uint32_t * bugger){
	return 0;
}

uint8_t load_dtex(char * texture_path, uint32_t ** bugger, uint16_t * height, uint16_t * width){
	dtex_header_t dtex_header;
	dpal_header_t dpal_header;

	FILE *texture_file = fopen(texture_path, "rb");
	if(!texture_file){return 1;}

	if(fread(&dtex_header, sizeof(dtex_header_t), 1, texture_file) != 1){fclose(texture_file); return 2;}

	if(memcmp(dtex_header.magic, "DTEX", 4)){fclose(texture_file); return 3;}

	uint8_t stride_setting = bit_extracted(dtex_header.type, 5, 0);
	bool stride = dtex_header.type & (1 << 25);
	bool twiddled = !(dtex_header.type & (1 << 26));	//Note this flag is TRUE if twiddled
	uint8_t pixel_format = bit_extracted(dtex_header.type, 3, 27);
	bool compressed = dtex_header.type & (1 << 30);
	bool mipmapped = dtex_header.type & (1 << 31);

	printf("Format details: str_set %d, str %d, twid %d, pix %d, comp %d, mip %d, whole %d\n",
		stride_setting, stride, twiddled, pixel_format, compressed, mipmapped, dtex_header.type);

	if(mipmapped || compressed || stride || pixel_format == 3 || pixel_format == 4){
		printf("Unsupported dtex format detected. Stopping conversion\n");
		fclose(texture_file);
		return 4;
	}

	//Allocate some space
	*bugger = (uint32_t *) malloc(sizeof(uint32_t) * dtex_header.height * dtex_header.width);

	// //Load the palette info
	// if(pixel_format >= 5){
	// 	FILE *palette_file = fopen(texture_path, "rb");
	// 	if(!palette_file){fclose(texture_file); return 4;}

	// 	if(fread(&dpal_header, sizeof(dpal_header_t), 1, palette_file) != 1){fclose(texture_file); fclose(palette_file); return 5;}
	// }

	uint16_t error = 0;
	switch(pixel_format){
		case 0:
		 	error = dtex_to_rgba8888(texture_file, &dtex_header, *bugger, 1, 5, 5, 5);
			break;
		case 1:
		 	error = dtex_to_rgba8888(texture_file, &dtex_header, *bugger, 0, 5, 6, 5);
			break;
		case 2:
		 	error = dtex_to_rgba8888(texture_file, &dtex_header, *bugger, 4, 4, 4, 4);
			break;
		case 5:
		case 6:
			//Palettes, do nothing
			break;
		default: //It should never get here
			fclose(texture_file);
			return 5;
	}
	fclose(texture_file);
	if(error){
		return 6;
	}

	//Arrange the pixels in the correct order using something similar to a Z-order curve,
		//except its "top left, bottom left, top right, bottom right"
	if(twiddled){
		;
	}

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
		printf("\t-Untwiddled\n");
		printf("\t-Stride\n");
		printf("\t-YUV422 and BUMPMAP pixel formats\n");
		printf("\t-Compressed\n");
		printf("\t-Mipmapped\n");

		return 1;
	}

	bool flag_png_preview;
	for(int i = 1; i < argC; i++){
		!strcmp(argV[i], "--preview") ? flag_png_preview = true : flag_png_preview = false;
		if(argC == 3){
			goto invalidInput;
		}
	}

	uint32_t * texture = NULL;
	uint16_t height;
	uint16_t width;
	if(0){
		//Test uint32_t to png example (NOTE THIS IS IN THE ARGB8888 FORMAT)
		height = 4;
		width = 4;
		texture = (uint32_t*) malloc(sizeof(uint32_t) * height * width);
		texture[0] = (255 << 24) + (255 << 16) + (255 << 8) + 255;		//White
		texture[1] = (255 << 24) + (0 << 16) + (0 << 8) + 0;			//Black
		texture[2] = (255 << 24) + (255 << 16) + (0 << 8) + 0;			//Red
		texture[3] = (255 << 24) + (0 << 16) + (255 << 8) + 0;			//Green
		texture[4] = (255 << 24) + (0 << 16) + (0 << 8) + 255;			//Blue
		texture[5] = (255 << 24) + (128 << 16) + (128 << 8) + 128;		//grey
		texture[6] = (255 << 24) + (255 << 16) + (128 << 8) + 128;		//salmon pink
		texture[7] = (255 << 24) + (128 << 16) + (255 << 8) + 128;		//pale green
		texture[8] = (255 << 24) + (128 << 16) + (128 << 8) + 255;		//lavendar???
		texture[9] = (255 << 24) + (128 << 16) + (128 << 8) + 0;		//Dark yellow
		texture[10] = (255 << 24) + (128 << 16) + (0 << 8) + 128;		//Dark purple
		texture[11] = (255 << 24) + (0 << 16) + (128 << 8) + 128;		//Dark torquoise
		texture[12] = (255 << 24) + (128 << 16) + (0 << 8) + 0;			//Dark red
		texture[13] = (255 << 24) + (0 << 16) + (128 << 8) + 0;			//Dark blue
		texture[14] = (255 << 24) + (0 << 16) + (0 << 8) + 128;			//Dark green
		texture[15] = (0 << 24) + (0 << 16) + (0 << 8) + 0;				//Transparent
	}
	else{
		uint8_t error_code = load_dtex(argV[1], &texture, &height, &width);
		if(error_code){
			printf("Error %d has occurred. Terminating program\n", error_code);
			free(texture);
			return 1;
		}
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
		printf("Names %s, %s\n", name, argV[1]);
		write_png_file(name, &p_det);
		printf("Processed\n");

		free(name);
	}
	free(texture);

	return 0;
}