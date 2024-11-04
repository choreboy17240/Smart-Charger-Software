/** 
 * @file oled.h
 *
 * @mainpage OLED display support
 * 
 */
#ifndef _OLED_H_
#define _OLED_H_

// Uses Adafruit SSD1306 driver with GFX library
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/**
 *  OLED display definitions
 */

/// I2C address for 128x32 display
#define ADDRESS_128x32  0x3C

/// I2C address for 128x64 display
#define ADDRESS_128x64  0x3D    

/// Reset GPIO number (set to -1 if no hard reset pin)
#define OLED_RESET      -1

/// Display size
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  32

// Private constants
#define NO_HARD_RESET   false
#define NO_WIRE_INIT    false
#define WIRE_INIT       true

// Text justification types
enum justification_t { LEFT, CENTER, RIGHT};

/**
 * Function prototypes
 */


/**
 * @brief OLED test message handler
 * @param display: Adafruit display to display message box
 * @param name: Name of the test being run
 * @param result: Result of the test being run (default: nullptr)
 * @note If result is omitted or set to nullptr, the test is incomplete.
 *       Display will be cleared and the test name will be displayed.
 *       Display will be updated to show the results if called with 
 *       results not set to nullptr value.
 */
void test_msg(Adafruit_SSD1306 &display, const char *name, const char *result=nullptr);


/**
 * @brief OLED display message box
 * @param display: Adafruit display to display message box
 * @param msg: Message to be displayed
 * @param pos: Message justification (left/right/center)
 */
void msg_box(Adafruit_SSD1306 &display, const char *msg, justification_t pos);

#endif