//Crayon libraries
#include "../crayon/dreamcast/memory.h"
#include "../crayon/dreamcast/debug.h"
#include "../crayon/dreamcast/graphics.h"
//#include "../crayon/dreamcast/render_structs.h"	//This is included in other crayon files
//#include "../crayon/dreamcast/texture_structs.h"	//This is included in other crayon files

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing

uint8_t graphics_easy_whole_ss(struct spritesheet *ss, float draw_x, float draw_y, float draw_z, uint8_t paletteNumber){
  const float x0 = draw_x;
  const float y0 = draw_y;
  const float x1 = draw_x + ss->spritesheet_dims;
  const float y1 = draw_y + ss->spritesheet_dims;
  const float z = draw_z;
  const float u0 = 0;
  const float v0 = 0;
  const float u1 = 1;
  const float v1 = 1;

  pvr_sprite_cxt_t context;
  if(ss->spritesheet_format == 4){  //PAL8BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(paletteNumber),
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else if(ss->spritesheet_format == 3){ //PAL4BPP format
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_4BPP_PAL(paletteNumber),
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else if(ss->spritesheet_format == 1 || ss->spritesheet_format == 2){  //RGB565 and RGB4444
    pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, (ss->spritesheet_format) << 27,
    ss->spritesheet_dims, ss->spritesheet_dims, ss->spritesheet_texture, PVR_FILTER_NONE);
  }
  else{ //Unknown format
    return 1;
  }

  pvr_sprite_hdr_t header;
  pvr_sprite_compile(&header, &context);
  pvr_prim(&header, sizeof(header));

  pvr_sprite_txr_t vert = {
    .flags = PVR_CMD_VERTEX_EOL,
    .ax = x0, .ay = y0, .az = z, .auv = PVR_PACK_16BIT_UV(u0, v0),
    .bx = x1, .by = y0, .bz = z, .buv = PVR_PACK_16BIT_UV(u1, v0),
    .cx = x1, .cy = y1, .cz = z, .cuv = PVR_PACK_16BIT_UV(u1, v1),
    .dx = x0, .dy = y1
  };
  pvr_prim(&vert, sizeof(vert));

  return 0;
}

int main(){
	vid_set_mode(DM_640x480_VGA, PM_RGB565);
	//lxdream terminal says:
		//vid_set_mode: 640x480IL NTSC
		//pvr: enabling vertical scaling for non-VGA
	//What :/

	pvr_init_defaults(); // The defaults only do OP and TR but not PT and the modifier OP and TR so thats why it wouldn't work before

	memory_mount_romdisk("/cd/colourMod.img", "/colourMod");
	spritesheet_t Fade, Enlarge, Blanka;

	memory_load_crayon_packer_sheet(&Fade, "/colourMod/Fade.dtex");
	memory_load_crayon_packer_sheet(&Enlarge, "/colourMod/Enlarge.dtex");
	memory_load_crayon_packer_sheet(&Blanka, "/colourMod/SF2.dtex");

	fs_romdisk_unmount("/colourMod");

	int done = 0;
	uint8_t frame = 0;
	uint8_t frame2 = 0;
	uint16_t frame_x;
	uint16_t frame_y;
	// uint16_t frame_x2;
	// uint16_t frame_y2;
	// uint16_t blanka_frame_x;
	// uint16_t blanka_frame_y;

	//The coin positions
	uint16_t draw_coords[6];	//3 sprites, format x,y,repeat
	uint16_t coin_frame_data[6];
	uint16_t coord_entries = 3;	// = num Sprites
	draw_coords[0] = 324;
	draw_coords[1] = 248;
	draw_coords[2] = 284;
	draw_coords[3] = 284;
	draw_coords[4] = 312;
	draw_coords[5] = 320;

	//All 10 blanka coords (Just in top row)
	uint16_t blanka_coords[20];
	uint16_t blanka_frame_data[2];
	uint16_t blanka_coord_entries;	//Will end up at 20
	for(blanka_coord_entries = 0; blanka_coord_entries < 20; blanka_coord_entries = blanka_coord_entries + 2){
		blanka_coords[blanka_coord_entries] = 0 + (64 * (blanka_coord_entries/2));	//Each Blanka has an x thats 64 greater than previous. Start at 0
		blanka_coords[blanka_coord_entries + 1] = 0;	//Y = 0
	}
	blanka_coord_entries = 10;

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
		graphics_setup_palette(1, &Blanka);	//Blanka's palette needs to start at either 0, 256, 512 or 768... :(

		pvr_stats_t stats;  //This can be defined outside the loop if you want
    	pvr_get_stats(&stats);
    	int curframe = stats.frame_count%30;  //256 frames of transition (This is kinda like modulo, 0xff means take the bottom 8 bits)
    	int curframe2 = stats.frame_count%10;
    	if(curframe == 0){
    		frame++;
    		if(frame >= Fade.spritesheet_animation_array[2].animation_frames){
    			frame = 0;
    		}
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[2], &frame_x, &frame_y, frame);	//Generates the new frame coordinates
    	}
    	if(curframe2 == 0){
    		frame2++;
    		// if(frame2 >= Fade.spritesheet_animation_array[4].animation_frames){
    		// 	frame2 = 0;
    		// }
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[4], coin_frame_data, coin_frame_data + 1, frame2 % 4);	//Not being set right
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[4], coin_frame_data + 2, coin_frame_data + 3, (frame2 + 1) % 4);	//Not being set right
    		graphics_frame_coordinates(&Fade.spritesheet_animation_array[4], coin_frame_data + 4, coin_frame_data + 5, (frame2 + 2) % 4);	//Not being set right
    		graphics_frame_coordinates(&Blanka.spritesheet_animation_array[0], blanka_frame_data, blanka_frame_data + 1, frame2 % 4);
    	}
    	// error_freeze("%d, %d, %d, %d, %d, %d", coin_frame_data[0], coin_frame_data[1], coin_frame_data[2], coin_frame_data[3], coin_frame_data[4], coin_frame_data[5]);

		graphics_draw_sprite(&Enlarge, &Enlarge.spritesheet_animation_array[0], 384, 176, 1, 16, 16, 0, 0, 1);	//Draws the right sprite 16 times larger
		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[0], 128, 176, 1, 1, 1, 0, 0, 0);	//The new draw-er for anims
		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[2], 295, 215, 1, 1, 1, frame_x, frame_y, 0);	//The "square wheel"
		graphics_draw_sprite(&Fade, &Fade.spritesheet_animation_array[3], 192, 360, 1, 1, 1, Fade.spritesheet_animation_array[3].animation_x, Fade.spritesheet_animation_array[3].animation_y, 0);	//The "bottom message"
		
		//graphics_draw_sprite(&Blanka, &Blanka.spritesheet_animation_array[0], 0, 0, 1, 1, 1, blanka_frame_x, blanka_frame_y, 1);	//Draw Blanka idle anim
		//graphics_draw_sprites(&Blanka, &Blanka.spritesheet_animation_array[0], blanka_coords, blanka_coord_entries, 1, 1, 1, blanka_frame_x, blanka_frame_y, 1);	//Draw Blanka idle anim
		graphics_draw_sprites(&Blanka, &Blanka.spritesheet_animation_array[0], blanka_coords, blanka_frame_data, 2, blanka_coord_entries, 1, 1, 1, 1);	//Draw Blanka idle anim

		//Unsure if I'm passing draw_coords in right
		//graphics_draw_sprites(&Fade, &Fade.spritesheet_animation_array[4], draw_coords, coord_entries, 1, 1, 1, frame_x2, frame_y2, 0);	//The pink coin
		graphics_draw_sprites(&Fade, &Fade.spritesheet_animation_array[4], draw_coords, coin_frame_data, 6, coord_entries, 1, 1, 1, 0);	//The pink coin

		pvr_list_finish();

		pvr_scene_finish();
   	}

   	int retVal2 = 0;
   	retVal2 += memory_free_crayon_packer_sheet(&Fade);
   	retVal2 += memory_free_crayon_packer_sheet(&Enlarge);
   	retVal2 += memory_free_crayon_packer_sheet(&Blanka);
	error_freeze("Free-ing result %d!\n", retVal2);

    return 0;
}