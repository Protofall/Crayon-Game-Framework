#ifndef MISC_CRAYON_H
#define MISC_CRAYON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#define CRAYON_MAJOR_NUMBER 0
#define CRAYON_MINOR_NUMBER 2
#define CRAYON_PATCH_NUMBER 0

// Gets the crayon version number in form MAJOR.MINOR.PATCH
char *crayon_misc_get_version();

// Takes two strings and returns a string which is source2 appended to source1
	// The user passes a reference to an un-allocated char pointer for the first parameter.
	// The user is also responsible for free-ing dest
uint8_t crayon_misc_combine_strings(char **dest, char *source1, char *source2);

// Searches for the last '.' in a string and makes copy, but with "extension" after that '.'
// Note that '.' is not a valid source and will result in an error (Since there's nothing before the '.')
	// dest is handled like the above function handles it
uint8_t crayon_misc_change_extension(char **dest, char *source, char *extension);

// UNTESTED
// Last param is used if your var already has been allocated space either by stack or previous malloc
uint8_t crayon_misc_read_file(void **buffer, char *path, size_t size_bytes, uint8_t allocated);

// UNTESTED
// Reads a number from a file stream. However the number may not be negative
	// Useful for reading numbers larger than 9 into a uint8_t var
	// It will always discard 1 char after reading
	// The last char point will be set to te last char read if pointer != NULL
	// this is useful for checking why it stopped reading a number
uint32_t crayon_misc_fgeti(FILE *f, int16_t *last_char);

// This function instead finds the next number and modifies the parameter to it's value
	// If the EOF is found before a number it returns 1, else it returns 0
int crayon_misc_fget_next_int(FILE *f, int *number);

// If given bit_length = 5 and offset = 2 we will return the bit_length bit offset to the left by offset
// For example if we give it number = 90 = 01011010 then we return 10110 = 22
uint32_t crayon_misc_extract_bits(uint32_t number, uint8_t bit_length, uint8_t offset);

// This simply insets your second number into the first number. I don't know what happens if number_2 is too long...
uint32_t crayon_misc_insert_bits(uint32_t number_1, uint32_t number_2, uint8_t bit_length, uint8_t offset);

// UNTESTED
// Despite the name, if you pass in a negative number it will decrement
uint32_t crayon_misc_increment_bits(uint32_t number, int32_t change_val, uint8_t bit_length, uint8_t offset);

// Checks if the computer running this code is big endian or not
uint8_t crayon_misc_is_big_endian();

void crayon_misc_endian_correction(uint8_t *buffer, size_t bytes);	// UNFINISHED. WIP

void crayon_misc_encode_to_buffer(uint8_t *buffer, uint8_t *data, size_t bytes);

// THE FUNCTIONS BELOW CAME FROM DREAMHAL
// https://github.com/Moopthehedgehog/DreamHAL/blob/master/modules/simple_print.c
	// NOTE: As of writting, Moop's github has gone down, but copies of DreamHAL can be found
	// on the Simulant discord channel

// NOTE: "out_string" is expected to be at least 12 characters long (10 for number, 1 for sign, 1 for \0 )
	// For unsigned you can get away with 11 since there's no sign

char *crayon_misc_uint_to_string(unsigned int in_number, char*out_string);

char *crayon_misc_int_to_string(int in_number, char*out_string);

#endif
