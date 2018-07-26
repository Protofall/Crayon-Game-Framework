#ifndef MS_SETUP_H
#define MS_SETUP_H

#include "../../Crayon/code/crayon/dreamcast/graphics.h"
#include "../../Crayon/code/crayon/dreamcast/debug.h"


#include "extra_structs.h"

#include <stdlib.h>
#include <string.h>

void setup_pos_lookup_table(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t anim_id);
void setup_OS_assets(MinesweeperOS_t *os, spritesheet_t *ss, uint8_t sys, uint8_t lang);
void setup_free_OS_struct(MinesweeperOS_t *os);

#endif
