#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//Controller stuff
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include <kos/fs.h>	//Might need it for fs_load()
#include <arch/exec.h>	//arch_exec()
#include <assert.h>	//assert()

#include <kos/dbgio.h>	//Better debug output to the screen

//Use on of these two for getting the VFS content
#include <sys/dirent.h>	//opendir, readdir
#include <kos/fs.h>	//fs_open, fs_readdir

#define CRAYON_SD_MODE 0

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

void indent(uint8_t hovering){
	if(hovering){
		dbgio_printf("> ");
	}
	else{
		dbgio_printf("  ");
	}
	return;
}

void print_dir(uint16_t cursor_pos, uint16_t counter_pos, uint16_t * file_count){
	file_t d;
	dirent_t * de;
	d = fs_open("/", O_RDONLY | O_DIR);
	while(1){
		if(counter_pos == 0){
			indent(counter_pos == cursor_pos);
			dbgio_printf("..\n");
		}
		else{
			de = fs_readdir(d);
			if(de == NULL){
				break;
			}
			indent(counter_pos == cursor_pos);
			dbgio_printf("/%s\n", de->name);
		}
		counter_pos++;
	}
	*file_count = counter_pos;	//Update the number of files
	// dbgio_printf("It is %d", counter_pos);
	int i;
	for(i = 1 + *file_count; i < 17; i++){	//17 - counter_pos = 12
		dbgio_printf("\n");
	}
	fs_close(d);
	return;
}

void clear_screen(){
	// dbgio_printf("\n\n\n\n\n\n\n\n\n\n\n\n");
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_write('\n');
	dbgio_flush();
}

int main(){
	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			dbgio_printf("Error. sdRes = %d\n", sdRes);
			while(1){;}
		}
	#endif

	//A test to see how dbgio works
	dbgio_dev_select("fb");
	uint16_t cursor_pos = 0;
	uint16_t counter_pos = 0;
	uint16_t file_count;

	//Only 17 lines onscreen at once

	// uint8_t data[27] = "testme\nletsgo\nannother one\n";
	// uint8_t seventeen[17] = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	// dbgio_write_buffer_xlat(&data, 27);

	print_dir(cursor_pos, counter_pos, &file_count);

	//Can be useful for the VFS explorer
	uint32_t previous_buttons = 0;	//Records the previous buttons polled
	maple_device_t  * controller;
	cont_state_t * st;

	while(1){	//For testing purposes
		controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);	//Reads the first plugged in controller
		st = (cont_state_t *)maple_dev_status(controller);	//State of controller

		//A press
		if((st->buttons & (1 << 2)) && !(previous_buttons & (1 << 2))){	//If you just pressed A
			// dbgio_write_buffer_xlat(&seventeen, 17);
			// dbgio_write_buffer_xlat(&data, 27);
			// dbgio_printf("\033[2J");
			// dbgio_printf("\033[0;0H");
			// dbgio_write('c');	//Flush not needed
			// dbgio_write('a');
			// dbgio_write('t');
			// dbgio_write('\n');
		}

		//Up press
		if((st->buttons & CONT_DPAD_UP) && !(previous_buttons & CONT_DPAD_UP)){
			if(cursor_pos != 0){
				cursor_pos--;
			}
			clear_screen();
			print_dir(cursor_pos, counter_pos, &file_count);
		}

		//Down press
		if((st->buttons & CONT_DPAD_DOWN) && !(previous_buttons & CONT_DPAD_DOWN)){
			if(cursor_pos < file_count - 1){
				cursor_pos++;
			}
			clear_screen();
			print_dir(cursor_pos, counter_pos, &file_count);
		}

		previous_buttons = st->buttons;
	}

	void *prog;
	ssize_t length = fs_load("/sd/Program.bin", &prog);	//The un-scrambled binary executable you want to launch
	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif
	assert(length > 0);
	arch_exec(prog, length);
}
