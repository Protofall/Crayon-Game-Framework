#include <stdarg.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <dc/biosfont.h>
#include <dc/video.h>
#include <dc/pvr.h>

#include <kos/fs_romdisk.h> //For romdisk swapping

//Crayon libraries
#include "savefile.h"

#define LOAD_CODE 1

#if CRAYON_SD_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <ext2/fs_ext2.h>
#endif

#if CRAYON_SD_MODE == 1
	#define MNT_MODE FS_EXT2_MOUNT_READWRITE	//Might manually change it so its not a define anymore

	static void unmount_ext2_sd(){
		fs_ext2_unmount("/sd");
		fs_ext2_shutdown();
		sd_shutdown();
	}

	static int mount_ext2_sd(){
		kos_blockdev_t sd_dev;
		uint8 partition_type;

		// Initialize the sd card if its present
		if(sd_init()){
			return 1;
		}

		// Grab the block device for the first partition on the SD card. Note that
		// you must have the SD card formatted with an MBR partitioning scheme
		if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
			return 2;
		}

		// Check to see if the MBR says that we have a Linux partition
		if(partition_type != 0x83){
			return 3;
		}

		// Initialize fs_ext2 and attempt to mount the device
		if(fs_ext2_init()){
			return 4;
		}

		//Mount the SD card to the sd dir in the VFS
		if(fs_ext2_mount("/sd", &sd_dev, MNT_MODE)){
			return 5;
		}
		return 0;
	}
#endif

//+ save icon should be 2 blocks long
typedef struct minesweeper_savefile{
	uint8_t options;	//XXXH LOSQ (Refresh rate (Hz), Language, OS, Sound, Questions)
	uint16_t times[6];	//First 3 are Single player, last 3 are multiplayer
	char record_names[6][16];	//6 names, 11 chars long (Last char is \0 so really 10)

	//Prefered grid settings
	uint8_t pref_height;
	uint8_t pref_width;
	uint8_t pref_mines;
} minesweeper_savefile_t;

//Contains game options and focus (Windows tab)
typedef struct MinesweeperOptions{
	crayon_savefile_details_t savefile_details;
	minesweeper_savefile_t savefile;

} MinesweeperOptions_t;

//Doesn't load the right thing
int16_t setup_vmu_icon_load(uint8_t * vmu_lcd_icon, char * icon_path){
	vmu_lcd_icon = (uint8_t *) malloc(6 * 32);	//6 * 32 because we have 48/32 1bpp so we need that / 8 bytes
	FILE * file_lcd_icon = fopen(icon_path, "rb");
	if(!file_lcd_icon){return -1;}

	// fseek(file_lcd_icon, 0, SEEK_END); // seek to end of file
	// int size = ftell(file_lcd_icon); // get current file pointer
	// fseek(file_lcd_icon, 0, SEEK_SET); // seek back to beginning of file
	// fread(vmu_lcd_icon, size, 1, file_lcd_icon);

	size_t res = fread(vmu_lcd_icon, 192, 1, file_lcd_icon);	//If the icon is right, it *must* byt 192 bytes
	fclose(file_lcd_icon);

	return res;
}

uint8_t * test(char * icon_path){
	uint8_t * vmu_lcd_icon = (uint8_t *) malloc(6 * 32);	//6 * 32 because we have 48/32 1bpp so we need that / 8 bytes
	FILE * file_lcd_icon = fopen(icon_path, "rb");
	if(!file_lcd_icon){return NULL;}
	if(fread(vmu_lcd_icon, 192, 1, file_lcd_icon) != 1){return NULL;}	//If the icon is right, it *must* byt 192 bytes
	fclose(file_lcd_icon);

	return vmu_lcd_icon;
}

void setup_vmu_icon_apply(uint8_t * vmu_lcd_icon, uint8_t valid_vmu_screens){
	crayon_vmu_display_icon(valid_vmu_screens, vmu_lcd_icon);
	free(vmu_lcd_icon);

	return;
}


uint8_t memory_mount_romdisk(char *filename, char *mountpoint){
	void *buffer;
	ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM

	if(size == -1){
		return 1;
	}
	
	fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
	return 0;
}

void error_freeze(const char *format, ...){
	va_list va_args;
	va_start(va_args, format);

	char line[80];
	vsprintf(line, format, va_args);

	pvr_shutdown(); //Stop any drawing in progress
	bfont_set_encoding(BFONT_CODE_ISO8859_1);
	bfont_draw_str(vram_s, 640, 1, line);

	while(1); //Freeze
}

pvr_init_params_t pvr_params = {
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 }, 512 * 1024, 0, 0, 0
};


//---------------------------------------------------------


int main(){
	MinesweeperOptions_t MS_options;	//Contains a bunch of vars related to options

	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			return 0;
		}
	#endif

	//Load the VMU icon data
	#if CRAYON_SD_MODE == 1
		memory_mount_romdisk("/sd/SaveFile.img", "/Save");
	#else
		memory_mount_romdisk("/cd/SaveFile.img", "/Save");
	#endif

	crayon_savefile_load_icon(&MS_options.savefile_details, "/Save/IMAGE.BIN", "/Save/PALLETTE.BIN");

	uint8_t * vmu_lcd_icon = NULL;

	#if LOAD_CODE == 1
		// int16_t load_res = setup_vmu_icon_load(vmu_lcd_icon, "/Save/LCD.bin");
		int16_t load_res;
		vmu_lcd_icon = test("/Save/LCD.bin");
		if(vmu_lcd_icon == NULL){load_res = 0;}
		else{load_res = 1;}
	#else
		int16_t load_res = 0;
		vmu_lcd_icon = (uint8_t *) malloc(6 * 32);	//6 * 32 because we have 48/32 1bpp so we need that / 8 bytes
		FILE * file_lcd_icon = fopen("/Save/LCD.bin", "rb");
		if(!file_lcd_icon){load_res = -1;}
		load_res = fread(vmu_lcd_icon, 192, 1, file_lcd_icon);	//If the icon is right, it *must* byt 192 bytes
		fclose(file_lcd_icon);
	#endif

	//This should be zero, but it isn't...
	int debug_first_row = vmu_lcd_icon[0] + vmu_lcd_icon[1] + vmu_lcd_icon[2] + vmu_lcd_icon[3] + vmu_lcd_icon[4] + vmu_lcd_icon[5];

	fs_romdisk_unmount("/SaveFile");

	MS_options.savefile_details.valid_vmu_screens = crayon_savefile_get_valid_screens();

	//Apply the VMU LCD icon (Apparently this is automatic if your savefile is an ICONDATA.VMS)
	//Also frees vmu_lcd_icon
	setup_vmu_icon_apply(vmu_lcd_icon, MS_options.savefile_details.valid_vmu_screens);

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
	}

	pvr_init(&pvr_params);

	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	error_freeze("First row sum: %hhu. Load_res: %d\n", debug_first_row, load_res);

	//Never gets beyond here ofc

	//Confirm everything was unloaded successfully (Should equal zero) This code is never triggered under normal circumstances
	//I'm probs forgetting a few things
	int retVal = 0;
	// retVal += crayon_memory_free_prop_font_sheet(&Tahoma_font);
	crayon_savefile_free_icon(&MS_options.savefile_details);

	error_freeze("Free-ing result %d!\n", retVal);

	return 0;
}
