#ifndef ASSIST_CRAYON_H
#define ASSIST_CRAYON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//UNTESTED
//Last param is used if your var already has been allocated space either by stack or previous malloc
extern uint8_t crayon_assist_read_file(void **buffer, char *path, size_t ize_bytes, uint8_t allocated);

//UNTESTED
//Reads a number from a file stream. However the number may not be negative
	//Useful for reading numbers larger than 9 into a uint8_t var
extern uint32_t crayon_assist_fgeti(FILE *f);

#endif
