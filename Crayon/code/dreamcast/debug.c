#include "debug.h"

extern void error_freeze(const char *format, ...){
	va_list va_args;
	va_start(va_args, format);

	char line[80]; //Do not allocate heap memory, as we might have run out of it
	vsprintf(line, format, va_args);

	pvr_shutdown(); //Stop any drawing in progress
	bfont_set_encoding(BFONT_CODE_ISO8859_1);
	bfont_draw_str(vram_s, 640, 1, line);
	// va_end(va_args);

	//Freeze
	while(1){;}
}