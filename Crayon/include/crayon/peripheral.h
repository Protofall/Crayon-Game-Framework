#ifndef CRAYON_PERIPHERAL_H
#define CRAYON_PERIPHERAL_H

#include "vector_structs.h"
#include "crayon.h"

#include <stdint.h> // For the uintX_t types

#if defined(_arch_dreamcast)

#include <dc/maple/vmu.h>
#include <dc/vmu_pkg.h>
#include <dc/vmufs.h>
#include <kos/fs.h>

#endif

// Converts the device id to a port (x) and slot (y)
vec2_s8_t crayon_peripheral_dreamcast_get_port_and_slot(int8_t savefile_device_id);

// Returns a bitmap of all vmus with a screen (a1a2b1b2c1c2d1d2)
uint8_t crayon_peripheral_dreamcast_get_screens();

// Displays the icon to all vmu screens specified by the bitmap
void crayon_peripheral_vmu_display_icon(uint8_t vmu_bitmap, void *icon);

// 0 if the peripheral doesn't have the function, 1 if it does
// Kind of a Dreamcast-specific function, but for later console ports this might be reusable
uint8_t crayon_peripheral_has_function(uint32_t function, int8_t save_device_id);

#endif