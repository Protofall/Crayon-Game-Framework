// Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/debug.h>
#include <crayon/crayon.h>

// For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){
	// Initialise Crayon
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		error_freeze("Unable to initialise crayon");
	}

	crayon_spritesheet_t Dwarf_SS;
	crayon_sprite_array_t Dwarf_Draw_Flip, Dwarf_Draw_Rotate, Dwarf_Draw_Scale, Dwarf_Draw_Frame,
		Dwarf_Draw_Colour_Blend, Dwarf_Draw_Colour_Add, Dwarf_Draw_Mash;
	crayon_font_mono_t BIOS;
	crayon_palette_t BIOS_P;

	// Load the romdisk
	crayon_memory_mount_romdisk("stuff.img", "/files", CRAYON_ADD_BASE_PATH);

	// Load the asset
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, "/files/BIOS_font.dtex",
		CRAYON_USE_EXACT_PATH, 0);
	crayon_memory_load_spritesheet(&Dwarf_SS, NULL, "/files/sprite.dtex",
		CRAYON_USE_EXACT_PATH, -1);

	fs_romdisk_unmount("/files");

	crayon_memory_init_sprite_array(&Dwarf_Draw_Flip, &Dwarf_SS, 0, NULL, 2, 1, CRAYON_MULTI_FLIP, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Flip.scale[0].x = 1;
	Dwarf_Draw_Flip.scale[0].y = 1;

	Dwarf_Draw_Flip.coord[0].x = 32;
	Dwarf_Draw_Flip.coord[0].y = 32;
	Dwarf_Draw_Flip.coord[1].x = Dwarf_Draw_Flip.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Flip, 0) + 32;
	Dwarf_Draw_Flip.coord[1].y = Dwarf_Draw_Flip.coord[0].y;

	Dwarf_Draw_Flip.layer[0] = 2;
	Dwarf_Draw_Flip.layer[1] = 2;
	Dwarf_Draw_Flip.flip[0] = 0;
	Dwarf_Draw_Flip.flip[1] = 1;
	Dwarf_Draw_Flip.rotation[0] = 0;
	Dwarf_Draw_Flip.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Flip.fade[0] = 0;
	Dwarf_Draw_Flip.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Flip, 0, 0);
	unsigned int i;
	for(i = 0; i < Dwarf_Draw_Flip.size; i++){
		Dwarf_Draw_Flip.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Rotate, &Dwarf_SS, 0, NULL, 4, 1, CRAYON_MULTI_ROTATE, PVR_FILTER_NONE, 0);
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
	Dwarf_Draw_Rotate.layer[1] = 2;
	Dwarf_Draw_Rotate.layer[2] = 2;
	Dwarf_Draw_Rotate.layer[3] = 2;
	Dwarf_Draw_Rotate.flip[0] = 0;
	Dwarf_Draw_Rotate.rotation[0] = 0;
	Dwarf_Draw_Rotate.rotation[1] = 81;
	Dwarf_Draw_Rotate.rotation[2] = 199;
	Dwarf_Draw_Rotate.rotation[3] = 226;
	Dwarf_Draw_Rotate.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Rotate.fade[0] = 0;
	Dwarf_Draw_Rotate.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Rotate, 0, 0);
	for(i = 0; i < Dwarf_Draw_Rotate.size; i++){
		Dwarf_Draw_Rotate.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Scale, &Dwarf_SS, 0, NULL, 3, 1, CRAYON_MULTI_SCALE, PVR_FILTER_NONE, 0);
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
	Dwarf_Draw_Scale.layer[1] = 2;
	Dwarf_Draw_Scale.layer[2] = 2;
	Dwarf_Draw_Scale.flip[0] = 0;
	Dwarf_Draw_Scale.rotation[0] = 0;
	Dwarf_Draw_Scale.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Scale.fade[0] = 0;
	Dwarf_Draw_Scale.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Scale, 0, 0);
	for(i = 0; i < Dwarf_Draw_Scale.size; i++){
		Dwarf_Draw_Scale.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Frame, &Dwarf_SS, 0, NULL, 3, 2, CRAYON_MULTI_FRAME, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Frame.scale[0].x = 1;
	Dwarf_Draw_Frame.scale[0].y = 1;

	Dwarf_Draw_Frame.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Frame.coord[0].y = Dwarf_Draw_Scale.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Frame, 0) + 32 + 6;	//6 because of scale over-stretching
	Dwarf_Draw_Frame.coord[1].x = Dwarf_Draw_Frame.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Frame, 0) + 32;
	Dwarf_Draw_Frame.coord[1].y = Dwarf_Draw_Frame.coord[0].y;
	Dwarf_Draw_Frame.coord[2].x = Dwarf_Draw_Frame.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Frame, 0) + 32;
	Dwarf_Draw_Frame.coord[2].y = Dwarf_Draw_Frame.coord[0].y;

	Dwarf_Draw_Frame.layer[0] = 2;
	Dwarf_Draw_Frame.layer[1] = 2;
	Dwarf_Draw_Frame.layer[2] = 2;
	Dwarf_Draw_Frame.flip[0] = 0;
	Dwarf_Draw_Frame.rotation[0] = 0;
	Dwarf_Draw_Frame.colour[0] = 0xFFFFFFFF;
	Dwarf_Draw_Frame.fade[0] = 0;
	Dwarf_Draw_Frame.frame_id[0] = 0;
	Dwarf_Draw_Frame.frame_id[1] = 1;
	Dwarf_Draw_Frame.frame_id[2] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Frame, 0, 0);
	crayon_memory_set_frame_uv(&Dwarf_Draw_Frame, 1, 1);
	for(i = 0; i < Dwarf_Draw_Frame.size; i++){
		Dwarf_Draw_Frame.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Colour_Blend, &Dwarf_SS, 0, NULL, 3, 1, CRAYON_MULTI_COLOUR_BLEND, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Colour_Blend.scale[0].x = 1;
	Dwarf_Draw_Colour_Blend.scale[0].y = 1;

	Dwarf_Draw_Colour_Blend.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Colour_Blend.coord[0].y = Dwarf_Draw_Frame.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Colour_Blend, 0) + 32;
	Dwarf_Draw_Colour_Blend.coord[1].x = Dwarf_Draw_Colour_Blend.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Blend, 0) + 32;
	Dwarf_Draw_Colour_Blend.coord[1].y = Dwarf_Draw_Colour_Blend.coord[0].y;
	Dwarf_Draw_Colour_Blend.coord[2].x = Dwarf_Draw_Colour_Blend.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Blend, 0) + 32;
	Dwarf_Draw_Colour_Blend.coord[2].y = Dwarf_Draw_Colour_Blend.coord[0].y;

	Dwarf_Draw_Colour_Blend.layer[0] = 2;
	Dwarf_Draw_Colour_Blend.layer[1] = 2;
	Dwarf_Draw_Colour_Blend.layer[2] = 2;

	Dwarf_Draw_Colour_Blend.colour[0] = 0xFF00FF00;	//Green
	Dwarf_Draw_Colour_Blend.fade[0] = 0;
	Dwarf_Draw_Colour_Blend.colour[1] = 0xFF00FF00;
	Dwarf_Draw_Colour_Blend.fade[1] = 128;
	Dwarf_Draw_Colour_Blend.colour[2] = 0xFF00FF00;
	Dwarf_Draw_Colour_Blend.fade[2] = 255;

	Dwarf_Draw_Colour_Blend.flip[0] = 0;
	Dwarf_Draw_Colour_Blend.rotation[0] = 0;
	Dwarf_Draw_Colour_Blend.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Colour_Blend, 0, 0);
	for(i = 0; i < Dwarf_Draw_Colour_Blend.size; i++){
		Dwarf_Draw_Colour_Blend.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Colour_Add, &Dwarf_SS, 0, NULL, 3, 1, CRAYON_MULTI_COLOUR_ADD, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Colour_Add.scale[0].x = 1;
	Dwarf_Draw_Colour_Add.scale[0].y = 1;

	Dwarf_Draw_Colour_Add.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Colour_Add.coord[0].y = Dwarf_Draw_Colour_Blend.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Colour_Add, 0) + 32;
	Dwarf_Draw_Colour_Add.coord[1].x = Dwarf_Draw_Colour_Add.coord[0].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Add, 0) + 32;
	Dwarf_Draw_Colour_Add.coord[1].y = Dwarf_Draw_Colour_Add.coord[0].y;
	Dwarf_Draw_Colour_Add.coord[2].x = Dwarf_Draw_Colour_Add.coord[1].x + crayon_graphics_get_draw_element_width(&Dwarf_Draw_Colour_Add, 0) + 32;
	Dwarf_Draw_Colour_Add.coord[2].y = Dwarf_Draw_Colour_Add.coord[0].y;

	Dwarf_Draw_Colour_Add.layer[0] = 2;
	Dwarf_Draw_Colour_Add.layer[1] = 2;
	Dwarf_Draw_Colour_Add.layer[2] = 2;

	Dwarf_Draw_Colour_Add.colour[0] = 0xFF0000FF;	// Blue
	Dwarf_Draw_Colour_Add.fade[0] = 0;
	Dwarf_Draw_Colour_Add.colour[1] = 0xFF0000FF;
	Dwarf_Draw_Colour_Add.fade[1] = 128;
	Dwarf_Draw_Colour_Add.colour[2] = 0xFF0000FF;
	Dwarf_Draw_Colour_Add.fade[2] = 255;

	Dwarf_Draw_Colour_Add.flip[0] = 0;
	Dwarf_Draw_Colour_Add.rotation[0] = 0;
	Dwarf_Draw_Colour_Add.frame_id[0] = 0;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Colour_Add, 0, 0);
	for(i = 0; i < Dwarf_Draw_Colour_Add.size; i++){
		Dwarf_Draw_Colour_Add.visible[i] = 1;
	}

	crayon_memory_init_sprite_array(&Dwarf_Draw_Mash, &Dwarf_SS, 0, NULL, 6, 2,
		CRAYON_MULTI_FRAME | CRAYON_MULTI_FLIP | CRAYON_MULTI_ROTATE, PVR_FILTER_NONE, 0);
	Dwarf_Draw_Mash.scale[0].x = 1;
	Dwarf_Draw_Mash.scale[0].y = 1;

	Dwarf_Draw_Mash.coord[0].x = Dwarf_Draw_Flip.coord[0].x;
	Dwarf_Draw_Mash.coord[0].y = Dwarf_Draw_Colour_Add.coord[0].y + crayon_graphics_get_draw_element_height(&Dwarf_Draw_Mash, 0) + 32;
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
	Dwarf_Draw_Mash.layer[1] = 2;
	Dwarf_Draw_Mash.layer[2] = 2;
	Dwarf_Draw_Mash.layer[3] = 2;
	Dwarf_Draw_Mash.layer[4] = 2;
	Dwarf_Draw_Mash.layer[5] = 2;

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

	Dwarf_Draw_Mash.frame_id[0] = 0;
	Dwarf_Draw_Mash.frame_id[1] = 1;
	Dwarf_Draw_Mash.frame_id[2] = 1;
	Dwarf_Draw_Mash.frame_id[3] = 1;
	Dwarf_Draw_Mash.frame_id[4] = 0;
	Dwarf_Draw_Mash.frame_id[5] = 1;
	crayon_memory_set_frame_uv(&Dwarf_Draw_Mash, 0, 0);
	crayon_memory_set_frame_uv(&Dwarf_Draw_Mash, 1, 1);
	for(i = 0; i < Dwarf_Draw_Mash.size; i++){
		Dwarf_Draw_Mash.visible[i] = 1;
	}

	crayon_graphics_setup_palette(&BIOS_P);
	while(1){
		pvr_wait_ready();

		pvr_scene_begin();

		pvr_list_begin(CRAYON_PT_LIST);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Flip, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Rotate, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Scale, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Frame, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Colour_Blend, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Colour_Add, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Dwarf_Draw_Mash, NULL, CRAYON_PT_LIST, CRAYON_DRAW_ENHANCED);
		pvr_list_finish();

		pvr_list_begin(CRAYON_OP_LIST);
			crayon_graphics_draw_text_mono("Controls:", &BIOS, CRAYON_OP_LIST, 450, 32, 3, 2, 2, BIOS_P.palette_id);
		pvr_list_finish();

		pvr_scene_finish();
	}

	// Also frees the spritesheet and palette
	crayon_memory_free_sprite_array(&Dwarf_Draw_Flip);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Rotate);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Scale);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Frame);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Colour_Blend);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Colour_Add);
	crayon_memory_free_sprite_array(&Dwarf_Draw_Mash);

	crayon_memory_free_spritesheet(&Dwarf_SS);
	crayon_memory_free_mono_font_sheet(&BIOS);

	crayon_memory_free_palette(&BIOS_P);

	crayon_shutdown();

	return 0;
}
