//Crayon libraries
#include "../crayon/dreamcast/memory.h"
#include "../crayon/dreamcast/debug.h"
#include "../crayon/dreamcast/graphics.h"
//#include "../crayon/dreamcast/render_structs.h"	//This is included in other crayon files
//#include "../crayon/dreamcast/texture_structs.h"	//This is included in other crayon files

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

int main(){
	vid_set_mode(DM_640x480_VGA, PM_RGB565);
	//lxdream terminal says:
		//vid_set_mode: 640x480IL NTSC
		//pvr: enabling vertical scaling for non-VGA
	//What :/

	pvr_init_defaults(); // The defaults only do OP and TR but not PT and the modifier OP and TR so thats why it wouldn't work before

	mount_romdisk("/cd/colourMod.img", "/colourMod");
	spritesheet_t Fade, Insta;

	//memory_load_dtex(&Fade, "/colourMod/Fade");
	int res = memory_load_crayon_packer_sheet(&Fade, "/colourMod/Fade.dtex");	//Need to finish memory_load_packer_sheet function

	//error_freeze("%d", res);
	//error_freeze("Results: %d, %d, %d", Fade.spritesheet_dims, Fade.spritesheet_format, Fade.spritesheet_color_count);

	memory_load_crayon_packer_sheet(&Insta, "/colourMod/Insta.dtex");

	fs_romdisk_unmount("/colourMod");

	int done = 0;
	while(!done){
	    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

	    if(st->buttons & CONT_START){ // Quits if start is pressed. Screen goes black (This code behaves weirdly, I don't get it)
	      done = 1;
	    }

   		MAPLE_FOREACH_END()

   		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);

		graphics_setup_palette(0, &Fade);
		graphics_setup_palette(1, &Insta);

		temp_graphics_draw_paletted_sprite(&Fade, 128, 176, 0);
		//old_graphics_draw_paletted_sprite(&Insta, 384, 176, 1);
		pvr_list_finish();

		pvr_scene_finish();
   	}

	error_freeze("Goodbye world!\n");

    return 0;
}