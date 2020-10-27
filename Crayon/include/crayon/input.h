#ifndef INPUT_CRAYON_H
#define INPUT_CRAYON_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

//For the controller stuff
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include "vector_structs.h"
#include "misc.h"

// Using triggers like digital buttons. Pass in the trigger values from KOS ATM
uint8_t crayon_input_trigger_pressed(uint8_t curr_trig, uint8_t prev_trig);
uint8_t crayon_input_trigger_released(uint8_t curr_trig, uint8_t prev_trig);
uint8_t crayon_input_trigger_held(uint8_t trig);

// Returns a bitmap showing which of the buttons meet the condition
uint32_t crayon_input_button_pressed(uint32_t current_buttons, uint32_t previous_buttons, uint32_t button_bitmap);
uint32_t crayon_input_button_released(uint32_t current_buttons, uint32_t previous_buttons, uint32_t button_bitmap);
uint32_t crayon_input_button_held(uint32_t current_buttons, uint32_t button_bitmap);

// Convert the controller's thumbstick reading into the D-PAD button bitmap. For menuing
	// Deadspace is a radius between 0 and 1 for the space in the middle we ignore
// The buttons aren't mapped right (?)
uint32_t crayon_input_thumbstick_to_dpad(int joyx, int joyy, float deadspace);
uint32_t crayon_input_thumbstick2_to_dpad2(int joyx, int joyy, float deadspace);

// Converts dpad inputs into the equivalent thumbstick vectors
vec2_f_t crayon_input_dpad_to_thumbstick(uint32_t buttons);

// Converts the DC's thumbstick value into the -1 to 1 range
float crayon_input_thumbstick_int_to_float(int joy);

#endif