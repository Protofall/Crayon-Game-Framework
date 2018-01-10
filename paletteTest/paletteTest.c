//Contains a lot of (modified) code from this tutorial: http://dcemulation.org/?title=PVR_Spritesheets
//Also note insta's bg colour is manually being replaced down in the main function

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <dc/pvr.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>  //For the "Press start to exit" thing
#include <kos/fs_romdisk.h> //For romdisk swapping

//#define Use_4bpp  //Comment this out if you want to do 8bpp (Note: Makefile would need to be adjusted to 8bpp in the texconv command)

#ifdef Use_4bpp
  #define bpp_Type 0x28000000 //The type is wrong and even then, the textures aren't being displayed right
  #define bpp_Pal_Etr 16
  #define bpp_txrfmt PVR_TXRFMT_PAL4BPP
#else //8bpp
  #define bpp_Type 0x30000000
  #define bpp_Pal_Etr 256
  #define bpp_txrfmt PVR_TXRFMT_PAL8BPP
#endif

struct dtex_header {
  uint8_t  magic[4]; //'DTEX'  I dunno why its called magic though, probs just the "magic number/code"
  uint16_t width; //Goes into sprite width
  uint16_t height;  //Same but for height
  uint32_t type;  //This seems to be for 8bpp or 4bpp
  uint32_t size;
};

//DTEX .pal palette files also have a header (this is from romdisks/rom1/file.dtex.pal)
struct dpal_header {
  uint8_t  magic[4]; // 'DPAL'
  uint32_t color_count;
};

struct sprite {
  uint16_t width;         // Dimensions of texture to compute UV coordinates of sprites
  uint16_t height;        // later for drawing.
  pvr_ptr_t texture;      // Pointer to texture in video memory,   from file *.dtex
  uint32_t palette[bpp_Pal_Etr];  // Palette of texture, up to 16/256 colors, from file *.dtex.pal
  uint16_t color_count;   // How many colors are actually used in the palette
};

struct sprite Fade, Insta;

//sprite: Where to store the loaded information
//image_filename: file.dtex
//palette_filename: file.dtex.pal
static int sprite_load(struct sprite * const sprite, char const * const image_filename, char const * const palette_filename){
  int result = 0;
  FILE *image_file = NULL;
  FILE *palette_file = NULL;
  pvr_ptr_t texture = NULL;


  // Open all files
  image_file = fopen(image_filename, "rb");
  palette_file = fopen(palette_filename, "rb");

  if(!image_file | !palette_file){
    result = 1;
    goto cleanup;
  }

  //Read DTEX texture header (And not the image data itself?)
  struct dtex_header dtex_header;
  if(fread(&dtex_header, sizeof(dtex_header), 1, image_file) != 1){
    result = 2;
    goto cleanup;
  }

  //Only allow 8 bit palette dtex files (dtex files are the basic image, dtex.pal are its palettes and the header contains useful info)
  if(memcmp(dtex_header.magic, "DTEX", 4) || dtex_header.type != bpp_Type){ //memcmp compares first 4 bytes of param 1 to param2
                                                                              //the hex type seems to be found within the file itself
                                                                              //But its not being set anywhere *scratches head
                                                                              //Is it defined in the fread above?
    result = 3;
    goto cleanup;
  }

  texture = pvr_mem_malloc(dtex_header.size); //For 4bpp, something appears to be going wrong when loading the header
  if(!texture){
    result = 4;
    goto cleanup;
  }

  //Reading the image data into the pointer
  if(fread(texture, dtex_header.size, 1, image_file) != 1){ //fread reads data from the image_file stream into the "texture" array
                                                            //Don't really get param 2 and 3, but they work, 3 might be how much it reads
                                                            //per...loop?
    result = 5;
    goto cleanup;
  }

  //Read DTEX palette
  struct dpal_header dpal_header;
  if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){
    result = 6;
    goto cleanup;
  }

  if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){
    result = 7;
    goto cleanup;
  }

  if(fread(sprite->palette, sizeof(uint32_t), dpal_header.color_count, palette_file) != dpal_header.color_count){
    result = 8;
    goto cleanup;
  }

  //Store info into the struct
  sprite->width        = dtex_header.width;
  sprite->height       = dtex_header.height;
  sprite->texture      = texture;
  sprite->color_count  = dpal_header.color_count;

cleanup:
  if(image_file){fclose(image_file);}
  if(palette_file){fclose(palette_file);}

  //If a failure occured somewhere
  if(result){
    pvr_mem_free(texture);
  }

  return result;
}

//Free ressources used by sprite, if any.
void sprite_free(struct sprite * const sprite){
  if(!sprite){
    return;
  }

  pvr_mem_free(sprite->texture);

  //memset(sprite, 0, sizeof(struct sprite)); //Seems to fill the sprite with "sizeof(struct sprite)"" amount of zeroes
                                            //Seems redundant though in fact it feels more useful not to have this so you know
                                            //Can see garbage info when you mess up as oppose to nothing

                                            //In the Crayon library it would be better to delete the struct here rather than use
                                            //memset. I'll have to think about how I pass stuff around if they're not global
}

static int init_txr(){
  int result = 0;
  //Load sprite (Why is it nessisary to zero these out first?)
  //memset(&Fade, 0, sizeof(Fade));
  //memset(&Insta, 0, sizeof(Insta));

  result = sprite_load(&Fade, "/levels/Fade.dtex", "/levels/Fade.dtex.pal");
  if(result){
    printf("Cannot load Fade sprite! Error %d\n", result);
    result = 1;
    goto cleanup;
  }

  result = sprite_load(&Insta, "/levels/Insta.dtex", "/levels/Insta.dtex.pal");
  if(result){
    printf("Cannot load Insta sprite! Error %d\n", result);
    result = 2;
    goto cleanup;
  }

cleanup:
  if(result) {
    sprite_free(&Fade);
    sprite_free(&Insta);
  }

  return result;
}

void setup_palette(uint32_t const * colors, uint16_t count, uint8_t palette_number) {
  pvr_set_pal_format(PVR_PAL_ARGB8888); //Setting 3 here has the same effect since its just a macro :P
  uint16_t i;
  for(i = 0; i < count; ++i){
    pvr_set_pal_entry(i + bpp_Pal_Etr * palette_number, colors[i]);
  }
}

void draw_sprite(struct sprite const * const sheet, float x, float y, uint8_t palette_number) {

  //Setting out draw position variables, since we don't have spritesheets we can simplify this
  float const x0 = x;
  float const y0 = y;
  float const x1 = x + sheet->width;
  float const y1 = y + sheet->height;
  float const z = 1;

  float const u0 = 0;
  float const v0 = 0;
  float const u1 = 1;
  float const v1 = 1;

  pvr_sprite_cxt_t context;
  pvr_sprite_hdr_t header;

  pvr_sprite_cxt_txr(&context, PVR_LIST_TR_POLY, bpp_txrfmt | PVR_TXRFMT_4BPP_PAL(palette_number), sheet->width, sheet->height, sheet->texture, PVR_FILTER_NONE);
  pvr_sprite_compile(&header, &context);

  pvr_prim(&header, sizeof(header));

  pvr_sprite_txr_t vert = {
    .flags = PVR_CMD_VERTEX_EOL,
    .ax = x0,
    .ay = y0,
    .az = z,

    .bx = x1,
    .by = y0,
    .bz = z,

    .cx = x1,
    .cy = y1,
    .cz = z,

    .dx = x0,
    .dy = y1,
    .auv = PVR_PACK_16BIT_UV(u0, v0),
    .buv = PVR_PACK_16BIT_UV(u1, v0),
    .cuv = PVR_PACK_16BIT_UV(u1, v1),
  };

  pvr_prim(&vert, sizeof(vert));
}

void draw_frame(void){
  pvr_wait_ready();
  pvr_scene_begin();

  pvr_list_begin(PVR_LIST_TR_POLY);
  setup_palette(Fade.palette, Fade.color_count, 0);
  setup_palette(Insta.palette, Insta.color_count, 1);

  //Fade is drawn last (So if they were overlapping Fade would be ontop) (Obviously Z value can change this)
  draw_sprite(&Fade, 128, 176, 0);
  draw_sprite(&Insta, 384, 176, 1);
  pvr_list_finish();

  pvr_scene_finish();
  //Insta.palette[0] = insta.palette[0] - 1;  //Gives it a weird sky blue to green transition effect and then abruptly jumps back to sky blue
}

void cleanup(){
  // Clean up the texture memory we allocated earlier
  sprite_free(&Fade);
  sprite_free(&Insta);

  // Shut down the pvr system
  pvr_shutdown();
}

int mount_romdisk(char *filename, char *mountpoint){
  void *buffer;
  ssize_t size = fs_load(filename, &buffer);  //Loads the file "filename" into RAM, user is responsible for freeing memory when done

  // Successfully read romdisk image
  if(size != -1){
    fs_romdisk_mount(mountpoint, buffer, 1);  //Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
      return 0;
    }
  else{
    return -1;
  }
}

int main(){
  pvr_init_defaults();  //The defaults only do OP and TR but not PT and the modifier OP and TR so thats why it wouldn't work before

  /*
  pvr_init_params_t pvr_params = {
    .opb_sizes = { PVR_BINSIZE_8, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_8 },
    .vertex_buf_size = 512 * 1024
  };
  if(pvr_init(&pvr_params)){
    return -1;
  }
  */

  //pvr_set_bg_color(0.3, 0.3, 0.3);  //Its useful-ish for debugging

  mount_romdisk("/cd/rom1.img", "/levels"); //Trying to mount the first img to the romdisk. This could be improved with sprintf

  if(init_txr()){
    printf("Cannot init textures");
    return -1;
  }

  fs_romdisk_unmount("/levels");

  //By default entry 0 is R=50, G=189, B=7 and entry 1 is R=G=B=255 (Pure White)
  //Insta.palette[0] = 0xff32BD07;  //This is the original green
  //Insta.palette[0] = 0xff0000;  //This sets the green for Insta to fully red
  //Insta.palette[0] = 0xffff0000 - 0x00ff0000 + 0x000000ff;  //Pure blue (Done by setting it to black then blue)

  int done = 0;

  while(!done){
    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

    if(st->buttons & CONT_START){  // Quits if start is pressed. Screen goes black (This code behaves weirdly, I don't get it)
      done = 1;
    }

    MAPLE_FOREACH_END()

    draw_frame();
  }

  cleanup();  //Free all usage of RAM and do the pvr_shutdown procedure

  return 0;
}

//The idea would be to make the pictures go from green/white to red/black or something. For Fade I think I'd increment
//every 8 bits individually (So if blue is already good then only modify green and red). Insta just converts instantly
//when Fade had done its transition. Fade effects this way would take 255 transitions max (So about 4.25 seconds for
//60Hz or about 5 seconds for 50Hz) an option to consider is larger increments hence reducing the transition time

/*
  //If you set a pvr bg then this can be an "alternative" to printfs (Since printfs don't work in .cdi programs)
  //However a combination of sprintf() and the BIOS font is a better option...

  while(1){
    pvr_wait_ready();
    pvr_scene_begin();
    pvr_scene_finish();
  }
*/