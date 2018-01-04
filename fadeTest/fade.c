#include <kos.h>
#include <png/png.h>	//For the png_to_texture function

// Texture
pvr_ptr_t pic;				//To store the image from pic.png

int fadeVal;	//Have to store it in an int because before when it was being a float it was being weird. Might be an emulator issue

// Init pic
void pic_init(){
    pic = pvr_mem_malloc(512 * 512 * 2);
    png_to_texture("/rd/pic.png", pic, PNG_NO_ALPHA);
}

void draw_pic(pvr_ptr_t name, int dim){
	int z = 1;
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, dim, dim, name, PVR_FILTER_BILINEAR);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;    //I think this is used to define the start of a new polygon

    //These define the verticies of the triangles "strips" (One triangle uses verticies of other triangle)
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


void draw_fade(int alpha, float r, float g, float b){
    int z = 100;

    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    //I convert the alpha variable to a float here because I had issues when I was modifying it as a float before
    vert.argb = PVR_PACK_COLOR((float)alpha/100, r, g, b);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    //These define the verticies of the triangles "strips" (One triangle uses verticies of other triangle)
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

// Draw one frame
void draw_frame(void){
    pvr_wait_ready();	//Wait until the pvr system is ready to output again
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_pic(pic, 512);	//Draw pic
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_fade(fadeVal, 0.0, 0.0, 0.0);	//Fade in from black (Or later fade to black)
    pvr_list_finish();

    pvr_scene_finish();
}

void cleanup(){
    // Clean up the texture memory we allocated earlier
    pvr_mem_free(pic);
	
    // Shut down libraries we used
    pvr_shutdown();
}

// Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void){
    int done = 0;

    pvr_init_defaults();		//Init kos

    pic_init();			//Init pic

    fadeVal = 100;	//Full opacity

    //Keep drawing frames until start is pressed
    while(!done){
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

        if(st->buttons & CONT_START)	//Quits if start is pressed. Screen goes black
            done = 1;

        MAPLE_FOREACH_END()

        draw_frame();
        if(fadeVal > 0){fadeVal--;}
    }

    while(fadeVal <= 100){	//For the fade out effect
        draw_frame();
        fadeVal++;
    }

    cleanup();	//Free all usage of RAM and do the pvr_shutdown procedure

    return 0;
}
