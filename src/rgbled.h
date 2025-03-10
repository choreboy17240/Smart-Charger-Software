/**
 * @file rgbled.h
 * @brief RGB LED class
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 */
#ifndef _RGBLED_H_
#define _RGBLED_H_

#include "obcharger.h"

/// @brief RGB LED class
class RGB_LED {
public:
    /**
     * @brief Default constructor
     */
    RGB_LED(void);

    /**
     * @brief Constructor with initialization
     * @param r: Pin used for red color
     * @param g: Pin used for green color
     * @param b: Pin used for blue color
     * @param color: Initial color for RGB LED
     */ 
    RGB_LED(PinNumber r, PinNumber g, PinNumber b, rgb_t color=LED_BLK);

    /**
     * @brief Initialize RGB LED
     * @param r: Pin used for red color
     * @param g: Pin used for green color
     * @param b: Pin used for blue color
     * @param color: Initial color for RGB LED
     * @returns Nothing
     */
    void begin(PinNumber r, PinNumber g, PinNumber b, rgb_t color=LED_BLK);

    /**
     * @brief Set color of RGB led
     * @param value: Color values for RGB LED
     * @returns Nothing
     */
    void color(rgb_t value);


private:
    PinNumber GP_LEDR;              ///< RBG LED red (0=on, 1=Off)
    PinNumber GP_LEDG;              ///< RGB LED green (0=on, 1=Off)
    PinNumber GP_LEDB;              ///< RGB LED blue (0=on, 1=Off)

    rgb_t rgb_color = {0,0,0};      ///< Default color is black (off)

};

#endif