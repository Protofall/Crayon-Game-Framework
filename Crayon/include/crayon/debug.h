#ifndef DEBUG_CRAYON_H
#define DEBUG_CRAYON_H

#include <stdarg.h>
#include <stdio.h>

#include <dc/biosfont.h>
#include <dc/video.h>
#include <dc/pvr.h>

// Write a message to the screen using the BIOS font, and hang
void error_freeze(const char *format, ...);

// Same as above, but with no pvr_shutdown()
void error_freeze_no_pvr(const char *format, ...);

#endif
