#include "savefile.h"

int save_uncompressed(uint8_t port, uint8_t unit, MinesweeperSaveFile_t * save){
	vmu_pkg_t pkg;
	uint8 *pkg_out, *data;
	int pkg_size;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;
	int rv = 0, blocks_freed = 0;
	file_t f;

	//Invalid controller/port
	if(port < 0 || port > 3 || unit < 1 || unit > 2){
		return -2;
	}

	// Make sure there's a VMU in port a1.
		//Change this later to check all slots or the requested slots
	if(!(vmu = maple_enum_dev(port, unit))){
		return -100;
	}

	if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_MEMCARD)){
		return -100;
	}

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", port + 97, unit);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "MINESWEEPER.s");

	int filesize = sizeof(MinesweeperSaveFile_t);
	data = (uint8_t *) malloc(filesize);
	if(data == NULL){
		free(data);
		return -1;
	}
	memcpy(data, save, sizeof(MinesweeperSaveFile_t));	//Last param is num of bytes and sizeof returns in bytes

	sprintf(pkg.desc_long, "Made with Crayon by Protofall");
	strcpy(pkg.desc_short, "Minesweeper");
	strcpy(pkg.app_id, "Proto_Minesweeper");
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	memcpy(pkg.icon_pal, save_file_palette, 32);
	pkg.icon_data = save_file_icon;
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

int load_uncompressed(uint8_t port, uint8_t unit, MinesweeperSaveFile_t * save){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;	//Used for checking if VMU is plugged in

	//Invalid controller/port
	if(port < 0 || port > 3 || unit < 1 || unit > 2){
		return 1;
	}

	if(!(vmu = maple_enum_dev(port, unit))){
		return 2;
	}

	if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_MEMCARD)){
		return 3;
	}

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", port + 97, unit);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "MINESWEEPER.s");

	//If the VMU isn't plugged in, this will fail anyways
	if(!(fp = fopen(savename, "rb"))){
		return -1;
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
	memcpy(save, pkg.data, sizeof(MinesweeperSaveFile_t));

	free(pkg_out);

	return 0;
}