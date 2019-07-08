//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For region and htz stuff
#include <dc/flashrom.h>

//Change this to only give room for PT list (Since other ones aren't used)
pvr_init_params_t pvr_params = {
		// Enable opaque, translucent and punch through polygons with size 16
			//To better explain, the opb_sizes or Object Pointer Buffer sizes
			//Work like this: Set to zero to disable. Otherwise the higher the
			//number the more space used (And more efficient under higher loads)
			//The lower the number, the less space used and less efficient under
			//high loads. You can choose 0, 8, 16 or 32
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },

		// Vertex buffer size 512K
		512 * 1024,

		// No DMA
		0,

		// No FSAA
		0,

		// Translucent Autosort enabled
		0
};

void set_screen(float * htz_adjustment){
	*htz_adjustment = 1.0;
	uint8_t region = flashrom_get_region();
	if(region < 0){	//If error we just default to green swirl. Apparently its possible for some DCs to return -1
		region = 0;	//Invalid region
	}

	if(vid_check_cable() == CT_VGA){	//If we have a VGA cable, use VGA
		vid_set_mode(DM_640x480_VGA, PM_RGB565);
	}
	else{	//Else its RGB. This handles composite, S-video, SCART, etc
		if(region == FLASHROM_REGION_EUROPE){
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);		//50Hz
			*htz_adjustment = 1.2;	//60/50Hz
		}
		else{
			vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	//60Hz
		}
	}

	return;
}

int main(){
	pvr_init(&pvr_params);	//Init the pvr system

	float htz_adjustment;
	set_screen(&htz_adjustment);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	while(1){
		pvr_wait_ready();
		
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
		
			crayon_graphics_draw_line(320, 240, 320, 0, 10, 0xFF000000, PVR_LIST_OP_POLY);	//North
			crayon_graphics_draw_line(320, 240, 0, 480, 10, 0xFF000000, PVR_LIST_OP_POLY);	//South West
			crayon_graphics_draw_line(320, 240, 0, 240, 10, 0xFF000000, PVR_LIST_OP_POLY);	//West
			crayon_graphics_draw_line(320, 240, 0, 0, 10, 0xFF000000, PVR_LIST_OP_POLY);	//North West

			crayon_graphics_draw_line(320, 240, 640, 0, 10, 0xFFFFFFFF, PVR_LIST_OP_POLY);	//North East
			crayon_graphics_draw_line(320, 240, 640, 240, 10, 0xFFFFFFFF, PVR_LIST_OP_POLY);	//East
			crayon_graphics_draw_line(320, 240, 640, 480, 10, 0xFFFFFFFF, PVR_LIST_OP_POLY);	//South East
			crayon_graphics_draw_line(320, 240, 320, 480, 10, 0xFFFFFFFF, PVR_LIST_OP_POLY);	//South

		pvr_list_finish();

		pvr_scene_finish();
	}

	return 0;
}
