//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For region and htz stuff
#include <dc/flashrom.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

#if CRAYON_BOOT_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <fat/fs_fat.h>
#endif


#if CRAYON_BOOT_MODE == 1
	#define MNT_MODE FS_FAT_MOUNT_READONLY

	static void unmount_fat_sd(){
		fs_fat_unmount("/sd");
		fs_fat_shutdown();
		sd_shutdown();
	}

	static int mount_fat_sd(){
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
			//I don't know what value I should be comparing against, hence this check is disabled for now
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

void set_screen(float * htz_adjustment){
	*htz_adjustment = 1.0;
	uint8_t region = flashrom_get_region();
	if(region < 0){	//If error we just default to green swirl. Apparently its possible for some DCs to return -1
		region = 0;	//Invalid region
	}

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		if(region == FLASHROM_REGION_EUROPE){
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
			*htz_adjustment = 1.2;	//60/50Hz
		}
		else{
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
		}
	}

	return;
}

int main(){
	pvr_init(&pvr_params);	//Init the pvr system

	#if CRAYON_BOOT_MODE == 1
		int sdRes = mount_fat_sd();	//This function should be able to mount a FAT32 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	float htz_adjustment;
	set_screen(&htz_adjustment);

	srand(time(0));	//Set the seed for rand()

	crayon_font_mono_t BIOS;
	crayon_font_prop_t Tahoma;
	crayon_palette_t BIOS_P, Tahoma_P;

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
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 0, "/files/BIOS.dtex");
	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, 1, "/files/Tahoma.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	char * version = crayon_misc_get_version();
	char version_msg[60];
	strcpy(version_msg, "Crayon version number: ");
	strcat(version_msg, version);
	free(version);

	pvr_set_bg_color(0.3, 0.3, 0.3);

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Tahoma_P);
	while(1){
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_text_prop("Tahoma", &Tahoma, PVR_LIST_PT_POLY, 32, 32, 1, 1, 1, Tahoma_P.palette_id);

			Tahoma.char_spacing.x = 16;
			Tahoma.char_spacing.y = 32;
			crayon_graphics_draw_text_prop("Here's another one\nBut this one is proportional\nStrange, isn't it?", &Tahoma, PVR_LIST_PT_POLY, 32, 332, 1, 1, 1, Tahoma_P.palette_id);
			Tahoma.char_spacing.x = 0;
			Tahoma.char_spacing.y = 0;

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_text_mono("BIOS", &BIOS, PVR_LIST_OP_POLY, 32, Tahoma.char_height + 32, 1, 1, 1, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono(version_msg, &BIOS, PVR_LIST_OP_POLY, 32, Tahoma.char_height + 32 + BIOS.char_height, 1, 1, 1,
				BIOS_P.palette_id);

			BIOS.char_spacing.x = 8;
			BIOS.char_spacing.y = 8;
			crayon_graphics_draw_text_mono("Modified spacing\nThis is a multi-line string\nFire at William", &BIOS, PVR_LIST_OP_POLY, 32,
				Tahoma.char_height + 32 + (2 * BIOS.char_height), 1, 1, 1, BIOS_P.palette_id);
			BIOS.char_spacing.x = 0;
			BIOS.char_spacing.y = 0;

		pvr_list_finish();

		pvr_scene_finish();
	}

	crayon_memory_free_mono_font_sheet(&BIOS);
	crayon_memory_free_prop_font_sheet(&Tahoma);

	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_palette(&Tahoma_P);

	return 0;
}
