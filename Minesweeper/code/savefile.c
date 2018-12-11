#include "savefile.h"

void vmu_load_icon(SaveFileDetails_t * save){

	file_t icon_files;

	icon_files = fs_open("/Save/IMAGE.BIN", O_RDONLY);
	save->save_file_icon = (unsigned char *) malloc(fs_total(icon_files));
	fs_read(icon_files, save->save_file_icon, fs_total(icon_files));
	fs_close(icon_files);

	icon_files = fs_open("/Save/PALLETTE.BIN", O_RDONLY);
	save->save_file_palette = (unsigned short *) malloc(fs_total(icon_files));
	fs_read(icon_files, save->save_file_palette, fs_total(icon_files));
	fs_close(icon_files);

	return;
}

void vmu_free_icon(SaveFileDetails_t * save){

	free(save->save_file_icon);
	free(save->save_file_palette);

	return;
}

/*

port	slot	bit
0		1		(1 << 7)
0		2		(1 << 6)
1		1		(1 << 5)
1		2		(1 << 4)
2		1		(1 << 3)
2		2		(1 << 2)
3		1		(1 << 1)
3		2		(1 << 0)

*/

uint8_t vmu_get_bit(uint8_t valid_vmus, int8_t port, int8_t slot){
	return !!(valid_vmus & (1 << ((2 - slot) + 6 - (2 * port))));
}

void vmu_set_bit(uint8_t * valid_vmus, int8_t port, int8_t slot){
	*valid_vmus |= (1 << ((2 - slot) + 6 - (2 * port)));
	return;
}

//Note this assumes the vmu chosen is valid
uint8_t vmu_check_for_savefile(SaveFileDetails_t * save, int8_t port, int8_t slot){
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

	//CURRENTLY THEY'RE ALL HARDCODED and don't work...
	//The IDs don't match
	// if(strcmp(pkg.app_id, save->save_id)){
	// 	return 0;
	// }
	// if(strcmp(pkg.app_id, "Proto_Minesweeper")){
	// 	return 0;
	// }
	return 1;
}

//Note Crayon doesn't bother supporting eye catchers. Also hard coded for 1 icon
	//I used the "vmu_pkg_build" function's source as a guide for this
uint16_t vmu_get_save_block_count(size_t savefile_size){
	int pkg_size, icon_cnt, data_len;
	icon_cnt = 1;
	data_len = savefile_size;

	//Get the total number of bytes. Keep in mind we need to think about the icon/s
	pkg_size = sizeof(vmu_hdr_t) + 512 * icon_cnt + data_len;

	return pkg_size >> 9;	//2^9 is 512 so this gets us the number of blocks
}

//UNFINISHED
uint8_t vmu_valid(SaveFileDetails_t * save){
	maple_device_t *vmu;
	uint8_t valid_vmus = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			//No device present
			if(!(vmu = maple_enum_dev(i, j))){
				continue;
			}

			//Not valid or a memory card
			if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_MEMCARD)){
				continue;
			}

			//Check if a save file DNE
			if(!vmu_check_for_savefile(save, i, j)){
				//Not enough free space for a savefile
				if(vmufs_free_blocks(vmu) < vmu_get_save_block_count(save->save_file_size)){
					continue;
				}
			}

			//If we get to here, this VMU is valid
			vmu_set_bit(&valid_vmus, i, j);	//Sets wrong bit
		}
	}

	return valid_vmus;
}

uint8_t vmu_valid_screens(){
	maple_device_t *vmu;
	uint8_t valid_screens = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			if(!(vmu = maple_enum_dev(i, j))){
				continue;
			}

			if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_LCD)){
				continue;
			}

			//If we got here, we have a screen
			vmu_set_bit(&valid_screens, i, j);
		}
	}

	return valid_screens;
}

uint8_t vmu_get_savefiles(SaveFileDetails_t * save){
	maple_device_t *vmu;
	uint8_t valid_saves = 0;	//a1a2b1b2c1c2d1d2

	int i, j;
	for(i = 0; i <= 3; i++){
		for(j = 1; j <= 2; j++){
			//No device present
			if(!(vmu = maple_enum_dev(i, j))){
				continue;
			}

			//Not valid or a memory card
			if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_MEMCARD)){
				continue;
			}

			if(vmu_check_for_savefile(save, i, j)){
				vmu_set_bit(&valid_saves, i, j);
			}
		}
	}

	return valid_saves;
}

int vmu_save_uncompressed(SaveFileDetails_t * save){
	vmu_pkg_t pkg;
	uint8 *pkg_out, *data;
	int pkg_size;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;
	int rv = 0, blocks_freed = 0;
	file_t f;

	vmu = maple_enum_dev(save->port, save->slot);

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", save->port + 97, save->slot);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "MINESWEEPER.s");

	int filesize = sizeof(MinesweeperSaveFile_t);
	data = (uint8_t *) malloc(filesize);
	if(data == NULL){
		free(data);
		return -1;
	}
	memcpy(data, &save->save_file, sizeof(MinesweeperSaveFile_t));	//Last param is num of bytes and sizeof returns in bytes

	sprintf(pkg.desc_long, "Made with Crayon by Protofall");
	strcpy(pkg.desc_short, "Minesweeper");
	strcpy(pkg.app_id, "Proto_Minesweeper");
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	memcpy(pkg.icon_pal, save->save_file_palette, 32);
	pkg.icon_data = save->save_file_icon;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data_len = sizeof(MinesweeperSaveFile_t);	//Double check this, but I think its right
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
		return -1;
	}

	if(fwrite(pkg_out, 1, pkg_size, fp) != (size_t)pkg_size){
		rv = -1;
	}

	fclose(fp);

	free(pkg_out);
	free(data);

	return rv;
}

uint8_t vmu_load_uncompressed(SaveFileDetails_t * save){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;
	char savename[32];
	// maple_device_t *vmu;	//Used for checking if VMU is plugged in

	// vmu = maple_enum_dev(save->port, save->slot);

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", save->port + 97, save->slot);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "MINESWEEPER.s");

	//If the VMU isn't plugged in, this will fail anyways
	if(!(fp = fopen(savename, "rb"))){
		return 1;
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
	memcpy(&save->save_file, pkg.data, sizeof(MinesweeperSaveFile_t));

	free(pkg_out);

	return 0;
}
