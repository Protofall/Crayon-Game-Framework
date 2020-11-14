//Crayon libraries
#include <crayon/memory.h>
#include <crayon/graphics.h>
#include <crayon/misc.h>
#include <crayon/debug.h>

//For region and htz stuff
#include <dc/flashrom.h>

//For the controller
#include <dc/maple.h>
#include <dc/maple/controller.h>

#if CRAYON_BOOT_MODE == 1
	//For mounting the sd dir
	#include <dc/sd.h>
	#include <kos/blockdev.h>
	#include <fat/fs_fat.h>
#endif


#if CRAYON_BOOT_MODE == 1
	#define MNT_MODE FS_FAT_MOUNT_READONLY

	static void unmount_fat_sd(){
		fs_fat_unmount("/sd");
		fs_fat_shutdown();
		sd_shutdown();
	}

	static int mount_fat_sd(){
		kos_blockdev_t sd_dev;
		uint8 partition_type;

		// Initialize the sd card if its present
		if(sd_init()){
			return 1;
		}

		// Grab the block device for the first partition on the SD card. Note that
		// you must have the SD card formatted with an MBR partitioning scheme
		if(sd_blockdev_for_partition(0, &sd_dev, &partition_type)){
			return 2;
		}

		// Check to see if the MBR says that we have a valid partition
		// if(partition_type != 0x83){
			//I don't know what value I should be comparing against, hence this check is disabled for now
			// This: https://en.wikipedia.org/wiki/Partition_type
				//Suggests there's multiple types for FAT...not sure how to handle this
			// return 3;
		// }

		// Initialize fs_fat and attempt to mount the device
		if(fs_fat_init()){
			return 4;
		}

		//Mount the SD card to the sd dir in the VFS
		if(fs_fat_mount("/sd", &sd_dev, MNT_MODE)){
			return 5;
		}
		return 0;
	}

#endif

float thumbstick_int_to_float(int joy){
	float ret_val;	//Range from -1 to 1

	if(joy > 0){	//Converting from -128, 127 to -1, 1
		ret_val = joy / 127.0;
	}
	else{
		ret_val = joy / 128.0;
	}

	return ret_val;
}

crayon_sprite_array_t Faces_Draw[3];
uint8_t frame_indexes[3];

void set_msg(char * buffer, uint8_t code){
	switch(code){
	case 0:
		strcpy(buffer, "Instructions.\n\
			Press A to cycle to next sprite\n\
			L-Trigger and R-Trigger to change options\n\
			D-PAD Down to decrease a value, D-PAD Up to increase it\n\
			D-PAD Left and D-PAD Right to change sub option (Colour)\n\
			Y to hide these instructions\n\
			Start to terminate program");
		break;
	default:
		strcpy(buffer, "Unknown code given");
	}
	return;
}

//0 = frame_id
//1 = colour
//2 = fade
//3 = scale x
//4 = scale y
//5 = flip
//6 = rotate
//7 = layer
void set_msg_option(char * buffer, uint8_t option, uint8_t sub_option, uint8_t sprite){
	uint8_t holder_value_u8;
	switch(option){
	case 0:
		sprintf(buffer, "Option: Frame. Value: %d", frame_indexes[sprite]);
		break;
	case 1:
		switch(sub_option){
		case 0:
			holder_value_u8 = crayon_misc_extract_bits(crayon_memory_get_colour(&Faces_Draw[sprite], 0, NULL), 8, 24);
			sprintf(buffer, "Option: Colour. Sub-Option: Alpha. Value: 0x%x", holder_value_u8);
			break;
		case 1:
			holder_value_u8 = crayon_misc_extract_bits(crayon_memory_get_colour(&Faces_Draw[sprite], 0, NULL), 8, 16);
			sprintf(buffer, "Option: Colour. Sub-Option: Red. Value: 0x%x", holder_value_u8);
			break;
		case 2:
			holder_value_u8 = crayon_misc_extract_bits(crayon_memory_get_colour(&Faces_Draw[sprite], 0, NULL), 8, 8);
			sprintf(buffer, "Option: Colour. Sub-Option: Green. Value: 0x%x", holder_value_u8);
			break;
		case 3:
			holder_value_u8 = crayon_misc_extract_bits(crayon_memory_get_colour(&Faces_Draw[sprite], 0, NULL), 8, 0);
			sprintf(buffer, "Option: Colour. Sub-Option: Blue. Value: 0x%x", holder_value_u8);
			break;
		}
		break;
	case 2:
		sprintf(buffer, "Option: Fade. Value: 0x%x", Faces_Draw[sprite].fade[0]);
		break;
	case 3:
		if(Faces_Draw[sprite].options & CRAY_COLOUR_ADD){
			sprintf(buffer, "Option: Colour-Mix-Mode: ADD");
		}
		else{
			sprintf(buffer, "Option: Colour-Mix-Mode: BLEND");
		}
		break;
	case 4:
		sprintf(buffer, "Option: Scale X. Value: %.2f", (double)Faces_Draw[sprite].scale[0].x);
		break;
	case 5:
		sprintf(buffer, "Option: Scale Y. Value: %.2f", (double)Faces_Draw[sprite].scale[0].y);
		break;
	case 6:
		sprintf(buffer, "Option: Flip. Value: %d", Faces_Draw[sprite].flip[0]);
		break;
	case 7:
		sprintf(buffer, "Option: Rotation. Value: %.2f", (double)Faces_Draw[sprite].rotation[0]);
		break;
	case 8:
		sprintf(buffer, "Option: Layer. Value: %d", Faces_Draw[sprite].layer[0]);
		break;
	}
}

void set_msg_sprite(char * buffer, uint8_t sprite){
	sprintf(buffer, "Sprite: %d", sprite);
	return;
}

int main(){
	#if CRAYON_BOOT_MODE == 1
		int sdRes = mount_fat_sd();	//This function should be able to mount a FAT32 formatted sd card to the /sd dir	
		if(sdRes != 0){
			error_freeze("SD card couldn't be mounted: %d", sdRes);
		}
	#endif

	crayon_graphics_init(CRAYON_ENABLE_OP | CRAYON_ENABLE_PT | CRAYON_ENABLE_TR);

	//load in assets here
	crayon_sprite_array_t Highlight_Draw;
	crayon_spritesheet_t Faces_SS;
	crayon_font_mono_t BIOS;
	crayon_palette_t Faces_P, BIOS_P;

	#if CRAYON_BOOT_MODE == 0
		crayon_memory_mount_romdisk("/cd/stuff.img", "/stuff");
	#elif CRAYON_BOOT_MODE == 1
		crayon_memory_mount_romdisk("/sd/stuff.img", "/stuff");
	#elif CRAYON_BOOT_MODE == 2
		crayon_memory_mount_romdisk("/pc/stuff.img", "/stuff");
	#else
		#error "Invalid crayon boot mode"
	#endif

	crayon_memory_load_spritesheet(&Faces_SS, &Faces_P, 0, "/stuff/opaque.dtex");
	crayon_memory_load_mono_font_sheet(&BIOS, &BIOS_P, 1, "/stuff/BIOS.dtex");

	fs_romdisk_unmount("/stuff");

	#if CRAYON_BOOT_MODE == 1
		unmount_fat_sd();	//Unmounts the SD dir to prevent corruption since we won't need it anymore
	#endif

	uint8_t error = 0;
	uint8_t i;
	for(i = 0; i < 3; i++){
		error = crayon_memory_init_sprite_array(&Faces_Draw[i], &Faces_SS, 0, &Faces_P, 1, 1, 0, PVR_FILTER_NONE, 0);
		if(error){break;}
		error += crayon_memory_set_layer(&Faces_Draw[i], 0, 50);
		error += crayon_memory_set_scale_x(&Faces_Draw[i], 0, 4);
		error += crayon_memory_set_scale_y(&Faces_Draw[i], 0, 4);
		error += crayon_memory_set_coord_x(&Faces_Draw[i], 0,
			((2 + i) * 640 / 6.0) - (crayon_graphics_get_draw_element_height(&Faces_Draw[0], 0) / 2.0));
		error += crayon_memory_set_coord_y(&Faces_Draw[i], 0,
			240 - (crayon_graphics_get_draw_element_height(&Faces_Draw[0], 0) / 2.0));
		error += crayon_memory_set_flip(&Faces_Draw[i], 0, 0);
		error += crayon_memory_set_rotation(&Faces_Draw[i], 0, 0);
		error += crayon_memory_set_colour(&Faces_Draw[i], 0, 0xFFFFFFFF);
		error += crayon_memory_set_fade(&Faces_Draw[i], 0, 0);
		error += crayon_memory_set_frame_id(&Faces_Draw[i], 0, 0);
		error += crayon_memory_set_visibility(&Faces_Draw[i], 0, 1);
		error += crayon_memory_set_frame_uv(&Faces_Draw[i], 0, 0 + i);
		if(error){break;}
	}

	if(error){error_freeze("An error in sprite arrays occurred");}

	frame_indexes[0] = 0;
	frame_indexes[1] = 1;
	frame_indexes[2] = 2;

	uint8_t option = 0;
	uint8_t sub_option = 0;
	uint8_t max_options = 9;
	uint8_t sprite = 0;
	uint8_t holder_value_u8 = 0;
	uint32_t holder_value_u32 = 0;
	uint8_t escape = 0;

	//The highlight box (Most of the details are set below)
	error = crayon_memory_init_sprite_array(&Highlight_Draw, NULL, 0, NULL, 1, 1, 0, PVR_FILTER_NONE, 0);
	if(error){error_freeze("An error in sprite arrays occurred");}
	error += crayon_memory_set_colour(&Highlight_Draw, 0, 0x88FF0000);
	error += crayon_memory_set_visibility(&Highlight_Draw, 0, 1);
	if(error){error_freeze("An error in sprite arrays occurred");}

	uint8_t hide_msg = 0;
	char msg[1024];
	char msg_option[80];
	//0 = frame_id
	//1 = colour
	//2 = fade
	//3 = Blend/Add colour modes
	//4 = scale x
	//5 = scale y
	//6 = flip
	//7 = rotate
	//8 = layer
	char msg_sprite[16];
	set_msg(msg, 0);
	set_msg_option(msg_option, option, sub_option, sprite);
	set_msg_sprite(msg_sprite, sprite);

	pvr_set_bg_color(0.3, 0.3, 0.3); // Its useful-ish for debugging

	crayon_graphics_setup_palette(&Faces_P);
	crayon_graphics_setup_palette(&BIOS_P);

	uint32_t prev_btns[4] = {0};
	vec2_u8_t prev_trigs[4] = {(vec2_u8_t){0,0}};
	vec2_f_t thumb = (vec2_f_t){0,0};
	uint8_t thumb_active;
	while(!escape){
		pvr_wait_ready();
		MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

			//Choose option
			if((st->ltrig > 0xFF * 0.1) && (prev_trigs[__dev->port].x <= 0xFF * 0.1)){
				if(option == 0){option = max_options - 1;}
				else{option--;}
				sub_option = 0;
				set_msg_option(msg_option, option, sub_option, sprite);
			}
			if((st->rtrig > 0xFF * 0.1) && (prev_trigs[__dev->port].y <= 0xFF * 0.1)){
				if(option == max_options - 1){option = 0;}
				else{option++;}
				sub_option = 0;
				set_msg_option(msg_option, option, sub_option, sprite);
			}

			//Choose sprite
			if((st->buttons & CONT_A) && !(prev_btns[__dev->port] & CONT_A)){
				sprite++;
				sprite %= 3;
				set_msg_sprite(msg_sprite, sprite);
				set_msg_option(msg_option, option, sub_option, sprite);	//To update the var
			}

			//Hid HUD text
			if((st->buttons & CONT_Y) && !(prev_btns[__dev->port] & CONT_Y)){
				hide_msg = !hide_msg;
			}

			if((st->buttons & CONT_START) && !(prev_btns[__dev->port] & CONT_START)){
				escape = 1;
			}

			thumb.x = thumbstick_int_to_float(st->joyx);
			thumb.y = thumbstick_int_to_float(st->joyy);
			thumb_active = ((thumb.x * thumb.x) + (thumb.y * thumb.y) > 0.4 * 0.4);

			if(thumb_active){
				Faces_Draw[sprite].coord[0].x += (thumb.x * 2.5);
				Faces_Draw[sprite].coord[0].y += (thumb.y * 2.5);
			}

			//For ones we want to keep increasing per frame
			if((st->buttons & CONT_DPAD_UP)){
				switch(option){
				case 1:
					holder_value_u32 = crayon_memory_get_colour(&Faces_Draw[sprite], 0, NULL);
					switch(sub_option){
					case 0:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 24);
						if(holder_value_u8 < 0xFF){holder_value_u8++;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 24);
						break;
					case 1:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 16);
						if(holder_value_u8 < 0xFF){holder_value_u8++;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 16);
						break;
					case 2:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 8);
						if(holder_value_u8 < 0xFF){holder_value_u8++;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 8);
						break;
					case 3:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 0);
						if(holder_value_u8 < 0xFF){holder_value_u8++;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 0);
						break;
					}
					crayon_memory_set_colour(&Faces_Draw[sprite], 0, holder_value_u32);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 2:
					holder_value_u8 = crayon_memory_get_fade(&Faces_Draw[sprite], 0, NULL);
					if(holder_value_u8 != 255){
						holder_value_u8++;
						crayon_memory_set_fade(&Faces_Draw[sprite], 0, holder_value_u8);
					}
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 4:
					crayon_memory_set_scale_x(&Faces_Draw[sprite], 0, crayon_memory_get_scale_x(&Faces_Draw[sprite], 0, NULL) + 0.05);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 5:
					crayon_memory_set_scale_y(&Faces_Draw[sprite], 0, crayon_memory_get_scale_y(&Faces_Draw[sprite], 0, NULL) + 0.05);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 7:
					crayon_memory_set_rotation(&Faces_Draw[sprite], 0, crayon_memory_get_rotation(&Faces_Draw[sprite], 0, NULL) + 1);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				}
			}
			if((st->buttons & CONT_DPAD_UP) && !(prev_btns[__dev->port] & CONT_DPAD_UP)){
				switch(option){
				case 0:
					frame_indexes[sprite]++;
					if(frame_indexes[sprite] >= Faces_SS.animation[0].frame_count){frame_indexes[sprite] = 0;}	//For some reason, this is triggering early
					crayon_memory_set_frame_uv(&Faces_Draw[sprite], 0, frame_indexes[sprite]);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 3:
					Faces_Draw[sprite].options ^= CRAY_COLOUR_ADD;	//Will toggle between Blend and Add modes
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 6:
					crayon_memory_set_flip(&Faces_Draw[sprite], 0, !crayon_memory_get_flip(&Faces_Draw[sprite], 0, NULL));
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 8:
					holder_value_u8 = crayon_memory_get_layer(&Faces_Draw[sprite], 0, NULL);
					if(holder_value_u8 != 255){
						holder_value_u8++;
						crayon_memory_set_layer(&Faces_Draw[sprite], 0, holder_value_u8);
					}
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				}
			}
			//Decrease value
			if((st->buttons & CONT_DPAD_DOWN)){
				switch(option){
				case 1:
					holder_value_u32 = crayon_memory_get_colour(&Faces_Draw[sprite], 0, NULL);
					switch(sub_option){
					case 0:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 24);
						if(holder_value_u8 > 0){holder_value_u8--;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 24);
						break;
					case 1:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 16);
						if(holder_value_u8 > 0){holder_value_u8--;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 16);
						break;
					case 2:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 8);
						if(holder_value_u8 > 0){holder_value_u8--;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 8);
						break;
					case 3:
						holder_value_u8 = crayon_misc_extract_bits(holder_value_u32, 8, 0);
						if(holder_value_u8 > 0){holder_value_u8--;}
						holder_value_u32 = crayon_misc_insert_bits(holder_value_u32, holder_value_u8, 8, 0);
						break;
					}
					crayon_memory_set_colour(&Faces_Draw[sprite], 0, holder_value_u32);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 2:
					holder_value_u8 = crayon_memory_get_fade(&Faces_Draw[sprite], 0, NULL);
					if(holder_value_u8 != 0){
						holder_value_u8--;
						crayon_memory_set_fade(&Faces_Draw[sprite], 0, holder_value_u8);
					}
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 4:
					crayon_memory_set_scale_x(&Faces_Draw[sprite], 0, crayon_memory_get_scale_x(&Faces_Draw[sprite], 0, NULL) - 0.05);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 5:
					crayon_memory_set_scale_y(&Faces_Draw[sprite], 0, crayon_memory_get_scale_y(&Faces_Draw[sprite], 0, NULL) - 0.05);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 7:
					crayon_memory_set_rotation(&Faces_Draw[sprite], 0, crayon_memory_get_rotation(&Faces_Draw[sprite], 0, NULL) - 1);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				}
			}
			if((st->buttons & CONT_DPAD_DOWN) && !(prev_btns[__dev->port] & CONT_DPAD_DOWN)){
				switch(option){
				case 0:
					if(frame_indexes[sprite] <= 0){frame_indexes[sprite] = Faces_SS.animation[0].frame_count;}
					frame_indexes[sprite]--;
					crayon_memory_set_frame_uv(&Faces_Draw[sprite], 0, frame_indexes[sprite]);
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 3:
					Faces_Draw[sprite].options ^= CRAY_COLOUR_ADD;
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 6:
					crayon_memory_set_flip(&Faces_Draw[sprite], 0, !crayon_memory_get_flip(&Faces_Draw[sprite], 0, NULL));
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				case 8:
					holder_value_u8 = crayon_memory_get_layer(&Faces_Draw[sprite], 0, NULL);
					if(holder_value_u8 != 1){
						holder_value_u8--;
						crayon_memory_set_layer(&Faces_Draw[sprite], 0, holder_value_u8);
					}
					set_msg_option(msg_option, option, sub_option, sprite);
					break;
				}
			}

			//Change between sub values (ARGB)
			if(option == 1){
				if((st->buttons & CONT_DPAD_LEFT) && !(prev_btns[__dev->port] & CONT_DPAD_LEFT)){
					if(sub_option != 0){
						sub_option--;
					}
					else{
						sub_option = 3;
					}
					set_msg_option(msg_option, option, sub_option, sprite);
				}
				if((st->buttons & CONT_DPAD_RIGHT) && !(prev_btns[__dev->port] & CONT_DPAD_RIGHT)){
					sub_option++;
					sub_option %= 4;
					set_msg_option(msg_option, option, sub_option, sprite);
				}
			}

			//Store the buttons and triggers for next loop
			prev_btns[__dev->port] = st->buttons;
			prev_trigs[__dev->port].x = st->ltrig;
			prev_trigs[__dev->port].y = st->rtrig;
		MAPLE_FOREACH_END()

		if(crayon_memory_get_layer(&Faces_Draw[sprite], 0, NULL) > 1){crayon_memory_set_layer(&Highlight_Draw, 0, crayon_memory_get_layer(&Faces_Draw[sprite], 0, NULL) - 1);}
		else{crayon_memory_set_layer(&Highlight_Draw, 0, 1);}
		crayon_memory_set_scale_x(&Highlight_Draw, 0, crayon_graphics_get_draw_element_width(&Faces_Draw[sprite], 0) + 4);
		crayon_memory_set_scale_y(&Highlight_Draw, 0, crayon_graphics_get_draw_element_height(&Faces_Draw[sprite], 0) + 4);
		crayon_memory_set_coord_x(&Highlight_Draw, 0, crayon_memory_get_coord_x(&Faces_Draw[sprite], 0, NULL) - 2);
		crayon_memory_set_coord_y(&Highlight_Draw, 0, crayon_memory_get_coord_y(&Faces_Draw[sprite], 0, NULL) - 2);
		crayon_memory_set_rotation(&Highlight_Draw, 0, crayon_memory_get_rotation(&Faces_Draw[sprite], 0, NULL));
		// Highlight_Draw.colour[0] = crayon_misc_insert_bits(crayon_misc_extract_bits(Faces_Draw[sprite].colour[0], 24, 0), crayon_misc_extract_bits(Faces_Draw[sprite].colour[0], 24, 0), 8, 24);

		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_TR_POLY);
			if(!hide_msg){
				crayon_graphics_draw_sprites(&Highlight_Draw, NULL, PVR_LIST_TR_POLY, CRAYON_DRAW_ENHANCED);
			}

			crayon_graphics_draw_sprites(&Faces_Draw[0], NULL, PVR_LIST_TR_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Faces_Draw[1], NULL, PVR_LIST_TR_POLY, CRAYON_DRAW_ENHANCED);
			crayon_graphics_draw_sprites(&Faces_Draw[2], NULL, PVR_LIST_TR_POLY, CRAYON_DRAW_ENHANCED);
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_OP_POLY);
			if(!hide_msg){
				crayon_graphics_draw_text_mono(msg, &BIOS, PVR_LIST_OP_POLY, 32, 480 - (BIOS.char_height * 10.5), 255, 1, 1, BIOS_P.palette_id);
				crayon_graphics_draw_text_mono(msg_option, &BIOS, PVR_LIST_OP_POLY, 32, 480 - (BIOS.char_height * 3), 255, 1, 1, BIOS_P.palette_id);
				crayon_graphics_draw_text_mono(msg_sprite, &BIOS, PVR_LIST_OP_POLY, 32, 480 - (BIOS.char_height * 2), 255, 1, 1, BIOS_P.palette_id);
			}
		pvr_list_finish();

		pvr_scene_finish();
	}

	crayon_memory_free_spritesheet(&Faces_SS);
	crayon_memory_free_mono_font_sheet(&BIOS);
	crayon_memory_free_palette(&BIOS_P);
	crayon_memory_free_palette(&Faces_P);
	crayon_memory_free_sprite_array(&Faces_Draw[0]);
	crayon_memory_free_sprite_array(&Faces_Draw[1]);
	crayon_memory_free_sprite_array(&Faces_Draw[2]);
	crayon_memory_free_sprite_array(&Highlight_Draw);

	pvr_shutdown();

	return 0;
}
