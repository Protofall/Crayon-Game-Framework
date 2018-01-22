#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <dc/pvr.h>
#include <kos/fs_romdisk.h> //For romdisk swapping
#include <zlib/zlib.h>	//Not too sure if I need this for the gz romdisk...

//Load a sprite from a path to the texture. The path is used to generate a dtex and dtex.pal paths
extern int memory_load_dtex(struct sprite *sprite, char *path);

//Loads a spritesheet
extern void memory_init_spritesheet(char *path, spritesheet *ss);

//Sets a palette for a spritesheet
extern void setup_palette(uint8_t palette_number, spritesheet *ss);

//Free any resources used by a sprite
extern void memory_sprite_free(struct sprite *sprite);

//Mount a romdisk
extern int mount_romdisk(char *filename, char *mountpoint);

//Mount a gz compressed romdisk
extern int mount_romdisk_gz(char *filename, char *mountpoint);

#endif
