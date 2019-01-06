#include "assist.h"

//For example, call this like so (Assuming file is 12 bytes long)
// uint8_t *my_stuff;
// crayon_assist_read_file((void *) &my_stuff, "romdisk/read_data.txt", 12);
extern uint8_t crayon_assist_read_file(void **buffer, char *path, size_t size_bytes, uint8_t allocated){
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

extern uint32_t crayon_assist_fgeti(FILE *f){
    uint32_t n = 0;
    uint8_t d;
    for(d = 0; d < 10; d = fgetc(f) - '0'){
        n = n * 10 + d;
    }
    return n;
}
