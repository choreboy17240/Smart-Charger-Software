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

/**
 *  @brief Charging parameters structure used to initialize `Charge_Cycle` objects
 *         and it's derived classes.
 */
struct charge_parm_t {
    current_ma_t current_target;            ///< Target charging current
    current_ma_t current_max;               ///< Maximum charging current
    voltage_mv_t voltage_target;            ///< Target battery voltage
    voltage_mv_t voltage_step;              ///< Step size for adjusting voltage
    time_ms_t charge_period_max;            ///< Maximum allowable charging time
    time_ms_t startup_period;               ///< Startup time period
    time_ms_t idle_period;                  ///< Idle time between charges (storage cycle only)
    time_ms_t led_on_period;                ///< Status LED on time while charging
    time_ms_t led_off_period;               ///< Status LED off time while charging
    rgb_t led_color;                        ///< Status LED color while charging
    const char *title_str;                  ///< Charge cycle title (6 chars) for LCD display (e.g. "FAST  ")
    const char *name_str;                   ///< Charge cycle name for serial output (e.g. 'fast')
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
 * Fast charging is limited to a maximum of FAST_TIMEOUT_MS to protect against
 * bad battery that won't take a charge.  The FAST_STARTUP_MS time provides
 * a delay to allow any surface charge voltage to dissipate before making
 * the decision to end fast charging.
 */
const charge_parm_t FAST_PARMS = { 
    .current_target = BATTERY_CAPACITY/7,   // @14% capacity
    .current_max = 1000,                    // 1 amp limit for power supply
    .voltage_target = 14400,
    .voltage_step = 10,
    .charge_period_max = 4*HOUR_MS,
    .startup_period = 60*SECOND_MS,
    .idle_period = 0,                       // Ignored for fast charging
    .led_on_period = 250,
    .led_off_period = 750,
    .led_color = LED_BLU_DRK,
    .title_str = "FAST  ",
    .name_str = "Fast",
};

/**
 * @brief Topping charge parameters
 * 
 * @details Constant voltage charge until current drops below 5% of battery capacity
 *          Recommended range of 2.30V to 2.35V/cell for maximum service life.
 */
const charge_parm_t TOP_PARMS = { 
    .current_target = BATTERY_CAPACITY/20,  // @5% capacity
    .current_max = 1000,                    // 1 amp limit for power supply
    .voltage_target = 14000,                // 14.0V => 2.33V/cell
    .voltage_step = 10,
    .charge_period_max = 12*HOUR_MS,
    .startup_period = 120*SECOND_MS,
    .idle_period = 0,                       // Ignored for topping charging
    .led_on_period = 250,
    .led_off_period = 1000,
    .led_color = LED_YLW_DRK,
    .title_str = "TOPPNG",
    .name_str = "Topping",
};

/**
 * @brief Trickle or float charge parameters
 * 
 * @details Constant voltage to maintain state of charge
 *          Recommended range of 2.25V to 2.27V/cell at 25 degrees C.
 */
const charge_parm_t TRCKL_PARMS = { 
    .current_target = 0,                    // Not applicable for trickle charging
    .current_max = 1000,                    // 1 amp limit for power supply
    .voltage_target = 13500,
    .voltage_step = 10,
    .charge_period_max = 24*HOUR_MS,
    .startup_period = 60*SECOND_MS,         // Ignored for trickle charging
    .idle_period = 0,                       // Ignored for trickle charging
    .led_on_period = 250,
    .led_off_period = 2750,
    .led_color = LED_GRN_DRK,
    .title_str = "TRCKLE",
    .name_str = "Trickle",
};

/**
 * @brief Storage charge parameters
 * 
 * @details Run weekly to maintain state of charge
 *          Constant voltage to maintain state of charge but avoid long-term damage
 */
const charge_parm_t STRG_PARMS = { 
    .current_target = 0,                    // Not applicable for storage charging
    .current_max = 1000,                    // 1 amp limit for power supply
    .voltage_target = 13500,
    .voltage_step = 100,
    .charge_period_max = 24*HOUR_MS,
    .startup_period = 60*SECOND_MS,           // Ignored for storage charging
    .idle_period = 6*DAY_MS,                  // Weekly
    .led_on_period = 100,
    .led_off_period = 1000,
    .led_color = LED_GRN_DRK,
    .title_str = "STORAG",
    .name_str = "Storage",
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

    // Time period settings
    time_ms_t message_period=SECOND_MS;     ///< Status message update time period (ms).
    time_ms_t charge_period_max;            ///< Maximum charge time period (ms).
    time_ms_t idle_period;                  ///< Idle time between charges for the storage cycle (ms).
    time_ms_t startup_period;               ///< Startup time period (ms) to allow parameters to stabilize.

    // LED settings
    time_ms_t led_off_period;               ///< RGB LED blink off time (ms).
    time_ms_t led_on_period;                ///< RGB LED blink on time (ms).
    rgb_t led_color;                        ///< RGB LED color.

    // Variables
    cycle_state_t state_code;               ///< Current charging cycle state.
    voltage_mv_t set_voltage;               ///< Current voltage regulator set voltage (mV).
    time_ms_t start_time;                   ///< millis() time at the start of the charging cycle.

    // Software timers
    time_ms_t message_timer;                ///< Message update timer (uses message_period)
    time_ms_t led_timer;                    ///< RGB LED update timer
    bool led_state;                         ///< RGB LED state (true=on, false=off)

    // Hardware alarm timers
    alarm_id_t charge_timer_id;             ///< Hardware charging timer ID provided by the `Alarm_Pool`.

    // Status message buffers
    char hms_str[9];                        ///< Buffer for 'HH:MM:SS' time string.
    char bv_str[5];                         ///< Buffer for 'XX.X' battery voltage string.
    char ov_str[5];                         ///< Buffer for 'XX.X' output voltage string

    // Status message strings
    const char *title_str;                  ///< Charge cycle title for LCD display messages (6 characters).
    const char *name_str;                   ///< Charge cycle name for serial console messages.


    /** 
     *  @brief Update the status of the RGB LED, based on the color and
     *         timing criteria specified for the current cycle.
     *  @returns Nothing
     */
    void status_led(void);

    /**
     *  @brief Write status information for the current cycle to the serial
     *         console and any attached displays.
     *  @returns Nothing
     */
    void status_message(void);
};

#endif