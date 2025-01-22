/**
 * @file standby.h
 * @brief Standby mode charging cycle handler for SLA batteries
 * 
 * @details
 * Called by the exec supervisor to maintain a standby mode charging cycle
 * from start to finish.  In the standby mode, the voltage regulator is
 * turned-off, and the handler simply:
 * - Maintains the count-down until active charging should be resumed,
 * - Updates the RGB LED to indicate the current charging status,
 * - Selects the appropriate charging cycle to be run once it's time
 *   for active charging to resume.
 * 
 * The global `Vreg` voltage regulator instance is used to control the
 * state of the hardware voltage regulator.
 * 
 * Settings for the standby mode are configured using the `STANDBY_PARMS`
 * structure in the `cycle.h` file.  Unlike active charging cycles, 
 * the voltage regulator is turned-off, so voltage and current thresholds
 * don't apply.
 * 
 * The `TIMEOUT` condition is the normal exit for this charge cycle.
 *
 * Typical standby cycle would be as follows:
 * 1. Create new `Standby` instance with appropriate settings
 * 2. Call the `start()` method once to begin a charge cycle
 * 3. Call `run()` method periodically (100 ms) intervals
 * 4. Standby cycle continues until the `TIMEOUT` condition is detected.
 * 
 * Hardware timer resources:
 * 1. Charging timer (`charge_timer_id`)
 *    Counts down from the specified timeout_ms time interval, which
 *    is the maximum amount of time allowed for a fast charge to
 *    to successfully complete.
 */
#ifndef _STANDBY_CHARGE_H_
#define _STANDBY_CHARGE_H_

#include "obcharger.h"
#include "cycle.h"
#include "regulator.h"
#include "battery.h"
#include "utility.h"
#include <stm32_time.h>

// OLED display support
#include <STM32_4kOLED.h>

/**
 * @brief Standby charging cycle handler for SLA batteries
 * 
 * @details Derived from the Charge_Cycle base object, with the run()
 * method overriden to support a trickle charging algorithm for SLA
 * (sealed lead-acid) batteries.
 * 
 * The algorithm implemented by this handler seeks to charge the battery
 * using a constant voltage (typically 2.25-2.27 volts/cell at 25 degrees C)
 * for an indefinite time period to maintain the battery's state of charge 
 * without degrading battery life.
 * 
 * Charging current is normally limited due to the modest voltage
 * applied to the battery, but will also be limited by the algorithm to the
 * maximum level specified by the user.  This limit will typically be driven
 * by either (1) the maximum safe charging current for the battery, or 
 * (2) the maximum current that can be provided by the voltage regulator.
 * 
 * To avoid over-stressing the battery and voltage regulator circuit,
 * the cycle will start with the regulator voltage set to slightly below
 * the current measured battery voltage. The charging voltage is ramped-up
 * at a rate determined by the step voltage specified by the user to align
 * the charging current between the target and maximum current levels.
 * 
 * These parameters are configurable and are set when the handler
 * is initialized using the init() method.  See the documentation
 * for the charge_parm_t structure for details on the configuration
 * parameters for this handler (and others derived from the
 * Charge_Cycle base class).
 * 
 * See the documentation for the Charge_Cycle base class for more
 * general information on the charging cycle handle infrastructure.
 * 
 */
class Standby_Charger : public Charge_Cycle {

public:

    /**
     * @brief Default constructor
     */
    Standby_Charger();

    /**
     * @brief Constructor with initialization
     * @param p: Charging parameters structure
     */
    Standby_Charger(charge_parm_t &p);

    /**
     *  @brief Destructor (best practice)
     */
    ~Standby_Charger();

    /**
     * @brief Run-time handler to manage charging cycle
     * @returns Charging state
     */
    cycle_state_t run(void);
 
    /**
     *  @brief Write status information for standby mode to the serial
     *         console and any attached displays. Overrides the 
     *         `status_message()` in the base class.
     *  @returns Nothing
     */
    void status_message(void);

private:

};

#endif