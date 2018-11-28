#include <kos.h>	//Replace this with vital only libs

// #include <dc/maple/vmu.h>
// #include <dc/vmu_pkg.h>
// #include <kos/fs.h>

#include <zlib/zlib.h>	//For the length of the text file in the gz archive

typedef struct SaveFile{
	uint32_t var1;
	uint8_t var2;
} SaveFile_t;

SaveFile_t save;
unsigned char * icon;		//uint8_t
unsigned short * palette;	//uint16_t

// icon_img and icon_pal

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

// int DC_CreateVMUSaveFile(char *src){
// 	char dst[32];

// 	vmu_pkg_t pkg;
// 	uint8_t *pkg_out;
// 	int pkg_size;

// 	//Setup the save file ready to go into a VMU header
// 	uint8_t * data;
// 	int filesize = sizeof(SaveFile_t);
// 	data = (uint8_t *) malloc(filesize);
// 	memcpy(data, &save, filesize);

// 	// Our VMU + full save name
// 	strcpy(dst, "/vmu/a1/");			//I think this is controller "a" slot "1". Have code to check for valid VMUs
// 	strcat(dst, src);

// 	// Required VMU header
// 	strcpy(pkg.desc_short, "Crayon\\DC");
// 	strcpy(pkg.desc_long, "Game Save");
// 	strcpy(pkg.app_id, "CRAYON_VMU_TEST\\DC");

// 	memcpy(&pkg.icon_pal[0], palette, 32);
// 	pkg.icon_data = icon;
// 	pkg.data_len = filesize;
// 	pkg.data = data;
// 	vmu_pkg_build(&pkg, &pkg_out, &pkg_size);

// 	pkg.icon_cnt = 1;
// 	pkg.icon_anim_speed = 0;
// 	pkg.eyecatch_type = VMUPKG_EC_NONE;

// 	// Save the newly created VMU save to the VMU
// 	file_t file = fs_open(dst, O_WRONLY);	//Errors out (lxdream)
// 	if(file == -1){
// 		printf("File DNE\n");
// 	}
// 	// printf("%d\n", file);
// 	fs_write(file, pkg_out, pkg_size);
// 	fs_close(file);							//Errors out (lxdream)

// 	// Free unused memory
// 	free(pkg_out);
// 	free(data);
// 	return 0;
// }

//Bluecrab's version. Modify it slightly to use my icon data and remove stuff that shouldn't be there
	//icon_img and icon_pal seem to be the last 2 things
int sms_write_cartram_to_file(const char *fn){
	vmu_pkg_t pkg;
	uint8 *pkg_out, *comp;
	int pkg_size, err;
	uint32 len;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;
	int rv = 0, blocks_freed = 0;
	file_t f;

	// Make sure there's a VMU in port A1.
		//Change this later to check all slots or the requested slots
	if(!(vmu = maple_enum_dev(0, 1))){
		return -100;
	}

	if(!vmu->valid || !(vmu->info.functions & MAPLE_FUNC_MEMCARD)){
		return -100;
	}

	sprintf(savename, "/vmu/a1/CRAYON_VMU");	//Only 20 chara allowed at max (21 if you include '\0')

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

	if(fn != NULL) {
		strncpy(pkg.desc_long, fn, 32);
		pkg.desc_long[31] = 0;
	}
	else {
		sprintf(pkg.desc_long, "Save test, hope it goes well!");
	}

	// unsigned char * icon;		//uint8_t
	// unsigned short * palette;

	strcpy(pkg.desc_short, "Proto's Save");
	strcpy(pkg.app_id, "Proto_Save");
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

int save_uncompressed(uint8_t port, uint8_t unit){
	vmu_pkg_t pkg;
	uint8 *pkg_out, *data;
	int pkg_size;
	// int err;
	// uint32 len;
	FILE *fp;
	char savename[32];
	maple_device_t *vmu;
	int rv = 0, blocks_freed = 0;
	file_t f;

	//Invalid controller/port
	if(port < 0 || port > 3 || unit < 1 || unit > 2){
		return -100;
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
	strcat(savename, "CRAYON_TEST");

	int filesize = sizeof(SaveFile_t);
	data = (uint8_t *) malloc(filesize);
	if(data == NULL){
		free(data);
		return -1;
	}
	memcpy(data, &save, sizeof(SaveFile_t) / 8);	//Last param is number of bytes, not bits

	sprintf(pkg.desc_long, "Longer Name");

	strcpy(pkg.desc_short, "Short Name");
	strcpy(pkg.app_id, "PROTO_0");
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	memcpy(pkg.icon_pal, palette, 32);
	pkg.icon_data = icon;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	// pkg.data_len = len;
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

// Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void){

	pvr_init_defaults();		// Init kos

	pvr_set_bg_color(0.46f, 0.22f, 0.49f);	//Solid purple screen

	save.var1 = 0;
	save.var2 = 41;

	file_t file;

	file = fs_open("/rd/IMAGE.BIN", O_RDONLY);
	int filesize = fs_total(file);
	icon = (unsigned char *) malloc(filesize);
	fs_read(file, icon, filesize);
	fs_close(file);

	file = fs_open("/rd/PALLETTE.BIN", O_RDONLY);
	int filesize2 = fs_total(file);
	palette = (unsigned short *) malloc(filesize2);
	fs_read(file, palette, filesize2);
	fs_close(file);

	// DC_CreateVMUSaveFile("hii");
	// sms_write_cartram_to_file(NULL);
	// uncompressed_BlueCrab_save();
	save_uncompressed(2, 1);

	while(1){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		if(st->buttons & CONT_START){	// Press start to quit
			break;
		}

		if(st->buttons & CONT_A){	// Press A to blah
			// break;
		}

		MAPLE_FOREACH_END()

		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);
		//stuff
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);
		//stuff
		pvr_list_finish();

		pvr_scene_finish();
	}

	free(icon);
	free(palette);

	return 0;
}

// A more in-depth version here
// http://dcemulation.org/phpBB/viewtopic.php?f=34&t=103666&p=1047688&hilit=vmu+save+file#p1047688

// OLD STUFF BELOW, IGNORE

// int DC_CompressedSaveToVMU(char *src){
// 	char dst[32];
// 	file_t file;
// 	int filesize = 0;
// 	unsigned long zipsize = 0;
// 	uint8 *data;
// 	uint8 *zipdata;
// 	vmu_pkg_t pkg;
// 	uint8 *pkg_out;
// 	int pkg_size;

// 	// Our VMU + full save name
// 	strcpy(dst, "/vmu/a1/");			//I think this is controller "a" slot "1". Have code to check for valid VMUs
// 	strcat(dst, src);

// 	// Reads in the file from the CWD
// 	file = fs_open(src, O_RDONLY);
// 	filesize = fs_total(file);
// 	data = (uint8*)malloc(filesize); // (uint8_t *) malloc(filesize); ?
// 	fs_read(file, data, filesize);
// 	fs_close(file);

// 	// Allocate some memory for compression
// 	zipsize = filesize * 2;
// 	zipdata = (uint8*)malloc(zipsize);

// 	// The compressed save
// 	compress(zipdata, &zipsize, data, filesize);

// 	// Required VMU header
// 	// You will have to have a VMU icon defined under icon_data
// 	strcpy(pkg.desc_short, "Crayon\\DC");
// 	strcpy(pkg.desc_long, "Game Save");
// 	strcpy(pkg.app_id, "CRAYON_VMU_TEST\\DC");
// 	pkg.icon_cnt = 1;
// 	pkg.icon_anim_speed = 0;
// 	memcpy(&pkg.icon_pal[0], icon_data, 32);
// 	pkg.icon_data = icon_data + 32;
// 	pkg.eyecatch_type = VMUPKG_EC_NONE;
// 	pkg.data_len = zipsize;
// 	pkg.data = zipdata;
// 	vmu_pkg_build(&pkg, &pkg_out, &pkg_size);

// 	// Save the newly created VMU save to the VMU
// 	fs_unlink(dst);
// 	file = fs_open(dst, O_WRONLY);
// 	fs_write(file, pkg_out, pkg_size);
// 	fs_close(file);

// 	// Free unused memory
// 	free(pkg_out);
// 	free(data);
// 	free(zipdata);
// 	return 0;
// }

// int DC_CompressedLoadFromVMU(char *dst){
// 	char src[32];
// 	int file;
// 	int filesize;
// 	unsigned long unzipsize;
// 	uint8* data;
// 	uint8* unzipdata;
// 	vmu_pkg_t pkg;

// 	// Our VMU + full save name
// 	strcpy(src, "/vmu/a1/");
// 	strcat(src, dst);

// 	// Remove VMU header
// 	file = fs_open(src, O_RDONLY);
// 	if(file == 0) return -1;
// 	filesize = fs_total(file);
// 	if(filesize <= 0) return -1;
// 	data = (uint8*)malloc(filesize);
// 	fs_read(file, data, filesize);
// 	vmu_pkg_parse(data, &pkg);
// 	fs_close(file);

// 	// Allocate memory for the uncompressed data
// 	unzipdata = (uint8 *)malloc(65536);
// 	unzipsize = 65536;

// 	// Uncompress the data to the CWD
// 	uncompress(unzipdata, &unzipsize, (uint8 *)pkg.data, pkg.data_len);
// 	fs_unlink(dst);
// 	file = fs_open(dst, O_WRONLY);
// 	fs_write(file, unzipdata, unzipsize);
// 	fs_close(file);

// 	// Free unused memory
// 	free(data);
// 	free(unzipdata);
// 	return 0;
// }