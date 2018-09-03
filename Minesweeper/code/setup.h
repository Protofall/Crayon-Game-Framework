#ifndef MS_SETUP_H
#define MS_SETUP_H

#include "../../Crayon/code/dreamcast/graphics.h"
#include "../../Crayon/code/dreamcast/debug.h"

#include "extra_structs.h"

#include <stdlib.h>
#include <string.h>

void setup_pos_lookup_table(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t iter, uint8_t sd);
void setup_OS_assets(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t lang, uint8_t sd);
void setup_free_OS_struct(MinesweeperOS_t *os);

void setup_bg_untextured_poly(crayon_untextured_array_t *Bg, uint8_t os, uint8_t sd);

#endif
