#include "crayon.h"

uint8_t __sd_present = 0;

uint8_t __attribute__((weak)) crayon_init(uint8_t platform, uint8_t boot_mode){
	__game_base_path = NULL;
	if(platform != CRAYON_PLATFORM_DREAMCAST){
		fprintf(stderr, "ERROR: Unsupported platform: %d\n", platform);
		return 1;
	}

	if(boot_mode != CRAYON_BOOT_OPTICAL && boot_mode != CRAYON_BOOT_SD && boot_mode != CRAYON_BOOT_PC_LAN){
		fprintf(stderr, "ERROR: Invalid boot mode: %d\n", boot_mode);
		return 1;
	}

	// For (platform == PC) I'll need to make sure the malloc is larger (Get the OS base path first then malloc)
	__game_base_path = malloc(5 * sizeof(char));
	if(!__game_base_path){
		return 1;
	}

	const char * paths[3] = {
		"/cd/",
		"/sd/",
		"/pc/"
	};
	strcpy(__game_base_path, paths[boot_mode]);

	if(boot_mode == CRAYON_BOOT_SD){
		__sd_present = 1;
	}
	else{
		__sd_present = 0;
	}

	// if(crayon_audio_init()){
	//  goto crayon_init_end1;
	// }

	if(crayon_graphics_init(CRAYON_ENABLE_OP | CRAYON_ENABLE_TR | CRAYON_ENABLE_PT)){
		goto crayon_init_end2;
	}

	if(__sd_present){
		if(crayon_sd_mount_fat()){
			goto crayon_init_end3;
		}
	}

	return 0;

	crayon_init_end3:

	crayon_graphics_shutdown();

	crayon_init_end2:

	// crayon_audio_shutdown();

	crayon_init_end1:

	free(__game_base_path);

	return 1;
}


uint8_t crayon_sd_mount_fat(){
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

	// Check to see if the MBR says that we have a valid partition
	// if(partition_type != 0x83){
		// I don't know what value I should be comparing against, hence this check is disabled for now
		// This: https://en.wikipedia.org/wiki/Partition_type
			//Suggests there's multiple types for FAT...not sure how to handle this
		// return 3;
	// }

	// Initialize fs_fat and attempt to mount the device
	if(fs_fat_init()){
		return 4;
	}

	//Mount the SD card to the sd dir in the VFS
	if(fs_fat_mount("/sd", &sd_dev, MNT_MODE)){
		return 5;
	}
	return 0;
}



void __attribute__((weak)) crayon_shutdown(){
	free(__game_base_path);

	if(__sd_present){
		crayon_sd_unmount_fat();
	}

	crayon_graphics_shutdown();
	// crayon_audio_shutdown();

	return;
}

void crayon_sd_unmount_fat(){
	fs_fat_unmount("/sd");
	fs_fat_shutdown();
	sd_shutdown();

	return;
}
