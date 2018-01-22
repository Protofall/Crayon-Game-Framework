#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dc/pvr.h>

typedef struct dtex_header{
  uint8_t magic[4]; //magic number "DTEX"
  uint16_t   width; //texture width in pixels
  uint16_t  height; //texture height in pixels
  uint32_t    type; //format (see https://github.com/tvspelsfreak/texconv)
  uint32_t    size; //texture size in bytes
} dtex_header_t;

typedef struct dpal_header{
  uint8_t     magic[4]; //magic number "DPAL"
  uint32_t color_count; //number of 24-bit ARGB palette entries
} dpal_header_t;

extern int memory_load_dtex(struct spritesheet *ss,
  const char *path){

  int result = 0;
  pvr_ptr_t texture = NULL;
  uint32_t *palette = NULL;

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

#undef ERROR

  // Cleanup
  //---------------------------------------------------------------------------
  cleanup:

  if(texture_file){fclose(texture_file);}

  // If a failure occured somewhere
  if(result && texture){pvr_mem_free(texture);}

  return result;

  //I don't think the headers need to be free-d since they aren't pointers, but I'll leave this comment here for future me
}

extern void memory_spritesheet_free(struct spritesheet *ss){
  if(ss){
    if(ss->format == 3 || ss->format == 4){ //Paletted
      free(ss->palette);
    }
    pvr_mem_free(ss->texture);
    //Then also a "free(ss)" when I get around to the spritesheet lists
  }
}

extern int mount_romdisk(char *filename, char *mountpoint){
  void *buffer;
  ssize_t size = fs_load(filename, &buffer); // Loads the file "filename" into RAM, user is responsible for freeing memory when done

  // Successfully read romdisk image
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
   
  // Mount
  fs_romdisk_mount(mountpoint, buffer, 1);
  return 0;
}