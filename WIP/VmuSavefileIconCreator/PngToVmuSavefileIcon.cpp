#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// unsigned int bits_getr( int x, int p, int n){
	// return( x >> (p + 1 - n)) & ~(~0 << n );
// }

// Load an image from file as texture, then process it
int loadTexture(std::string fileName){
	int x, y, n;
    // stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(fileName.c_str(), &x, &y, &n, 0);
	uint32_t palette[16];	//4bpp palette, only 16 entries
	uint8_t entries_used = 0;

	if(n == 3){
		// printf("Got %d channels\n", n);
		printf("No alpha isn't currently unsupported\n");
		return -1;
	}
	else if(n == 4){    //For pictures with alpha
		// printf("Got %d channels\n", n);
	}
	else {
		fprintf(stderr, "Image pixels are not RGB. Texture may not have loaded correctly.\n");
		return -1;
	}

	//size = x * y * n * 8?
	// while((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0){
		// process bytesRead worth of data in buffer
	// }

	// FILE *file = fopen(fileName,"rb");  // r for read, b for binary
	FILE *write_ptr;
	write_ptr = fopen("test.bin","wb");  // w for write, b for binary
	unsigned char *traversal = data;
	uint32_t extracted;
	uint8_t buffer[4];
	for(int i = 0; i < x * y * n / 4; i++){
	// 	// fread(&extracted, 1, sizeof(uint32_t), traversal);
	// 	memcpy(&extracted, traversal, 4);

	// 	bool found = false;
	// 	if(entries_used != 0){
	// 		for(int j = 0; j < entries_used; j++){
	// 			if(extracted == palette[j]){
	// 				found = true;
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	if(found){
	// 		palette[entries_used] = extracted;
	// 	}

	// 	traversal += sizeof(uint32_t);
		// fread(&extracted, 1, sizeof(uint32_t), traversal);
		// memcpy(&extracted, traversal, 4);

		// bool found = false;
		// if(entries_used != 0){
		// 	for(int j = 0; j < entries_used; j++){
		// 		if(extracted == palette[j]){
		// 			found = true;
		// 			break;
		// 		}
		// 	}
		// }
		// if(found){
		// 	palette[entries_used] = extracted;
		// }

		// traversal += sizeof(uint32_t);

		memcpy(&extracted, traversal, n);	//Extracts all 4 channels for ARGB and just 3 for RGB
		buffer[0] = 0;
		buffer[1] = 15;
		buffer[2] = 1;
		buffer[3] = 40;
		fwrite(buffer, sizeof(uint8_t), sizeof(buffer), write_ptr);

		traversal += sizeof(uint8_t) * n;

		break;
	}

	fclose(write_ptr);

	// for(int i = 0; i < entries_used; i++){
		// printf("Entry %d is %d\n", i, palette[i]);
	// }

	// for(int i = 0; i < 10; ++i){
		// extracted = bits_get(data[i], 0, 8 * n); 
	// }



	stbi_image_free(data);

	return 0;
}

void getARGB(uint8_t * argb, uint32_t extracted){
	argb[0] = extracted >> 24;
	argb[1] = (extracted >> 16) % (1 << 8);
	argb[2] = (extracted >> 8) % (1 << 8);
	argb[3] = extracted % (1 << 8);
	return;
}

int pngToVmuLcdIcon(std::string source, std::string dest){
	int x, y, n;
	// stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(source.c_str(), &x, &y, &n, 0);

	if(n != 3 && n != 4){	//Not argb or rgb
		fprintf(stderr, "Image pixels are not A/RGB. Texture may not have loaded correctly.\n");
		return 1;
	}

	FILE *write_ptr;
	write_ptr = fopen(dest.c_str(),"wb");  // w for write, b for binary
	unsigned char *traversal = data;
	uint32_t extracted;
	uint8_t buffer[1];
	uint8_t argb[4];
	uint8_t * colour = argb;	//For some reason I can't pass a reference to argb into getARGB() :roll_eyes:
	for(int i = 0; i < x * y * n / 4 / 8; i++){
		buffer[0] = 0;

		//Need to pack 8 pixels into one byte
		for(int j = 0; j < 8; j++){
			memcpy(&extracted, traversal, n);	//Extracts all 4 channels for ARGB and just 3 for RGB

			getARGB(colour, extracted);
			// printf("a %d, r %d, g %d, b %d\n", argb[0], argb[1], argb[2], argb[3]);

			if(argb[0] + argb[1] + argb[2] + argb[3] <= 128 * 4){
				buffer[0] += (0 << j);	//VMU icons use 0 for no pixel and 1 for pixel
			}
			else{
				buffer[0] += (1 << j);
			}
			traversal += sizeof(uint8_t) * n;
		}

		fwrite(buffer, sizeof(uint8_t), sizeof(buffer), write_ptr);

	}

	fclose(write_ptr);
	stbi_image_free(data);

	return 0;
}

int pngToVmuSavefileIcon(std::string fileName, std::string dest){
	printf("Unimplemented as of now\n");
	return 0;
}

int main(int argC, char *argV[]){

	if(argC != 4){
		printf("Wrong number of params, please pass in file names for\nA png source, a binary output and a 0 or 1 for savefile or LCD icon\n");
		return 1;
	}

	// loadTexture(argV[1]);
	if(atoi(argV[3]) == 0){
		pngToVmuSavefileIcon(argV[1], argV[2]);
	}
	else if(atoi(argV[3]) == 1){
		pngToVmuLcdIcon(argV[1], argV[2]);
	}
	else{
		printf("Invalid mode option, please choose 0 for savefile or 1 for VMU LCD\n");
		return 1;
	}
	
	return 0;
}
