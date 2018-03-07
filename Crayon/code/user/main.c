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

	memory_load_crayon_packer_sheet(&Fade, "/colourMod/Fade.dtex");	//Need to finish memory_load_packer_sheet function
	memory_load_dtex(&Insta, "/colourMod/Insta");

	fs_romdisk_unmount("/colourMod");

	int done = 0;
	uint8_t frame = 0;
	uint16_t frame_x;
	uint16_t frame_y;
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

		pvr_stats_t stats;  //This can be defined outside the loop if you want
    	pvr_get_stats(&stats);
    	int curframe = stats.frame_count%30;  //256 frames of transition (This is kinda like modulo, 0xff means take the bottom 8 bits)
    	if(curframe == 0){
    		frame++;
    		if(frame >= Fade.spritesheet_animation_array[2].animation_frames){
    			frame = 0;
    		}
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[2], &frame_x, &frame_y, frame);	//Generates the new frame coordinates
    	}

		graphics_draw_paletted_sprite(&Fade, &Fade.spritesheet_animation_array[0], 128, 176, 0, 0, 0);	//The new draw-er for anims
		graphics_draw_paletted_sprite(&Fade, &Fade.spritesheet_animation_array[2], 295, 215, 0, frame_x, frame_y);	//The "square wheel"
		
		old_graphics_draw_paletted_sprite(&Insta, 384, 176, 1);	//The old drawer that only draws single sprites
		pvr_list_finish();

		pvr_scene_finish();
   	}

	error_freeze("Goodbye world!\n");

    return 0;
}