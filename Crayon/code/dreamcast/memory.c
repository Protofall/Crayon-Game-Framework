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

extern int memory_load_palette(crayon_palette_t *cp, char *path){
  int result = 0;
  #define PAL_ERROR(n) {result = (n); goto PAL_cleanup;}
  
  FILE *palette_file = fopen(path, "rb");
  if(!palette_file){PAL_ERROR(1);}

  dpal_header_t dpal_header;
  if(fread(&dpal_header, sizeof(dpal_header), 1, palette_file) != 1){PAL_ERROR(2);}

  if(memcmp(dpal_header.magic, "DPAL", 4) | !dpal_header.color_count){PAL_ERROR(3);}

  cp->palette = malloc(dpal_header.color_count * sizeof(uint32_t));
  if(!cp->palette){PAL_ERROR(4);}

  if(fread(cp->palette, sizeof(uint32_t), dpal_header.color_count,
    palette_file) != dpal_header.color_count){PAL_ERROR(5);}

  #undef PAL_ERROR

  cp->colour_count = dpal_header.color_count;

  PAL_cleanup:

  if(palette_file){fclose(palette_file);}

  return result;
}

extern int memory_load_crayon_packer_sheet(struct crayon_spritesheet *ss, char *path){
  int result = 0;
  pvr_ptr_t texture = NULL;
  ss->palette_data = NULL;

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

  //This assumes no mip-mapping, no stride, twiddled on, its uncompressed and no stride setting (I might edit this later to allow for compressed)
  if(dtex_header.type == 0x00000000){ //ARGB1555
    ss->spritesheet_format = 0;
  }
  if(dtex_header.type == 0x08000000){ //RGB565
    ss->spritesheet_format = 1;
  }
  else if(dtex_header.type == 0x10000000){ //ARGB4444
    ss->spritesheet_format = 2;
  }
  else if(dtex_header.type == 0x28000000){ //PAL4BPP
    ss->spritesheet_format = 5;
  }
  else if(dtex_header.type == 0x30000000){ //PAL8BPP
    ss->spritesheet_format = 6;
  }
  else{    
    ERROR(6);
  }

  /*
  The correct formats are
  bits 27-29 : Pixel format
  0 = ARGB1555
  1 = RGB565
  2 = ARGB4444
  3 = YUV422
  4 = BUMPMAP
  5 = PAL4BPP
  6 = PAL8BPP
  */

  int temp = strlen(path);
  if(ss->spritesheet_format == 5 || ss->spritesheet_format == 6){
    ss->palette_data = (crayon_palette_t *) malloc(sizeof(crayon_palette_t));
    ss->palette_data->palette = NULL;

    char *pathPal = (char *) malloc((temp+5)*sizeof(char));  //Add a check here to see if it failed
    if(!pathPal){ERROR(7);}
    strcpy(pathPal, path);
    pathPal[temp] = '.';
    pathPal[temp + 1] = 'p';
    pathPal[temp + 2] = 'a';
    pathPal[temp + 3] = 'l';
    pathPal[temp + 4] = '\0';
    int resultPal = memory_load_palette(ss->palette_data, pathPal); //The function will modify the palette and colour count
    free(pathPal);
    if(resultPal){ERROR(7 + resultPal);}
  }

  char *pathTxt = (char *) malloc((temp)*sizeof(char));  //Add a check here to see if it failed
  if(!pathTxt){ERROR(13);}
  strncpy(pathTxt, path, temp - 4);
  pathTxt[temp - 4] = 't';
  pathTxt[temp - 3] = 'x';
  pathTxt[temp - 2] = 't';
  pathTxt[temp - 1] = '\0';

  sheet_file = fopen(pathTxt, "rb");
  free(pathTxt);
  if(!sheet_file){ERROR(14);}
  fscanf(sheet_file, "%hhu\n", &ss->spritesheet_animation_count);

  ss->spritesheet_animation_array = (crayon_animation_t *) malloc(sizeof(crayon_animation_t) * ss->spritesheet_animation_count);
  if(!ss->spritesheet_animation_array){ERROR(15);}
  
  int i;
  for(i = 0; i < ss->spritesheet_animation_count; i++){
    //Check the length of the name
    char anim_name_part = '0';
    int count = -1;
    while(anim_name_part != ' '){
      anim_name_part = getc(sheet_file);
      count++;
    }
    ss->spritesheet_animation_array[i].animation_name = (char *) malloc((count + 1) * sizeof(char));

    fseek(sheet_file, -count - 1, SEEK_CUR);  //Go back so we can store the name
    int scanned = fscanf(sheet_file, "%s %hu %hu %hu %hu %hu %hu %hhu\n",
                ss->spritesheet_animation_array[i].animation_name,
                &ss->spritesheet_animation_array[i].animation_x,
                &ss->spritesheet_animation_array[i].animation_y,
                &ss->spritesheet_animation_array[i].animation_sheet_width,
                &ss->spritesheet_animation_array[i].animation_sheet_height,
                &ss->spritesheet_animation_array[i].animation_frame_width,
                &ss->spritesheet_animation_array[i].animation_frame_height,
                &ss->spritesheet_animation_array[i].animation_frames);
    if(scanned != 8){
      free(ss->spritesheet_animation_array);
      ERROR(16);
    }
  }

  #undef ERROR

  cleanup:

  if(texture_file){fclose(texture_file);}
  if(sheet_file){fclose(sheet_file);} //May need to enclode this in an if "res >= 12" if statement

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  //If we allocated memory for the palette and error out
  if(result && ss->palette_data != NULL){
    if(ss->palette_data->palette != NULL){ //If we allocated memory for the palette itself, free that
      free(ss->palette_data->palette);
    }
    free(ss->palette_data);
  }
  // if(result >= 7 && ss->palette_data != NULL){
  //   if(result >= 12 || ss->palette_data->palette != NULL){ //If we allocated memory for the palette itself, free that
  //     free(ss->palette_data->palette);
  //   }
  //   free(ss->palette_data);
  // }

  return result;
}

//Free Texture, anim array and palette (Maybe the anim/ss names later on?). Doesn't free the spritesheet struct itself
extern int memory_free_crayon_packer_sheet(struct crayon_spritesheet *ss, uint8_t free_palette){
  if(ss){
    if(free_palette && ss->palette_data != NULL){ //Free the palette
      if(ss->palette_data->palette != NULL){
        free(ss->palette_data->palette);
      }
      free(ss->palette_data);
    }
    pvr_mem_free(ss->spritesheet_texture);

    int i;
    for(i = 0; i < ss->spritesheet_animation_count; i++){
      free(ss->spritesheet_animation_array[i].animation_name);
    }
    free(ss->spritesheet_animation_array);

    //We don't free the ss because it could be on the stack and we can't confirm if a pointer points to the heap or stack.
    //If it were on the heap then we would free it

    return 0;
  }
  return 1;
}

extern int memory_mount_romdisk(char *filename, char *mountpoint){
  void *buffer;
  ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM

  if(size != -1){
    fs_romdisk_mount(mountpoint, buffer, 1); // Now mount that file as a romdisk, buffer will be freed when romdisk is unmounted
    return 0;
  }
  return 1;
}

extern int memory_mount_romdisk_gz(char *filename, char *mountpoint){
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
  buffer = malloc(length);  //Might need an (if(!buffer) check here)
  gzread(file, buffer, length);
  gzclose(file);

  fs_romdisk_mount(mountpoint, buffer, 1);
  return 0;
}
