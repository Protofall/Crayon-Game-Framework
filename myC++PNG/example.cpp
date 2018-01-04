/* png example for KOS 1.1.x
 * Jeffrey McBeth / Morphogenesis
 * <mcbeth@morphogenesis.2y.net>
 *
 * Heavily borrowed from from 2-D example
 * AndrewK / Napalm 2001
 * <andrewk@napalm-x.com>
 *
 * This is a modified version by Protofall
 * This is not to be sold or whatever and only exists to help myself and others learn KOS
 *
 */

#include <kos.h>
#include <png/png.h>	//For the png_to_texture function
#include <zlib/zlib.h>	//For the length of the text file in the gz archive

//Getting zlib to work in C++
extern "C" {
  int zlib_getlength(const char *filename);
}

// Font data
extern char wfont[];		//Font comes from wfont.bin

// Textures
pvr_ptr_t font_tex;
pvr_ptr_t back_tex;
char *data;

// Init background
void back_init(){
    back_tex = pvr_mem_malloc(512 * 512 * 2);	//Alocating enoough Video ram for the texture We're allocating 0.5MB here
    png_to_texture("/rd/background.png", back_tex, PNG_NO_ALPHA);
}

// Init font
void font_init(){
    int i, x, y, c;
    unsigned short * temp_tex;

    font_tex = pvr_mem_malloc(256 * 256 * 2);	//16bpp (ARGB4444 Mode), 256 by 256 texture.
                                                //We must allocate 2^n by 2^n space because of hardware limitations.

    temp_tex = (unsigned short *)malloc(256 * 128 * 2);	//We can do *any* size for temp stuff (Draws into top half of texture)

    c = 0;

    for(y = 0; y < 128 ; y += 16){
        for(x = 0; x < 256 ; x += 8){
            for(i = 0; i < 16; i++){
                temp_tex[x + (y + i) * 256 + 0] = 0xffff * ((wfont[c + i] & 0x80) >> 7);	//0xffff is white pixel
                temp_tex[x + (y + i) * 256 + 1] = 0xffff * ((wfont[c + i] & 0x40) >> 6);	//0x0000 is black pixel in texture
                temp_tex[x + (y + i) * 256 + 2] = 0xffff * ((wfont[c + i] & 0x20) >> 5);	//1 bit in wfont.bin = 1 pixel
                temp_tex[x + (y + i) * 256 + 3] = 0xffff * ((wfont[c + i] & 0x10) >> 4);
                temp_tex[x + (y + i) * 256 + 4] = 0xffff * ((wfont[c + i] & 0x08) >> 3);
                temp_tex[x + (y + i) * 256 + 5] = 0xffff * ((wfont[c + i] & 0x04) >> 2);
                temp_tex[x + (y + i) * 256 + 6] = 0xffff * ((wfont[c + i] & 0x02) >> 1);
                temp_tex[x + (y + i) * 256 + 7] = 0xffff * (wfont[c + i] & 0x01);
            }

            c += 16;
        }
    }
    //256 seems to be the hoizontal line number
        //1st loop...I dunno
        //2nd loop goes over each line of the final result
        //3rd loop controls reading from the source file and also the final result

    pvr_txr_load_ex(temp_tex, font_tex, 256, 256, PVR_TXRLOAD_16BPP);	//Texture load 16bpp

    free(temp_tex);	//Originally temp_tex wasn't free-d, very naughty!
}

void text_init(){
    int length = zlib_getlength("/rd/text.gz");
    gzFile f;

    data = (char *)malloc(length + 1); // Not currently freeing it anywhere

    f = gzopen("/rd/text.gz", "r");
    gzread(f, data, length);
    data[length] = 0;	// '\0' == 0 so this is the null terminator. Also '0' == 48
    gzclose(f);

    printf("length [%d]\n", length);
}

// Draw background
void draw_back(void){
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 512, 512, back_tex, PVR_FILTER_BILINEAR);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    vert.x = 1;
    vert.y = 1;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 640;
    vert.y = 1;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 1;
    vert.y = 480;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 640;
    vert.y = 480;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

void draw_char(float x1, float y1, float z1, float a, float r, float g, float b, int c, float xs, float ys){
    pvr_vertex_t    vert;
    int             ix, iy;
    float           u1, v1, u2, v2;

    ix = (c % 32) * 8;	//C is the character to draw
    iy = (c / 32) * 16;
    u1 = (ix + 0.5f) * 1.0f / 256.0f;
    v1 = (iy + 0.5f) * 1.0f / 256.0f;
    u2 = (ix + 7.5f) * 1.0f / 256.0f;
    v2 = (iy + 15.5f) * 1.0f / 256.0f;

    vert.flags = PVR_CMD_VERTEX;
    vert.x = x1;
    vert.y = y1 + 16.0f * ys;
    vert.z = z1;
    vert.u = u1;
    vert.v = v2;
    vert.argb = PVR_PACK_COLOR(a, r, g, b);
    vert.oargb = 0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x1;
    vert.y = y1;
    vert.u = u1;
    vert.v = v1;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x1 + 8.0f * xs;
    vert.y = y1 + 16.0f * ys;
    vert.u = u2;
    vert.v = v2;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.x = x1 + 8.0f * xs;
    vert.y = y1;
    vert.u = u2;
    vert.v = v1;
    pvr_prim(&vert, sizeof(vert));
}

// Draw a string
void draw_string(float x, float y, float z, float a, float r, float g, float b, char *str, float xs, float ys){
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    float orig_x = x;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, 256, 256, font_tex, PVR_FILTER_BILINEAR);	//Draws characters in ARGB4444 mode
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    while(*str){
        if(*str == '\n'){
            x = orig_x;
            y += 40;
            str++;
            continue;
        }

        draw_char(x, y, z, a, r, g, b, *str++, xs, ys);
        x += 8 * xs;
    }
}

int y = 0;	// Base y coordinate


// Draw one frame
void draw_frame(void){
    pvr_wait_ready();
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_back();
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);

    /* 1720 and 480 are magic numbers directly related to the text scrolling
     * 1720 is enough room for the whole text to scroll from the bottom of
     * the screen to off the top.  31 lines * 40 pixels + 480 pixel high screen
     * 480 is the height of the screen (starts the text at the bottom)
     */
    draw_string(0, y % 1720 + 440, 3, 1, 1, 1, 1, data, 2, 2);

    pvr_list_finish();
    pvr_scene_finish();

    y--;
}

// Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void){
    int done = 0;

    pvr_init_defaults();		// Init kos

    font_init();			// Init font

    back_init();			// Init background

    text_init();			// Init text

    // Keep drawing frames until start is pressed
    while(!done){
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

        if(st->buttons & CONT_START)	// Quits if start is pressed. Screen goes black
            done = 1;

        MAPLE_FOREACH_END()

        draw_frame();
    }

    return 0;
}

/*

Options for vid_set_mode();

DM_320x240			320 x 240, 60Hz (or VGA)
DM_640x480			640 x 480, 60Hz (or VGA)
DM_800x608			800 x 608, 60Hz (or VGA)
DM_256x256			256 x 256, 60Hz (or VGA)
DM_768x480			768 x 480, 60Hz (or VGA)
DM_768x576			768 x 576, 60Hz (or VGA)
DM_640x480_PAL_IL	640 x 480, 50Hz
DM_256x256_PAL_IL	256 x 256, 50Hz
DM_768x480_PAL_IL	768 x 480, 50Hz
DM_768x576_PAL_IL	768 x 576, 50Hz

PM_RGB555	15-bit (xRRRRRGGGGGBBBBB)
PM_RGB565	16-bit (RRRRRGGGGGGBBBBB)
PM_RGB888	24-bit (RRRRRRRR GGGGGGGG BBBBBBBB)

*/