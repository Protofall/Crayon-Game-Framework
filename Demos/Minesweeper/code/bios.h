#ifndef MS_BIOS_H
#define MS_BIOS_H

#include <crayon/graphics.h>
#include <crayon/memory.h>

#include "extra_structs.h"
#include "legacy.h"

#include <dc/maple.h>
#include <dc/maple/controller.h>

#include <dc/flashrom.h>

void BIOS_menu(MinesweeperOptions_t * MS_options, float * htz_adjustment, pvr_init_params_t * pvr_params, uint8_t region, uint8_t first_time);

#endif
