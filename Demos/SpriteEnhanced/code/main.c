//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For region and htz stuff
#include <dc/flashrom.h>

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

	crayon_spritesheet_t Man, Opaque;
	crayon_sprite_array_t Man_Draw, Opaque_Blend_Draw, Opaque_Add_Draw;
	crayon_palette_t Man_P;

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
	crayon_memory_load_spritesheet(&Man, &Man_P, 0, "/files/Man.dtex");
	crayon_memory_load_spritesheet(&Opaque, NULL, -1, "/files/Opaque.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_sprite_array(&Man_Draw, &Man, &Man.animation_array[0], &Man_P, 1, 1, 0, 0);
	Man_Draw.pos[0] = 300;
	Man_Draw.pos[1] = 0;
	Man_Draw.layer[0] = 2;
	Man_Draw.scale[0] = 7;
	Man_Draw.scale[1] = 7;
	Man_Draw.flip[0] = 0;
	Man_Draw.rotation[0] = 0;
	// Man_Draw.colour[0] = 0xFFFFFFFF;
	Man_Draw.colour[0] = 0;
	Man_Draw.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(Man_Draw.animation, Man_Draw.frame_coord_map + 0, Man_Draw.frame_coord_map + 1, 0);

	crayon_memory_init_sprite_array(&Opaque_Blend_Draw, &Opaque, &Opaque.animation_array[0], NULL, 2, 1, (1 << 5), 0);
	Opaque_Blend_Draw.pos[0] = 0;
	Opaque_Blend_Draw.pos[1] = 0;
	Opaque_Blend_Draw.pos[2] = 100;
	Opaque_Blend_Draw.pos[3] = 0;
	Opaque_Blend_Draw.layer[0] = 1;
	Opaque_Blend_Draw.scale[0] = 12;
	Opaque_Blend_Draw.scale[1] = 12;
	Opaque_Blend_Draw.flip[0] = 0;
	Opaque_Blend_Draw.rotation[0] = 0;
	// Opaque_Blend_Draw.colour[0] = 0xFFFFFFFF;
	// Opaque_Blend_Draw.colour[1] = 0xFFFFFFFF;
	Opaque_Blend_Draw.colour[0] = 0;
	Opaque_Blend_Draw.colour[1] = 0;
	Opaque_Blend_Draw.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(Opaque_Blend_Draw.animation, Opaque_Blend_Draw.frame_coord_map + 0, Opaque_Blend_Draw.frame_coord_map + 1, 0);

	crayon_memory_init_sprite_array(&Opaque_Add_Draw, &Opaque, &Opaque.animation_array[0], NULL, 2, 1, (1 << 5) + (1 << 6), 0);
	Opaque_Add_Draw.pos[0] = 0;
	Opaque_Add_Draw.pos[1] = 100;
	Opaque_Add_Draw.pos[2] = 100;
	Opaque_Add_Draw.pos[3] = 100;
	Opaque_Add_Draw.layer[0] = 1;
	Opaque_Add_Draw.scale[0] = 12;
	Opaque_Add_Draw.scale[1] = 12;
	Opaque_Add_Draw.flip[0] = 0;
	Opaque_Add_Draw.rotation[0] = 0;
	// Opaque_Add_Draw.colour[0] = 0xFFFFFFFF;
	// Opaque_Add_Draw.colour[1] = 0xFFFFFFFF;
	Opaque_Add_Draw.colour[0] = 0;
	Opaque_Add_Draw.colour[1] = 0;
	Opaque_Add_Draw.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(Opaque_Add_Draw.animation, Opaque_Add_Draw.frame_coord_map + 0, Opaque_Add_Draw.frame_coord_map + 1, 0);

	while(1){
		pvr_wait_ready();

		crayon_graphics_setup_palette(&Man_P);

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw(&Opaque_Blend_Draw, PVR_LIST_OP_POLY, 0);
			crayon_graphics_draw(&Opaque_Add_Draw, PVR_LIST_OP_POLY, 0);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw(&Man_Draw, PVR_LIST_PT_POLY, 0);
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Man_Draw, 1, 1);
	crayon_memory_free_sprite_array(&Opaque_Blend_Draw, 0, 0);	//Won't free ss
	crayon_memory_free_sprite_array(&Opaque_Add_Draw, 1, 0);	//But this one will

	return 0;
}
