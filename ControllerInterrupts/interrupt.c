#include <kos.h>
#include <png/png.h>    //For the png_to_texture function

// Texture
pvr_ptr_t pic;              //To store the image from pic.png

int picX;
int picY;

// Init pic
void pic_init(){
    pic = pvr_mem_malloc(64 * 64 * 2);
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
    vert.x = picX;
    vert.y = picY;
    vert.z = z;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = picX + 64;
    vert.y = picY;
    vert.z = z;
    vert.u = 1;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = picX;
    vert.y = picY + 64;
    vert.z = z;
    vert.u = 0.0;
    vert.v = 1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = picX + 64;
    vert.y = picY + 64;
    vert.z = z;
    vert.u = 1;
    vert.v = 1;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

// Draw one frame
void draw_frame(void){
    pvr_wait_ready();   //Wait until the pvr system is ready to output again
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_pic(pic, 64);  //Draw pic
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

void endProg(uint8 addr, uint32 btns) {
    // addr is the device that caused the interrupt
    // btns is a mask of the buttons that were pressed when the interrupt occurred
    exit(0);
}

int main(void){
    int done = 0;

    pvr_init_defaults();        //Init pvr

    pic_init();         //Init pic

    picX = 0;
    picY = 0;

    cont_btn_callback(maple_addr(0,0), CONT_A|CONT_B|CONT_START, endProg);    //Strangely "CONT_A|CONT_B|CONT_X|CONT_Y|CONT_START" doesn't seem to work
    //When A+B+X+Y+Start is held down on controller (0,0), repeatedly call the function end
    //The docs say it's for things like quit the game

    //Keep drawing frames forever and take directional input
    //Sometimes this input behaves strangley
    //For example pressing left and right cancel each other out, but then adding down to the mix does nothing, but adding up moves it up
    //I don't know why exactly
    //Also doing Right + Down then adding left doesn't do anything
    while(!done){
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

        if(st->buttons & CONT_DPAD_UP){
            picY--;
        }
        if(st->buttons & CONT_DPAD_RIGHT){
            picX++;
        }
        if(st->buttons & CONT_DPAD_DOWN){
            picY++;
        }
        if(st->buttons & CONT_DPAD_LEFT){
            picX--;
        }

        MAPLE_FOREACH_END()

        draw_frame();
    }

    cleanup();  //Free all usage of RAM and do the pvr_shutdown procedure

    return 0;
}