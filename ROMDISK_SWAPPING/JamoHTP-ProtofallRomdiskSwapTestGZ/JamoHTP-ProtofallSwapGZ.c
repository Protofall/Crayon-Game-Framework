#include <kos.h>
#include <zlib/zlib.h>
#include <png/png.h>

// Texture
pvr_ptr_t face1;        //To store the image from image.png
pvr_ptr_t face2;        //To store the image from grump.png
pvr_ptr_t face3;        //To store the image from notMaccas.png
pvr_ptr_t face4;        //To store the image from nerstr.png

//KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);

//name is a pointer to the pvr_ptr_t, *name is the pvr_ptr_t
void faces_init(int dim, char * location, pvr_ptr_t * name){
  *name = pvr_mem_malloc(dim * dim * 2);
  png_to_texture(location, *name, PNG_NO_ALPHA);
}

void draw_face(pvr_ptr_t name, int dim, int x, int y){
  int z = 1;
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;

  pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, dim, dim, name, PVR_FILTER_BILINEAR);
  pvr_poly_compile(&hdr, &cxt);
  pvr_prim(&hdr, sizeof(hdr));

  vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
  vert.oargb = 0;
  vert.flags = PVR_CMD_VERTEX;

  vert.x = x;
  vert.y = y;
  vert.z = z;
  vert.u = 0.0;
  vert.v = 0.0;
  pvr_prim(&vert, sizeof(vert));

  vert.x = x + dim;
  vert.y = y;
  vert.z = z;
  vert.u = 1;
  vert.v = 0.0;
  pvr_prim(&vert, sizeof(vert));

  vert.x = x;
  vert.y = y + dim;
  vert.z = z;
  vert.u = 0.0;
  vert.v = 1;
  pvr_prim(&vert, sizeof(vert));

  vert.x = x + dim;
  vert.y = y + dim;
  vert.z = z;
  vert.u = 1;
  vert.v = 1;
  vert.flags = PVR_CMD_VERTEX_EOL;
  pvr_prim(&vert, sizeof(vert));
}

void draw_frame(void){
  pvr_wait_ready();
  pvr_scene_begin();

  pvr_list_begin(PVR_LIST_OP_POLY);
  draw_face(face1, 64, 0, 0); //Draw image.png at 0,0
  draw_face(face2, 64, 64, 0); //Draw grump.png on the right side of image.png
  draw_face(face3, 128, 128, 0); //Draw notMaccas.png on the right side of grump.png
  pvr_list_finish();

  pvr_scene_finish();
}

void cleanup(){
  // Clean up the texture memory we allocated earlier
  pvr_mem_free(face1);
  pvr_mem_free(face2);
  pvr_mem_free(face3);

  // Shut down the pvr system
  pvr_shutdown();
}

//There might be an issue with zlib_getlength since it might not be able to read the correct length. Check this
int mount_romdisk_gz(char *filename, char *mountpoint){
  printf("Attempting to mount the romdisk\n");
  void *buffer;
  int length = zlib_getlength(filename);

  //Check against available main ram here
   
  // Check failure
  if(length == 0){
      printf("Length was zero\n");
      return -1;
  }
   
  // Open file
  gzFile file = gzopen(filename, "rb"); //Seems to be the replacement of fs_load() along with gzread()
  if(!file){
      printf("Can't open the file\n");
      return -1;
  }

  // Allocate memory, read file
  buffer = malloc(length);
  gzread(file, buffer, length);
  gzclose(file);
   
  // Mount
  fs_romdisk_mount(mountpoint, buffer, 1);
  printf("Something was mounted\n");
  return 0;
}

int main(){
  pvr_init_defaults();

  int res1 = mount_romdisk_gz("/cd/level1.img.gz", "/levels"); //Trying to mount the first img to the romdisk
  //printf("Res1 = %d\n",res1);
  faces_init(64, "/levels/image.png", &face1);
  fs_romdisk_unmount("/levels");

  int res2 = mount_romdisk_gz("/cd/level2.img.gz", "/levels"); //Trying to mount the second img to the romdisk
  //printf("Res2 = %d\n",res2);
  faces_init(64, "/levels/grump.png", &face2);
  fs_romdisk_unmount("/levels");

  int res3 = mount_romdisk_gz("/cd/level3.img.gz", "/levels"); //Trying to mount the second img to the romdisk
  //printf("Res3 = %d\n",res3);
  faces_init(128, "/levels/notMaccas.png", &face3);
  faces_init(128, "/levels/nerstr.png", &face4);
  fs_romdisk_unmount("/levels");

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