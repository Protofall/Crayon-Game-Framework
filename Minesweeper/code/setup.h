#ifndef MS_SETUP_H
#define MS_SETUP_H

#include <crayon/graphics.h>
#include <crayon/memory.h>

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
void setup_option_untextured_poly(crayon_untextured_array_t *Options, crayon_sprite_array_t * num_changers, uint8_t os);

void setup_keys(MinesweeperKeyboard_t * keyboard, crayon_font_prop_t * fontsheet);

void setup_pre_1_3_0_savefile_conversion(minesweeper_savefile_t * new_savefile, pre_1_3_0_minesweeper_savefile_t * pre_1_3_0_savefile);
uint8_t setup_check_for_old_savefile(crayon_savefile_details_t * pre_1_2_0_savefile_details, uint8_t savefile_port, uint8_t savefile_slot);
uint8_t setup_update_old_saves(crayon_savefile_details_t * new_savefile_details);

int16_t setup_vmu_icon_load(uint8_t ** vmu_lcd_icon, char * icon_path);
void setup_vmu_icon_apply(uint8_t * vmu_lcd_icon, uint8_t valid_vmu_screens);

uint8_t setup_sanitise_savefile(minesweeper_savefile_t * savefile);

#endif
