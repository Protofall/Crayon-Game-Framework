//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/crayon.h>
#include <crayon/debug.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

int main(){

	//Note:, first parameter is ignore for now since Crayon is only on Dreamcast ATM
	if(crayon_init(CRAYON_PLATFORM_DREAMCAST, CRAYON_BOOT_MODE)){
		error_freeze("Crayon failed to initialise");
	}

	srand(time(0));	//Set the seed for rand()

	crayon_spritesheet_t Logo;
	crayon_sprite_array_t Logo_Draw;
	crayon_palette_t Logo_P;

	uint8_t error = 0;
	//Load the logo
	#if CRAYON_BOOT_MODE == 2
		error = crayon_memory_mount_romdisk("/pc/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == 1
		error = crayon_memory_mount_romdisk("/sd/stuff.img", "/files");
	#elif CRAYON_BOOT_MODE == 0
		error = crayon_memory_mount_romdisk("/cd/stuff.img", "/files");
	#else
		#error Invalid CRAYON_BOOT_MODE value
	#endif

	if(error){error_freeze("Error mounting romdisk");}

	//Load the asset
	error = crayon_memory_load_spritesheet(&Logo, &Logo_P, 0, "/files/logo.dtex");

	fs_romdisk_unmount("/files");
	if(error){error_freeze("Unable to load spritesheet");}

	//3 Dwarfs, first shrunk, 2nd normal, 3rd enlarged. Scaling looks off in emulators like lxdream though (But thats a emulator bug)
	error = crayon_memory_init_sprite_array(&Logo_Draw, &Logo, 0, &Logo_P, 1, 1, CRAYON_NO_MULTIS, PVR_FILTER_NONE, 0);
	if(error){error_freeze("Can't create logo sprite array");}
	error += crayon_memory_set_coord_x(&Logo_Draw, 0, (crayon_graphics_get_window_width() - Logo.animation[0].frame_width) / 2);
	error += crayon_memory_set_coord_y(&Logo_Draw, 0, (crayon_graphics_get_window_height() - Logo.animation[0].frame_height) / 2);
	error += crayon_memory_set_layer(&Logo_Draw, 0, 2);
	error += crayon_memory_set_scale_x(&Logo_Draw, 0, 1);
	error += crayon_memory_set_scale_y(&Logo_Draw, 0, 1);
	error += crayon_memory_set_flip(&Logo_Draw, 0, 0);
	error += crayon_memory_set_rotation(&Logo_Draw, 0, 0);
	error += crayon_memory_set_colour(&Logo_Draw, 0, 0xFFFFFFFF);
	error += crayon_memory_set_frame_id(&Logo_Draw, 0, 0);
	error += crayon_memory_set_visibility(&Logo_Draw, 0, 1);
	error += crayon_memory_set_frame_uv(&Logo_Draw, 0, 0);
	if(error){error_freeze("Can't set variables of sprite array");}

	uint32_t colours[5];
	colours[0] = Logo_P.palette[1];	//Orange
	colours[1] = 0xFF002FFF;	//Blue
	colours[2] = 0xFFCE21FF;	//Purple
	colours[3] = 0xFFFF228D;	//Pink
	colours[4] = 0xFFFFED00;	//Yellow

	uint8_t begin = 0;
	uint8_t moving = 0;
	//__hz is set in crayon_init()
	float shrink_time = 2.5 * __hz;	//Time it takes to shrink (In seconds, __hz fixes for 50Hz)

	//Positive is bottom right, negative is top left
	int8_t x_dir = rand() % 2;
	if(!x_dir){x_dir = -1;}
	int8_t y_dir = rand() % 2;
	if(!y_dir){y_dir = -1;}

	//Once shrunk, this will be the new width/height
	float new_width = crayon_graphics_get_draw_element_width(&Logo_Draw, 0);
	float new_height = crayon_graphics_get_draw_element_height(&Logo_Draw, 0);

	uint8_t loop = 1;
	while(loop){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
			//If any button is pressed, start the game (Doesn't check thumbstick)
			if(st->buttons || st->ltrig > 255 * 0.1 || st->rtrig > 255 * 0.1){
				begin = 1;
			}

			//Press all 4 facebuttons at once to exit
			if(st->buttons & (CONT_A | CONT_B | CONT_X | CONT_Y)){
				loop = 0;
				break;
			}

		MAPLE_FOREACH_END()

		//Make the object bounce around
		if(moving){
			//Collision detection
			if(Logo_Draw.coord[0].x < 0){	//Off left side
				x_dir = 1;
				Logo_P.palette[1] = colours[rand() % 5];
			} 
			if(Logo_Draw.coord[0].y < 0){	//Off top side
				y_dir = 1;
				Logo_P.palette[1] = colours[rand() % 5];
			}
			if(Logo_Draw.coord[0].x + new_width > 640){	//Off right side
				x_dir = -1;
				Logo_P.palette[1] = colours[rand() % 5];
			}
			if(Logo_Draw.coord[0].y + new_height > 480){	//Off bottom side
				y_dir = -1;
				Logo_P.palette[1] = colours[rand() % 5];
			}

			//colours Dark Blue, Purple, Pink, Orange, vright Green, Yellow

			//Movement
			//NOTE: __hz_adjustment is set in crayon_graphics_init() or crayon_init()
			Logo_Draw.coord[0].x += 1.5 * __hz_adjustment * x_dir;
			Logo_Draw.coord[0].y += 1.5 * __hz_adjustment * y_dir;
		}

		//Shrinking process
		if(begin && Logo_Draw.scale[0].x > 0.4 && Logo_Draw.scale[0].y > 0.3){
			Logo_Draw.scale[0].x -= (0.6/shrink_time) * __hz_adjustment;
			Logo_Draw.scale[0].y -= (0.7/shrink_time) * __hz_adjustment;
			new_width = crayon_graphics_get_draw_element_width(&Logo_Draw, 0);
			new_height = crayon_graphics_get_draw_element_height(&Logo_Draw, 0);
			Logo_Draw.coord[0].x = (crayon_graphics_get_window_width() - new_width) / 2;
			Logo_Draw.coord[0].y = (crayon_graphics_get_window_height() - new_height) / 2;
		}
		else{
			if(begin){
				moving = 1;
			}
		}

		crayon_graphics_setup_palette(&Logo_P);	//Could live outside the loop, but later we will change the palette when it hits the corners

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_PT_POLY);
			crayon_graphics_draw_sprites(&Logo_Draw, NULL, PVR_LIST_PT_POLY, CRAYON_DRAW_SIMPLE);
		pvr_list_finish();

		pvr_scene_finish();
	}

	crayon_memory_free_sprite_array(&Logo_Draw);

	crayon_memory_free_spritesheet(&Logo);
	crayon_memory_free_palette(&Logo_P);

	crayon_shutdown();

	return 0;
}
