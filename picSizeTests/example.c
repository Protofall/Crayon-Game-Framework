#include <kos.h>
#include <png/png.h>	//For the png_to_texture function

//Textures
pvr_ptr_t dino;				//To store the image from dino.png

//Init dino texture
void dino_init(){
    dino = pvr_mem_malloc(64 * 64 * 2);
    png_to_texture("/rd/dino64.png", dino, PNG_FULL_ALPHA);
}

void draw_translucent(pvr_ptr_t name, int dim, int x, int y, int z){
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

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

//Draw one frame
void draw_frame(void){
    pvr_wait_ready();	//Wait until the pvr system is ready to output again
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_TR_POLY);

    draw_translucent(dino, 64, 0, 0, 1);

    pvr_list_finish();
    pvr_scene_finish();
}

void cleanup(){
	//Clean up the texture memory we allocated earlier
	pvr_mem_free(dino);
	
	//Shut down libraries we used
	pvr_shutdown();
}

//Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void){
    int done = 0;

    pvr_init_defaults();		//Init kos
    dino_init();			//Init dino

    pvr_set_bg_color(1.0f, 1.0f, 1.0f);	//Should make a solid white screen (That isn't affected by draw_frame() )

    //Keep drawing frames until start is pressed
    while(!done){
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

        if(st->buttons & CONT_START)	//Quits if start is pressed. Screen goes black (This code behaves weirdly, I don't get it)
            done = 1;

        MAPLE_FOREACH_END()

        draw_frame();
    }

    cleanup();	//Free all usage of RAM and do the pvr_shutdown procedure

    return 0;
}