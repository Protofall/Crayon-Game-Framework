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

	memory_mount_romdisk("/cd/colourMod.img", "/colourMod");
	spritesheet_t Fade, Enlarge;

	memory_load_crayon_packer_sheet(&Fade, "/colourMod/Fade.dtex");
	memory_load_crayon_packer_sheet(&Enlarge, "/colourMod/Enlarge.dtex");

	fs_romdisk_unmount("/colourMod");

	int done = 0;
	uint8_t frame = 0;
	uint8_t frame2 = 0;
	uint16_t frame_x;
	uint16_t frame_y;
	uint16_t frame_x2;
	uint16_t frame_y2;
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
		graphics_setup_palette(1, &Enlarge);

		pvr_stats_t stats;  //This can be defined outside the loop if you want
    	pvr_get_stats(&stats);
    	int curframe = stats.frame_count%30;  //256 frames of transition (This is kinda like modulo, 0xff means take the bottom 8 bits)
    	int curframe2 = stats.frame_count%10;  //256 frames of transition (This is kinda like modulo, 0xff means take the bottom 8 bits)
    	if(curframe == 0){
    		frame++;
    		if(frame >= Fade.spritesheet_animation_array[2].animation_frames){
    			frame = 0;
    		}
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[2], &frame_x, &frame_y, frame);	//Generates the new frame coordinates
    	}
    	if(curframe2 == 0){
    		frame2++;
    		if(frame2 >= Fade.spritesheet_animation_array[4].animation_frames){
    			frame2 = 0;
    		}
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[4], &frame_x2, &frame_y2, frame2);	//Generates the new frame coordinates
    	}

		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[0], 128, 176, 1, 1, 1, 0, 0, 0);	//The new draw-er for anims
		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[2], 295, 215, 1, 1, 1, frame_x, frame_y, 0);	//The "square wheel"
		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[4], 312, 320, 1, 1, 1, frame_x2, frame_y2, 0);	//The pink coin
		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[3], 192, 360, 1, 1, 1, Fade.spritesheet_animation_array[3].animation_x, Fade.spritesheet_animation_array[3].animation_y, 0);	//The "bottom message"

		graphics_draw_sprite(&Enlarge, &Enlarge.spritesheet_animation_array[0], 384, 176, 1, 16, 16, 0, 0, 1);	//Draws the right sprite 16 times larger

		pvr_list_finish();

		pvr_scene_finish();
   	}

   	int retVal2 = 0;
   	retVal2 += memory_free_crayon_packer_sheet(&Fade);
   	retVal2 += memory_free_crayon_packer_sheet(&Enlarge);
	error_freeze("Free-ing result %d!\n", retVal2);

    return 0;
}