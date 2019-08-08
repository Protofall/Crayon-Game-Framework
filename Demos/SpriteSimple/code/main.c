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

	crayon_spritesheet_t Dwarf_SS;
	crayon_draw_array_t Dwarf_Draw_Flip, Dwarf_Draw_Rotate, Dwarf_Draw_Scale, Dwarf_Draw_Frame, Dwarf_Draw_Mash;
	crayon_font_mono_t BIOS;
	crayon_palette_t BIOS_P;

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
	crayon_memory_load_spritesheet(&Dwarf_SS, NULL, -1, "/files/sprite.dtex");

	fs_romdisk_unmount("/files");

	#if CRAYON_BOOT_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	crayon_memory_init_draw_array(&Dwarf_Draw_Flip, &Dwarf_SS, 0, NULL, 2, 1, CRAY_MULTI_FLIP, PVR_FILTER_NONE);
	Dwarf_Draw_Flip.scale[0].x = 1;
	Dwarf_Draw_Flip.scale[0].y = 1;

	Dwarf_Draw_Flip.coord[0].x = 32;
	Dwarf_Draw_Flip.coord[0].y = 32;
	Dwarf_Draw_Flip.coord[1].x = Dwarf_Draw_Flip.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Flip, 0) + 32;
	Dwarf_Draw_Flip.coord[1].y = Dwarf_Draw_Flip.coord[0].y;

	Dwarf_Draw_Flip.layer[0] = 2;
	Dwarf_Draw_Flip.flip[0] = 0;
	Dwarf_Draw_Flip.flip[1] = 1;
	Dwarf_Draw_Flip.rotation[0] = 0;
	Dwarf_Draw_Flip.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Flip.fade[0] = 0;
	Dwarf_Draw_Flip.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Flip, 0, 0);

	crayon_memory_init_draw_array(&Dwarf_Draw_Rotate, &Dwarf_SS, 0, NULL, 4, 1, CRAY_MULTI_ROTATE, PVR_FILTER_NONE);
	Dwarf_Draw_Rotate.scale[0].x = 1;
	Dwarf_Draw_Rotate.scale[0].y = 1;

	Dwarf_Draw_Rotate.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Rotate.coord[0].y = Dwarf_Draw_Flip.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Flip, 0) + 32;
	Dwarf_Draw_Rotate.coord[1].x = Dwarf_Draw_Rotate.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Rotate, 0) + 32;
	Dwarf_Draw_Rotate.coord[1].y = Dwarf_Draw_Rotate.coord[0].y;
	Dwarf_Draw_Rotate.coord[2].x = Dwarf_Draw_Rotate.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Rotate, 0) + 32;
	Dwarf_Draw_Rotate.coord[2].y = Dwarf_Draw_Rotate.coord[1].y;
	Dwarf_Draw_Rotate.coord[3].x = Dwarf_Draw_Rotate.coord[2].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Rotate, 0) + 32;
	Dwarf_Draw_Rotate.coord[3].y = Dwarf_Draw_Rotate.coord[2].y;

	Dwarf_Draw_Rotate.layer[0] = 2;
	Dwarf_Draw_Rotate.flip[0] = 0;
	Dwarf_Draw_Rotate.rotation[0] = 0;
	Dwarf_Draw_Rotate.rotation[1] = 90;
	Dwarf_Draw_Rotate.rotation[2] = 180;
	Dwarf_Draw_Rotate.rotation[3] = 270;
	Dwarf_Draw_Rotate.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Rotate.fade[0] = 0;
	Dwarf_Draw_Rotate.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Rotate, 0, 0);

	crayon_memory_init_draw_array(&Dwarf_Draw_Scale, &Dwarf_SS, 0, NULL, 3, 1, CRAY_MULTI_SCALE, PVR_FILTER_NONE);
	Dwarf_Draw_Scale.scale[0].x = 1;
	Dwarf_Draw_Scale.scale[0].y = 1;
	Dwarf_Draw_Scale.scale[1].x = 0.5;
	Dwarf_Draw_Scale.scale[1].y = 0.5;
	Dwarf_Draw_Scale.scale[2].x = 2;
	Dwarf_Draw_Scale.scale[2].y = 2;

	Dwarf_Draw_Scale.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Scale.coord[0].y = Dwarf_Draw_Rotate.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Scale, 0) + 32;
	Dwarf_Draw_Scale.coord[1].x = Dwarf_Draw_Scale.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Scale, 0) + 32;
	Dwarf_Draw_Scale.coord[1].y = Dwarf_Draw_Scale.coord[0].y;
	Dwarf_Draw_Scale.coord[2].x = Dwarf_Draw_Scale.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Scale, 0);
	Dwarf_Draw_Scale.coord[2].y = Dwarf_Draw_Scale.coord[0].y;

	Dwarf_Draw_Scale.layer[0] = 2;
	Dwarf_Draw_Scale.flip[0] = 0;
	Dwarf_Draw_Scale.rotation[0] = 0;
	Dwarf_Draw_Scale.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Scale.fade[0] = 0;
	Dwarf_Draw_Scale.frame_coord_key[0] = 0;
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Scale, 0, 0);

	crayon_memory_init_draw_array(&Dwarf_Draw_Frame, &Dwarf_SS, 0, NULL, 3, 2, CRAY_MULTI_FRAME, PVR_FILTER_NONE);
	Dwarf_Draw_Frame.scale[0].x = 1;
	Dwarf_Draw_Frame.scale[0].y = 1;

	Dwarf_Draw_Frame.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Frame.coord[0].y = Dwarf_Draw_Scale.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Frame, 0) + 32 + 6;	//6 because of scale over-stretching
	Dwarf_Draw_Frame.coord[1].x = Dwarf_Draw_Frame.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Frame, 0) + 32;
	Dwarf_Draw_Frame.coord[1].y = Dwarf_Draw_Frame.coord[0].y;
	Dwarf_Draw_Frame.coord[2].x = Dwarf_Draw_Frame.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Frame, 0) + 32;
	Dwarf_Draw_Frame.coord[2].y = Dwarf_Draw_Frame.coord[0].y;

	Dwarf_Draw_Frame.layer[0] = 2;
	Dwarf_Draw_Frame.flip[0] = 0;
	Dwarf_Draw_Frame.rotation[0] = 0;
	Dwarf_Draw_Frame.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Frame.fade[0] = 0;
	Dwarf_Draw_Frame.frame_coord_key[0] = 0;
	Dwarf_Draw_Frame.frame_coord_key[1] = 1;
	Dwarf_Draw_Frame.frame_coord_key[2] = 0;
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Frame, 0, 0);
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Frame, 1, 1);

	crayon_memory_init_draw_array(&Dwarf_Draw_Mash, &Dwarf_SS, 0, NULL, 6, 2, CRAY_MULTI_FRAME + CRAY_MULTI_FLIP + CRAY_MULTI_ROTATE, PVR_FILTER_NONE);
	Dwarf_Draw_Mash.scale[0].x = 1;
	Dwarf_Draw_Mash.scale[0].y = 1;

	Dwarf_Draw_Mash.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Mash.coord[0].y = Dwarf_Draw_Frame.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[1].x = Dwarf_Draw_Mash.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[1].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[2].x = Dwarf_Draw_Mash.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[2].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[3].x = Dwarf_Draw_Mash.coord[2].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[3].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[4].x = Dwarf_Draw_Mash.coord[3].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[4].y = Dwarf_Draw_Mash.coord[0].y;
	Dwarf_Draw_Mash.coord[5].x = Dwarf_Draw_Mash.coord[4].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Mash, 0) + 32;
	Dwarf_Draw_Mash.coord[5].y = Dwarf_Draw_Mash.coord[0].y;

	Dwarf_Draw_Mash.layer[0] = 2;

	Dwarf_Draw_Mash.flip[0] = 0;
	Dwarf_Draw_Mash.flip[1] = 1;
	Dwarf_Draw_Mash.flip[2] = 0;
	Dwarf_Draw_Mash.flip[3] = 1;
	Dwarf_Draw_Mash.flip[4] = 1;
	Dwarf_Draw_Mash.flip[5] = 0;

	Dwarf_Draw_Mash.rotation[0] = 0;
	Dwarf_Draw_Mash.rotation[1] = 167;
	Dwarf_Draw_Mash.rotation[2] = 51;
	Dwarf_Draw_Mash.rotation[3] = -113;
	Dwarf_Draw_Mash.rotation[4] = 218;
	Dwarf_Draw_Mash.rotation[5] = 299;

	Dwarf_Draw_Mash.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Mash.fade[0] = 0;

	Dwarf_Draw_Mash.frame_coord_key[0] = 0;
	Dwarf_Draw_Mash.frame_coord_key[1] = 1;
	Dwarf_Draw_Mash.frame_coord_key[2] = 1;
	Dwarf_Draw_Mash.frame_coord_key[3] = 1;
	Dwarf_Draw_Mash.frame_coord_key[4] = 0;
	Dwarf_Draw_Mash.frame_coord_key[5] = 1;
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Mash, 0, 0);
	crayon_graphics_frame_coordinates(&Dwarf_Draw_Mash, 1, 1);

	crayon_graphics_setup_palette(&BIOS_P);
	while(1){
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw(&Dwarf_Draw_Flip, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw(&Dwarf_Draw_Rotate, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw(&Dwarf_Draw_Scale, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw(&Dwarf_Draw_Frame, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
			crayon_graphics_draw(&Dwarf_Draw_Mash, PVR_LIST_PT_POLY, CRAY_DRAW_SIMPLE);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_text_mono("Controls:", &BIOS, PVR_LIST_OP_POLY, 450, 32, 3, 2, 2, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();
	}

	//Also frees the spritesheet and palette
	crayon_memory_free_draw_array(&Dwarf_Draw_Flip);
	crayon_memory_free_draw_array(&Dwarf_Draw_Rotate);
	crayon_memory_free_draw_array(&Dwarf_Draw_Scale);
	crayon_memory_free_draw_array(&Dwarf_Draw_Frame);
	crayon_memory_free_draw_array(&Dwarf_Draw_Mash);

	crayon_memory_free_spritesheet(&Dwarf_SS);
	crayon_memory_free_mono_font_sheet(&BIOS);

	crayon_memory_free_palette(&BIOS_P);

	return 0;
}
