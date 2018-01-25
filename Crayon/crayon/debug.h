#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>

#include <dc/biosfont.h>
#include <dc/video.h>
#include <dc/pvr.h>

//Write a message to the screen using the BIOS font, and hang
extern void error_freeze(const char *format, ...);

#endif
