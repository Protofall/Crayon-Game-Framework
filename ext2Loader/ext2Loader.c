#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//Controller stuff
#include <dc/maple.h>
#include <dc/maple/controller.h>

//For mounting the sd dir
#include <dc/sd.h>
#include <kos/blockdev.h>
#include <ext2/fs_ext2.h>

#include "error.h"	//Doesn't seem to work for some reason...

#include <kos/fs.h>	//Might need it for fs_load()
#include <arch/exec.h>	//arch_exec()
#include <assert.h>	//assert()

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

int main(){
	int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
	if(sdRes != 0){
		error_freeze("sdRes = %d\n", sdRes);
	}

	void *prog;
	ssize_t length = fs_load("/sd/Minesweeper.bin", &prog);
	unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	assert(length > 0);
	arch_exec(prog, length);

	//This code below might be useful for when I convert this into a full-blown program
	// uint32_t previous_buttons = 0;	//Records the previous buttons polled
	// maple_device_t  * controller;
	// cont_state_t * st;
 
	// while(1){
	// controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);	//Reads the first plugged in controller
	// 	st = (cont_state_t *)maple_dev_status(controller);	//State of controller

	// 	previous_buttons = st->buttons;
	// }
}
