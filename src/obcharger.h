/**
 * @file obcharger.h
 * @brief On-board charger type definitions and constants
 */
#ifndef _OB_CHARGER_H_
#define _OB_CHARGER_H_

#include <Arduino.h>
#include <Wire.h>

//
// Debug and verbose modes
//
const bool DEBUG_MODE = true;                   ///< Enable debug mode (true or false)
const bool VERBOSE_MODE = true;                 ///< Enable verbose mode (true or false)

//
// Software version information (update with new releases)
//
#define OBC_VERSION     "0.5"                   ///< Software revision number (x.x)
#define OBC_RELDATE     "01/24/2025"            ///< Software release date (MM/DD/YYYY)

//
// Application-specific type definitions
//
typedef uint32_t    PinNumber;                  ///< GPIO pin number (Arduino PinName counterpart)

typedef uint32_t    time_ms_t;                  ///< Time in milliseconds

typedef uint32_t    voltage_mv_t;               ///< Voltage in integer format (mV)
typedef uint32_t    voltage_uv_t;               ///< Voltage in integer format (uV)
typedef uint32_t    current_ma_t;               ///< Current in integer format (mA)

//
// A/D converter constants
// STM32 G030 series provides a 12-bit A/D converter
//
const uint32_t AN_READ_BITS = 12;               ///< A/D converter resolution
const uint32_t AN_READ_MAX  = (1 << 12);        ///< A/D converter max value
const voltage_mv_t AN_REF_VOLTAGE = 3300;       ///< A/D converter reference

//
// Application-specific type definitions and constants
//

const time_ms_t LOOP_DELAY = 100;           ///< Loop timer delay

/**
 *  @brief Global charger states
 */
enum charger_state_t {
    CHARGER_STARTUP = 1,                    ///< Startup initialization
    CHARGER_MENU = 2,                       ///< Menu selection (not implemented)
    CHARGER_FAST = 3,                       ///< Fast charge
    CHARGER_TOPPING = 4,                    ///< Topping charge
    CHARGER_TRICKLE = 5,                    ///< Trickle charge
    CHARGER_STANDBY = 6,                    ///< Standby mode
    CHARGER_SHUTDOWN = 7,                   ///< Shutdown (error condition?)
    CHARGER_LOAD_TEST = 8,                  ///< Battery load test (not implemented)
    CHARGER_CONDITION = 9                   ///< Battery conditioning (not implemented)
};

/**
 *  @brief Charging cyle states
 */
enum cycle_state_t {
    CYCLE_INIT = 1,                         ///< Cycle in initialization phase
    CYCLE_STARTUP = 2,                      ///< Cycle running normally in startup phase
    CYCLE_RUNNING = 3,                      ///< Cycle running normally
    CYCLE_DONE = 4,                         ///< Cycle terminated normally
    CYCLE_ERROR = 5,                        ///< Hardware error detected
    CYCLE_TIMEOUT = 6,                      ///< Cycle timed-out without reaching target
};

/**
 *  @brief Display devices for showing status messages
 */
enum display_t {
    DISPLAY_NONE = 0,                       ///< No display
    DISPLAY_CONSOLE = 1,                    ///< Serial console
    DISPLAY_OLED = 2,                       ///< OLED display (optional)
};

//
// Define time intervals in milliseconds
//
const time_ms_t SECOND_MS = 1000;           ///< Second inverval in ms
const time_ms_t MINUTE_MS = 60000;          ///< Minute interval in ms
const time_ms_t HOUR_MS   = 3600000;        ///< Hour interval in ms
const time_ms_t DAY_MS    = 86400000;       ///< Day interval in ms
const time_ms_t WEEK_MS   = 604800000;      ///< Week interval in ms

//
// GPIO pins
//
const PinNumber GP_VREG_ENABLE = PB9;       ///< XL6008 regulator enable (0=off,1=on)

const PinNumber GP_LEDR = PB8;              ///< RBG LED red (0=on, 1=Off)
const PinNumber GP_LEDG = PB7;              ///< RGB LED green (0=on, 1=Off)
const PinNumber GP_LEDB = PB6;              ///< RGB LED blue (0=on, 1=Off)

/**
 *  @brief RGB LED color value structure
 */
typedef struct {
    uint8_t r;                              ///< Red color value
    uint8_t g;                              ///< Green color value
    uint8_t b;                              ///< Blue color value
} rgb_t;

const rgb_t LED_BLK = { 0, 0, 0 };          ///< Black
const rgb_t LED_CYN = { 3, 232, 252 };      ///< Cyan
const rgb_t LED_RED = { 255, 0, 0 };        ///< Red
const rgb_t LED_GRN = { 0, 255, 0 };        ///< Bright green
const rgb_t LED_GRN_DRK = { 0, 64, 10 };    ///< Dark green
const rgb_t LED_BLU = { 0, 0, 255 };        ///< Bright blue
const rgb_t LED_BLU_DRK = { 0, 0, 128 };    ///< Dark blue
const rgb_t LED_PUR = { 248, 3, 252 };      ///< Purple
const rgb_t LED_ORG = { 252, 207, 3 };      ///< Orange
const rgb_t LED_YLW = { 244, 252, 3};       ///< Bright yellow
const rgb_t LED_YLW_DRK = { 73, 76, 1 };    ///< Dark yellow
const rgb_t LED_WHT = { 255, 255, 255 };    ///< White

// RGB LED status flag values
const bool LED_ON = true;                   ///< LED enabled (on)
const bool LED_OFF = false;                 ///< LED disabled (off)

//
// I2C buses and devices
// I2C0 is connected to the MCP4726A0 DAC and the INA219 current sensor
// I2C1 is unused
//
// TwoWire *i2c0 = &Wire;                      ///< I2C0 device
const PinNumber I2C0_SCL_GPIO = PA11;       ///< I2C0 SCL pin
const PinNumber I2C0_SDA_GPIO = PA12;       ///< I2C0 SDA pin
const uint32_t I2C0_BAUDRATE = 100000;      ///< I2C0 default clock rate

//
// INA219x I2C current/voltage sensor
//
const uint8_t  INA219B_I2C_ADDRESS = 0x40;  ///< INA219 sensor I2C address
const uint16_t INA219B_CALIBRATION = 4071;  ///< Sensor calibration value

//
// MCP4726 12-bit I2C DAC
//
const uint8_t  DAC_I2C_ADDRESS = 0x60;      ///< MCP4726A0 DAC I2C address

//
// Battery parameters
//
const PinNumber GP_AN_BATTERY = PA0;        ///< Battery voltage A/D

// Resistor divider ratio for battery voltage scaling
// Vad = Vin * (R_BATT_LO)/(R_BATT_LO+R_BATT_HI)
// Vin = Vad * (R_BATT_LO+R_BATT_HI)/(R_BATT_LO)
const uint32_t R_BATT_LO = 10;              ///< Resistor divider lower value (K)
const uint32_t R_BATT_HI = 39;              ///< Resistor divider upper value (K)

const uint16_t BATTERY_CAPACITY = 5500;     ///< Battery capacity in mA/hours.

//
// Voltage regulator parameters
//
const voltage_mv_t VREG_VOLTAGE_MIN = 5000;  ///< Voltage regulator minimum allowable voltage (mV).
const voltage_mv_t VREG_VOLTAGE_MAX = 16000; ///< Voltage regulator maximum allowable voltage (mV).

#define RB_CHARGING_CURRENT_SAMPLES     10  ///< Charging current samples to keep in ring buffer

/** 
 *  @brief Threshold voltage (mV) used to determine initial charge state.  Charger will
 *  jump to fast charging if below the threshold, or topping charging if at or above
 *  this voltage.
 */
const voltage_mv_t BATTERY_DISCHARGED_MV = 13000;

/**
 *  @brief Voltage hystersis limits (mV) for charging cycles.
 *  Used to adjust upper/lower limits to reduce dithering in the output
 *  and smooth out voltage adjustments.
 */
const voltage_mv_t VOLTS_HYSTERESIS = 100;

#endif