#include "input.h"

uint8_t crayon_input_trigger_pressed(uint8_t curr_trig, uint8_t prev_trig){
	return ((curr_trig > 255 * 0.1) && !(prev_trig > 255 * 0.1));
}

uint8_t crayon_input_trigger_released(uint8_t curr_trig, uint8_t prev_trig){
	return (!(curr_trig > 255 * 0.1) && (prev_trig > 255 * 0.1));
}

uint8_t crayon_input_trigger_held(uint8_t trig){
	return (trig > 255 * 0.1);
}

uint32_t crayon_input_button_pressed(uint32_t current_buttons, uint32_t previous_buttons, uint32_t button_bitmap){
	return ((current_buttons & button_bitmap) && !(previous_buttons & button_bitmap));
}

uint32_t crayon_input_button_released(uint32_t current_buttons, uint32_t previous_buttons, uint32_t button_bitmap){
	return (!(current_buttons & button_bitmap) && (previous_buttons & button_bitmap));
}

uint32_t crayon_input_button_held(uint32_t current_buttons, uint32_t button_bitmap){
	return (current_buttons & button_bitmap);
}

uint32_t crayon_input_thumbstick_to_dpad(int joyx, int joyy, float deadspace){
	float thumb_x = crayon_input_thumbstick_int_to_float(joyx);
	float thumb_y = crayon_input_thumbstick_int_to_float(joyy);

	// If the thumbstick is inside the deadspace then we don't check angle
	if((thumb_x * thumb_x) + (thumb_y * thumb_y) < deadspace * deadspace){
		return 0;
	}

	// 8 options. N, NE, E, SE, S, SW, W, NW.

	// Rotate the thumbstick coordinate 22.5 degrees (Or 22.5 * (PI/180) ~= 0.3927 radians) clockwise
		// 22.5 degrees is 1/16th of 360 degrees, this makes it easier to check which region the coords are in
	float angle = 22.5 * M_PI / 180.0;	// In radians

	vec2_f_t point = crayon_misc_rotate_point((vec2_f_t){0, 0}, (vec2_f_t){thumb_x, thumb_y}, angle);
	thumb_x = point.x;
	thumb_y = point.y;

	float abs_x = fabs(thumb_x);
	float abs_y = fabs(thumb_y);

	uint32_t bitmap;
	if(thumb_y < 0){	// This check always works
		if(thumb_x > 0){
			if(abs_y > abs_x){	// N
				bitmap = CONT_DPAD_UP;
			}
			else{	// NE
				bitmap = CONT_DPAD_UP | CONT_DPAD_RIGHT;
			}
		}
		else{
			if(abs_y < abs_x){	// W
				bitmap = CONT_DPAD_LEFT;
			}
			else{	// NW
				bitmap = CONT_DPAD_UP | CONT_DPAD_LEFT;
			}
		}
	}
	else{
		if(thumb_x > 0){
			if(abs_y < abs_x){	// E
				bitmap = CONT_DPAD_RIGHT;
			}
			else{	// SE
				bitmap = CONT_DPAD_DOWN | CONT_DPAD_RIGHT;
			}
		}
		else{
			if(abs_y > abs_x){	// S
				bitmap = CONT_DPAD_DOWN;
			}
			else{	// SW
				bitmap = CONT_DPAD_DOWN | CONT_DPAD_LEFT;
			}
		}
	}

	return bitmap;
}

uint32_t crayon_input_thumbstick2_to_dpad2(int joyx, int joyy, float deadspace){
	float thumb_x = crayon_input_thumbstick_int_to_float(joyx);
	float thumb_y = crayon_input_thumbstick_int_to_float(joyy);

	// If the thumbstick is inside the deadspace then we don't check angle
	if((thumb_x * thumb_x) + (thumb_y * thumb_y) < deadspace * deadspace){
		return 0;
	}

	// 8 options. N, NE, E, SE, S, SW, W, NW.

	// Rotate the thumbstick coordinate 22.5 degrees (Or 22.5 * (PI/180) ~= 0.3927 radians) clockwise
		// 22.5 degrees is 1/16th of 360 degrees, this makes it easier to check which region the coords are in
	float angle = 22.5 * M_PI / 180.0;	// In radians

	vec2_f_t point = crayon_misc_rotate_point((vec2_f_t){0, 0}, (vec2_f_t){thumb_x, thumb_y}, angle);
	thumb_x = point.x;
	thumb_y = point.y;

	float abs_x = fabs(thumb_x);
	float abs_y = fabs(thumb_y);

	uint32_t bitmap;
	if(thumb_y < 0){	// This check always works
		if(thumb_x > 0){
			if(abs_y > abs_x){	// N
				bitmap = CONT_DPAD2_UP;
			}
			else{	// NE
				bitmap = CONT_DPAD2_UP | CONT_DPAD2_RIGHT;
			}
		}
		else{
			if(abs_y < abs_x){	// W
				bitmap = CONT_DPAD2_LEFT;
			}
			else{	// NW
				bitmap = CONT_DPAD2_UP | CONT_DPAD2_LEFT;
			}
		}
	}
	else{
		if(thumb_x > 0){
			if(abs_y < abs_x){	// E
				bitmap = CONT_DPAD2_RIGHT;
			}
			else{	// SE
				bitmap = CONT_DPAD2_DOWN | CONT_DPAD2_RIGHT;
			}
		}
		else{
			if(abs_y > abs_x){	// S
				bitmap = CONT_DPAD2_DOWN;
			}
			else{	// SW
				bitmap = CONT_DPAD2_DOWN | CONT_DPAD2_LEFT;
			}
		}
	}

	return bitmap;
}

vec2_f_t crayon_input_dpad_to_thumbstick(uint32_t buttons){
	if(buttons & CONT_DPAD_UP){
		if(buttons & CONT_DPAD_LEFT){
			return (vec2_f_t) {-1 * sqrt(1), -1 * sqrt(1)};
		}
		if(buttons & CONT_DPAD_RIGHT){
			return (vec2_f_t) {sqrt(1), -1 * sqrt(1)};
		}
		return (vec2_f_t) {0, -1};
	}
	if(buttons & CONT_DPAD_DOWN){
		if(buttons & CONT_DPAD_LEFT){
			return (vec2_f_t) {-1 * sqrt(1), sqrt(1)};
		}
		if(buttons & CONT_DPAD_RIGHT){
			return (vec2_f_t) {sqrt(1), sqrt(1)};
		}
		return (vec2_f_t) {0, 1};
	}
	if(buttons & CONT_DPAD_LEFT){
		return (vec2_f_t) {-1, 0};
	}
	if(buttons & CONT_DPAD_RIGHT){
		return (vec2_f_t) {1, 0};
	}

	return (vec2_f_t) {0, 0};
}

vec2_f_t crayon_input_dpad2_to_thumbstick(uint32_t buttons){
	if(buttons & CONT_DPAD2_UP){
		if(buttons & CONT_DPAD2_LEFT){
			return (vec2_f_t) {-1 * sqrt(1), -1 * sqrt(1)};
		}
		if(buttons & CONT_DPAD2_RIGHT){
			return (vec2_f_t) {sqrt(1), -1 * sqrt(1)};
		}
		return (vec2_f_t) {0, -1};
	}
	if(buttons & CONT_DPAD2_DOWN){
		if(buttons & CONT_DPAD2_LEFT){
			return (vec2_f_t) {-1 * sqrt(1), sqrt(1)};
		}
		if(buttons & CONT_DPAD2_RIGHT){
			return (vec2_f_t) {sqrt(1), sqrt(1)};
		}
		return (vec2_f_t) {0, 1};
	}
	if(buttons & CONT_DPAD2_LEFT){
		return (vec2_f_t) {-1, 0};
	}
	if(buttons & CONT_DPAD2_RIGHT){
		return (vec2_f_t) {1, 0};
	}

	return (vec2_f_t) {0, 0};
}

float crayon_input_thumbstick_int_to_float(int joy){
	if(joy > 0){	// Converting from -128, 127 to -1, 1
		return joy / 127.0;
	}
	return joy / 128.0;
}
