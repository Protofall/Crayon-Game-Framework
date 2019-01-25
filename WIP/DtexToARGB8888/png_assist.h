#ifndef PNG_ASSIST_H
#define PNG_ASSIST_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <png.h>

//The content of these functions comes from this post on github by niw
	//https://gist.github.com/niw/5963798
void read_png_file(char *filename);
void write_png_file(char *filename);
void process_png_file();

#endif
