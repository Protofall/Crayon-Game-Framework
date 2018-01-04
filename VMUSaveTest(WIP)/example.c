#include <kos.h>
#include <png/png.h>	//For the png_to_texture function
#include <zlib/zlib.h>	//For the length of the text file in the gz archive

// Font data
extern char wfont[];		//Font comes from wfont.bin

// Textures
pvr_ptr_t font_tex;
pvr_ptr_t icon_data;
pvr_ptr_t fade_white;
char *data;

// Init icon_data
void icon_data_init(){
    icon_data = pvr_mem_malloc(64 * 64 * 2);
    png_to_texture("/rd/icon64.png", icon_data, PNG_NO_ALPHA);
}

// Draw the icon in the top right corner at "original" resolution, note that u and v control what part of the texture gets mapped to polygon.
void draw_icon(){
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 64, 64, icon_data, PVR_FILTER_NONE);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    //Not sure what these 3 do, but they are vital for some reason
    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    //These define the verticies of the triangles "strips" (One triangle uses verticies of other triangle)
    vert.x = 0;
    vert.y = 0;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 64;
    vert.y = 0;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 0;
    vert.y = 64;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = 64;
    vert.y = 64;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

// Draw one frame
void draw_frame(void){
    pvr_wait_ready();	//wait until the pvr system is ready to output again
    pvr_scene_begin();

    //pvr_list_begin(PVR_LIST_OP_POLY); //This parameter matters depending on what you want to draw
    //draw_icon();
    //pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    pvr_list_finish();
    
    pvr_scene_finish();
}

int DC_SaveToVMU(char *src){
   char dst[32];
   file_t file;
   int filesize = 0;
   unsigned long zipsize = 0;
   uint8 *data;
   uint8 *zipdata;
   vmu_pkg_t pkg;
   uint8 *pkg_out;
   int pkg_size;
   // Our VMU + full save name
   strcpy(dst, "/vmu/a1/");			//I think this is controller "a" slot "1"
   strcat(dst, src);
   // Reads in the file from the CWD
   file = fs_open(src, O_RDONLY);
   filesize = fs_total(file);
   data = (uint8*)malloc(filesize);
   fs_read(file, data, filesize);
   fs_close(file);
   // Allocate some memory for compression
   zipsize = filesize * 2;
   zipdata = (uint8*)malloc(zipsize);
   // The compressed save
   compress(zipdata, &zipsize, data, filesize);
   // Required VMU header
   // You will have to have a VMU icon defined under icon_data
   strcpy(pkg.desc_short, "Wolf4SDL\\DC");
   strcpy(pkg.desc_long, "Game Save");
   strcpy(pkg.app_id, "Wolf4SDL\\DC");
   pkg.icon_cnt = 1;
   pkg.icon_anim_speed = 0;
   memcpy(&pkg.icon_pal[0], icon_data, 32);
   pkg.icon_data = icon_data + 32;
   pkg.eyecatch_type = VMUPKG_EC_NONE;
   pkg.data_len = zipsize;
   pkg.data = zipdata;
   vmu_pkg_build(&pkg, &pkg_out, &pkg_size);
   // Save the newly created VMU save to the VMU
   fs_unlink(dst);
   file = fs_open(dst, O_WRONLY);
   fs_write(file, pkg_out, pkg_size);
   fs_close(file);
   // Free unused memory
   free(pkg_out);
   free(data);
   free(zipdata);
   return 0;
}

int DC_LoadFromVMU(char *dst){
   char src[32];
   int file;
   int filesize;
   unsigned long unzipsize;
   uint8* data;
   uint8* unzipdata;
   vmu_pkg_t pkg;
   // Our VMU + full save name
   strcpy(src, "/vmu/a1/");
   strcat(src, dst);
   // Remove VMU header
   file = fs_open(src, O_RDONLY);
   if(file == 0) return -1;
   filesize = fs_total(file);
   if(filesize <= 0) return -1;
   data = (uint8*)malloc(filesize);
   fs_read(file, data, filesize);
   vmu_pkg_parse(data, &pkg);
   fs_close(file);
   // Allocate memory for the uncompressed data
   unzipdata = (uint8 *)malloc(65536);
   unzipsize = 65536;
   // Uncompress the data to the CWD
   uncompress(unzipdata, &unzipsize, (uint8 *)pkg.data, pkg.data_len);
   fs_unlink(dst);
   file = fs_open(dst, O_WRONLY);
   fs_write(file, unzipdata, unzipsize);
   fs_close(file);
   // Free unused memory
   free(data);
   free(unzipdata);
   return 0;
}

// Romdisk
extern uint8 romdisk_boot[];
KOS_INIT_ROMDISK(romdisk_boot);

int main(void){
    int done = 0;

    pvr_init_defaults();		// Init kos
    icon_data_init();			// Init icon_data for VMU save (Needs to be modified into a 32 by 32)

    pvr_set_bg_color(0.46f, 0.22f, 0.49f);	//Should make a solid purple screen (That isn't affected by draw_frame() )

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

