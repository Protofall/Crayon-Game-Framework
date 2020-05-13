#include "misc.h"

extern char * crayon_misc_get_version(){
	char * version = malloc(sizeof(char) * 16);
	if(!version){
		return NULL;
	}

	// sprintf(version, "%d.%d.%d", CRAYON_MAJOR_NUMBER, CRAYON_MINOR_NUMBER, CRAYON_PATCH_NUMBER);
	sprintf(version, "%d.%d.%d-dev", CRAYON_MAJOR_NUMBER, CRAYON_MINOR_NUMBER, CRAYON_PATCH_NUMBER);
	return version;
}

extern uint8_t crayon_misc_combine_strings(char ** dest, char * source1, char * source2){
	if(!source1 || !source2){
		return 1;
	}

	uint16_t s1_length = strlen(source1);	//Remember for "good\0" strlen returns 4
	uint16_t s2_length = strlen(source2);
	*dest = malloc(sizeof(char) * (s1_length + s2_length + 1));	// +1 for null-terminator
	if(!(*dest)){	//Somehow the malloc failed. Thats bad!
		return 1;
	}

	memcpy((*dest), source1, s1_length);
	memcpy((*dest + s1_length), source2, s2_length + 1);

	return 0;
}

extern uint8_t crayon_misc_change_extension(char ** dest, char * source, char * extension){
	if(!source || !extension){
		return 1;
	}

	uint16_t i;
	uint16_t source_length = strlen(source);
	for(i = source_length - 1; i > 0; i--){	//Find the beginning of the extension (Works backwards to find last full stop)
		if(source[i] == '.'){i++; break;}
	}

	//There was no dot present, therefore we stop
	if(i == 0){
		return 1;
	}

	uint8_t ext_length = strlen(extension);
	*dest = malloc(sizeof(char) * (i + ext_length + 1));
	if(!(*dest)){
		return 1;
	}

	//Add the part of the source we're keeping then the extension
	memcpy((*dest), source, i);
	memcpy((*dest + i), extension, ext_length + 1);

	return 0;
}

//For example, call this like so (Assuming file is 12 bytes long)
// uint8_t *my_stuff;
// crayon_misc_read_file((void *) &my_stuff, "romdisk/read_data.txt", 12);
extern uint8_t crayon_misc_read_file(void ** buffer, char * path, size_t size_bytes, uint8_t allocated){
	if(!allocated){
		*buffer = malloc(size_bytes);
	}
	FILE * file = fopen(path, "rb");
	if(!file){return 1;}
	size_t res = fread(*buffer, size_bytes, 1, file);
	fclose(file);
	if(res != 1){
		return res;
	}
	return 0;
}

extern uint32_t crayon_misc_fgeti(FILE * f, int16_t * last_char){
	uint32_t n = 0;
	int16_t d;	//EOF is -1 so we can't use a uint8_t
	for(d = 0; d < 10; d = fgetc(f) - '0'){
		n = n * 10 + d;
	}
	if(last_char){
		*last_char = d;
	}
	return n;
}

extern int crayon_misc_fget_next_int(FILE * f, int * number){
	*number = 0;
	uint8_t negative = 0;
	uint8_t started = 0;
	int16_t d;	//EOF is -1 so we can't use a uint8_t
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

extern uint32_t crayon_misc_extract_bits(uint32_t number, uint8_t bit_length, uint8_t offset){
	return (((1 << bit_length) - 1) & (number >> offset));
}

extern uint32_t crayon_misc_insert_bits(uint32_t number_1, uint32_t number_2, uint8_t bit_length, uint8_t offset){
	return (number_1 & ~(((1UL << bit_length) - 1) << offset)) | (number_2 << offset);
}
