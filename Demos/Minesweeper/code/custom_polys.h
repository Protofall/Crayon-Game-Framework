#ifndef MS_CUSTOM_POLYS_H
#define MS_CUSTOM_POLYS_H

#include <crayon/graphics.h>
#include <crayon/debug.h>

#include <stdlib.h>

void custom_poly_boarder(uint8_t thickness, uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x,
	uint16_t dim_y, uint32_t colour1, uint32_t colour2);	//x and y are the x and y of thing being boardered. Kinda breaks with thickness < 1
void custom_poly_2000_topbar(uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x, uint16_t dim_y);
void custom_poly_2000_boarder(uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x, int16_t dim_y);
void custom_poly_2000_text_boarder(uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x, int16_t dim_y);
void custom_poly_XP_boarder(uint16_t x, uint16_t y, uint8_t z, uint16_t dim_x, int16_t dim_y);

void custom_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t z);

#endif