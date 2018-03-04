#include "memory.h"

typedef struct dtex_header{
  uint8_t magic[4]; //magic number "DTEX"
  uint16_t   width; //texture width in pixels
  uint16_t  height; //texture height in pixels
  uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
  uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
  uint8_t     magic[4]; //magic number "DPAL"
  uint32_t color_count; //number of 32-bit ARGB palette entries
} dpal_header_t;

extern int memory_load_dtex(struct spritesheet *ss, char *path){  //Note: It doesn't set the name

  int result = 0;
  pvr_ptr_t texture = NULL;
  uint32_t *palette = NULL; //The list of palettes entries

  // Open all files
  //---------------------------------------------------------------------------

  #define ERROR(n) {result = (n); goto cleanup;}

  //If you somehow have a longer path then please increase the size of texName and palName
  char texName[80];
  sprintf(texName, "%s.dtex", path);

  FILE *texture_file = fopen(texName, "rb");

  if(!texture_file){ERROR(1);}

  // Load texture
  //---------------------------------------------------------------------------

  dtex_header_t dtex_header;
  if(fread(&dtex_header, sizeof(dtex_header), 1, texture_file) != 1){ERROR(2);}

  if(memcmp(dtex_header.magic, "DTEX", 4)){ERROR(3);}

  texture = pvr_mem_malloc(dtex_header.size);
  if(!texture){ERROR(4);}

  if(fread(texture, dtex_header.size, 1, texture_file) != 1){ERROR(5);}

  // Write all metadata except palette stuff
  //---------------------------------------------------------------------------

  ss->spritesheet_width       = dtex_header.width;
  ss->spritesheet_height      = dtex_header.height;
  ss->spritesheet_texture     = texture;

  //This assumes no mip-mapping, no stride, twiddled on, uncompressed and no stride setting (I'm doing this to save on space)
  if(dtex_header.type == 0x08000000){ //RGB565
    ss->spritesheet_format = 1;
  }
  else if(dtex_header.type == 0x10000000){ //ARGB4444
    ss->spritesheet_format = 2;
  }
  else if(dtex_header.type == 0x28000000){ //PAL4BPP
    ss->spritesheet_format = 3;
  }
  else if(dtex_header.type == 0x30000000){ //PAL8BPP
    ss->spritesheet_format = 4;
  }
  else{
    ss->spritesheet_format = 0;
    ERROR(6);
  }

  // Load palette if needed
  //---------------------------------------------------------------------------

  #define PAL_ERROR(n) {result = (n); goto PAL_cleanup;}
  
  if(ss->spritesheet_format == 3 || ss->spritesheet_format == 4){

    char palName[84];
    sprintf(palName, "%s.dtex.pal", path);
    FILE *palette_file = fopen(palName, "rb");
    if(!palette_file){PAL_ERROR(7);}

    dpal_header_t dpal_header;
    if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){PAL_ERROR(8);}

    if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){PAL_ERROR(9);}

    palette = malloc(dpal_header.color_count * sizeof(uint32_t));
    if(!palette){PAL_ERROR(10);}

    if(fread(palette, sizeof(uint32_t), dpal_header.color_count,
      palette_file) != dpal_header.color_count){PAL_ERROR(11);}

  #undef PAL_ERROR

  // Write palette metadata
  //---------------------------------------------------------------------------

    ss->spritesheet_palette     = palette;
    ss->spritesheet_color_count = dpal_header.color_count;

    // Palette Cleanup
    //---------------------------------------------------------------------------
    PAL_cleanup:

    if(palette_file){fclose(palette_file);}
    goto cleanup;
  }
  else{
    ss->spritesheet_palette = NULL; //color_count doesn't need to be defined...nor does palette really...
  }

  //Insert the path + ".txt" code here. Need to figure out how I'll build that .txt first
  //ASince every spritesheet has a txt file to go with it, I'll assume that it must be here hence its not optional

  #undef ERROR

  // Cleanup
  //---------------------------------------------------------------------------
  cleanup:

  if(texture_file){fclose(texture_file);}

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  return result;
}

//Need to work on this. It will contain a lot of content from memory_load_dtex, but more specialised stuff and call "memory_load_palette"
extern int memory_load_crayon_packer_sheet(struct spritesheet *ss, char *path){
  //The goal of this it to take in a .dtex from a texturepacker png and store its info in the spritesheet struct and each of its "anims"
  //in anim structs. The spritesheet stores a list of all anim structs related to it

  int result = 0;
  pvr_ptr_t texture = NULL;
  //uint32_t *palette = NULL; //The list of palettes entries (Should probs move this into "load palette" section)

  #define ERROR(n) {result = (n); goto cleanup;}

  FILE *texture_file = fopen(path, "rb");

  if(!texture_file){ERROR(1);}

  // Load texture
  //---------------------------------------------------------------------------

  dtex_header_t dtex_header;
  if(fread(&dtex_header, sizeof(dtex_header), 1, texture_file) != 1){ERROR(2);}

  if(memcmp(dtex_header.magic, "DTEX", 4)){ERROR(3);}

  texture = pvr_mem_malloc(dtex_header.size);
  if(!texture){ERROR(4);}

  if(fread(texture, dtex_header.size, 1, texture_file) != 1){ERROR(5);}

  // Write all metadata except palette stuff
  //---------------------------------------------------------------------------

  if(dtex_header.width > dtex_header.height){
    ss->spritesheet_dims = dtex_header.width;
  }
  else{
    ss->spritesheet_dims = dtex_header.height;
  }

  //error_freeze("%d, %d", dtex_header.width, dtex_header.height);  //Need to figure out how non-square textures work
  //error_freeze("dims: %d", ss->spritesheet_dims);  //Need to figure out how non-square textures work

  //ss->spritesheet_width = dtex_header.width;  //With spritesheet_dims, I don't think we need width and height anymore
  //ss->spritesheet_height = dtex_header.height;
  ss->spritesheet_texture = texture;

  //This assumes no mip-mapping, no stride, twiddled on, its uncompressed and no stride setting (I'm doing this to save on space)
  if(dtex_header.type == 0x08000000){ //RGB565
    ss->spritesheet_format = 1;
  }
  else if(dtex_header.type == 0x10000000){ //ARGB4444
    ss->spritesheet_format = 2;
  }
  else if(dtex_header.type == 0x28000000){ //PAL4BPP
    ss->spritesheet_format = 3;
  }
  else if(dtex_header.type == 0x30000000){ //PAL8BPP
    ss->spritesheet_format = 4;
  }
  else{
    //ss->spritesheet_format = 0;
    ERROR(6);
  }

  int temp = strlen(path);
  if(ss->spritesheet_format == 3 || ss->spritesheet_format == 4){
    char *palPath = (char*) malloc((temp+5)*sizeof(char));  //Add a check here to see if it failed
    palPath = path;
    palPath[temp+0] = '.';
    palPath[temp+1] = 'p';
    palPath[temp+2] = 'a';
    palPath[temp+3] = 'l';
    palPath[temp+4] = '\0';
    //error_freeze("%s", palPath);
    int resultPal = memory_load_palette(&ss->spritesheet_palette, &ss->spritesheet_color_count, path);
      //If it fails it needs to return an error code. If succeeds then it must return the palette and colour count (And return 0)
      //That must mean we need to pass it pointers to the ss struct to save into and make it return an int
    error_freeze("%s", resultPal);
    free(palPath);
    if(resultPal != 0){ //Might be able to remove != 0 and have same result
      ERROR(6 + resultPal);
    }
  }
  else{
    ss->spritesheet_palette = NULL; //color_count doesn't need to be defined...nor does palette really...
  }

  //Load the txt and use that data to make the anim structs

  #undef ERROR

  cleanup:

  if(texture_file){fclose(texture_file);}

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  return result;
}

extern int memory_load_palette(uint32_t **palette, uint16_t *palColours, char *path){
  int result = 0;

  #define PAL_ERROR(n) {result = (n); goto PAL_cleanup;}
  
  //error_freeze("%d", strcmp(path, "/colourMod/Fade.dtex.pal")); //Returns 0   (Its dtex.pal you derp!)
  FILE *palette_file = fopen(path, "rb");
  if(!palette_file){PAL_ERROR(1);}

  //error_freeze("Defiance!");

  dpal_header_t dpal_header;
  if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){PAL_ERROR(2);}

  if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){PAL_ERROR(3);}

  palette = malloc(dpal_header.color_count * sizeof(uint32_t));
  if(!palette){PAL_ERROR(4);}

  if(fread(palette, sizeof(uint32_t), dpal_header.color_count,
    palette_file) != dpal_header.color_count){PAL_ERROR(5);}

  #undef PAL_ERROR

  // Write palette metadata
  //---------------------------------------------------------------------------

  *palette    = palette;
  *palColours = dpal_header.color_count;
  //error_freeze("%d", dpal_header.color_count);
  //error_freeze("%d", palColours);

  // Palette Cleanup
  //---------------------------------------------------------------------------
  PAL_cleanup:

  if(palette_file){fclose(palette_file);}

  return result;
}

//Path would be the path to the dtex file, except without the .dtex attached. An example would be "/levels/Fade"
//Might remove this later since its kinda pointless
extern int memory_init_spritesheet(char *path, struct spritesheet *ss){
  int result = memory_load_dtex(ss, path);
  if(result){
    //error_freeze("Cannot load Fade sprite! Error %d\n", result);
    return 1;
  }
  return 0;
}

//Need to adapt this to take in an ss list object instead so we don't loose the later half of the list
extern int memory_spritesheet_free(struct spritesheet *ss){
  if(ss){
    if(ss->spritesheet_format == 3 || ss->spritesheet_format == 4){ //Paletted
      free(ss->spritesheet_palette);
    }
    pvr_mem_free(ss->spritesheet_texture);

    //Free all anim structs too

    //free(ss); //Need to make sure it's neighbours link to each other before doing this
                //Also free all anims before this
    return 0;
  }
  return 1;
}

extern int mount_romdisk(char *filename, char *mountpoint){
  void *buffer;
  ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM

  if(size != -1){
    fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
    return 0;
  }
  return 1;
}

extern int mount_romdisk_gz(char *filename, char *mountpoint){
  void *buffer;
  int length = zlib_getlength(filename);

  //Later add check to see if theres enough available main ram
   
  // Check failure
  if(length == 0){
      return 1;
  }
   
  // Open file
  gzFile file = gzopen(filename, "rb"); //Seems to be the replacement of fs_load() along with gzread()
  if(!file){
      return 1;
  }

  // Allocate memory, read file
  buffer = malloc(length);
  gzread(file, buffer, length);
  gzclose(file);

  fs_romdisk_mount(mountpoint, buffer, 1);
  return 0;
}