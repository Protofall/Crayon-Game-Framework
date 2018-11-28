#include <kos.h>	//Replace this with vital only libs

// #include <dc/maple/vmu.h>
// #include <dc/vmu_pkg.h>
// #include <kos/fs.h>

#include <zlib/zlib.h>	//For the length of the text file in the gz archive

#include "graphics.h"
#include "memory.h"
#include "savefile.h"

// Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void){

	pvr_init_defaults();		// Init kos

	//Load in the fontsheet
	crayon_font_mono_t wfont;
	crayon_palette_t wfont_p;
	crayon_memory_load_mono_font_sheet(&wfont, &wfont_p, 0, "/rd/font.dtex");

	save.var1 = 0;
	save.var2 = 41;

	file_t file;

	file = fs_open("/rd/IMAGE.BIN", O_RDONLY);
	int filesize = fs_total(file);
	icon = (unsigned char *) malloc(filesize);
	fs_read(file, icon, filesize);
	fs_close(file);

	file = fs_open("/rd/PALLETTE.BIN", O_RDONLY);
	int filesize2 = fs_total(file);
	palette = (unsigned short *) malloc(filesize2);
	fs_read(file, palette, filesize2);
	fs_close(file);

	uint8_t state = 0;
	uint32_t previous_buttons = 0;
	char buffer[200];

	while(1){
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

		if(st->buttons & CONT_START){	// Press start to quit
			break;
		}

		if((previous_buttons & CONT_A) && !(st->buttons & CONT_A)){
			save.var1++;
			save_uncompressed(0, 1);	//Controller port A, port 1 (front)
			state = 1;
		}
		else if((previous_buttons & CONT_B) && !(st->buttons & CONT_B)){
			save.var1++;
			save_compressed(0, 1);	//Controller port A, port 1 (front)
			state = 2;
		}
		else if((previous_buttons & CONT_X) && !(st->buttons & CONT_X)){
			if(load_uncompressed(0, 1) >= 0){	//Controller port A, port 1 (front)
				state = 3;
			}
		}
		else if((previous_buttons & CONT_Y) && !(st->buttons & CONT_Y)){
			if(load_compressed(0, 1) >= 0){	//Controller port A, port 1 (front)
				state = 4;
			}
		}

		previous_buttons = st->buttons;

		MAPLE_FOREACH_END()

		pvr_wait_ready();
		pvr_scene_begin();

		graphics_setup_palette(&wfont_p);

		pvr_list_begin(PVR_LIST_OP_POLY);
		
			graphics_draw_text_mono(&wfont, PVR_LIST_OP_POLY, 30, 30, 10, 1, 1, wfont_p.palette_id,
				"Press A to make an uncompressed save\nPress B to make a compressed save\nPress X to load an uncompressed save\nPress Y to load a compressed save\0");
			if(state == 1){
				graphics_draw_text_mono(&wfont, PVR_LIST_OP_POLY, 30, 100, 10, 1, 1, wfont_p.palette_id, "Uncompressed save complete\0");
			}
			else if(state == 2){
				graphics_draw_text_mono(&wfont, PVR_LIST_OP_POLY, 30, 100, 10, 1, 1, wfont_p.palette_id, "Compressed save complete\0");
			}
			else if(state == 3){
				sprintf(buffer, "Uncompressed load complete.\nSave has been saved \"%lu\" times\nMagic number is %d", save.var1, save.var2);
				graphics_draw_text_mono(&wfont, PVR_LIST_OP_POLY, 30, 100, 10, 1, 1, wfont_p.palette_id, buffer);
			}
			else if(state == 4){
				sprintf(buffer, "Compressed load complete.\nSave has been saved \"%lu\" times\nMagic number is %d", save.var1, save.var2);
				graphics_draw_text_mono(&wfont, PVR_LIST_OP_POLY, 30, 100, 10, 1, 1, wfont_p.palette_id, buffer);
			}

		pvr_list_finish();

		pvr_scene_finish();

	}

	free(icon);
	free(palette);
	crayon_memory_free_mono_font_sheet(&wfont);
	crayon_memory_free_palette(&wfont_p);

	return 0;
}

// A more in-depth version here
// http://dcemulation.org/phpBB/viewtopic.php?f=34&t=103666&p=1047688&hilit=vmu+save+file#p1047688

// OLD STUFF BELOW, IGNORE



// int DC_CompressedLoadFromVMU(char *dst){
// 	char src[32];
// 	int file;
// 	int filesize;
// 	unsigned long unzipsize;
// 	uint8* data;
// 	uint8* unzipdata;
// 	vmu_pkg_t pkg;

// 	// Our VMU + full save name
// 	strcpy(src, "/vmu/a1/");
// 	strcat(src, dst);

// 	// Remove VMU header
// 	file = fs_open(src, O_RDONLY);
// 	if(file == 0) return -1;
// 	filesize = fs_total(file);
// 	if(filesize <= 0) return -1;
// 	data = (uint8*)malloc(filesize);
// 	fs_read(file, data, filesize);
// 	vmu_pkg_parse(data, &pkg);
// 	fs_close(file);

// 	// Allocate memory for the uncompressed data
// 	unzipdata = (uint8 *)malloc(65536);
// 	unzipsize = 65536;

// 	// Uncompress the data to the CWD
// 	uncompress(unzipdata, &unzipsize, (uint8 *)pkg.data, pkg.data_len);
// 	fs_unlink(dst);
// 	file = fs_open(dst, O_WRONLY);
// 	fs_write(file, unzipdata, unzipsize);
// 	fs_close(file);

// 	// Free unused memory
// 	free(data);
// 	free(unzipdata);
// 	return 0;
// }