//Crayon libraries
#include "../../Crayon/code/dreamcast/memory.h"
#include "../../Crayon/code/dreamcast/debug.h"
#include "../../Crayon/code/dreamcast/graphics.h"

// #include <dc/maple.h>
// #include <dc/maple/controller.h> //For the "Press start to exit" thing

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
	vid_set_mode(DM_640x480_VGA, PM_RGB565);

	pvr_init(&pvr_params);

	crayon_spritesheet_t Dwarf, Enlarge;
	crayon_textured_array_t Dwarf_Draw, Enlarge_Draw;
	crayon_font_prop_t Tahoma;
	crayon_font_mono_t BIOS;
	crayon_palette_t Tahoma_P, BIOS_P;

	crayon_memory_mount_romdisk("/cd/colourMod.img", "/files");

	crayon_memory_load_prop_font_sheet(&Tahoma, &Tahoma_P, 0, "/files/Fonts/Tahoma_font.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/files/Fonts/BIOS_font.dtex");
	crayon_memory_load_spritesheet(&Enlarge, NULL, -1, "/files/Enlarge.dtex");
	crayon_memory_load_spritesheet(&Dwarf, NULL, -1, "/files/Dwarf.dtex");

	fs_romdisk_unmount("/files");

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
	graphics_frame_coordinates(Dwarf_Draw.animation, Dwarf_Draw.frame_coord_map + 0, Dwarf_Draw.frame_coord_map + 1, 0);

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
	graphics_frame_coordinates(Enlarge_Draw.animation, Enlarge_Draw.frame_coord_map + 0, Enlarge_Draw.frame_coord_map + 1, 0);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	while(1){
		pvr_wait_ready();
		pvr_scene_begin();

		graphics_setup_palette(&BIOS_P);
		graphics_setup_palette(&Tahoma_P);

		pvr_list_begin(PVR_LIST_PT_POLY);

			crayon_graphics_draw_sprites(&Dwarf_Draw, PVR_LIST_PT_POLY);
			graphics_draw_text_prop(&Tahoma, PVR_LIST_PT_POLY, 50, 150, 30, 1, 1, Tahoma_P.palette_id, "Tahoma\0");
			graphics_draw_text_mono(&BIOS, PVR_LIST_PT_POLY, 50, 170, 30, 1, 1, BIOS_P.palette_id, "BIOS\0");

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