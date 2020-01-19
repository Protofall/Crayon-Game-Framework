#include "savefile.h"

//Note this assumes the vmu chosen is valid
	//THIS CAN BE OPTIMISED. But when I fopen() a vmu file it automatically caches the entire file.
uint8_t crayon_savefile_check_for_save(crayon_savefile_details_t * savefile_details, int8_t savefile_port, int8_t savefile_slot){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;

	//Only 25 charaters allowed at max (26 if you include '\0')
	//port gets converted to a, b, c or d. unit is unit
	//Its more efficient to do it this way than with sprintf
	char savename[32] = "/vmu/";
	savename[5] = savefile_port + 'a';
	savename[6] = savefile_slot + '0';
	savename[7] = '/';
	savename[8] = '\0';
	strcat(savename, savefile_details->save_name);

	//File DNE
	if(!(fp = fopen(savename, "rb"))){
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	pkg_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pkg_out = (uint8_t *)malloc(pkg_size);
	fread(pkg_out, pkg_size, 1, fp);
	fclose(fp);

	vmu_pkg_parse(pkg_out, &pkg);

	free(pkg_out);

	//If the IDs don't match, then thats an error
	if(strcmp(pkg.app_id, savefile_details->app_id)){
		return 2;
	}

	return 0;
}

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

//Rounds the number down to nearest multiple of 512, then adds 1 if there's a remainder
uint16_t crayon_savefile_bytes_to_blocks(size_t bytes){
	return (bytes >> 9) + !!(bytes & ((1 << 9) - 1));
}

//I used the "vmu_pkg_build" function's source as a guide for this. Doesn't work with compressed saves
int16_t crayon_savefile_get_save_block_count(crayon_savefile_details_t * savefile_details){
	int pkg_size, data_len;
	data_len = savefile_details->savefile_size;
	uint16_t eyecatch_size  = 0;
	switch(savefile_details->eyecatch_type){
		case VMUPKG_EC_NONE:
			eyecatch_size = 0; break;
		case VMUPKG_EC_16BIT:
			eyecatch_size = 8064; break;
		case VMUPKG_EC_256COL:
			eyecatch_size = 512 + 4032; break;
		case VMUPKG_EC_16COL:
			eyecatch_size = 32 + 2016; break;
		default:
			return -1;
	}

	//Get the total number of bytes. Keep in mind we need to think about the icon/s and EC
	pkg_size = sizeof(vmu_hdr_t) + (512 * savefile_details->icon_anim_count) +
		eyecatch_size + data_len;

	return crayon_savefile_bytes_to_blocks(pkg_size);
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
	size_data = ftell(icon_data_file);	//This will account for multi-frame icons
	fseek(icon_data_file, 0, SEEK_SET);

	savefile_details->icon = (unsigned char *) malloc(size_data);
	fread(savefile_details->icon, size_data, 1, icon_data_file);
	fclose(icon_data_file);


	//--------------------------------

	FILE * icon_palette_file;
	int size_palette;

	icon_palette_file = fopen(palette, "rb");

	fseek(icon_palette_file, 0, SEEK_END);
	size_palette = ftell(icon_palette_file);
	fseek(icon_palette_file, 0, SEEK_SET);

	savefile_details->icon_palette = (unsigned short *) malloc(size_palette);
	fread(savefile_details->icon_palette, size_palette, 1, icon_palette_file);
	fclose(icon_palette_file);

	return;
}

void crayon_savefile_free_icon(crayon_savefile_details_t * savefile_details){
	free(savefile_details->icon);
	free(savefile_details->icon_palette);
	return;
}

uint8_t crayon_savefile_load_eyecatch(crayon_savefile_details_t * savefile_details, char * eyecatch_path){
	FILE * eyecatch_data_file = fopen(eyecatch_path, "rb");
	if(!eyecatch_data_file){
		return 1;
	}

	//Get the size of the file
	fseek(eyecatch_data_file, 0, SEEK_END);
	int size_data = ftell(eyecatch_data_file);	//Confirming its the right size
	fseek(eyecatch_data_file, 0, SEEK_SET);

	switch(size_data){
		case 8064:
			savefile_details->eyecatch_type = VMUPKG_EC_16BIT; break;
		case 4544:
			savefile_details->eyecatch_type = VMUPKG_EC_256COL; break;
		case 2048:
			savefile_details->eyecatch_type = VMUPKG_EC_16COL; break;
		default:
			return 2;
	}

	savefile_details->eyecatch_data = (uint8_t *) malloc(size_data);
	if(fread(savefile_details->eyecatch_data, size_data, 1, eyecatch_data_file) != 1){
		return 4;
	}
	fclose(eyecatch_data_file);

	return 0 ;
}

void crayon_savefile_free_eyecatch(crayon_savefile_details_t * savefile_details){
	free(savefile_details->eyecatch_data);
	savefile_details->eyecatch_type = 0;
	return;
}

void crayon_savefile_update_valid_saves(crayon_savefile_details_t * savefile_details, uint8_t modes){
	maple_device_t *vmu;
	uint8_t valid_saves = 0;	//a1a2b1b2c1c2d1d2
	uint8_t valid_memcards = 0;

	uint8_t get_saves = 0;
	uint8_t get_memcards = 0;

	if(modes & CRAY_SAVEFILE_UPDATE_MODE_SAVE_PRESENT){
		get_saves = 1;
	}
	if(modes & CRAY_SAVEFILE_UPDATE_MODE_MEMCARD_PRESENT){
		get_memcards = 1;
	}

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			//Check if device is a memory card
			if(crayon_savefile_check_for_device(i, j, MAPLE_FUNC_MEMCARD)){
				continue;
			}

			//Check if a save file DNE
			if(crayon_savefile_check_for_save(savefile_details, i, j)){	//Returns 1 if DNE
				if(!get_memcards){continue;}

				//If we have enough space for a new savefile
				vmu = maple_enum_dev(i, j);
				if(vmufs_free_blocks(vmu) >= crayon_savefile_get_save_block_count(savefile_details)){
					crayon_savefile_set_vmu_bit(&valid_memcards, i, j);
				}
			}
			else{
				if(!get_saves){continue;}
				crayon_savefile_set_vmu_bit(&valid_saves, i, j);
				crayon_savefile_set_vmu_bit(&valid_memcards, i, j);
			}
		}
	}

	if(get_saves){savefile_details->valid_saves = valid_saves;}
	if(get_memcards){savefile_details->valid_memcards = valid_memcards;}

	return;
}

uint8_t crayon_savefile_get_valid_function(uint32_t function){
	uint8_t valid_function = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			//Check if device contains this function bitmap
			if(crayon_savefile_check_for_device(i, j, function)){
				continue;
			}

			//If we got here, we have a screen
			crayon_savefile_set_vmu_bit(&valid_function, i, j);
		}
	}

	return valid_function;
}

//Make sure to call this after making a new savefile struct otherwise you can get strange results
	//Also don't try and and load the eyecatcher first otherwise the eyecatch_type will be overwritten
uint8_t crayon_savefile_init_savefile_details(crayon_savefile_details_t * savefile_details,  uint8_t * savefile_data, size_t savefile_size,
	uint8_t icon_anim_count, uint16_t icon_anim_speed, char * desc_long, char * desc_short, char * app_id, char * save_name){

	if(icon_anim_count > 3){
		return 1;
	}

	savefile_details->savefile_data = savefile_data;
	savefile_details->savefile_size = savefile_size;
	savefile_details->icon_anim_count = icon_anim_count;
	savefile_details->icon_anim_speed = icon_anim_speed;
	savefile_details->eyecatch_type = VMUPKG_EC_NONE;	//The default

	strcpy(savefile_details->desc_long, desc_long);
	strcpy(savefile_details->desc_short, desc_short);
	strcpy(savefile_details->app_id, app_id);
	strcpy(savefile_details->save_name, save_name);

	crayon_savefile_update_valid_saves(savefile_details, CRAY_SAVEFILE_UPDATE_MODE_BOTH);
	savefile_details->valid_vmu_screens = crayon_savefile_get_valid_screens();

	savefile_details->savefile_port = -1;
	savefile_details->savefile_slot = -1;

	return 0;
}

uint8_t crayon_savefile_load(crayon_savefile_details_t * savefile_details){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;

	//If you call everying in the right order, this is redundant. But just incase you didn't, here it is
	//Also we use device and not save because the rest of the load code can automatically check if a save exists so its faster this way
	//(Since this function and check_for_save share alot of the same code)
	if(crayon_savefile_check_for_device(savefile_details->savefile_port, savefile_details->savefile_slot, MAPLE_FUNC_MEMCARD)){
		return 1;
	}

	//Only 25 charaters allowed at max (26 if you include '\0')
	//port gets converted to a, b, c or d. unit is unit
	//Its more efficient to do it this way than with sprintf
	char savename[32] = "/vmu/";
	savename[5] = savefile_details->savefile_port + 'a';
	savename[6] = savefile_details->savefile_slot + '0';
	savename[7] = '/';
	savename[8] = '\0';
	strcat(savename, savefile_details->save_name);

	//If the savefile DNE, this will fail
	if(!(fp = fopen(savename, "rb"))){
		return 2;
	}

	fseek(fp, 0, SEEK_END);
	pkg_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pkg_out = (uint8_t *)malloc(pkg_size);
	fread(pkg_out, pkg_size, 1, fp);
	fclose(fp);

	vmu_pkg_parse(pkg_out, &pkg);

	//Read the pkg data into my struct
	memcpy(savefile_details->savefile_data, pkg.data, savefile_details->savefile_size);	//Last param is num of bytes and sizeof returns in bytes

	free(pkg_out);

	return 0;
}

uint8_t crayon_savefile_save(crayon_savefile_details_t * savefile_details){
	//The requested VMU is not a valid memory card
	if(!crayon_savefile_get_vmu_bit(savefile_details->valid_memcards, savefile_details->savefile_port, savefile_details->savefile_slot)){
		return 1;
	}

	//If you call everying in the right order, this is redundant
	//But just incase you didn't, here it is
	if(crayon_savefile_check_for_device(savefile_details->savefile_port, savefile_details->savefile_slot, MAPLE_FUNC_MEMCARD)){
		return 2;
	}

	vmu_pkg_t pkg;
	uint8 *pkg_out, *data;	//pkg_out is allocated in vmu_pkg_build
	int pkg_size;
	FILE *fp;
	file_t f;
	maple_device_t *vmu;
	uint16_t blocks_freed = 0;
	uint8_t rv = 0;

	vmu = maple_enum_dev(savefile_details->savefile_port, savefile_details->savefile_slot);

	//Only 25 charaters allowed at max (26 if you include '\0')
	//port gets converted to a, b, c or d. unit is unit
	//Its more efficient to do it this way than with sprintf
	char savename[32] = "/vmu/";
	savename[5] = savefile_details->savefile_port + 'a';
	savename[6] = savefile_details->savefile_slot + '0';
	savename[7] = '/';
	savename[8] = '\0';
	strcat(savename, savefile_details->save_name);

	int filesize = savefile_details->savefile_size;
	data = (uint8_t *) malloc(filesize);
	if(data == NULL){
		free(data);
		return 3;
	}
	memcpy(data, savefile_details->savefile_data, savefile_details->savefile_size);

	sprintf(pkg.desc_long, savefile_details->desc_long);
	strcpy(pkg.desc_short, savefile_details->desc_short);
	strcpy(pkg.app_id, savefile_details->app_id);
	pkg.icon_cnt = savefile_details->icon_anim_count;
	pkg.icon_anim_speed = savefile_details->icon_anim_speed;
	memcpy(pkg.icon_pal, savefile_details->icon_palette, 32);
	pkg.icon_data = savefile_details->icon;
	pkg.eyecatch_type = savefile_details->eyecatch_type;
	if(pkg.eyecatch_type){
		pkg.eyecatch_data = savefile_details->eyecatch_data;
	}
	pkg.data_len = savefile_details->savefile_size;
	pkg.data = data;

	vmu_pkg_build(&pkg, &pkg_out, &pkg_size);

	//Check if a file exists with that name, since we'll overwrite it.
	f = fs_open(savename, O_RDONLY);
	if(f != FILEHND_INVALID){
		int fs_size = fs_total(f);
		blocks_freed = crayon_savefile_bytes_to_blocks(fs_size);;
		fs_close(f);
	}

	//Make sure there's enough free space on the VMU.
	if(vmufs_free_blocks(vmu) + blocks_freed < crayon_savefile_bytes_to_blocks(pkg_size)){
		free(pkg_out);
		free(data);
		return 4;
	}

	//Can't open file for some reason
	if(!(fp = fopen(savename, "wb"))){
		free(pkg_out);
		free(data);
		return 5;
	}

	if(fwrite(pkg_out, 1, pkg_size, fp) != (size_t)pkg_size){
		rv = 6;
	}

	fclose(fp);

	free(pkg_out);
	free(data);

	return rv;
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
