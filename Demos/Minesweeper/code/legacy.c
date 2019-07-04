#include "legacy.h"

uint8_t OLD_crayon_graphics_draw_text_mono(const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number, char * string){

	return crayon_graphics_draw_text_mono(fm, poly_list_mode, draw_x, draw_y, layer, scale_x, scale_y, palette_number, string);
}

uint8_t OLD_crayon_graphics_draw_text_prop(const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number, char * string){
	
	return crayon_graphics_draw_text_prop(fp, poly_list_mode, draw_x, draw_y, layer, scale_x, scale_y, palette_number, string);
}