#include "savefile.h"

//Returns true if device has certain function/s
uint8_t crayon_savefile_check_for_device(int8_t port, int8_t slot, uint32_t function){
	maple_device_t *vmu;

	//Invalid controller/port
	if(port < 0 || port > 3 || slot < 1 || slot > 2){
		return 1;
	}

	//Make sure there's a device in the port/slot
	if(!(vmu = maple_enum_dev(port, slot))){
		return 2;
	}

	//Check the device is valid and it has a certain function
	if(!vmu->valid || !(vmu->info.functions & function)){
		return 3;
	}

	return 0;
}

uint8_t crayon_savefile_get_vmu_bit(uint8_t vmu_bitmap, int8_t savefile_port, int8_t savefile_slot){
	return !!(vmu_bitmap & (1 << ((2 - savefile_slot) + 6 - (2 * savefile_port))));
}

void crayon_savefile_set_vmu_bit(uint8_t * vmu_bitmap, int8_t savefile_port, int8_t savefile_slot){
	*vmu_bitmap |= (1 << ((2 - savefile_slot) + 6 - (2 * savefile_port)));
	return;
}

void crayon_savefile_load_icon(crayon_savefile_details_t * savefile_details, char * image, char * palette){
	FILE * icon_data_file;
	int size_data;

	icon_data_file = fopen(image, "rb");

	//Get the size of the file
	fseek(icon_data_file, 0, SEEK_END);
	size_data = ftell(icon_data_file);
	fseek(icon_data_file, 0, SEEK_SET);

	savefile_details->savefile_icon = (unsigned char *) malloc(size_data);
	fread(savefile_details->savefile_icon, size_data, 1, icon_data_file);
	fclose(icon_data_file);


	//--------------------------------

	FILE * icon_palette_file;
	int size_palette;

	icon_palette_file = fopen(palette, "rb");

	fseek(icon_palette_file, 0, SEEK_END);
	size_palette = ftell(icon_palette_file);
	fseek(icon_palette_file, 0, SEEK_SET);

	savefile_details->savefile_palette = (unsigned short *) malloc(size_palette);
	fread(savefile_details->savefile_palette, size_palette, 1, icon_palette_file);
	fclose(icon_palette_file);

	return;
}

void crayon_savefile_free_icon(crayon_savefile_details_t * savefile_details){
	free(savefile_details->savefile_icon);
	free(savefile_details->savefile_palette);
	return;
}

uint8_t crayon_savefile_get_valid_screens(){
	uint8_t valid_screens = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			//Check if device has an LCD
			if(crayon_savefile_check_for_device(i, j, MAPLE_FUNC_LCD)){
				continue;
			}

			//If we got here, we have a screen
			crayon_savefile_set_vmu_bit(&valid_screens, i, j);
		}
	}

	return valid_screens;
}

//Why is valid screens a part of Savefile details, but this function isn't?
void crayon_vmu_display_icon(uint8_t vmu_bitmap, void * icon){
	maple_device_t *vmu;
	uint8_t i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(crayon_savefile_get_vmu_bit(vmu_bitmap, i, j)){	//We want to display on this VMU
				if(!(vmu = maple_enum_dev(i, j))){	//Device not present
					continue;
				}
				vmu_draw_lcd(vmu, icon);
			}
		}
	}

	return;
}
