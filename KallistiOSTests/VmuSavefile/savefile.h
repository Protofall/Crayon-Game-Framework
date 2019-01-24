#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> //For the uintX_t types

#include <dc/maple/vmu.h>
#include <dc/vmu_pkg.h>
#include <dc/vmufs.h>
#include <kos/fs.h>

#include <zlib/zlib.h>	//For the length of the text file in the gz archive

typedef struct SaveFile{
	uint32_t var1;
	uint8_t var2;
} SaveFile_t;

unsigned char * icon;		//uint8_t
unsigned short * palette;	//uint16_t

SaveFile_t save;

int save_uncompressed(uint8_t port, uint8_t unit);
int load_uncompressed(uint8_t port, uint8_t unit);

int save_compressed(uint8_t port, uint8_t unit);
int load_compressed(uint8_t port, uint8_t unit);

#endif
