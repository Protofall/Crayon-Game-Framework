#ifndef ASSIST_CRAYON_H
#define ASSIST_CRAYON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define CRAYON_MAJOR_NUMBER 0
#define CRAYON_MINOR_NUMBER 2
#define CRAYON_PATCH_NUMBER 0

//Gets the crayon version number in form MAJOR.MINOR.PATCH
extern char * crayon_get_version();

//Takes two strings and returns a string which is source2 appended to source1
extern uint8_t crayon_assist_append_extension(char ** dest, char * source1, char * source2);

//Searches for the last '.' in a string and makes copy, but with "extension" after it
//Note that "." is not a valid source and will result in an error (Since there's nothing before the '.')
extern uint8_t crayon_assist_change_extension(char ** dest, char * source, char * extension);

//UNTESTED
//Last param is used if your var already has been allocated space either by stack or previous malloc
extern uint8_t crayon_assist_read_file(void ** buffer, char * path, size_t size_bytes, uint8_t allocated);

//UNTESTED
//Reads a number from a file stream. However the number may not be negative
	//Useful for reading numbers larger than 9 into a uint8_t var
	//It will always discard 1 char after reading
	//The last char point will be set to te last char read if pointer != NULL
	//this is useful for checking why it stopped reading a number
extern uint32_t crayon_assist_fgeti(FILE * f, int16_t * last_char);

#endif
