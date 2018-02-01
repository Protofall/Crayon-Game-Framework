//Crayon libraries
#include "crayon/memory.h"
#include "crayon/debug.h"
#include "crayon/draw.h"
//#include "crayon/render_structs.h"	//This is included in other crayon files
//#include "crayon/texture_structs.h"	//This is included in other crayon files

int main(){
	vid_set_mode(DM_640x480_VGA, PM_RGB565);
	//lxdream terminal says:
		//vid_set_mode: 640x480IL NTSC
		//pvr: enabling vertical scaling for non-VGA
	//What :/

	pvr_init_defaults(); // The defaults only do OP and TR but not PT and the modifier OP and TR so thats why it wouldn't work before

	error_freeze("Goodbye world!\n");

    return 0;
}