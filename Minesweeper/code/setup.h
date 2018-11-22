#ifndef MS_SETUP_H
#define MS_SETUP_H

#include "../../Crayon/code/dreamcast/graphics.h"
#include "../../Crayon/code/dreamcast/memory.h"

#include "extra_structs.h"

#include <stdlib.h>
#include <string.h>

void setup_pos_lookup_table(MinesweeperOS_t *os, uint8_t sys, uint8_t i, uint8_t sd);
void setup_OS_assets(MinesweeperOS_t *os, crayon_spritesheet_t *ss, crayon_palette_t *pal, uint8_t sys, uint8_t lang,
	uint8_t sd);
void setup_OS_assets_icons(MinesweeperOS_t *os, crayon_spritesheet_t *Icons, crayon_palette_t *Icons_P, uint8_t sys,
	uint8_t region);
void setup_free_OS_struct(MinesweeperOS_t *os);

void setup_bg_untextured_poly(crayon_untextured_array_t *Bg, uint8_t os, uint8_t sd);
void setup_option_untextured_poly(crayon_untextured_array_t *Options, crayon_textured_array_t * num_changers, uint8_t os);

#endif
