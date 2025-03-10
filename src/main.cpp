/**
 *  @file main.cpp
 *  @brief Main source file with setup() and loop() functions
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */

#include <Arduino.h>

#include "obcharger.h"
#include "rgbled.h"
#include "battery.h"
#include "fast.h"
#include "topping.h"
#include "trickle.h"
#include "standby.h"

// Libraries
#include <i2c_busio.h>
#include <ina219.h>
#include <ringbuffer.h>

// OLED display support
#include <STM32_4kOLED.h>

/// I2C address for 128x32 display
#define ADDRESS_128x32  0x3C

/// I2C address for 128x64 display
#define ADDRESS_128x64  0x3D    

// Utility function from cycle module
extern void milliunits_to_string(uint32_t milliunits, uint8_t places, char *buffer, uint8_t buffer_len);

//=============================================================================
// Global variables
//=============================================================================

/// Master start time (beginning of program)
time_ms_t start_time;

/// Loop timer
time_ms_t loop_timer;

/// Master charger state
charger_state_t charger_state;

/// Timer support
Alarm_Pool timer_pool;

/**
 *  @brief Timer pool interrupt handler used by the Alarm class
 */
void timer_pool_handler(void) {
    timer_pool.dec();
}

/// Fast charging cycle handler, derived from the `Charge_Cycle` class
Fast_Charger fast_charger;
/// Topping charging cycle handler, derived from the `Charge_Cycle` class
Topping_Charger topping_charger;
/// Trickle charging cycle handler, derived from the `Charge_Cycle` class
Trickle_Charger trickle_charger;
/// Standby mode handler, derived from the `Charge_Cycle` class
Standby_Charger standby_charger;

/// I2C bus object
I2C main_i2c_bus = I2C(&Wire, I2C0_SCL_GPIO, I2C0_SDA_GPIO, I2C0_BAUDRATE);

/// Battery object
Battery battery;

/// Current sensor object
INA219 sensor;

/// DAC object
MCP4726 dac;

/// Voltage regulator object
Vreg vreg;

/// RGB LED object
RGB_LED rgb_led;

/// Indicates whether OLED display was detected
bool oled_found = false;   

// SSD1306 display object
// SSD1306PrintDevice oled(&tiny4koled_begin_wire, &tiny4koled_beginTransmission_wire, &datacute_write_wire, &datacute_endTransmission_wire);
SSD1306PrintDevice oled;

/**
 * @brief Ring buffer for current readings
 * @details
 * This buffer stores historical current readings to allow calculating a
 * running average to reduce volatility of the charging current value.
 */
RingBuffer16 rb_charging_current(RB_CHARGING_CURRENT_SAMPLES);

//=============================================================================
// Utility functions
//=============================================================================

/**
 *  @brief Print-out I2C scan results to the serial console.
 *  @param addresses_found: Table containing status of I2C addresses from scan.
 *  @note Active I2C addresses found during the I2C bus scan will be set to a
 *        `true` value.  Inactive I2C addresses will be set to a `false` value.
 */
void display_i2c_map(bool *addresses_found) {
    Serial.printf("    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
 
    for (int addr = 0; addr < 128; ++addr) {
        if (addr % 16 == 0) {
            Serial.printf("%02x  ", addr);
        }
 
        if (addresses_found[addr] == 1)
            Serial.printf("X");
        else
            Serial.printf(".");
        Serial.printf(addr % 16 == 15 ? "\n" : "  ");
    }
}

/**
 *  @brief Display software version information to the serial console.
 *  @note Uses the global string buffer to temporarily hold version
 *        string information.
 */
void display_library_versions(void) {
    char version[6];  // Version string (xx.x) with \0
    char reldate[12]; // Release date string (MM/DD/YYYY) with \0

    // Hardware timer library
    timer_pool.version(version, sizeof(version));
    timer_pool.reldate(reldate, sizeof(reldate));
    Serial.printf("STM32 Hardware Timer library v%s (%s)\n", version, reldate);

    // I2C Bus I/O library
    main_i2c_bus.version(version, sizeof(version));
    main_i2c_bus.reldate(reldate, sizeof(reldate));
    Serial.printf("I2C Bus I/O library v%s (%s)\n", version, reldate);

    // INA219 sensor library
    sensor.version(version, sizeof(version));
    sensor.reldate(reldate, sizeof(reldate));
    Serial.printf("INA219 current/power sensor library v%s (%s)\n", version, reldate);

    // MCP4726 DAC library
    dac.version(version, sizeof(version));
    dac.reldate(reldate, sizeof(reldate));
    Serial.printf("MCP4726 DAC library v%s (%s)\n", version, reldate);

    // Ring buffer library
    rb_charging_current.version(version, sizeof(version));
    rb_charging_current.reldate(reldate, sizeof(reldate));
    Serial.printf("Ring buffer library v%s (%s)\n", version, reldate);
}


/**
 *  @brief Arduino setup function.
 *  @returns Nothing
 */
void setup() {

    // Configure Serial port
    // Default Serial uses UART2 with PA2 (TX) and PA3 (RX)
    Serial.begin(115200);
    
    // Configure I2C bus
    Wire.setSCL(PA11);
    Wire.setSDA(PA12);
    Wire.begin();

    // Greeting messages
    Serial.printf("\n");
    Serial.printf("On-board Battery Charger v%s (%s)\n", OBC_VERSION, OBC_RELDATE);
    display_library_versions();
    Serial.printf("\n");

    // Record system start-up time
    Serial.printf("Starting initialization now\n");
    start_time = millis();

    // Scan I2C buses
    // Use dynamic array to hold results, memory is released below
    Serial.printf("Scanning I2C Wire bus... ");
    bool *addresses_found = new bool[128];
    int number_found = main_i2c_bus.scan(addresses_found, false);
    Serial.printf("Done!\n");

    // Display results
    Serial.printf("Found %u devices on Wire I2C bus \n", number_found);
    Serial.printf("\n");
    Serial.printf("Results of the I2C scan:\n");
    display_i2c_map(addresses_found);
    Serial.printf("\n");

    // Check if the optional OLED I2C display is installed
    // Initialize it if successfully detected on the I2C bus
    Serial.printf("Checking for OLED display on I2C bus ");
    oled_found = addresses_found[ADDRESS_128x32];
    delete [] addresses_found;  // Release memory
    if (oled_found) {
        Serial.printf("- found at address 0x%x\n", ADDRESS_128x32);
        Serial.printf("Initializing OLED display ");
        oled.begin();
        oled.setRotation(1);
        oled.setInternalIref(true);     // Lower brightness
        oled.setContrast(40);           // Approx 16% brightness
        oled.setFont(FONT8X16P);        // 8x16 proportional font
        oled.clear();
        oled.on();
        oled.switchRenderFrame();       // Switch to non-display page
        Serial.printf("- Done\n");
    } else {
        Serial.printf("- NOT found at address 0x%x\n", ADDRESS_128x32);
    };

    //
    // Initialize and test I/O drivers
    //
    Serial.printf("Initializing voltage regulator (off) ");
    digitalWrite(GP_VREG_ENABLE, LOW);
    pinMode(GP_VREG_ENABLE, OUTPUT);
    sensor.init(&main_i2c_bus, INA219B_I2C_ADDRESS); 
    dac.init(&main_i2c_bus, DAC_I2C_ADDRESS);
    vreg.begin(GP_VREG_ENABLE, &sensor, &dac);
    Serial.printf("- Done\n");

    Serial.printf("Initializing RGB LED (off) ");
    rgb_led.begin(GP_LEDR, GP_LEDG, GP_LEDB, LED_BLK);
    Serial.printf("- Done\n"); 

    // Initialize A/D converter support to 12-bit resolution
    analogReadResolution(12);

    // Initialize the alarm pool
    Serial.printf("Initializing the timer pool ");
    timer_pool.setup(TIM3, timer_pool_handler);
    Serial.printf("- Done\n");

    // Initialize the charging cycle handlers
    Serial.printf("Initializing charging cycle handlers ");
    fast_charger.init(FAST_PARMS);
    topping_charger.init(TOP_PARMS);
    trickle_charger.init(TRCKL_PARMS);
    standby_charger.init(STANDBY_PARMS);
    Serial.printf("- Done\n");
    Serial.printf("\n");
    
    // Initial state of charger
    charger_state = CHARGER_STARTUP;

    // Initialize loop timer
    loop_timer = millis();
}

/**
 *  @brief Arduino main loop function.
 *  @returns Nothing
 */
void loop() {
    // Former charging supervisor routine moved to loop() to be consistent
    // with the usual Arduino approach
    if ((millis() - loop_timer) >= LOOP_DELAY) {
        // Run charging supervisor
        loop_timer = millis();

        // Update cached charging current readings
        rb_charging_current.append((uint16_t)(vreg.get_current_average_mA()));

        switch (charger_state) {
            case CHARGER_STARTUP: {
                // Starting-up initialization
                Serial.printf("Entering startup initialization state\n");

                // Check initial battery voltage to determine the appropriate
                // charging cycle. Fast if discharged heavily, topping otherwise.
                voltage_mv_t battery_voltage = battery.get_voltage_mV();
                if (battery_voltage <= BATTERY_DISCHARGED_MV) {
                    Serial.printf("Battery voltage @ %u.%u volts, initiating fast charge\n\n", 
                                   battery_voltage/1000, battery_voltage%1000);
                    charger_state = CHARGER_FAST;
                    fast_charger.start();
                } else {
                    Serial.printf("Battery voltage @ %u.%u volts, initiating topping charge\n\n", 
                                   battery_voltage/1000, battery_voltage%1000);
                    charger_state = CHARGER_TOPPING;
                    topping_charger.start();
                }
                break;
            }

            case CHARGER_FAST: {
                // Fast-charging is in process
                switch (fast_charger.run()) {
                    case CYCLE_STARTUP:
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_DONE:
                        // Charging done, move to topping charge
                        Serial.printf("Fast charging cycle completed\n\n");
                        charger_state = CHARGER_TOPPING;
                        topping_charger.start();
                        break;
                    case CYCLE_TIMEOUT:
                        // Charging timed-out, something's not right
                        Serial.printf("Fast charging cycle timed-out!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    case CYCLE_ERROR:
                        // Hardware error detected
                        Serial.printf("Fast charging cycle aborted by error condition!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    default:
                        Serial.printf("Fast charging cycle returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(fast)
                break;
            }

            case CHARGER_TOPPING: {
                // Topping charging is in process
                switch (topping_charger.run()) {
                    case CYCLE_STARTUP:
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_DONE:
                        // Charge done, move to trickle charge
                        Serial.printf("Topping charging cycle completed\n\n");
                        charger_state = CHARGER_TRICKLE;
                        trickle_charger.start();
                        break;
                    case CYCLE_TIMEOUT:
                        // Charge timed-out, something's not right
                        Serial.printf("Topping charging cycle timed-out!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    case CYCLE_ERROR:
                        // Hardware error detected
                        Serial.printf("Topping charging cycle aborted by error condition!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    default:
                        Serial.printf("Topping charging cycle returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(topping)
                break;
            }

            case CHARGER_TRICKLE: {
                // Trickle charge is in process
                switch (trickle_charger.run()) {
                    case CYCLE_STARTUP:
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_DONE:
                    case CYCLE_TIMEOUT:
                        // Charge done, move to standby mode
                        Serial.printf("Trickle charging cycle completed\n\n");
                        charger_state = CHARGER_STANDBY;
                        standby_charger.start();
                        break;
                    case CYCLE_ERROR:
                        // Hardware error detected
                        Serial.printf("Trickle charging cycle aborted by error condition!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    default:
                        Serial.printf("Trickle charging cycle returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(trickle)
                break;
            }

            case CHARGER_STANDBY: {
                // Currently in standby mode
                switch (standby_charger.run()) {
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_TIMEOUT:
                        // Standby mode is over, time to restart active charging
                        Serial.printf("Exiting standby mode\n\n");
                        // Check current battery voltage to determine the appropriate
                        // charging cycle. Fast if discharged heavily, trickle otherwise.
                        char bv_str[6];  // Temporary buffer for battery voltage
                        milliunits_to_string(battery.get_voltage_average_mV(), 1, bv_str, sizeof(bv_str));
                        if (battery.get_voltage_average_mV() <= BATTERY_DISCHARGED_MV) {
                            Serial.printf("Battery voltage @ %s volts, starting fast charge\n", bv_str);
                            charger_state = CHARGER_FAST;
                            fast_charger.start();
                        } else {
                            Serial.printf("Battery voltage @ %s volts, starting trickle charge\n", bv_str);
                            charger_state = CHARGER_TRICKLE;
                            trickle_charger.start();
                        };
                        break;
                    default:
                        Serial.printf("Standby mode handler returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(standby)
                break;
            }

            case CHARGER_SHUTDOWN:
                break;

            case CHARGER_LOAD_TEST:
                // Battery load test not implemented
                Serial.printf("Battery load test not implemented\n");
                break;

            default:
                // Fatal error - we should never get here!
                Serial.printf("Fatal error: Invalid charger state code '%u'!", charger_state);
                while (1);
        } // switch(charger_state)
    }  // charging supervisor

}  // loop()
