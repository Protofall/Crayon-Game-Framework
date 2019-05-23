//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For the controller and mouse
#include <dc/maple.h>
#include <dc/maple/controller.h>

#if CRAYON_BOOT_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <ext2/fs_ext2.h>
#endif

#if CRAYON_BOOT_MODE == 1
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

//Change this to only give room for PT list (Since other ones aren't used)
pvr_init_params_t pvr_params = {
		// Enable opaque, translucent and punch through polygons with size 16
			//To better explain, the opb_sizes or Object Pointer Buffer sizes
			//Work like this: Set to zero to disable. Otherwise the higher the
			//number the more space used (And more efficient under higher loads)
			//The lower the number, the less space used and less efficient under
			//high loads. You can choose 0, 8, 16 or 32
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },

		// Vertex buffer size 512K
		512 * 1024,

		// No DMA
		0,

		// No FSAA
		0,

		// Translucent Autosort enabled
		0
};

int main(){
	pvr_init(&pvr_params);	//Init the pvr system

	#if CRAYON_BOOT_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	crayon_spritesheet_t Logo;
	crayon_sprite_array_t Logo_Draw;
	crayon_palette_t Logo_P;

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

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	crayon_memory_init_sprite_array(&Logo_Draw, 1, 1, 0, 0, 0, 0, 0, 0, 0, &Logo, &Logo.animation_array[0], &Logo_P);
	Logo_Draw.positions[0] = 0;
	Logo_Draw.positions[1] = 0;
	Logo_Draw.draw_z[0] = 2;
	Logo_Draw.scales[0] = 1;
	Logo_Draw.scales[1] = 1;
	Logo_Draw.flips[0] = 0;
	Logo_Draw.rotations[0] = 0;
	Logo_Draw.colours[0] = 0;
	Logo_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Logo_Draw.animation, Logo_Draw.frame_coord_map + 0, Logo_Draw.frame_coord_map + 1, 0);

	// crayon_memory_swap_colour(&Logo_P, 0xFFE7561F, 0xFF0000FF, 0);	//This will change the orange to a blue

	uint8_t end = 0;
	while(!end){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			if(st->buttons & CONT_START){
				end = 1;
			}
		MAPLE_FOREACH_END()

		crayon_graphics_setup_palette(&Logo_P);	//Could live outside the loop, but later we will change the palette when it hits the corners

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Logo_Draw, PVR_LIST_PT_POLY);
			// crayon_graphics_draw_untextured_poly(0, 0, 1, 640, 480, 0xFF008844, PVR_LIST_PT_POLY);
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Logo_Draw, 1, 1);

	return 0;
}
