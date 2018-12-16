#include "savefile.h"

//Note this assumes the vmu chosen is valid
uint8_t crayon_savefile_check_for_save(crayon_savefile_details_t * savefile_details, int8_t port, int8_t slot){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;
	char savename[32];

	sprintf(savename, "/vmu/%c%d/", port + 97, slot);
	strcat(savename, "MINESWEEPER.s");

	//File DNE
	if(!(fp = fopen(savename, "rb"))){
		return 0;
	}

	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	pkg_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pkg_out = (uint8 *)malloc(pkg_size);
	fread(pkg_out, pkg_size, 1, fp);
	fclose(fp);

	vmu_pkg_parse(pkg_out, &pkg);

	free(pkg_out);

	//This uses the old broken off id and the new id
	if(strcmp(pkg.app_id, savefile_details->app_id)){
		return 0;
	}
	return 1;
}

uint8_t crayon_savefile_check_for_device(int8_t port, int8_t slot, uint32_t function){
	maple_device_t *vmu;

	//Invalid controller/port
	if(port < 0 || port > 3 || slot < 1 || slot > 2){
		return 0;
	}

	//Make sure there's a device in the port/slot
	if(!(vmu = maple_enum_dev(port, slot))){
		return 0;
	}

	//Check the device is valid and it has a certain function
	if(!vmu->valid || !(vmu->info.functions & function)){
		return 0;
	}

	return 1;
}

//Note Crayon doesn't bother supporting eye catchers. Also hard coded for 1 icon
	//I used the "vmu_pkg_build" function's source as a guide for this
uint16_t crayon_savefile_get_save_block_count(crayon_savefile_details_t * savefile_details){
	int pkg_size, data_len;
	data_len = savefile_details->savefile_size;

	//Get the total number of bytes. Keep in mind we need to think about the icon/s
	pkg_size = sizeof(vmu_hdr_t) + (512 * savefile_details->anim_count) + data_len;

	return pkg_size >> 9;	//2^9 is 512 so this gets us the number of blocks
}

uint8_t crayon_savefile_get_vmu_bitmap(uint8_t vmu_bitmap, int8_t port, int8_t slot){
	return !!(vmu_bitmap & (1 << ((2 - slot) + 6 - (2 * port))));
}

void crayon_savefile_set_vmu_bitmap(uint8_t * vmu_bitmap, int8_t port, int8_t slot){
	*vmu_bitmap |= (1 << ((2 - slot) + 6 - (2 * port)));
	return;
}

void crayon_savefile_load_icon(crayon_savefile_details_t * savefile_details, char * image, char * palette){
	file_t icon_files;

	icon_files = fs_open(image, O_RDONLY);
	savefile_details->savefile_icon = (unsigned char *) malloc(fs_total(icon_files));
	fs_read(icon_files, savefile_details->savefile_icon, fs_total(icon_files));
	fs_close(icon_files);

	icon_files = fs_open(palette, O_RDONLY);
	savefile_details->savefile_palette = (unsigned short *) malloc(fs_total(icon_files));
	fs_read(icon_files, savefile_details->savefile_palette, fs_total(icon_files));
	fs_close(icon_files);

	return;
}

void crayon_savefile_free_icon(crayon_savefile_details_t * savefile_details){
	free(savefile_details->savefile_icon);
	free(savefile_details->savefile_palette);

	return;
}

void crayon_savefile_init_savefile_details(crayon_savefile_details_t * savefile_details,  uint8_t * savefile_data, size_t savefile_size,
	uint8_t anim_count, uint16_t anim_speed, char * desc_long, char * desc_short, char * app_id, char * save_name){
	savefile_details->savefile_data = savefile_data;
	savefile_details->savefile_size = savefile_size;
	savefile_details->anim_count = anim_count;
	savefile_details->anim_speed = anim_speed;


	strcpy(savefile_details->desc_long, desc_long);
	strcpy(savefile_details->desc_short, desc_short);
	strcpy(savefile_details->app_id, app_id);
	strcpy(savefile_details->save_name, save_name);

	savefile_details->savefile_port = -1;
	savefile_details->savefile_slot = -1;
	savefile_details->valid_vmus = 0;
	savefile_details->valid_vmu_screens = 0;
	return;
}

uint8_t crayon_savefile_get_valid_vmus(crayon_savefile_details_t * savefile_details){
	maple_device_t *vmu;
	uint8_t valid_vmus = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(!crayon_savefile_check_for_device(i, j, MAPLE_FUNC_MEMCARD)){
				continue;
			}

			//Check if a save file DNE
			if(!crayon_savefile_check_for_save(savefile_details, i, j)){
				//Not enough free space for a savefile
				vmu = maple_enum_dev(i, j);
				if(vmufs_free_blocks(vmu) < crayon_savefile_get_save_block_count(savefile_details)){
					continue;
				}
			}

			//If we get to here, this VMU is valid
			crayon_savefile_set_vmu_bitmap(&valid_vmus, i, j);	//Sets wrong bit
		}
	}

	return valid_vmus;
}

uint8_t crayon_savefile_get_valid_saves(crayon_savefile_details_t * savefile_details){
	uint8_t valid_saves = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(!crayon_savefile_check_for_device(i, j, MAPLE_FUNC_MEMCARD)){
				continue;
			}

			if(crayon_savefile_check_for_save(savefile_details, i, j)){
				crayon_savefile_set_vmu_bitmap(&valid_saves, i, j);
			}
		}
	}

	return valid_saves;
}

uint8_t crayon_savefile_get_valid_screens(){
	uint8_t valid_screens = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(!crayon_savefile_check_for_device(i, j, MAPLE_FUNC_LCD)){
				continue;
			}

			//If we got here, we have a screen
			crayon_savefile_set_vmu_bitmap(&valid_screens, i, j);
		}
	}

	return valid_screens;
}

uint8_t crayon_savefile_load_uncompressed_save(crayon_savefile_details_t * savefile_details){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;
	char savename[32];

	//If you call everying in the right order, this is redundant
	//But just incase you didn't, here it is
	if(!crayon_savefile_check_for_device(savefile_details->savefile_port, savefile_details->savefile_slot, MAPLE_FUNC_MEMCARD)){
		return 0;
	}

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", savefile_details->savefile_port + 97, savefile_details->savefile_slot);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, savefile_details->save_name);

	//If the VMU isn't plugged in, this will fail anyways
	if(!(fp = fopen(savename, "rb"))){
		return 0;
	}

	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	pkg_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pkg_out = (uint8 *)malloc(pkg_size);
	fread(pkg_out, pkg_size, 1, fp);
	fclose(fp);

	vmu_pkg_parse(pkg_out, &pkg);

	//Read the pkg data into my struct
	memcpy(savefile_details->savefile_data, pkg.data, savefile_details->savefile_size);	//Last param is num of bytes and sizeof returns in bytes

	free(pkg_out);

	return 1;
}

int crayon_savefile_save_uncompressed_save(crayon_savefile_details_t * savefile_details){
	//No valid VMUs, no-go
	if(!savefile_details->valid_vmus){
		return -5;
	}

	//If you call everying in the right order, this is redundant
	//But just incase you didn't, here it is
	if(!crayon_savefile_check_for_device(savefile_details->savefile_port, savefile_details->savefile_slot, MAPLE_FUNC_MEMCARD)){
		return -4;
	}

	vmu_pkg_t pkg;
	uint8 *pkg_out, *data;
	int pkg_size;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;
	int rv = 0, blocks_freed = 0;
	file_t f;

	vmu = maple_enum_dev(savefile_details->savefile_port, savefile_details->savefile_slot);

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", savefile_details->savefile_port + 97, savefile_details->savefile_slot);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, savefile_details->save_name);

	int filesize = savefile_details->savefile_size;
	data = (uint8_t *) malloc(filesize);
	if(data == NULL){
		free(data);
		return -3;
	}
	memcpy(data, savefile_details->savefile_data, savefile_details->savefile_size);

	sprintf(pkg.desc_long, savefile_details->desc_long);
	strcpy(pkg.desc_short, savefile_details->desc_short);
	strcpy(pkg.app_id, savefile_details->app_id);
	pkg.icon_cnt = savefile_details->anim_count;
	pkg.icon_anim_speed = savefile_details->anim_speed;
	memcpy(pkg.icon_pal, savefile_details->savefile_palette, 32);
	pkg.icon_data = savefile_details->savefile_icon;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data_len = savefile_details->savefile_size;
	pkg.data = data;

	vmu_pkg_build(&pkg, &pkg_out, &pkg_size);

	// See if a file exists with that name, since we'll overwrite it.
	f = fs_open(savename, O_RDONLY);
	if(f != FILEHND_INVALID){
		blocks_freed = fs_total(f) >> 9;
		fs_close(f);
	}

	// Make sure there's enough free space on the VMU.
	if(vmufs_free_blocks(vmu) + blocks_freed < (pkg_size >> 9)){
		free(pkg_out);
		free(data);
		return pkg_size >> 9;
	}

	if(!(fp = fopen(savename, "wb"))){
		free(pkg_out);
		free(data);
		return -2;
	}

	if(fwrite(pkg_out, 1, pkg_size, fp) != (size_t)pkg_size){
		rv = -1;
	}

	fclose(fp);

	free(pkg_out);
	free(data);

	return rv;
}
