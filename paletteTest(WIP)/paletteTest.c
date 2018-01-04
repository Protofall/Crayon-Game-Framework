#include <kos.h>
#include <zlib/zlib.h>
#include <png/png.h>

// Texture
pvr_ptr_t thing1;        //To store the image from Insta.png
pvr_ptr_t thing2;        //To store the image from Fade.png

//KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);

//name is a pointer to the pvr_ptr_t, *name is the pvr_ptr_t
void thing_init(int dim, char * location, pvr_ptr_t * name){
  *name = pvr_mem_malloc(dim * dim * 2);
  png_to_texture(location, *name, PNG_FULL_ALPHA);
}

void draw_thing(pvr_ptr_t name, int dim, int x, int y){
  int z = 1;
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;

  //pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_TWIDDLED, dim, dim, name, PVR_FILTER_BILINEAR);
  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, dim, dim, name, PVR_FILTER_BILINEAR);
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

  pvr_list_begin(PVR_LIST_TR_POLY);
  //draw_thing(thing1, 128, 256, 80);
  //draw_thing(thing2, 128, 256, 272);
  draw_thing(thing1, 128, 128, 176); //(x is horizontal 640, y is vertical 480)
  draw_thing(thing2, 128, 384, 176);
  pvr_list_finish();

  pvr_scene_finish();
}

void cleanup(){
  // Clean up the texture memory we allocated earlier
  pvr_mem_free(thing1);
  pvr_mem_free(thing2);

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
  pvr_init_defaults();

  mount_romdisk("/cd/assets1.img", "/levels"); //Trying to mount the first img to the romdisk
  thing_init(128, "/levels/Insta.png", &thing1);
  thing_init(128, "/levels/Fade.png", &thing2);
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