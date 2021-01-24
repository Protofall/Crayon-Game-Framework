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

	crayon_spritesheet_t Ball, Ball2;
	crayon_sprite_array_t Ball_Draw, Ball2_Draw;
	crayon_font_mono_t BIOS;
	crayon_palette_t Ball_P, BIOS_P;

	// Load the circles
	crayon_memory_mount_romdisk("stuff.img", "/files", CRAYON_ADD_BASE_PATH);

	// Load the asset
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, "/files/BIOS_font.dtex",
		CRAYON_USE_EXACT_PATH, 0);
	crayon_memory_load_spritesheet(&Ball, &Ball_P, "/files/logo.dtex",
		CRAYON_USE_EXACT_PATH, 1);
	crayon_memory_load_spritesheet(&Ball2, NULL, "/files/logo2.dtex",
		CRAYON_USE_EXACT_PATH, 1);

	fs_romdisk_unmount("/files");

	crayon_memory_init_sprite_array(&Ball_Draw, &Ball, 0, &Ball_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Ball_Draw.coord[0].x = 0;
	Ball_Draw.coord[0].y = (480 - Ball.animation[0].frame_height) / 2;
	Ball_Draw.layer[0] = 2;
	Ball_Draw.scale[0].x = 1;
	Ball_Draw.scale[0].y = 1;
	Ball_Draw.flip[0] = 0;
	Ball_Draw.rotation[0] = 0;
	Ball_Draw.colour[0] = 0;
	Ball_Draw.frame_id[0] = 0;
	Ball_Draw.visible[0] = 1;
	crayon_memory_set_frame_uv(&Ball_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Ball2_Draw, &Ball2, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	Ball2_Draw.coord[0].x = (640 - Ball2.animation[0].frame_width);
	Ball2_Draw.coord[0].y = (480 - Ball2.animation[0].frame_height) / 2;
	Ball2_Draw.layer[0] = 2;
	Ball2_Draw.scale[0].x = 1;
	Ball2_Draw.scale[0].y = 1;
	Ball2_Draw.flip[0] = 0;
	Ball2_Draw.rotation[0] = 0;
	Ball2_Draw.colour[0] = 0;
	Ball2_Draw.frame_id[0] = 0;
	Ball2_Draw.visible[0] = 1;
	crayon_memory_set_frame_uv(&Ball2_Draw, 0, 0);

	crayon_graphics_setup_palette(&BIOS_P);
	crayon_graphics_setup_palette(&Ball_P);
	while(1){
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Ball_Draw, NULL, PVR_LIST_PT_POLY, CRAYON_DRAW_SIMPLE);	// Broken on most emulators, but not hardware
			crayon_graphics_draw_sprites(&Ball2_Draw, NULL, PVR_LIST_PT_POLY, CRAYON_DRAW_SIMPLE);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_text_mono("PAL8BPP", &BIOS, PVR_LIST_OP_POLY, 50, 32, 3, 2, 2, BIOS_P.palette_id);
			crayon_graphics_draw_text_mono("ARGB1555", &BIOS, PVR_LIST_OP_POLY, 450, 32, 3, 2, 2, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();
	}

	// Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Ball_Draw);
	crayon_memory_free_sprite_array(&Ball2_Draw);

	crayon_memory_free_spritesheet(&Ball);
	crayon_memory_free_mono_font_sheet(&BIOS);

	crayon_memory_free_palette(&Ball_P);
	crayon_memory_free_palette(&BIOS_P);

	crayon_shutdown();

	return 0;
}
