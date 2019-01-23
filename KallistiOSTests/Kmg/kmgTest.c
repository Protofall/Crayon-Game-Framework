#include <kos/img.h> //Must go before kmg.h since that depends on this
#include <kmg/kmg.h>
#include <dc/pvr.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include <stdio.h>

pvr_ptr_t back;
pvr_ptr_t face;

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

pvr_ptr_t load_kmg(char const* filename, uint32* w, uint32* h, uint32* format){
    kos_img_t img;
    pvr_ptr_t rv;
    if(kmg_to_img(filename, &img)) {
       printf("Failed to load image file: %s\n", filename);
       return NULL;
    }
    if(!(rv = pvr_mem_malloc(img.byte_count))) {
       printf("Couldn't allocate memory for texture!\n");
       kos_img_free(&img, 0);
       return NULL;
    }
    pvr_txr_load_kimg(&img, rv, 0);
    kos_img_free(&img, 0);
    *w = img.w;
    *h = img.h;
    *format = img.fmt;
    return rv;
}

void draw_translucent(pvr_ptr_t name, int dim, int x, int y, int z){
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    //When using KMGs don't forget PVR_TXRFMT_VQ_ENABLE otherwise they don't render right
    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, dim, dim, name, PVR_FILTER_BILINEAR);
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

    vert.x = dim + x;
    vert.y = y;
    vert.z = z;
    vert.u = 1;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x;
    vert.y = dim + y;
    vert.z = z;
    vert.u = 0.0;
    vert.v = 1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = dim + x;
    vert.y = dim + y;
    vert.z = z;
    vert.u = 1;
    vert.v = 1;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

void draw_opaque(pvr_ptr_t name, int dim){
    int z = 1;
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    //When using KMGs don't forget PVR_TXRFMT_VQ_ENABLE otherwise they don't render right
    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED, dim, dim, name, PVR_FILTER_BILINEAR);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    vert.x = 0;
    vert.y = 0;
    vert.z = z;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 640;
    vert.y = 0;
    vert.z = z;
    vert.u = 1;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 0;
    vert.y = 480;
    vert.z = z;
    vert.u = 0.0;
    vert.v = 1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 640;
    vert.y = 480;
    vert.z = z;
    vert.u = 1;
    vert.v = 1;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

//Draw one frame
void draw_frame(void){
    pvr_wait_ready(); //Wait until the pvr system is ready to output again
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY); //This parameter matters depending on what type you want to draw
    draw_opaque(back, 512);
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_translucent(face, 128, 300, 240, 1);
    pvr_list_finish();

    pvr_scene_finish();
}

void cleanup(){
    //Clean up the texture memory we allocated earlier
    pvr_mem_free(back);
    pvr_mem_free(face);
  
    //Shut down libraries we used
    pvr_shutdown();
}

int main(){
    pvr_init_defaults();    //Init kos

    uint32 w, h, format;
    back = load_kmg("/rd/image.kmg", &w, &h, &format);
    face = load_kmg("/rd/nerstr.kmg", &w, &h, &format);
    //if(back){
    //   printf("Loaded /rd/image.kmg with dimensions %ux%u, format %u\n", w, h, format);
    //}
    int done = 0;

    while(!done){
      MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

      if(st->buttons & CONT_START)  //Quits if start is pressed. Screen goes black (This code behaves weirdly, I don't get it)
        done = 1;

      MAPLE_FOREACH_END()

      draw_frame();
    }

    cleanup();  //Free all usage of RAM and do the pvr_shutdown procedure

    return 0;
 }