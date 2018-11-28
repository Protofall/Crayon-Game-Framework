#include "savefile.h"

/*

But it's up to you to encode the palette as a "unsigned short *" and the image as a "unsigned char*"
Bin2c will do part 2 if you wanna store it in header files, and you could modify it to do shorts instead for the palette
Or if you wanna store it in binary, it gives you those directly

Palette is pretty easy, it's a 16 short array of rgb565 encoded colors (I think he meant argb4444)

Also these are "simple" icons. 32x32, 16 colors. But also only take 544 bytes

unsigned short is 16 bits (2 bytes)

16, 16bit colors = 32 bytes. This means there's 4 bits per channel hence arbg4444

The black and white palette is FF FF 00 F0 00 (Then the rest is zeroes)

The colours are fully white and fully black. Then means 1 hex latter = 1 channel.

So we have FF FF which is white and 00 F0 (or F0 00) which is black

*/

int save_uncompressed(uint8_t port, uint8_t unit){
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
	strcat(savename, "UNCOMPRESS.s");

	int filesize = sizeof(SaveFile_t);
	data = (uint8_t *) malloc(filesize);
	if(data == NULL){
		free(data);
		return -1;
	}
	memcpy(data, &save, sizeof(SaveFile_t) / 8);	//Last param is number of bytes, not bits

	sprintf(pkg.desc_long, "Uncompressed save file!");
	strcpy(pkg.desc_short, "Uncompressed");
	strcpy(pkg.app_id, "Proto_uncomp_save");
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	memcpy(pkg.icon_pal, palette, 32);
	pkg.icon_data = icon;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data_len = sizeof(SaveFile_t);	//Double check this, but I think its right
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

int load_uncompressed(uint8_t port, uint8_t unit){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	FILE *fp;
	char savename[32];

	//Invalid controller/port
	if(port < 0 || port > 3 || unit < 1 || unit > 2){
		return -2;
	}

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", port + 97, unit);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "UNCOMPRESS.s");

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
	memcpy(&save, pkg.data, sizeof(SaveFile_t) / 8);	//Last param is number of bytes, not bits

	free(pkg_out);

	return 0;
}

int save_compressed(uint8_t port, uint8_t unit){
	vmu_pkg_t pkg;
	uint8 *pkg_out, *comp;
	int pkg_size, err;
	uint32 len;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;
	int rv = 0, blocks_freed = 0;
	file_t f;

	//Invalid controller/port
	if(port < 0 || port > 3 || unit < 1 || unit > 2){
		return -2;
	}

	// Make sure there's a VMU in port A1.
		//Change this later to check all slots or the requested slots
	if(!(vmu = maple_enum_dev(0, 1))){
		return -100;
	}

	if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_MEMCARD)){
		return -100;
	}

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", port + 97, unit);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "COMPRESS.s");

	comp = (uint8 *)malloc(0x10000);
	len = 0x10000;

	// uint8_t * data;
	// int filesize = sizeof(SaveFile_t);
	// data = (uint8_t *) malloc(filesize);
	// memcpy(data, &save, filesize);

	//Try to fully understand these two lines...
	// uint8 sms_cart_ram[0x8000];
	err = compress2(comp, &len, &save, sizeof(SaveFile_t) / 8, 9);	//Warning suggests this is wrong
	// err = compress2(comp, &len, sms_cart_ram, 0x8000, 9);
	//dest, dest length, source(buffer), source length, level
		//The level parameter has the same meaning as in deflateInit. Initializes the internal stream state for compression
		//sourceLen is the byte length of the source buffer

	// 	compress(zipdata, &zipsize, data, filesize);

	if(err != Z_OK) {
		free(comp);
		return -1;
	}

	sprintf(pkg.desc_long, "Compressed save file!");
	strcpy(pkg.desc_short, "Compressed");
	strcpy(pkg.app_id, "Proto_comp_save");
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	memcpy(pkg.icon_pal, palette, 32);
	pkg.icon_data = icon;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data_len = len;
	pkg.data = comp;

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
		free(comp);
		return pkg_size >> 9;
	}

	if(!(fp = fopen(savename, "wb"))){
		free(pkg_out);
		free(comp);
		return -1;
	}

	if(fwrite(pkg_out, 1, pkg_size, fp) != (size_t)pkg_size){
		rv = -1;
	}

	fclose(fp);

	free(pkg_out);
	free(comp);

	return rv;
}


int load_compressed(uint8_t port, uint8_t unit){
	vmu_pkg_t pkg;
	uint8 *pkg_out;
	int pkg_size;
	// char prodcode[7];
	FILE *fp;
	char savename[32];
	uint32_t real_size = sizeof(SaveFile_t) / 8;

	//Invalid controller/port
	if(port < 0 || port > 3 || unit < 1 || unit > 2){
		return -2;
	}

	//Only 20 chara allowed at max (21 if you include '\0')
	sprintf(savename, "/vmu/%c%d/", port + 97, unit);	//port gets converted to a, b, c or d. unit is unit
	strcat(savename, "COMPRESS.s");

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

	uncompress(&save, &real_size, pkg.data, pkg.data_len);
	// ZEXTERN int ZEXPORT uncompress OF((Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen));

	free(pkg_out);

	return 0;
}

