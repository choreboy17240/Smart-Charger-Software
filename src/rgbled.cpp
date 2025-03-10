/**
 * @file rgbled.cpp
 * @brief RGB LED support functions
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 */

#include "rgbled.h"

// Default constructor
RGB_LED::RGB_LED(void) {
    rgb_color = {0,0,0};
}

// Constructor with initialization
RGB_LED::RGB_LED(PinNumber r, PinNumber g, PinNumber b, rgb_t color) {
    begin(r, g, b, color);
}

void RGB_LED::begin(PinNumber r, PinNumber g, PinNumber b, rgb_t color) {
    // Setup red GPIO
    GP_LEDR = r;
    digitalWrite(GP_LEDR, HIGH);
    pinMode(GP_LEDR, OUTPUT_OPEN_DRAIN);

    // Setup green GPIO
    GP_LEDG = g;
    digitalWrite(GP_LEDG, HIGH);
    pinMode(GP_LEDG, OUTPUT_OPEN_DRAIN);

    // Setup blue GPIO
    GP_LEDB = b;
    digitalWrite(GP_LEDB, HIGH);
    pinMode(GP_LEDB, OUTPUT_OPEN_DRAIN);

    // Set RGB LED color
    RGB_LED::color(color);
}

// Set color of RGB led
// Invert the color value since we're active low connection to LED
void RGB_LED::color(rgb_t value) {
    analogWrite(GP_LEDR, 255- value.r);
    analogWrite(GP_LEDG, 255- value.g);
    analogWrite(GP_LEDB, 255- value.b);

    // Cache RGB color value
    rgb_color = value;
}
