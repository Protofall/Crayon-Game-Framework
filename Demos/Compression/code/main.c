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
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	float htz_adjustment;
	set_screen(&htz_adjustment);

	srand(time(0));	//Set the seed for rand()

	crayon_spritesheet_t Ball, Ball2;
	crayon_sprite_array_t Ball_Draw, Ball2_Draw;
	crayon_font_mono_t BIOS;
	crayon_palette_t Ball_P, BIOS_P;

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
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 0, "/files/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Ball, &Ball_P, 1, "/files/logo.dtex");
	crayon_memory_load_spritesheet(&Ball2, NULL, 1, "/files/logo2.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_sprite_array(&Ball_Draw, &Ball, 0, &Ball_P, 1, 1, 0, 0);
	Ball_Draw.coord[0].x = 0;
	Ball_Draw.coord[0].y = (480 - Ball.animation[0].frame_height) / 2;
	Ball_Draw.layer[0] = 2;
	Ball_Draw.scale[0].x = 1;
	Ball_Draw.scale[0].y = 1;
	Ball_Draw.flip[0] = 0;
	Ball_Draw.rotation[0] = 0;
	Ball_Draw.colour[0] = 0;
	Ball_Draw.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(&Ball_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Ball2_Draw, &Ball2, 0, NULL, 1, 1, 0, 0);
	Ball2_Draw.coord[0].x = (640 - Ball2.animation[0].frame_width);
	Ball2_Draw.coord[0].y = (480 - Ball2.animation[0].frame_height) / 2;
	Ball2_Draw.layer[0] = 2;
	Ball2_Draw.scale[0].x = 1;
	Ball2_Draw.scale[0].y = 1;
	Ball2_Draw.flip[0] = 0;
	Ball2_Draw.rotation[0] = 0;
	Ball2_Draw.colour[0] = 0;
	Ball2_Draw.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(&Ball2_Draw, 0, 0);

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Ball_P);
	while(1){
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Ball_Draw, PVR_LIST_PT_POLY, CRAY_SCREEN_DRAW_SIMPLE);	//Broken on most emulators, but not hardware
			crayon_graphics_draw_sprites(&Ball2_Draw, PVR_LIST_PT_POLY, CRAY_SCREEN_DRAW_SIMPLE);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_text_mono("PAL8BPP", &BIOS, PVR_LIST_OP_POLY, 50, 32, 3, 2, 2, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono("ARGB1555", &BIOS, PVR_LIST_OP_POLY, 450, 32, 3, 2, 2, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Ball_Draw);
	crayon_memory_free_sprite_array(&Ball2_Draw);

	crayon_memory_free_spritesheet(&Ball);
	crayon_memory_free_mono_font_sheet(&BIOS);

	crayon_memory_free_palette(&Ball_P);
	crayon_memory_free_palette(&BIOS_P);

	return 0;
}
