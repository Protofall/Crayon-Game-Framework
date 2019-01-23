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

//For mounting the sd dir
#include <dc/sd.h>
#include <kos/blockdev.h>
#include <ext2/fs_ext2.h>

#define MNT_MODE FS_EXT2_MOUNT_READWRITE	//Might manually change it so its not a define anymore

//Contains a lot of (modified) code from this tutorial:
//  http://dcemulation.org/?title=PVR_Spritesheets

//The idea is to make the texture backgrounds go from green to blue.
//Fade's transition takes 256 frames (4.27s at 60Hz or 5.12s at 50Hz). Every fram it modifies the blue and green values by one
//Insta just switches to the other colour every 256 frames. Note Insta uses the original texture's green
//whereas Fade goes from solid green to solid blue.

sprite_t Fade, Insta;

static void unmount_ext2_sd(){
	fs_ext2_unmount("/sd");
    fs_ext2_shutdown();
    sd_shutdown();
}

static int mount_ext2_sd(){
	kos_blockdev_t sd_dev;
    uint8 partition_type;

    // Initialize the sd card if its present
	if(sd_init()){
        return 1;
    }

    // Grab the block device for the first partition on the SD card. Note that
    // you must have the SD card formatted with an MBR partitioning scheme
    if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
        return 2;
    }

    // Check to see if the MBR says that we have a Linux partition
    if(partition_type != 0x83){
        return 3;
    }

    // Initialize fs_ext2 and attempt to mount the device
    if(fs_ext2_init()){
        return 4;
    }

    //Mount the SD card to the sd dir in the VFS
    if(fs_ext2_mount("/sd", &sd_dev, MNT_MODE)){
        return 5;
    }
    return 0;
}


static void init_txr(){ //Currently this only checks for RGB565, ARGB4444, 4bpp and 8bpp textures
  int result;

  result = sprite_load(&Fade, "/levels/Fade");
  if(result){error_freeze("Cannot load Fade sprite! Error %d\n", result);}

  result = sprite_load(&Insta, "/levels/Insta");
  if(result){error_freeze("Cannot load Insta sprite! Error %d\n", result);}
}

static void setup_palette(const uint32_t *colors, uint16_t count, uint8_t palette_number, uint8_t bpp){
  int entries;
  if(bpp == 3){
    entries = 16;
  }
  else if(bpp == 4){
    entries = 256;
  }
  else{
    error_freeze("Wrong palette format! %d passed into bpp param\n", bpp);
  }

  pvr_set_pal_format(PVR_PAL_ARGB8888);
  uint16_t i;
  for(i = 0; i < count; ++i){
    pvr_set_pal_entry(i + entries * palette_number, colors[i]);
  }
}

static void draw_frame(void){
  pvr_wait_ready();
  pvr_scene_begin();

  pvr_list_begin(PVR_LIST_TR_POLY);
  setup_palette(Fade.palette, Fade.color_count, 0, Fade.format);
  setup_palette(Insta.palette, Insta.color_count, 1, Insta.format);

  draw_sprite(&Fade, 128, 176, 0, Fade.format);
  draw_sprite(&Insta, 384, 176, 1, Insta.format);
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

  int sdRes = mount_ext2_sd();	//This function should be able to mount an ext2 formatted sd card to the /sd dir	
  if(sdRes != 0){
  	error_freeze("sdRes = %d\n", sdRes);
  }

  int res = mount_romdisk("/sd/rom0.img", "/levels");
  //error_freeze("Res = %d\n", res);

  unmount_ext2_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore

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