//These libraries are also in sprite.h and error.h
//#include <stdio.h>
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
//#include <math.h>

//#include <dc/pvr.h>

#include <dc/maple.h>
#include <dc/maple/controller.h> //For the "Press start to exit" thing
#include <kos/fs_romdisk.h> //For romdisk swapping

#include "sprite.h"
#include "error.h"

//Contains a lot of (modified) code from this tutorial:
//  http://dcemulation.org/?title=PVR_Spritesheets

//The idea is to make the texture backgrounds go from green to blue.
//Fade's transition takes 256 frames (4.27s at 60Hz or 5.12s at 50Hz). Every fram it modifies the blue and green values by one
//Insta just switches to the other colour every 256 frames. Note Insta uses the original texture's green
//whereas Fade goes from solid green to solid blue.

//Comment Use_4bpp out if you want to do 8bpp (Note: Makefile would need to be adjusted
//to 8BPP in the texconv command and you must run make clean first to regenerate the
//textures in the right format)

#define Use_4bpp

#ifdef Use_4bpp
  #define bpp_Type 0x28000000
  #define bpp_Pal_Etr 16
  #define bpp_mode 4
#else //Use_8bpp
  #define bpp_Type 0x30000000
  #define bpp_Pal_Etr 256
  #define bpp_mode 8
#endif

sprite_t Fade, Insta;

static void init_txr(){
  int result;

  result = sprite_load(&Fade, "/levels/Fade.dtex", "/levels/Fade.dtex.pal");
  if(result){error_freeze("Cannot load Fade sprite! Error %d\n", result);}
  if(Fade.type != bpp_Type){error_freeze("Fade.dtex has the wrong type: 0x%08x", Fade.type);}

  result = sprite_load(&Insta, "/levels/Insta.dtex", "/levels/Insta.dtex.pal");
  if(result){error_freeze("Cannot load Insta sprite! Error %d\n", result);}
  if(Insta.type != bpp_Type){error_freeze("Insta.dtex has the wrong type: 0x%08x", Insta.type);}

}

static void setup_palette(const uint32_t *colors, uint16_t count, uint8_t palette_number){
  pvr_set_pal_format(PVR_PAL_ARGB8888);
  uint16_t i;
  for(i = 0; i < count; ++i){
    pvr_set_pal_entry(i + bpp_Pal_Etr * palette_number, colors[i]);
  }
}

static void draw_frame(void){
  pvr_wait_ready();
  pvr_scene_begin();

  pvr_list_begin(PVR_LIST_TR_POLY);
  setup_palette(Fade.palette, Fade.color_count, 0);
  setup_palette(Insta.palette, Insta.color_count, 1);

  draw_sprite(&Fade, 128, 176, 0, bpp_mode);
  draw_sprite(&Insta, 384, 176, 1, bpp_mode);
  pvr_list_finish();

  pvr_scene_finish();
}

static void cleanup(){
  // Clean up the texture memory we allocated earlier
  sprite_free(&Fade);
  sprite_free(&Insta);

  // Shut down the pvr system
  pvr_shutdown();
}

static int mount_romdisk(char *filename, char *mountpoint){
  void *buffer;
  ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM, user is responsible for freeing memory when done

  // Successfully read romdisk image
  if(size != -1){
    fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
      return 0;
  }
  return -1;
}

int main(){
  pvr_init_defaults(); // The defaults only do OP and TR but not PT and the modifier OP and TR so thats why it wouldn't work before

  /*
  pvr_init_params_t pvr_params = {
    .opb_sizes = { PVR_BINSIZE_8, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_8 },
    .vertex_buf_size = 512 * 1024
  };
  if (pvr_init(&pvr_params)) {
    return -1;
  }
  */

  // pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

  mount_romdisk("/cd/rom0.img", "/levels"); // Trying to mount the first img to the romdisk. This could be improved with sprintf

  init_txr();

  fs_romdisk_unmount("/levels");

  // By default entry 0 is R=50, G=189, B=7 and entry 1 is R=G=B=255 (Pure White)
  // Insta.palette[0] = 0xffff0000 - 0x00ff0000 + 0x000000ff; // Pure blue (Done by setting it to black then blue)

  int done = 0;
  int flipflop = 0;

  while(!done){
    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

    if(st->buttons & CONT_START){ // Quits if start is pressed. Screen goes black (This code behaves weirdly, I don't get it)
      done = 1;
    }

    MAPLE_FOREACH_END()

    // Insta.palette[0] = insta.palette[0] - 1; // 00ff00 (green), 00feff (cyan) ... 00fe00 (green), 00fdff (cyan), and so on

    pvr_stats_t stats;  //This can be defined outside the loop if you want
    pvr_get_stats(&stats);
    int anim_frame = stats.frame_count & 0xff;  //256 frames of transition (This is kinda like modulo, 0xff means take the bottom 8 bits)

    Fade.palette[0] = 0xff000000 | (anim_frame << 0) | ((255 - anim_frame) << 8);  //This does the palette modification for Fade

    if(anim_frame == 255){  //This does the Insta transition effect
      if(flipflop == 0){
        Insta.palette[0] = 0xff0000ff; // This is fully blue
        flipflop++;
      }
      else{
        Insta.palette[0] = 0xff32BD07; // This is the original green
        flipflop--;
      }
    }

    draw_frame();
  }

  cleanup(); // Free all memory and do the pvr_shutdown procedure

  return 0;
}