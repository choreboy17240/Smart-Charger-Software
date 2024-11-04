/**
 * @file fast.h
 * 
 * @mainpage Fast charging handler
 * Called by the exec supervisor to manage the fast charging cycle from
 * start to finish. The fast charging algorithm used to charge the battery
 * uses constant CURRENT_TARGET current level until VOLTS_TARGET battery
 * voltage is achieved.
 *
 * Charging relies on a global voltage regulator object 'vreg' that
 * is declared in globals.h and defined in globals.cpp.
 *
 * Start-up time is specified to allow the battery voltage and current
 * to stabilize to consistent levels, preventing premature exit from
 * the fast charging cycle.
 *
 * A maximum charging time is specified to prevent over-charging a
 * damaged or defective battery.  If the target current and voltage
 * are not reached by the end of this timer period, the charging cycle
 * will stop automatically and the state will be set to the 'TIMEOUT'
 * value.
 *
 * A typical charge cycle would be as follows:
 * 1. Create new Fast_Charger object with appropriate settings
 * 2. Call start method once to begin a charge cycle
 * 3. Call run method periodically (100 ms) intervals
 * 4. Charge cycle continues until VOLTS_TARGET is reached, or until an
 *   error condition or timeout is detected
 * 
 * Hardware timer resources:
 * 1. Charging timer (charge_timer_id)
 *    Counts down from the specified timeout_ms time interval, which
 *    is the maximum amount of time allowed for a fast charge to
 *    to successfully complete.    
 * 
 */
#ifndef _FAST_CHARGE_H_
#define _FAST_CHARGE_H_

#include "cycle.h"
#include "obcharger.h"
#include "regulator.h"
#include "battery.h"
#include "utility.h"
#include <stm32_time.h>

/**
 * @brief Fast charge cycle class
 * 
 * @details Derived from the Charge_Cycle base object, with the following
 * methods overriden to support application-specific functionality:
 * 
 * * run()
 * * status_message()
 * 
 */
class Fast_Charger : public Charge_Cycle {

public:

    /// @brief Default constructor
    Fast_Charger();

    /// @brief Constructor with initialization
    /// @param p: Charging parameters structure
    Fast_Charger(charge_parm_t &p);

    /// @brief Destructor (best practice)
    ~Fast_Charger();

    /// @brief Run-time handler to manage charging cycle
    /// @returns Charging state
    cycle_state_t run(void);

private:

};

#endif