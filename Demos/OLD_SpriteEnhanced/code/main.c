// Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>

// For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){
	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		return 1;
	}

	crayon_spritesheet_t Man_SS, Opaque_SS;
	crayon_palette_t Man_P;
	crayon_sprite_array_t Man_Draw, Opaque_Blend_Draw, Opaque_Add_Draw;

	crayon_font_mono_t BIOS_Font;
	crayon_palette_t BIOS_P;

	crayon_memory_mount_romdisk("stuff.img", "/files", CRAYON_ADD_BASE_PATH);

	// Load the asset
	crayon_memory_load_spritesheet(&Man_SS, &Man_P, "/files/Man.dtex",
		CRAYON_USE_EXACT_PATH, 0);
	crayon_memory_load_spritesheet(&Opaque_SS, NULL, "/files/Opaque.dtex",
		CRAYON_USE_EXACT_PATH, -1);
	crayon_memory_load_mono_font_sheet(&BIOS_Font, &BIOS_P, "/files/Fonts/BIOS_font.dtex",
		CRAYON_USE_EXACT_PATH, 6);	// REMOVE LATER

	fs_romdisk_unmount("/files");

	crayon_memory_init_sprite_array(&Man_Draw, &Man_SS, 0, &Man_P, 1, 1, 0, PVR_FILTER_NONE, 0);
	Man_Draw.layer[0] = 2;
	Man_Draw.scale[0].x = 7;
	Man_Draw.scale[0].y = 7;
	Man_Draw.coord[0].x = (640 - crayon_graphics_get_draw_element_width(&Man_Draw, 0)) / 2.0f;
	Man_Draw.coord[0].y = (480 - crayon_graphics_get_draw_element_height(&Man_Draw, 0)) / 2.0f;;
	Man_Draw.flip[0] = 1;
	Man_Draw.rotation[0] = 0;
	Man_Draw.colour[0] = 0xFF0000FF;
	Man_Draw.fade[0] = 255;
	Man_Draw.frame_id[0] = 0;
	Man_Draw.visible[0] = 1;
	crayon_memory_set_frame_uv(&Man_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Opaque_Blend_Draw, &Opaque_SS, 0, NULL, 2, 1, CRAYON_MULTI_COLOUR | CRAYON_COLOUR_BLEND, PVR_FILTER_NONE, 0);
	Opaque_Blend_Draw.scale[0].x = 12;
	Opaque_Blend_Draw.scale[0].y = 12;
	Opaque_Blend_Draw.coord[0].x = 0;
	Opaque_Blend_Draw.coord[0].y = 0;
	Opaque_Blend_Draw.coord[1].x = 4 + crayon_graphics_get_draw_element_width(&Opaque_Blend_Draw, 0);
	Opaque_Blend_Draw.coord[1].y = Opaque_Blend_Draw.coord[0].y;
	Opaque_Blend_Draw.layer[0] = 1;
	Opaque_Blend_Draw.layer[1] = 1;
	Opaque_Blend_Draw.flip[0] = 0;
	Opaque_Blend_Draw.rotation[0] = 0;
	Opaque_Blend_Draw.colour[0] = 0xFF00FF00;
	Opaque_Blend_Draw.colour[1] = 0xFFFF0000;
	Opaque_Blend_Draw.fade[0] = 255;
	Opaque_Blend_Draw.fade[1] = 255;
	Opaque_Blend_Draw.frame_id[0] = 0;
	uint8_t i;
	for(i = 0; i < Opaque_Blend_Draw.size; i++){
		Opaque_Blend_Draw.visible[i] = 1;
	}
	crayon_memory_set_frame_uv(&Opaque_Blend_Draw, 0, 0);

	crayon_memory_init_sprite_array(&Opaque_Add_Draw, &Opaque_SS, 0, NULL, 2, 1, CRAYON_MULTI_COLOUR | CRAYON_COLOUR_ADD, PVR_FILTER_NONE, 0);
	Opaque_Add_Draw.scale[0].x = 12;
	Opaque_Add_Draw.scale[0].y = 12;
	Opaque_Add_Draw.coord[0].x = 0;
	Opaque_Add_Draw.coord[0].y = 4 + crayon_graphics_get_draw_element_height(&Opaque_Blend_Draw, 0);
	Opaque_Add_Draw.coord[1].x = 4 + crayon_graphics_get_draw_element_width(&Opaque_Add_Draw, 0);
	Opaque_Add_Draw.coord[1].y = Opaque_Add_Draw.coord[0].y;
	Opaque_Add_Draw.layer[0] = 1;
	Opaque_Add_Draw.layer[1] = 1;
	Opaque_Add_Draw.flip[0] = 0;
	Opaque_Add_Draw.rotation[0] = 0;
	Opaque_Add_Draw.colour[0] = 0xFF00FF00;
	Opaque_Add_Draw.colour[1] = 0xFFFF0000;
	Opaque_Add_Draw.fade[0] = 255;
	Opaque_Add_Draw.fade[1] = 255;
	Opaque_Add_Draw.frame_id[0] = 0;
	for(i = 0; i < Opaque_Add_Draw.size; i++){
		Opaque_Add_Draw.visible[i] = 1;
	}
	crayon_memory_set_frame_uv(&Opaque_Add_Draw, 0, 0);

	crayon_graphics_set_bg_colour(0.3, 0.3, 0.3); // Its useful-ish for debugging
	char buffer[15];

	while(1){
		pvr_wait_ready();
		// MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			//If any button is pressed, start the game (Doesn't check thumbstick)
			// if(st->buttons & (CONT_DPAD_RIGHT)){
			// 	Man_Draw.rotation[0]++;
			// }
			// if(st->buttons & (CONT_DPAD_LEFT)){
			// 	Man_Draw.rotation[0]--;
			// }
		// MAPLE_FOREACH_END()

		crayon_graphics_setup_palette(&Man_P);
		crayon_graphics_setup_palette(&BIOS_P);

		sprintf(buffer, "Angle: %d", (int)Man_Draw.rotation[0]);

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&Opaque_Blend_Draw, NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Opaque_Add_Draw, NULL, PVR_LIST_OP_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_text_mono(buffer, &BIOS_Font, PVR_LIST_OP_POLY, 280, 360, 30, 1, 1, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Man_Draw, NULL, PVR_LIST_PT_POLY, CRAYON_DRAW_ENHANCED);
		pvr_list_finish();

		pvr_scene_finish();

		// Rotate the man and keep it within the 0 - 360 range
		Man_Draw.rotation[0]++;
		if(Man_Draw.rotation[0] > 360){
			Man_Draw.rotation[0] -= 360;
		}
	}

	// Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Man_Draw);
	crayon_memory_free_sprite_array(&Opaque_Blend_Draw);
	crayon_memory_free_sprite_array(&Opaque_Add_Draw);

	crayon_memory_free_mono_font_sheet(&BIOS_Font);

	crayon_memory_free_spritesheet(&Man_SS);
	crayon_memory_free_spritesheet(&Opaque_SS);

	crayon_memory_free_palette(&Man_P);
	crayon_memory_free_palette(&BIOS_P);

	crayon_shutdown();

	return 0;
}
