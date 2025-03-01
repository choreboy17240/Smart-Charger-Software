/**
 *  @file topping.h
 *  @brief Topping charging cycle handler for SLA batteries
 * 
 *  @details
 *  Called by the exec supervisor to manage the topping charging cycle from
 *  start to finish.  The topping charging algorithm used to charge the 
 *  battery applies constant VOLTS_TARGET voltage level to the battery until
 *  the charging current drops below the CURRENT_TARGET level.
 * 
 *  Charging relies on a global voltage regulator object 'vreg' that
 *  is declared in globals.h and defined in globals.cpp.
 * 
 *  Start-up time is specified to allow the battery voltage and current
 *  to stabilize to consistent levels, preventing premature exit from
 *  the charging cycle.
 * 
 *  A maximum charging time is specified to prevent over-charging a
 *  damaged or defective battery.  If the target current and voltage
 *  are not reached by the end of this timer period, the charging cycle
 *  will stop automatically and the state will be set to the 'TIMEOUT'
 *  value.
 * 
 *  A typical charge cycle would be as follows:
 *  1. Create new Topping_Charger object with appropriate settings
 *  2. Call start method once to begin a charge cycle
 *  3. Call run method periodically (100 ms) intervals
 *  4. Charge cycle continues until current drops below CURRENT_TARGET, or 
 *    until an error condition or timeout is detected
 */
#ifndef _TOPPING_CHARGE_H_
#define _TOPPING_CHARGE_H_

#include "obcharger.h"
#include "cycle.h"
#include "regulator.h"
#include "battery.h"
#include "utility.h"
#include <stm32_time.h>

/**
 * @brief Topping charging cycle handler for SLA batteries
 * 
 * @details Derived from the `Charge_Cycle` base object, with the `run()`
 * method overriden to support a topping charging algorithm for SLA
 * (sealed lead-acid) batteries.
 * 
 * The algorithm implemented by this handler seeks to charge the battery
 * using a constant voltage (typically 2.30-2.35 volts/cell at 25 degrees
 * C for maximum service), until the charging current drops below a target
 * level (typically around 5% of the battery capacity in mA-hours).
 * 
 * The algorithm limits charging current to the maximum level specified
 * by the user.  This limit will typically be driven by either (1) the maximum 
 * safe charging current for the battery, or (2) the maximum current that
 * can be provided by the voltage regulator.
 * 
 * To avoid over-stressing the battery and voltage regulator circuit,
 * the cycle will start with the regulator voltage set to slightly below
 * the current measured battery voltage. The charging voltage is ramped-up
 * at a rate determined by the step voltage specified by the user to align
 * the charging current between the target and maximum current levels.
 * 
 * These parameters are configurable and are set when the handler
 * is initialized using the `init()` method.  See the documentation
 * for the `charge_parm_t` structure for details on the configuration
 * parameters for this handler.
 * 
 * See the documentation for the `Charge_Cycle base` class and the README
 * file for more general information on the charging cycle handle 
 * infrastructure.
 */
class Topping_Charger : public Charge_Cycle {

public:

    /// @brief Default constructor
    Topping_Charger();

    /// @brief Constructor with initialization
    /// @note See the init() member function for details
    Topping_Charger(charge_parm_t &p);

    /// @brief Destructor (best practice)
    ~Topping_Charger();

    /// @brief Run-time handler to manage charging cycle
    /// @returns Charging state
    cycle_state_t run(void);

private:

};

#endif