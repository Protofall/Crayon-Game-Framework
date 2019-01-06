//Crayon libraries
#include <crayon/memory.h>
#include <crayon/debug.h>
#include <crayon/graphics.h>

// #include <dc/maple.h>
// #include <dc/maple/controller.h> //For the "Press start to exit" thing

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
	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD care couldn't be mounted: %d", sdRes);
		}
	#endif

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB and we default to NTSC interlace
		vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
	}

	pvr_init(&pvr_params);

	crayon_spritesheet_t Dwarf, Enlarge;
	crayon_textured_array_t Dwarf_Draw, Enlarge_Draw;
	crayon_font_prop_t Tahoma;
	crayon_font_mono_t BIOS;
	crayon_palette_t Tahoma_P, BIOS_P;

	#if CRAYON_SD_MODE == 1
		crayon_memory_mount_romdisk("/sd/colourMod.img", "/files");
	#else
		crayon_memory_mount_romdisk("/cd/colourMod.img", "/files");
	#endif

	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, 0, "/files/Fonts/Tahoma_font.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/files/Fonts/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Enlarge, NULL, -1, "/files/Enlarge.dtex");
	crayon_memory_load_spritesheet(&Dwarf, NULL, -1, "/files/Dwarf.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	//Basic test to show valid_string checking. Top fails, bottom works
	// const char * string = " Weird littole 347 test strin\ng \thellow";
	const char * string = " Weird littole 347 test strin\ng hellow~3";
	if(crayon_graphics_valid_string(string, Tahoma.num_chars)){
		return 1;
	}

	crayon_memory_set_sprite_array(&Dwarf_Draw, 1, 1, 0, 0, 0, 0, 0, 0, 0, &Dwarf, &Dwarf.spritesheet_animation_array[0], NULL);
	Dwarf_Draw.positions[0] = 50;
	Dwarf_Draw.positions[1] = 50;
	Dwarf_Draw.draw_z[0] = 18;
	Dwarf_Draw.scales[0] = 1;	//Looks off on lxdream with higher scale
	Dwarf_Draw.scales[1] = 1;
	Dwarf_Draw.flips[0] = 0;
	Dwarf_Draw.rotations[0] = 0;
	Dwarf_Draw.colours[0] = 0;
	Dwarf_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Dwarf_Draw.animation, Dwarf_Draw.frame_coord_map + 0, Dwarf_Draw.frame_coord_map + 1, 0);

	crayon_memory_set_sprite_array(&Enlarge_Draw, 1, 1, 0, 0, 0, 0, 0, 0, 0, &Enlarge, &Enlarge.spritesheet_animation_array[0], NULL);
	Enlarge_Draw.positions[0] = 150;
	Enlarge_Draw.positions[1] = 50;
	Enlarge_Draw.draw_z[0] = 17;
	Enlarge_Draw.scales[0] = 8;
	Enlarge_Draw.scales[1] = 8;
	Enlarge_Draw.flips[0] = 0;
	Enlarge_Draw.rotations[0] = 0;
	Enlarge_Draw.colours[0] = 0;
	Enlarge_Draw.frame_coord_keys[0] = 0;
	crayon_graphics_frame_coordinates(Enlarge_Draw.animation, Enlarge_Draw.frame_coord_map + 0, Enlarge_Draw.frame_coord_map + 1, 0);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	while(1){
		pvr_wait_ready();
		pvr_scene_begin();

		crayon_graphics_setup_palette(&BIOS_P);
		crayon_graphics_setup_palette(&Tahoma_P);

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, PVR_LIST_PT_POLY);
			crayon_graphics_draw_text_prop(&Tahoma, PVR_LIST_PT_POLY, 50, 150, 30, 1, 1, Tahoma_P.palette_id, "Tahoma\0");
			crayon_graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, 50, 170, 30, 1, 1, BIOS_P.palette_id, "BIOS\0");

		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);

			crayon_graphics_draw_sprites(&Enlarge_Draw, PVR_LIST_OP_POLY);

		pvr_list_finish();

		pvr_scene_finish();

	}

	//Confirm everything was unloaded successfully (Should equal zero)
	// int retVal = 0;
	// retVal += memory_free_crayon_spritesheet(&Fade, 1);
	// retVal += memory_free_crayon_spritesheet(&Enlarge, 1);
	// retVal += memory_free_crayon_spritesheet(&Blanka, 1);
	// error_freeze("Free-ing result %d!\n", retVal);

	return 0;
}