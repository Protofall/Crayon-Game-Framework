//Crayon libraries
#include <crayon/memory.h>

//For the controller and mouse
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include <dc/pvr.h>

#include "extra_structs.h"
#include "setup.h"

#define CRAYON_DEBUG 0

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

pvr_ptr_t font_tex;
uint8_t error = 0;

//Init font
void font_init(){
	int i, x, y, c;
	unsigned short * temp_tex;

	font_tex = pvr_mem_malloc(256 * 256 * 2);	//16bpp (ARGB4444 Mode), 256 by 256 texture.
												//We must allocate 2^n by 2^n space because of hardware limitations.

	temp_tex = (unsigned short *)malloc(256 * 128 * 2);	//We can do *any* size for temp stuff (Draws into top half of texture)

	//Load the file into memory (Well, we have a baked-in romdisk...but still fine anyways)
	uint16_t header_size = 265;
	uint16_t file_size = 4361;
	#if CRAYON_SD_MODE == 1
		FILE * texture_file = fopen("sd/fixed-fiction.pbm", "rb");
	#else
		FILE * texture_file = fopen("cd/fixed-fiction.pbm", "rb");
	#endif
	if(!texture_file){error |= (1 << 0);}
	fseek(texture_file, header_size, SEEK_SET);	//Move file pointer forwards 0x109 or 265 bytes
	char * wfont = (char *)malloc(file_size - header_size);
	if(!wfont){error |= (1 << 1);}
	fread(wfont, (file_size - header_size), 1, texture_file);
	fclose(texture_file);

	c = 0;
	for(y = 0; y < 128 ; y += 16){
		for(x = 0; x < 256 ; x += 8){
			for(i = 0; i < 16; i++){
				temp_tex[x + (y + i) * 256 + 0] = 0xffff * ((wfont[c + i] & 0x80) >> 7);	//0xffff is white pixel
				temp_tex[x + (y + i) * 256 + 1] = 0xffff * ((wfont[c + i] & 0x40) >> 6);	//0x0000 is black pixel in texture
				temp_tex[x + (y + i) * 256 + 2] = 0xffff * ((wfont[c + i] & 0x20) >> 5);	//1 bit in wfont.bin = 1 pixel
				temp_tex[x + (y + i) * 256 + 3] = 0xffff * ((wfont[c + i] & 0x10) >> 4);
				temp_tex[x + (y + i) * 256 + 4] = 0xffff * ((wfont[c + i] & 0x08) >> 3);
				temp_tex[x + (y + i) * 256 + 5] = 0xffff * ((wfont[c + i] & 0x04) >> 2);
				temp_tex[x + (y + i) * 256 + 6] = 0xffff * ((wfont[c + i] & 0x02) >> 1);
				temp_tex[x + (y + i) * 256 + 7] = 0xffff * (wfont[c + i] & 0x01);
			}

			c += 16;
		}
	}

	pvr_txr_load_ex(temp_tex, font_tex, 256, 256, PVR_TXRLOAD_16BPP);	//Texture load 16bpp

	free(wfont);
	free(temp_tex);
}

void draw_char(float x1, float y1, float z1, uint8_t a, uint8_t r, uint8_t g, uint8_t b, int c, float xs, float ys){
	pvr_vertex_t    vert;
	int             ix, iy;
	float           u1, v1, u2, v2;

	ix = (c % 32) * 8;	//C is the character to draw
	iy = (c / 32) * 16;
	u1 = (ix + 0.5f) * 1.0f / 256.0f;
	v1 = (iy + 0.5f) * 1.0f / 256.0f;
	u2 = (ix + 7.5f) * 1.0f / 256.0f;
	v2 = (iy + 15.5f) * 1.0f / 256.0f;

	vert.flags = PVR_CMD_VERTEX;
	vert.x = x1;
	vert.y = y1 + 16.0f * ys;
	vert.z = z1;
	vert.u = u1;
	vert.v = v2;
	vert.argb = (a << 24) + (r << 16) + (g << 8) + b;
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x1;
	vert.y = y1;
	vert.u = u1;
	vert.v = v1;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x1 + 8.0f * xs;
	vert.y = y1 + 16.0f * ys;
	vert.u = u2;
	vert.v = v2;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.x = x1 + 8.0f * xs;
	vert.y = y1;
	vert.u = u2;
	vert.v = v1;
	pvr_prim(&vert, sizeof(vert));
}

//Draw a string
//x, y, "argb" params and str are self explanatory. xs and ys are kind like scaling/how large to make it
//Keep in mind this works best with xs/xy == 2, smaller spaces the lines too far apart and too much doesn't space them enough
void draw_string(float x, float y, float z, uint8_t a, uint8_t r, uint8_t g, uint8_t b, char *str, float xs, float ys){
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	float orig_x = x;

	pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, 256, 256, font_tex, PVR_FILTER_NONE);	//Draws characters in ARGB4444 mode
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	while(*str){
		if(*str == '\n'){
			x = orig_x;
			y += 40;
			str++;
			continue;
		}

		draw_char(x, y, z, a, r, g, b, *str++, xs, ys);
		x += 8 * xs;
	}
}

int main(){
	#if CRAYON_SD_MODE == 1
		int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
		if(sdRes == 0){
			MS_options.sd_present = 1;
		}
		else{
			return 0;
		}
	#endif

	crayon_savefile_details_t savefile_details;
	minesweeper_savefile_t savefile;
	crayon_savefile_init_savefile_details(&savefile_details, (uint8_t *)&savefile,
		sizeof(minesweeper_savefile_t), 3, 15, "Crayon's VMU demo\0", "Save Demo\0",
		"ProtoSaveDemo2\0", "SAVE_DEMO.s\0");

	//Load the VMU icon data
	#if CRAYON_SD_MODE == 1
		crayon_memory_mount_romdisk("/sd/sf_icon.img", "/Save");
	#else
		crayon_memory_mount_romdisk("/cd/sf_icon.img", "/Save");
	#endif

	uint8_t * vmu_lcd_icon = NULL;
	setup_vmu_icon_load(&vmu_lcd_icon, "/Save/LCD.bin");

	
	crayon_savefile_load_icon(&savefile_details, "/Save/image.bin", "/Save/palette.bin");
	uint8_t res = crayon_savefile_load_eyecatch(&savefile_details, "/Save/eyecatch3.bin");	//Must be called AFTER init

	fs_romdisk_unmount("/SaveFile");

	//Apply the VMU LCD icon (Apparently this is automatic if your savefile is an ICONDATA.VMS)
	setup_vmu_icon_apply(vmu_lcd_icon, savefile_details.valid_vmu_screens);
	// free(vmu_lcd_icon);	//Already free-d within the above function

	//Find the first savefile (if it exists)
	int iter;
	int jiter;
	for(iter = 0; iter <= 3; iter++){
		for(jiter = 1; jiter <= 2; jiter++){
			if(crayon_savefile_get_vmu_bit(savefile_details.valid_saves, iter, jiter)){	//Use the left most VMU
				savefile_details.savefile_port = iter;
				savefile_details.savefile_slot = jiter;
				goto Exit_loop_1;
			}
		}
	}
	Exit_loop_1:

	//Try and load savefile
	crayon_savefile_load(&savefile_details);

	//No savefile yet
	if(savefile_details.valid_memcards && savefile_details.savefile_port == -1 &&
		savefile_details.savefile_slot == -1){
		//If we don't already have a savefile, choose a VMU
		if(savefile_details.valid_memcards){
			for(iter = 0; iter <= 3; iter++){
				for(jiter = 1; jiter <= 2; jiter++){
					if(crayon_savefile_get_vmu_bit(savefile_details.valid_memcards, iter, jiter)){	//Use the left most VMU
						savefile_details.savefile_port = iter;
						savefile_details.savefile_slot = jiter;
						goto Exit_loop_2;
					}
				}
			}
		}
		Exit_loop_2:
		;
	}

	uint16_t save_res = 0;
	if(savefile_details.valid_memcards){
		save_res = crayon_savefile_save(&savefile_details);
		savefile_details.valid_saves = crayon_savefile_get_valid_saves(&savefile_details);
	}

	pvr_init_defaults();	//Init kos
	font_init();

	#if CRAYON_SD_MODE == 1
		unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	char buffer[70];
	if(!res){
	sprintf(buffer, "Save created\nUses %d blocks and has %d frames of\nanimation",
		crayon_savefile_get_save_block_count(&savefile_details),
		savefile_details.icon_anim_count);
	}
	else{
		sprintf(buffer, "It failed with code %d", res);
	}

	uint8_t end = 0;
	while(!end){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			if(st->buttons & CONT_START){
				end = 1;
			}
		MAPLE_FOREACH_END()

		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);
			switch(save_res){
			case 0:
				draw_string(30, 30, 1, 255, 255, 216, 0, &buffer, 2, 2); break;
			case 1:
				draw_string(30, 30, 1, 255, 255, 216, 0, "Selected device isn't a VMU", 2, 2); break;
			case 2:
				draw_string(30, 30, 1, 255, 255, 216, 0, "Selected VMU doesn't have enough space", 2, 2); break;
			case 3:
				draw_string(30, 30, 1, 255, 255, 216, 0, "Ran out of memory when making savefile", 2, 2); break;
			case 4:
				draw_string(30, 30, 1, 255, 255, 216, 0, "Not enough space on VMU for savefile", 2, 2); break;
			case 5:
				draw_string(30, 30, 1, 255, 255, 216, 0, "Couldn't access savefile on VMU", 2, 2); break;
			case 6:
				draw_string(30, 30, 1, 255, 255, 216, 0, "Couldn't write to VMU", 2, 2); break;
		}
		pvr_list_finish();


		pvr_scene_finish();
	}

	crayon_savefile_free_icon(&savefile_details);
	crayon_savefile_free_eyecatch(&savefile_details);
	pvr_mem_free(font_tex);

	return 0;
}
