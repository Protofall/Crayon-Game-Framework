#ifndef MS_SETUP_H
#define MS_SETUP_H

#include "../../Crayon/code/crayon/dreamcast/graphics.h"
#include "../../Crayon/code/crayon/dreamcast/debug.h"


#include "extra_structs.h"

#include <stdlib.h>
#include <string.h>

void setup_pos_lookup_table(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t iter);
void setup_OS_assets(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t lang);
void setup_free_OS_struct(MinesweeperOS_t *os);

void setup_grid_untextured_poly(crayon_untextured_array_t *Grid_polys, uint8_t grid_x, uint8_t grid_y,
	uint16_t grid_start_x, uint16_t grid_start_y);
void setup_bg_untextured_poly(crayon_untextured_array_t *Bg);

#endif
