#ifndef ASSIST_CRAYON_H
#define ASSIST_CRAYON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#define CRAYON_MAJOR_NUMBER 0
#define CRAYON_MINOR_NUMBER 2
#define CRAYON_PATCH_NUMBER 0

//Gets the crayon version number in form MAJOR.MINOR.PATCH
extern char * crayon_misc_get_version();

//Takes two strings and returns a string which is source2 appended to source1
extern uint8_t crayon_misc_combine_strings(char ** dest, char * source1, char * source2);

//Searches for the last '.' in a string and makes copy, but with "extension" after it
//Note that "." is not a valid source and will result in an error (Since there's nothing before the '.')
extern uint8_t crayon_misc_change_extension(char ** dest, char * source, char * extension);

//UNTESTED
//Last param is used if your var already has been allocated space either by stack or previous malloc
extern uint8_t crayon_misc_read_file(void ** buffer, char * path, size_t size_bytes, uint8_t allocated);

//UNTESTED
//Reads a number from a file stream. However the number may not be negative
	//Useful for reading numbers larger than 9 into a uint8_t var
	//It will always discard 1 char after reading
	//The last char point will be set to te last char read if pointer != NULL
	//this is useful for checking why it stopped reading a number
extern uint32_t crayon_misc_fgeti(FILE * f, int16_t * last_char);

//This function instead finds the next number and modifies the parameter to add in it.
	//If the EOF is found before a number, then this returns 0
extern int crayon_misc_fget_next_int(FILE * f, int * number);

// If given bit_length = 5 and offset = 2 we will return the bit_length bit offset to the left by offset
// For example if we give it number = 90 = 01011010 then we return 10110 = 22
extern uint32_t crayon_misc_extract_bits(uint32_t number, uint8_t bit_length, uint8_t offset);

//This simply insets your second number into the first number. I don't know what happens if number_2 is too long...
extern uint32_t crayon_misc_insert_bits(uint32_t number_1, uint32_t number_2, uint8_t bit_length, uint8_t offset);

#endif
