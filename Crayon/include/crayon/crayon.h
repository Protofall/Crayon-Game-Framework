#ifndef CRAYON_CRAYON_H
#define CRAYON_CRAYON_H

#include "memory.h"
#include "graphics.h"
// #include "audio.h"

#define MNT_MODE FS_FAT_MOUNT_READONLY

#include <dc/sd.h>
#include <kos/blockdev.h>
#include <fat/fs_fat.h>

uint8_t __sd_present;

#define CRAYON_PLATFORM_DREAMCAST 0
#define CRAYON_PLATFORM_PC 1

#define CRAYON_BOOT_OPTICAL 0	//For DC this is /cd/
#define CRAYON_BOOT_SD 1		//For DC this is /sd/
#define CRAYON_BOOT_PC_LAN 2	//For DC this is /pc/

//Give this and shutdown the weak attribute like KOS and try to automatically call them
extern uint8_t crayon_init(uint8_t platform, uint8_t boot_mode);

//NEED TO HANDLE RETURN CODES BETTER
extern uint8_t crayon_sd_mount_fat();

extern void crayon_shutdown();

extern void crayon_sd_unmount_fat();



#endif
