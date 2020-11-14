#ifndef CRAYON_CRAYON_H
#define CRAYON_CRAYON_H

// Setting defines for different platforms
#if defined(__APPLE__) || defined(__linux__) || defined(_WIN32)
	#define _arch_pc
#endif

#if !(defined(_arch_dreamcast) || defined(_arch_pc))
	#error "UNSUPPORTED ARCHITECTURE/PLATFORM"
#endif

// Debug mode
#ifndef CRAYON_DEBUG
#define CRAYON_DEBUG 0
#endif

#include "memory.h"
#include "graphics.h"
// #include "audio.h"

#define MNT_MODE FS_FAT_MOUNT_READONLY

#include <dc/sd.h>
#include <kos/blockdev.h>
#include <fat/fs_fat.h>

extern uint8_t __sd_present;

#define CRAYON_PLATFORM_DREAMCAST 0
#define CRAYON_PLATFORM_PC 1

#define CRAYON_BOOT_OPTICAL 0	// For DC this is /cd/
#define CRAYON_BOOT_SD 1		// For DC this is /sd/
#define CRAYON_BOOT_PC_LAN 2	// For DC this is /pc/

// Give this and shutdown the weak attribute like KOS and try to automatically call them
	// ATM the weak attribute is causing a crash
uint8_t crayon_init(uint8_t platform, uint8_t boot_mode);
// uint8_t __attribute__((weak)) crayon_init(uint8_t platform, uint8_t boot_mode);

// NEED TO HANDLE RETURN CODES BETTER
uint8_t crayon_sd_mount_fat();

void crayon_shutdown();
// void __attribute__((weak)) crayon_shutdown();

void crayon_sd_unmount_fat();


#endif
