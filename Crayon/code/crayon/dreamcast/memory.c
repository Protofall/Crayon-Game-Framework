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

  if(dtex_header.width > dtex_header.height){
    ss->spritesheet_dims = dtex_header.width;
  }
  else{
    ss->spritesheet_dims = dtex_header.height;
  }

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

  #undef ERROR

  // Cleanup
  //---------------------------------------------------------------------------
  cleanup:

  if(texture_file){fclose(texture_file);}

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  return result;
}

//Need to work on this. Also it doesn't set the ss name
extern int memory_load_crayon_packer_sheet(struct spritesheet *ss, char *path){
  //The goal of this it to take in a .dtex from a texturepacker png and store its info in the spritesheet struct and each of its "animations"
  //in animation structs. The spritesheet stores a list of all animation structs related to it

  int result = 0;
  pvr_ptr_t texture = NULL;

  #define ERROR(n) {result = (n); goto cleanup;}

  FILE *sheet_file = NULL;
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

  ss->spritesheet_texture = texture;
  ss->spritesheet_palette = NULL; //If we don't set it, memory_load_palette possibly won't work correctly

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
    ERROR(6);
  }

  int temp = strlen(path);
  if(ss->spritesheet_format == 3 || ss->spritesheet_format == 4){
    char *pathPal = (char *) malloc((temp+5)*sizeof(char));  //Add a check here to see if it failed
    strcpy(pathPal, path);
    pathPal[temp] = '.';
    pathPal[temp+1] = 'p';
    pathPal[temp+2] = 'a';
    pathPal[temp+3] = 'l';
    pathPal[temp+4] = '\0';
    int resultPal = memory_load_palette(&ss->spritesheet_palette, &ss->spritesheet_color_count, pathPal); //The function will modify the palette and colour count
    free(pathPal);
    if(resultPal){
      ERROR(6 + resultPal);
    }
  }
  else{
    ss->spritesheet_palette = NULL; //color_count doesn't need to be defined...nor does palette really...
  }

  char *pathTxt = (char *) malloc((temp)*sizeof(char));  //Add a check here to see if it failed
  strncpy(pathTxt, path, temp-4);
  pathTxt[temp-4] = 't';
  pathTxt[temp-3] = 'x';
  pathTxt[temp-2] = 't';
  pathTxt[temp-1] = '\0';

  sheet_file = fopen(pathTxt, "rb");
  free(pathTxt);
  if(!sheet_file){ERROR(12);}
  fscanf(sheet_file, "%d\n", &ss->spritesheet_animation_count);

  ss->spritesheet_animation_array = (animation_t *) malloc(sizeof(animation_t) * ss->spritesheet_animation_count);
  
  int i;
  for(i = 0; i < ss->spritesheet_animation_count; i++){
    int scanned = fscanf(sheet_file, "%s %hu %hu %hu %hu %hu %hu %hhu\n",
                                                  ss->spritesheet_animation_array[i].animation_name,  //Fix anim_name in texture_structs later
                                                  &ss->spritesheet_animation_array[i].animation_x,
                                                  &ss->spritesheet_animation_array[i].animation_y,
                                                  &ss->spritesheet_animation_array[i].animation_sheet_width,
                                                  &ss->spritesheet_animation_array[i].animation_sheet_height,
                                                  &ss->spritesheet_animation_array[i].animation_frame_width,
                                                  &ss->spritesheet_animation_array[i].animation_frame_height,
                                                  &ss->spritesheet_animation_array[i].animation_frames);
    if(scanned != 8){ERROR(13)}
  }

  #undef ERROR

  cleanup:

  if(texture_file){fclose(texture_file);}
  if(sheet_file){fclose(sheet_file);} //May need to enclode this in an if "res >= 12" if statement

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  return result;
}

extern int memory_load_palette(uint32_t **palette, uint16_t *colourCount, char *path){
  int result = 0;
  #define PAL_ERROR(n) {result = (n); goto PAL_cleanup;}
  
  FILE *palette_file = fopen(path, "rb");
  if(!palette_file){PAL_ERROR(1);}

  dpal_header_t dpal_header;
  if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){PAL_ERROR(2);}

  if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){PAL_ERROR(3);}

  *palette = malloc(dpal_header.color_count * sizeof(uint32_t));
  if(!*palette){PAL_ERROR(4);}

  if(fread(*palette, sizeof(uint32_t), dpal_header.color_count,
    palette_file) != dpal_header.color_count){PAL_ERROR(5);}

  #undef PAL_ERROR

  *colourCount = dpal_header.color_count;

  PAL_cleanup:

  if(palette_file){fclose(palette_file);}

  return result;
}

//This function needs to be re-made
extern int memory_spritesheet_free(struct spritesheet *ss){
  if(ss){
    if(ss->spritesheet_format == 3 || ss->spritesheet_format == 4){ //Paletted
      free(ss->spritesheet_palette);
    }
    pvr_mem_free(ss->spritesheet_texture);

    //Free all animation structs too

    //free(ss); //Need to make sure it's neighbours link to each other before doing this
                //Also free all animations before this
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