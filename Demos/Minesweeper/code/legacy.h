//Minesweeper is an old project that was originally based on older versions of Crayon
//Because of this function signatures have changed and rather than re-ordering the
//parameter orders I decided it would just be easier to make this file that re-defines
//them

#ifndef MS_LEGACY_H
#define MS_LEGACY_H

#include <crayon/graphics.h>

//Draw string using mono font (string must be null-terminated)
uint8_t OLD_crayon_graphics_draw_text_mono(const crayon_font_mono_t *fm, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number, char * string);

//Draw string using propertional font (string must be null-terminated)
uint8_t OLD_crayon_graphics_draw_text_prop(const crayon_font_prop_t *fp, uint8_t poly_list_mode, float draw_x,
	float draw_y, uint8_t layer, float scale_x, float scale_y, uint8_t palette_number, char * string);

#endif
