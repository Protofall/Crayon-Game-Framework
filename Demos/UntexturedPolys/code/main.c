//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>

//For region and htz stuff
#include <dc/flashrom.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

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

	crayon_sprite_array_t rotate;

	crayon_memory_init_sprite_array(&rotate, NULL, 0, NULL, 1, 0, 0, PVR_FILTER_NONE, 0);
	rotate.layer[0] = 2;
	rotate.scale[0].x = 300;
	rotate.scale[0].y = 225;
	rotate.coord[0].x = (640 - rotate.scale[0].x) / 2.0f;
	rotate.coord[0].y = (480 - rotate.scale[0].y) / 2.0f;
	rotate.rotation[0] = 0;
	rotate.colour[0] = 0xFFFFFFFF;

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	while(1){
		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
			crayon_graphics_draw_sprites(&rotate, NULL, PVR_LIST_OP_POLY, 0);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_PT_POLY);
		pvr_list_finish();

		pvr_scene_finish();

		//Rotate the box
		rotate.rotation[0]++;
		if(rotate.rotation[0] > 360){
			rotate.rotation[0] -= 360;
		}
	}

	crayon_memory_free_sprite_array(&rotate);

	return 0;
}
