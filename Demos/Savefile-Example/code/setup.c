#include "setup.h"

// NOTE: This function contains the default values only for the current version
// Thats why we can reference the variables directly like this
void savefile_defaults(){
	sf_var2[0] = 5.5;

	// Don't need 32 atm, but if adding more than 65536 vars, you're going to need this
	uint32_t i;
	// uint32_t j;
	for(i = 0; i < sf_var4_length; i++){
		sf_lol[i][0] = 2;

		// I use strncpy instead of strcpy so we know the value
		// of all characters in the buffer
		strncpy(sf_name[i], "PLACEHOLDER", sf_name_length);
	}

	sf_myspace[0] = 1;

	for(i = 0; i < sf_speedrun_times_length; i++){
		sf_speedrun_times[i] = -1;
	}

	sf_coins[0] = 300;

	sf_rand_var_1[0] = 0;
	sf_rand_var_2[0] = 0;
	sf_rand_var_3[0] = 0;
	sf_rand_var_4[0] = 0;
	sf_rand_var_5[0] = 0;

	return;
}

// We have "sf_var4_length" lots of hi and 1 for coins
	// garbage and dunno are forgotten so we don't bother getting them
uint32_t remove_indexes[sf_var4_length + 1];

// This function is designed to handle variables that don't exist anymore in the latest version
// "loaded_variables" is an array of pointers  to where the loaded variables are stored. Note the array
// length is the number of all variables ever, so access the right element with the ID values.
// Variables that still exist in the current version are already handled

// THIS IS USED BY THE CRAYON SAVEFILE DESERIALISER WHEN LOADING A SAVE FROM AN OLDER VERSION
// THERE IS NO NEED TO CALL THIS MANUALLY
int8_t upgrade_savefile(void **loaded_variables, crayon_savefile_version_t loaded_version,
	crayon_savefile_version_t latest_version){

	uint16_t i, j;
	int32_t *s32_ptr;
	uint16_t *u16_ptr;

	// We want to keep the values from hi in a new form and we don't care about "dunno"s value
	if(loaded_version < SFV_MISTAKES_MADE){
		for(i = 0; i < sf_var4_length; i++){
			s32_ptr = loaded_variables[remove_indexes[i + 1]];
			// if(!s32_ptr){continue;}	// Only needed if we didn't have the first if check
			for(j = 0; j < sf_hi_length; j++){
				sf_lol[i][0] += !(s32_ptr[j] % 2);	// If it was even, add one to lol
			}
		}

		u16_ptr = loaded_variables[remove_indexes[0]];
		// if(!u16_ptr){break;}
		sf_coins[0] = u16_ptr[0];
	}

	return 0;
}

uint8_t setup_savefile(crayon_savefile_details_t * details){
	uint8_t i, error;

	#if defined(_arch_pc)

	crayon_savefile_set_base_path("saves/");

	#else
	
	crayon_savefile_set_base_path(NULL);	// Dreamcast ignores the parameter anyways
											// (Assumes "/vmu/") so its still fine to
											// do the method above for all platforms
	#endif

	error = crayon_savefile_init_savefile_details(details, "SAVE_DEMO3.s", SFV_CURRENT,
		savefile_defaults, upgrade_savefile);
	if(error){return 1;}

	error += crayon_savefile_set_app_id(details, "ProtoSaveDemo3");
	error += crayon_savefile_set_short_desc(details, "Save Demo 3");
	error += crayon_savefile_set_long_desc(details, "Crayon's Savefile demo");
	if(error){return 1;}

	#if defined(_arch_dreamcast)

	uint8_t * vmu_lcd_icon = NULL;
	uint8_t vmu_screens_bitmap;
	setup_vmu_icon_load(&vmu_lcd_icon, "/FILES/LCD.bin", &vmu_screens_bitmap);

	// Apply the VMU LCD icon (Apparently this is automatic if your savefile is an ICONDATA.VMS)
	crayon_peripheral_vmu_display_icon(vmu_screens_bitmap, vmu_lcd_icon);
	free(vmu_lcd_icon);	// Already free-d within the above function
	
	if(crayon_savefile_set_icon(details, "/FILES/image.bin", "/FILES/palette.bin", 3, 15)){return 1;}
	if(crayon_savefile_set_eyecatcher(details, "/FILES/eyecatch3.bin")){return 1;}

	#endif

	// v1 vars

	// Now lets construct our history
	remove_indexes[0] = crayon_savefile_add_variable(details, NULL, sf_old_coins_type, sf_old_coins_length,
		SFV_INITIAL, SFV_MISTAKES_MADE);
	crayon_savefile_add_variable(details, &sf_var2, sf_var2_type, sf_var2_length, SFV_INITIAL, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, NULL, sf_dunno_type, sf_dunno_length, SFV_INITIAL, SFV_MISTAKES_MADE);
	for(i = 0; i < sf_var4_length; i++){
		crayon_savefile_add_variable(details, &sf_lol[i], sf_lol_type, sf_lol_length, SFV_INITIAL, VAR_STILL_PRESENT);
		remove_indexes[i + 1] = crayon_savefile_add_variable(details, NULL, sf_hi_type, sf_hi_length, SFV_INITIAL,
			SFV_MISTAKES_MADE);
		crayon_savefile_add_variable(details, &sf_name[i], sf_name_type, sf_name_length, SFV_INITIAL, VAR_STILL_PRESENT);
	}

	// v2 vars

	crayon_savefile_add_variable(details, &sf_myspace, sf_myspace_type, sf_myspace_length, SFV_SPEEDRUNNER,
		VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, &sf_speedrun_times, sf_speedrun_times_type, sf_speedrun_times_length,
		SFV_SPEEDRUNNER, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, NULL, sf_garbage_type, sf_garbage_length, SFV_SPEEDRUNNER,
		SFV_MISTAKES_MADE);


	// v3 vars

	crayon_savefile_add_variable(details, &sf_coins, sf_coins_type,
		sf_coins_length, SFV_MISTAKES_MADE, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, &sf_rand_var_1, sf_rand_var_1_type,
		sf_rand_var_1_length, SFV_MISTAKES_MADE, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, &sf_rand_var_2, sf_rand_var_2_type,
		sf_rand_var_2_length, SFV_MISTAKES_MADE, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, &sf_rand_var_3, sf_rand_var_3_type,
		sf_rand_var_3_length, SFV_MISTAKES_MADE, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, &sf_rand_var_4, sf_rand_var_4_type,
		sf_rand_var_4_length, SFV_MISTAKES_MADE, VAR_STILL_PRESENT);
	crayon_savefile_add_variable(details, &sf_rand_var_5, sf_rand_var_5_type,
		sf_rand_var_5_length, SFV_MISTAKES_MADE, VAR_STILL_PRESENT);

	// Set the savefile
	if(crayon_savefile_solidify(details)){return 1;}

	return 0;
}

// We use a double pointer because we want to modify the pointer itself with malloc
int16_t setup_vmu_icon_load(uint8_t **vmu_lcd_icon, char *icon_path, uint8_t *vmu_bitmap){
	#if defined(_arch_dreamcast)

	*vmu_bitmap = crayon_peripheral_dreamcast_get_screens();

	*vmu_lcd_icon = malloc(6 * 32);	// 6 * 32 because we have 48/32 1bpp so we need that / 8 bytes
	if(*vmu_lcd_icon == NULL){
		return -1;
	}

	FILE * file_lcd_icon = fopen(icon_path, "rb");
	if(!file_lcd_icon){return -1;}

	size_t res = fread(*vmu_lcd_icon, 192, 1, file_lcd_icon);	// If the icon is right, it *must* be 192 bytes
	fclose(file_lcd_icon);

	return res;

	#else

	return 0;
	
	#endif
}
