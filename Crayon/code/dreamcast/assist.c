#include "assist.h"

//How to call: if(crayon_assist_new_extension(&last_path, first_path, "dtex")){return 1;}
extern uint8_t crayon_assist_new_extension(char ** dest, char * source, char * extension){
	if(!source || !extension){
		return 1;
	}
	uint16_t i;
	uint16_t source_length = strlen(source);	//Remember for "good\0" strlen returns 4
	for(i = source_length - 1; i > 0; i--){	//Find the beginning of the extension (Works backwards to find last full stop)
		if(source[i] == '.'){i++; break;}
	}
	if(i == 0){
		return 2;
	}
	*dest = malloc(sizeof(char) * (i + strlen(extension)));
	uint16_t j;
	for(j = 0; j < i; j++){	//Add the all of source except for everything after the last '.'
	    (*dest)[j] = source[j];
	}
	for(i = 0; i < strlen(extension); i++){	//Append the extension
	    (*dest)[j + i] = extension[i];
	}
	return 0;
}

//For example, call this like so (Assuming file is 12 bytes long)
// uint8_t *my_stuff;
// crayon_assist_read_file((void *) &my_stuff, "romdisk/read_data.txt", 12);
extern uint8_t crayon_assist_read_file(void ** buffer, char * path, size_t size_bytes, uint8_t allocated){
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

extern uint32_t crayon_assist_fgeti(FILE * f, int16_t * last_char){
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
