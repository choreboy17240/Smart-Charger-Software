/**
 * @file cycle.h
 * @brief Base class for creating custom battery charging cycles handlers
 * 
 * @details 
 * Provides a framework for defining customized battery charging
 * cycles, providing a consistent base for code re-use.  This object
 * will be subclassed by the parent charging class, which will apply
 * it's unique algorithm to the run() method to perform the desired
 * charging operation.
 *
 * Charging relies on a global voltage regulator object 'vreg' that
 * is declared in globals.h and defined in globals.cpp.
 *
 * Start-up time is specified to allow the battery voltage and current
 * to stabilize to consistent levels, preventing premature exit from
 * the fast charging cycle.
 *
 * A maximum charging time is specified to prevent over-charging a
 * damaged or defective battery.  If the desired charging state is
 * not reached by the end of this timer period, the charging cycle
 * will stop automatically and the state will be set to the 'TIMEOUT'
 * value.
 *
 * A typical charge cycle would be as follows:
 * 1. Create new charging object with appropriate settings
 * 2. Call start method once to begin a charge cycle
 * 3. Call run method periodically (100 ms) intervals
 * 4. Charge cycle continues until VOLTS_TARGET is reached, or until an
 *    error condition or timeout is detected
 * 
 * Hardware timer resources:
 * 1. Charging timer (charge_timer_id)
 *    Counts down from the specified timeout_ms time interval, which
 *    is the maximum amount of time allowed for a fast charge to
 *    to successfully complete.
 */
#ifndef _CYCLE_H_
#define _CYCLE_H_

#include "obcharger.h"
#include "regulator.h"
#include "battery.h"
#include "rgbled.h"
#include "utility.h"
#include <stm32_time.h>

// OLED display support
#include <STM32_4kOLED.h>

// Ring buffer
#include <ringbuffer.h>

/**
 *  @brief Charging parameters structure used to initialize `Charge_Cycle` objects
 *         and it's derived classes.
 */
struct charge_parm_t {
    current_ma_t current_target;            ///< Target charging current
    current_ma_t current_max;               ///< Maximum charging current
    voltage_mv_t voltage_target;            ///< Target battery voltage
    voltage_mv_t voltage_step;              ///< Step size for adjusting voltage
    time_ms_t charge_period_max;            ///< Maximum allowable cycle time
    time_ms_t startup_period;               ///< Startup time period
    time_ms_t led_on_period;                ///< Status LED on time while charging
    time_ms_t led_off_period;               ///< Status LED off time while charging
    rgb_t led_color;                        ///< Status LED color while charging
    const char *title_str;                  ///< Charge cycle title (6 chars) for LCD display (e.g. "FAST  ")
    const char *name_str;                   ///< Charge cycle name for serial output (e.g. 'fast')
    time_ms_t display_period;               ///< Time between OLED display updates
    time_ms_t message_period;               ///< Time between serial console message updates
};

/**
 * @brief Fast charging parameters
 *
 * @details Fast charging charges the battery at a constant current rate of up
 * to 25% of battery capacity until the battery voltage reaches the targeted 
 * level.  To protect battery life, suggest using conservative limits with 
 * the target charging current at 17% of battery capacity and maximum charging
 * current at 20% of battery capacity.
 * 
 * Fast charging is limited to a maximum of `FAST_TIMEOUT_MS` to protect against
 * bad battery that won't take a charge.  The `FAST_STARTUP_MS` time provides
 * a delay to allow any surface charge voltage to dissipate before making
 * the decision to end fast charging.
 */
const charge_parm_t FAST_PARMS = { 
    .current_target = BATTERY_CAPACITY/7,   // @14% capacity
    .current_max = 600,                     // 600 mA due to regulator temp rise
    .voltage_target = 14400,
    .voltage_step = 10,
    .charge_period_max = 4*HOUR_MS,
    .startup_period = 60*SECOND_MS,
    .led_on_period = 250,
    .led_off_period = 750,
    .led_color = LED_BLU_DRK,
    .title_str = "FAST  ",
    .name_str = "Fast",
    .display_period = 1000,
    .message_period = 1000,
};

/**
 * @brief Topping charge parameters
 * 
 * @details
 * Sets the voltage regulator to a constant voltage and maintains it until the
 * charging current drops below 5% of battery capacity.  The recommended voltage
 * range is 2.30V to 2.35V/cell for maximum service life.
 */
const charge_parm_t TOP_PARMS = { 
    .current_target = BATTERY_CAPACITY/20,  // @5% capacity
    .current_max = 600,                     // 600 mA due to regulator temp rise
    .voltage_target = 14000,                // 14.0V => 2.33V/cell
    .voltage_step = 10,
    .charge_period_max = 8*HOUR_MS,
    .startup_period = 120*SECOND_MS,
    .led_on_period = 250,
    .led_off_period = 1000,
    .led_color = LED_YLW_DRK,
    .title_str = "TOPPNG",
    .name_str = "Topping",
    .display_period = 1000,
    .message_period = 1000,
};

/**
 * @brief Trickle or float charge parameters
 * 
 * @details
 * Set the voltage regulator to a constant voltage to maintain the battery's
 * state of charge.  The recommended voltage range is 2.25V to 2.27V/cell at 25 
 * degrees C.
 */
const charge_parm_t TRCKL_PARMS = { 
    .current_target = 0,                    // Not applicable for trickle charging
    .current_max = 600,                     // 600 mA due to regulator temp rise
    .voltage_target = 13500,
    .voltage_step = 10,
    .charge_period_max = 8*HOUR_MS,
    .startup_period = 0,                    // Ignored for trickle charging
    .led_on_period = 250,
    .led_off_period = 2750,
    .led_color = LED_GRN_DRK,
    .title_str = "TRCKLE",
    .name_str = "Trickle",
    .display_period = 1000,
    .message_period = 60000,
};

/**
 * @brief Standby mode parameters
 * 
 * @details
 * The voltage regulator is turned-off and we're waiting until the end of the
 * standby cycle to resume active charging.
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 */
const charge_parm_t STANDBY_PARMS = { 
    .current_target = 0,                        // Regulator turned-off
    .current_max = 0,
    .voltage_target = 0,                        
    .voltage_step = 0,
    .charge_period_max = WEEK_MS,
    .startup_period = 0,                        // Ignored in standby mode
    .led_on_period = 250,                       // Short green pulse every minute
    .led_off_period = 59750,
    .led_color = LED_GRN_DRK,
    .title_str = "STNDBY",
    .name_str = "Standby",
    .display_period = 1000,
    .message_period = 60000,
};

/**
 * @brief Base class for deriving charge cycle handler classes
 * 
 * @details Provides a framework for creating customized battery charging
 * handlers that provide a specific portion of the complete battery charging
 * process (e.g. fast charging, trickle charging, etc.).
 * 
 * The derived classes should override the run() method with an
 * application-specific version to execute the desired charging
 * algorithm.  The run() method will be called periodically by the user
 * to allow it to monitor the charging process and make adjustments
 * to tailor it to the battery state and desired outcome for the charging
 * cycle.
 * 
 * The destructor method may also be overridden if there is need
 * to perform shutdown operations.
 */
class Charge_Cycle {

public:
    /**
     *  @brief Default constructor
     */
    Charge_Cycle();

    /**
     *  @brief Constructor with initialization.
     *  @param p: Parameters to configure the charging cycle.
     */
    Charge_Cycle(charge_parm_t &p);

    /** 
     *  @brief Virtual destructor to support inheritance (best practice).
     *  @note Virtual function that may be overridden by custom charging
     *        cycle handlers derived from this class.
     */
    virtual ~Charge_Cycle();

    /**
     *  @brief Initialize charging cycle handler.
     *  @param p: Charging parameters structure
     *  @note Using `init` for the method name rather than `begin` to avoid 
     *      potential confusion with the `start` method used to start a new
     *       charging cycle.
     */
    void init(const charge_parm_t &p);

    /**
     *  @brief Start a new charge cycle.
     *  @returns Nothing
     */
    void start(void);

    /**
     *  @brief Run-time handler called periodically to manage charging cycle
     *  @returns Charging state
     *  @note Virtual function that will be overridden by custom charging
     *        cycle handlers derived from this class.
     */
    virtual cycle_state_t run(void);
 
    /**
     *  @brief Stop the current charging cycle.
     *  @returns Nothing
     */
    void stop(void);

    /**
     *  @brief Gets the current charging state.
     *  @returns Charging state
     */
    cycle_state_t state(void);

    /**
     *  @brief Gets remaining startup time in the current cycle.
     *  @returns Remaining startup time (ms)
     *  @note Startup time is allowed at the beginning of a charging
     *        cycle to allow parameters to stabilize before deciding
     *        if the target criteria for the cycle have been reached.
     */
    time_ms_t startup_time_remaining(void);

    /**
     *  @brief Gets remaining charging time in the current charging cycle.
     *  @returns Remaining charging time (ms)
     */
    time_ms_t charging_time_remaining(void);

    /** 
     *  @brief Gets elapsed charging time in the current charging cycle.
     *  @returns Elapsed charging time (ms)
     *  @note The elapsed time will include any startup time specified
     *        for the beginning of a charge cycle.
     */
    time_ms_t charging_time_elapsed(void);

protected:
    // Charging settings
    voltage_mv_t target_voltage;            ///< Target battery voltage to be achieved (mV).
    voltage_mv_t step_voltage;              ///< Step size used for adjusting regulator voltage (mV).
    current_ma_t target_current;            ///< Target current to be used for charging battery (mA).
    current_ma_t max_current;               ///< Maximum current to be used for charging battery (mA).

    // Hardware alarm timers
    alarm_id_t charge_timer_id;             ///< Hardware charging timer ID provided by the `Alarm_Pool`.

    // Software timers
    time_ms_t display_timer;                ///< Timer for OLED display updates
    time_ms_t message_timer;                ///< Timer for writing console status messages
    time_ms_t led_timer;                    ///< Timer for updating the RGB LED

    // Time period settings
    time_ms_t display_period;               ///< Time period between OLED display updates (ms).
    time_ms_t message_period;               ///< Time period between console status messages (ms).
    time_ms_t charge_period_max;            ///< Maximum time period allowed for the cycle to complete (ms).
    time_ms_t startup_period;               ///< Time period to allow at start of the cycle for things to stabilize (ms).

    // RGB LED settings
    bool led_state;                         ///< RGB LED state (true=on, false=off)
    time_ms_t led_off_period;               ///< RGB LED blink off time (ms).
    time_ms_t led_on_period;                ///< RGB LED blink on time (ms).
    rgb_t led_color;                        ///< RGB LED color.

    // Variables
    cycle_state_t state_code;               ///< Current charging cycle state.
    voltage_mv_t set_voltage;               ///< Current voltage regulator set voltage (mV).
    time_ms_t start_time;                   ///< millis() time at the start of the charging cycle.

    // Status message buffers
    char hms_str[9];                        ///< Buffer for 'HH:MM:SS' time string.
    char bv_str[6];                         ///< Buffer for 'XX.X' battery voltage string.
    char ov_str[6];                         ///< Buffer for 'XX.X' output voltage string

    // Status message strings
    const char *title_str;                  ///< Charge cycle title for LCD display messages (6 characters).
    const char *name_str;                   ///< Charge cycle name for serial console messages.

    // Private functions

    /** 
     *  @brief Update the status of the RGB LED, based on the color and
     *         timing criteria specified for the current cycle.
     *  @returns Nothing
     */
    void status_led(void);

    /**
     *  @brief Write status information for the current charging cycle to the
     *         selected display device
     *  @param device: Target display device for displaying status information
     *  @returns Nothing
     */
    virtual void status_message(display_t device);
};

/**
 * @brief Utility function to calculate powers of 10 using integer math
 * @param exponent Non-negative exponent (e.g., 0 for 10^0, 1 for 10^1, etc.).
 * @return Result of 10^exponent as an unsigned integer.
 * @note
 * Using the pow() function in the standard library uses about 18-20K
 * additional flash memory and also increased RAM usage. This simple utility
 * function avoids that overhead.
 */
uint32_t pow10(uint8_t exponent);

/**
 * @brief Utility function to convert value in milliunits to a rounded decimal string
 * @param milliunits: Value in milliunits (e.g. 12435 mV for 12.435V)
 * @param places: Number of decimal places in the result
 * @param buffer: Buffer to hold the string representation
 * @param buffer_len: Length of the buffer for holding string representation
 * @returns String with rounded decimal representation
 * @note
 * Using the sprintf() functions in the standard library adds signifcant
 * overhead for the floating point library. This simple utility function
 * formats the battery voltage, etc. in decimal units (e.g. 13.1) in string
 * form for display to the console and OLED.  The decimal string will be 
 * rounded-off appropriately to the number of places specified by `places`.
 * This function relies on the `pow10()` function for formatting the 
 * decimal strings.
 */
void milliunits_to_string(uint32_t milliunits, uint8_t places, char *buffer, uint8_t buffer_len);

#endif