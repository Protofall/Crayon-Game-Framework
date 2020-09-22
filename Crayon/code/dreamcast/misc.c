#include "misc.h"

char *crayon_misc_get_version(){
	char *version = malloc(sizeof(char) * 16);
	if(!version){
		return NULL;
	}

	char major_ver[12];
	char minor_ver[12];
	char patch_ver[12];

	crayon_misc_int_to_string(CRAYON_MAJOR_NUMBER, major_ver);
	crayon_misc_int_to_string(CRAYON_MINOR_NUMBER, minor_ver);
	crayon_misc_int_to_string(CRAYON_PATCH_NUMBER, patch_ver);

	// sprintf(version, "%s.%s.%s", major_ver, minor_ver, patch_ver);
	sprintf(version, "%s.%s.%s-dev", major_ver, minor_ver, patch_ver);
	return version;
}

uint8_t crayon_misc_combine_strings(char **dest, char *source1, char *source2){
	if(!source1 || !source2){
		return 1;
	}

	uint16_t s1_length = strlen(source1);	// Remember for "good\0" strlen returns 4
	uint16_t s2_length = strlen(source2);
	*dest = malloc(sizeof(char) * (s1_length + s2_length + 1));	// +1 for null-terminator
	if(!(*dest)){	// Somehow the malloc failed. Thats bad!
		return 1;
	}

	memcpy((*dest), source1, s1_length);
	memcpy((*dest + s1_length), source2, s2_length + 1);

	return 0;
}

uint8_t crayon_misc_change_extension(char **dest, char *source, char *extension){
	if(!source || !extension){
		return 1;
	}

	char *dot_pos = strchr(source, '.');
	// No dot present
	if(!dot_pos){
		return 1;
	}

	// Get the offset of the dot
	uint16_t src_length = 1 + (dot_pos - source);

	// Dot at the start of the string (eg ".bashrc") isn't valid
	if(src_length == 1){
		return 1;
	}

	uint8_t ext_length = strlen(extension);
	*dest = malloc(sizeof(char) * (src_length + ext_length + 1));
	if(!(*dest)){
		return 1;
	}

	// Add the part of the source we're keeping then the extension
	memcpy((*dest), source, src_length);
	memcpy((*dest + src_length), extension, ext_length + 1);

	return 0;
}

// For example, call this like so (Assuming file is 12 bytes long)
// uint8_t *my_stuff;
// crayon_misc_read_file((void *) &my_stuff, "romdisk/read_data.txt", 12);
uint8_t crayon_misc_read_file(void **buffer, char *path, size_t size_bytes, uint8_t allocated){
	if(!allocated){
		*buffer = malloc(size_bytes);
	}
	FILE *file = fopen(path, "rb");
	if(!file){return 1;}
	size_t res = fread(*buffer, size_bytes, 1, file);
	fclose(file);
	if(res != 1){
		return res;
	}
	return 0;
}

uint32_t crayon_misc_fgeti(FILE *f, int16_t *last_char){
	uint32_t n = 0;
	int16_t d;	// EOF is -1 so we can't use a uint8_t
	for(d = 0; d < 10; d = fgetc(f) - '0'){
		n = n *10 + d;
	}
	if(last_char){
		*last_char = d;
	}
	return n;
}

int crayon_misc_fget_next_int(FILE *f, int *number){
	*number = 0;
	uint8_t negative = 0;
	uint8_t started = 0;
	int16_t d;	// EOF is -1 so we can't use a uint8_t
	while((d = fgetc(f)) != EOF){
		if((char)d == '-'){
			negative = 1;
			started = 1;
		}
		else if(isdigit(d)){
			started = 1;
			*number *= 10;
			*number += (d - '0');
		}
		else if(started){
			break;
		}
	}

	if(!started){return 1;}

	if(negative){
		*number *= -1;
	}

	return 0;
}

uint32_t crayon_misc_extract_bits(uint32_t number, uint8_t bit_length, uint8_t offset){
	return (((1 << bit_length) - 1) & (number >> offset));
}

uint32_t crayon_misc_insert_bits(uint32_t number_1, uint32_t number_2, uint8_t bit_length, uint8_t offset){
	return (number_1 & ~(((1UL << bit_length) - 1) << offset)) | (number_2 << offset);
}

uint32_t crayon_misc_increment_bits(uint32_t number, int32_t change_val, uint8_t bit_length, uint8_t offset){
	return crayon_misc_insert_bits(number,
		crayon_misc_extract_bits(number, bit_length, offset) + change_val, bit_length, offset);
}

uint8_t crayon_misc_is_big_endian(){
	int a = 1;
	return !((char*)&a)[0];
}

void crayon_misc_endian_correction(uint8_t *buffer, size_t bytes){
	;
	return;
}

void crayon_misc_encode_to_buffer(uint8_t *buffer, uint8_t *data, size_t bytes){
	while(bytes--){*buffer++ = *data++;}
	return;
}

// Convert unsigned int to string
// 'out_string' buffer is assumed to be large enough.
// Requires an 11-byte output buffer for the string.
// The biggest decimal number is 4294967295, which is 10 characters‬ (excluding null term).
// Returns pointer to out_string.
char *crayon_misc_uint_to_string(unsigned int in_number, char *out_string){
	int i;

	out_string[10] = '\0'; // Null term

	for(i = 9; i >= 0; i--){
		if((!in_number) && (i < 9)){
			memmove(out_string, &(out_string[i + 1]), 11-i); // realign string with beginning
			return out_string;
		}
		else{
			out_string[i] = (in_number % 10) + '0';
		}
		in_number /= 10;
	}

	return out_string;
}

// Convert signed int to string
// 'out_string' buffer is assumed to be large enough.
// Requires a 12-byte output buffer for the string.
// The longest signed decimal numbers are 10 characters‬ (excluding null term and sign).
// Returns pointer to out_string.
char *crayon_misc_int_to_string(int in_number, char*out_string){
	int i;
	int need_neg = 0;
	int neg = 0;

	out_string[11] = '\0'; // Null term

	if(in_number < 0){
		need_neg = 1;
		neg = 1;
	}

	// 10 digits for the number, plus one char for the potential (-) sign
	for(i = 10; i >= 0; i--){
		if(need_neg && (!in_number) && (i < 10)){
			out_string[i] = '-';
			need_neg = 0;
		}
		else if((!in_number) && (i < 10)){
			memmove(out_string, &(out_string[i + 1]), 12-i); // realign string with beginning
			return out_string;
		}
		else{
			if(neg){
				out_string[i] = (-(in_number % 10)) + '0'; // mod of negative number is still negative
			}
			else{
				out_string[i] = (in_number % 10) + '0';
			}
		}
		in_number /= 10;
	}

	return out_string;
}