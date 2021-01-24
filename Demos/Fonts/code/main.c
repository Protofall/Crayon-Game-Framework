// Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>

// For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){
	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_OPTICAL)){
		return 1;
	}

	crayon_font_mono_t BIOS;
	crayon_font_prop_t Tahoma;
	crayon_palette_t BIOS_P, Tahoma_P;

	// Load the stuff
	crayon_memory_mount_romdisk("stuff.img", "/files", CRAYON_ADD_BASE_PATH);

	// Load the asset
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, "/files/BIOS.dtex",
		CRAYON_USE_EXACT_PATH, 0);
	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, "/files/Tahoma.dtex",
		CRAYON_USE_EXACT_PATH, 1);

	fs_romdisk_unmount("/files");

	char *version = crayon_misc_get_version();
	char version_msg[60];
	strcpy(version_msg, "Crayon version number: ");
	strcat(version_msg, version);
	free(version);

	crayon_graphics_set_bg_colour(0.3, 0.3, 0.3);

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

	crayon_shutdown();

	return 0;
}
