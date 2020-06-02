//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

// #if CRAYON_BOOT_MODE == 1
// 	//For mounting the sd dir
// 	#include <dc/sd.h>
// 	#include <kos/blockdev.h>
// 	#include <fat/fs_fat.h>
// #endif


// #if CRAYON_BOOT_MODE == 1
// 	#define MNT_MODE FS_FAT_MOUNT_READONLY

// 	static void unmount_fat_sd(){
// 		fs_fat_unmount("/sd");
// 		fs_fat_shutdown();
// 		sd_shutdown();
// 	}

// 	static int mount_fat_sd(){
// 		kos_blockdev_t sd_dev;
// 		uint8 partition_type;

// 		// Initialize the sd card if its present
// 		if(sd_init()){
// 			return 1;
// 		}

// 		// Grab the block device for the first partition on the SD card. Note that
// 		// you must have the SD card formatted with an MBR partitioning scheme
// 		if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
// 			return 2;
// 		}

// 		// Check to see if the MBR says that we have a valid partition
// 		// if(partition_type != 0x83){
// 			//I don't know what value I should be comparing against, hence this check is disabled for now
// 			// This: https://en.wikipedia.org/wiki/Partition_type
// 				//Suggests there's multiple types for FAT...not sure how to handle this
// 			// return 3;
// 		// }

// 		// Initialize fs_fat and attempt to mount the device
// 		if(fs_fat_init()){
// 			return 4;
// 		}

// 		//Mount the SD card to the sd dir in the VFS
// 		if(fs_fat_mount("/sd", &sd_dev, MNT_MODE)){
// 			return 5;
// 		}
// 		return 0;
// 	}

// #endif

int main(){
	// #if CRAYON_BOOT_MODE == 1
	// 	int sdRes = mount_fat_sd();	//This function should be able to mount a FAT32 formatted sd card to the /sd dir	
	// 	if(sdRes != 0){
	// 		error_freeze("SD card couldn't be mounted: %d", sdRes);
	// 	}
	// #endif

	//Note:, first parameter is ignore for now since Crayon is only on Dreamcast ATM
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		error_freeze("Crayon failed to initialise");
	}

	srand(time(0));	//Set the seed for rand()

	crayon_spritesheet_t Logo;
	crayon_sprite_array_t Logo_Draw;
	crayon_palette_t Logo_P;

	crayon_memory_mount_romdisk("/pc/stuff.img", "/files");

	//Load the logo
	#if CRAYON_BOOT_MODE == 2
		crayon_memory_mount_romdisk("/pc/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == 1
		crayon_memory_mount_romdisk("/sd/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == 0
		crayon_memory_mount_romdisk("/cd/stuff.img", "/files");
	#else
		#error Invalid CRAYON_BOOT_MODE value
	#endif

	//Load the asset
	crayon_memory_load_spritesheet(&Logo, &Logo_P, 0, "/files/logo.dtex");

	fs_romdisk_unmount("/files");

	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Logo_Draw, &Logo, 0, &Logo_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Logo_Draw.coord[0].x = (crayon_graphics_get_window_width() - Logo.animation[0].frame_width) / 2;
	Logo_Draw.coord[0].y = (crayon_graphics_get_window_height() - Logo.animation[0].frame_height) / 2;
	Logo_Draw.layer[0] = 2;
	Logo_Draw.scale[0].x = 1;
	Logo_Draw.scale[0].y = 1;
	Logo_Draw.flip[0] = 0;
	Logo_Draw.rotation[0] = 0;
	Logo_Draw.colour[0] = 0;
	Logo_Draw.fade[0] = 0;
	Logo_Draw.frame_id[0] = 0;
	Logo_Draw.visible[0] = 1;
	crayon_memory_set_frame_uv(&Logo_Draw, 0, 0);

	uint32_t colours[5];
	colours[0] = Logo_P.palette[1];	//Orange
	colours[1] = 0xFF002FFF;	//Blue
	colours[2] = 0xFFCE21FF;	//Purple
	colours[3] = 0xFFFF228D;	//Pink
	colours[4] = 0xFFFFED00;	//Yellow

	uint8_t begin = 0;
	uint8_t moving = 0;
	//__htz is set in cryaon_init()
	float shrink_time = 2.5 * __htz;	//Time it takes to shrink (In seconds, htz_adjust fixes for 50Hz)

	//Positive is bottom right, negative is top left
	int8_t x_dir = rand() % 2;
	if(!x_dir){x_dir = -1;}
	int8_t y_dir = rand() % 2;
	if(!y_dir){y_dir = -1;}

	//Once shrunk, this will be the new width/height
	float new_width = crayon_graphics_get_draw_element_width(&Logo_Draw, 0);
	float new_height = crayon_graphics_get_draw_element_height(&Logo_Draw, 0);

	uint8_t loop = 1;
	while(loop){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			//If any button is pressed, start the game (Doesn't check thumbstick)
			if(st->buttons || st->ltrig > 255 * 0.1 || st->rtrig > 255 * 0.1){
				begin = 1;
			}

			//Press all 4 facebuttons at once to exit
			if(st->buttons & (CONT_A | CONT_B | CONT_X | CONT_Y)){
				loop = 0;
				break;
			}

		MAPLE_FOREACH_END()

		//Make the object bounce around
		if(moving){
			//Collision detection
			if(Logo_Draw.coord[0].x < 0){	//Off left side
				x_dir = 1;
				Logo_P.palette[1] = colours[rand() % 5];
			} 
			if(Logo_Draw.coord[0].y < 0){	//Off top side
				y_dir = 1;
				Logo_P.palette[1] = colours[rand() % 5];
			}
			if(Logo_Draw.coord[0].x + new_width > 640){	//Off right side
				x_dir = -1;
				Logo_P.palette[1] = colours[rand() % 5];
			}
			if(Logo_Draw.coord[0].y + new_height > 480){	//Off bottom side
				y_dir = -1;
				Logo_P.palette[1] = colours[rand() % 5];
			}

			//colours Dark Blue, Purple, Pink, Orange, vright Green, Yellow

			//Movement
			//NOTE: __htz_adjustment is set in crayon_graphics_init() or crayon_init()
			Logo_Draw.coord[0].x += 1.5 * __htz_adjustment * x_dir;
			Logo_Draw.coord[0].y += 1.5 * __htz_adjustment * y_dir;
		}

		//Shrinking process
		if(begin && Logo_Draw.scale[0].x > 0.4 && Logo_Draw.scale[0].y > 0.3){
			Logo_Draw.scale[0].x -= (0.6/shrink_time) * __htz_adjustment;
			Logo_Draw.scale[0].y -= (0.7/shrink_time) * __htz_adjustment;
			new_width = crayon_graphics_get_draw_element_width(&Logo_Draw, 0);
			new_height = crayon_graphics_get_draw_element_height(&Logo_Draw, 0);
			Logo_Draw.coord[0].x = (crayon_graphics_get_window_width() - new_width) / 2;
			Logo_Draw.coord[0].y = (crayon_graphics_get_window_height() - new_height) / 2;
		}
		else{
			if(begin){
				moving = 1;
			}
		}

		crayon_graphics_setup_palette(&Logo_P);	//Could live outside the loop, but later we will change the palette when it hits the corners

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Logo_Draw, NULL, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
		pvr_list_finish();

		pvr_scene_finish();
	}

	crayon_memory_free_sprite_array(&Logo_Draw);

	crayon_memory_free_spritesheet(&Logo);
	crayon_memory_free_palette(&Logo_P);

	crayon_shutdown();

	return 0;
}
