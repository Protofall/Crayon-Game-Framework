//Contains a lot of (modified) code from this tutorial: http://dcemulation.org/?title=PVR_Spritesheets
//Also note insta's bg colour is manally be replaced down on line 300 or something

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <arch/timer.h> //Might not be needed for this example
#include <dc/pvr.h>
#include <dc/maple.h>   //For the "Press start" thing
#include <dc/maple/controller.h>
#include <kos/fs_romdisk.h> //For romdisk swapping

struct dtex_header {
  uint8_t  magic[4]; //'DTEX'
  uint16_t width; //Goes into ss width
  uint16_t height;  //Same but for height
  uint32_t type;  //I'm guessing this is for 8bpp or 4bpp
  uint32_t size;
};

//DTEX .pal palette files also have a header (this is from romdisks/rom1/file.dtex.pal)
struct dpal_header {
  uint8_t  magic[4]; // 'DPAL'  I dunno why its called magic though, probs just the "magic number/code"
  uint32_t color_count;
};

struct spritesheet {
  uint16_t width;         // Dimensions of texture to compute UV coordinates of sprites
  uint16_t height;        // later for drawing.
  pvr_ptr_t texture;      // Pointer to texture in video memory,   from file *.dtex
  uint32_t palette[256];  // Palette of texture, up to 256 colors, from file *.dtex.pal
  uint16_t color_count;   // How many colors are actually used in the palette
};

struct spritesheet Fade, Insta;

//spritesheet: Where to store the loaded information
//image_filename: file.dtex
//palette_filename: file.dtex.pal
static int spritesheet_load(struct spritesheet * const spritesheet, char const * const image_filename, char const * const palette_filename){
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
  if(memcmp(dtex_header.magic, "DTEX", 4) || dtex_header.type != 0x30000000){ //memcmp compares first 4 bytes of param 1 to param2
                                                                              //the hex type seems to be found within the file itself
                                                                              //But its not being set anywhere *scratches head
                                                                              //Is it defined in the fread above?
    result = 3;
    goto cleanup;
  }

  texture = pvr_mem_malloc(dtex_header.size);
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

  if(fread(spritesheet->palette, sizeof(uint32_t), dpal_header.color_count, palette_file) != dpal_header.color_count){
    result = 8;
    goto cleanup;
  }

  // Store info into the struct
  spritesheet->width        = dtex_header.width;
  spritesheet->height       = dtex_header.height;
  spritesheet->texture      = texture;
  spritesheet->color_count  = dpal_header.color_count;

cleanup:
  if(image_file){fclose(image_file);}
  if(palette_file){fclose(palette_file);}

  //If a failure occured somewhere
  if(result){
    pvr_mem_free(texture);
  }

  return result;
}

//Free ressources used by spritesheet, if any.
void spritesheet_free(struct spritesheet * const spritesheet) {
  if(!spritesheet){
    return;
  }

  pvr_mem_free(spritesheet->texture);

  memset(spritesheet, 0, sizeof(struct spritesheet)); //Seems to fill the spritesheet with "sizeof(struct spritesheet)"" amount of zeroes
                                                      //Seems redundant though in fact it feels more useful not to have this so you know
                                                      //Can see garbage info when you mess up as oppose to nothing

                                                      //In the Crayon library it would be better to delete the struct here rather than use
                                                      //memset. I'll have to think about how I pass stuff around if they're not global
}

static int init_pvr() {
  int result = 0;

  /* Initialize the PVR graphics chip.
   * Set how many polygons can be stored in each of the PVRpolygon lists.
   * Only opaque and punchthru are enabled in this tutorial, only punchthru is used.
   */
  pvr_init_params_t pvr_params = {
    .opb_sizes = { PVR_BINSIZE_8, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_8 },
    .vertex_buf_size = 512 * 1024
  };
  if(pvr_init(&pvr_params)) {
    result = 1;
  }
  //pvr_set_bg_color(0.3, 0.3, 0.3);  //Remove this later (Appears to crash before it gets here)

  return result;
}

static int init_tex(){
  int result = 0;
  //Load spritesheets (Why is it nessisary to zero these out first?)
  memset(&Fade, 0, sizeof(Fade));
  memset(&Insta, 0, sizeof(Insta));

  result = spritesheet_load(&Fade, "/levels/Fade.dtex", "/levels/Fade.dtex.pal");
  if(result){
    printf("Cannot load stage1_actors spritesheet! Error %d\n", result);
    result = 1;
    goto cleanup;
  }

  result = spritesheet_load(&Insta, "/levels/Insta.dtex", "/levels/Insta.dtex.pal");
  if(result){
    printf("Cannot load ui spritesheet! Error %d\n", result);
    result = 2;
    goto cleanup;
  }

cleanup:
  if(result) {
    spritesheet_free(&Fade);
    spritesheet_free(&Insta);
  }

  return result;
}

void setup_palette(uint32_t const * colors, uint16_t count, uint8_t palette_number) {
  pvr_set_pal_format(PVR_PAL_ARGB8888);
  uint16_t i;
  for(i = 0; i < count; ++i){
    pvr_set_pal_entry(i + 256 * palette_number, colors[i]);
  }
}

void draw_sprite(struct spritesheet const * const sheet, float x, float y, uint8_t palette_number) {

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

  pvr_sprite_cxt_txr(&context, PVR_LIST_PT_POLY, PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(palette_number), sheet->width, sheet->height, sheet->texture, PVR_FILTER_NONE);
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

  pvr_list_begin(PVR_LIST_PT_POLY);
  setup_palette(Fade.palette, Fade.color_count, 0);
  setup_palette(Insta.palette, Insta.color_count, 1);

  draw_sprite(&Fade, 128, 176, 0);
  draw_sprite(&Insta, 384, 176, 1);
  pvr_list_finish();

  pvr_scene_finish();
  //Insta.palette[0] = insta.palette[0] - 1;  //Gives it a weird sky blue to green transition effect and then abruptly jumps back to sky blue
}

void cleanup(){
  // Clean up the texture memory we allocated earlier

  spritesheet_free(&Fade);
  spritesheet_free(&Insta);

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
  //pvr_init_defaults();

  if(init_pvr()){ //Seems to work fine
    puts("Cannot init pvr");
    return -1;
  }

  mount_romdisk("/cd/rom1.img", "/levels"); //Trying to mount the first img to the romdisk

  if(init_tex()){
    puts("Cannot init textures");
    return -1;
  }

  fs_romdisk_unmount("/levels");

  //Testing modifying the palettes
  Insta.palette[0] = 0xffff0000;  //Should set the background of Insta to full red (Don't forget alpha).
  //Insta.palette[0] = 0xffff0000 - 0x00ff0000 + 0x000000ff;  //Pure blue. ALso palette[1] is the text itself (This is same as setting it to zero)

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

  /*
  //If you set a pvr bg then this can be an "alternative" to printfs (Since printf's don't work on CDIs)
  while(1){
    pvr_wait_ready();
    pvr_scene_begin();
    pvr_scene_finish();
  }
  */