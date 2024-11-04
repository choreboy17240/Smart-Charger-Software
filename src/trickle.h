/**
 * @file trickle.h
 * 
 * @mainpage Trickle charging handler
 *  Called by the exec supervisor to manage the trickle charging cycle from
 *  start to finish.  The trickle charging algorithm used to charge the battery
 *  applies constant VOLTS_TARGET voltage level to the battery indefinitely.
 * 
 *  Charging relies on a global voltage regulator object 'vreg' that
 *  is declared in globals.h and defined in globals.cpp.
 * 
 *  Unlike the more aggressive charging cycles, the start-up time doesn't
 *  play a factor in the trickle charging algorithm.
 * 
 *  A maximum charging time is specified to prevent over-charging the
 *  battery. A TIMEOUT condition is a normal exit in this case.
 *
 * Typical charge cycle would be as follows:
 * 1. Create new Trickle_Charger object with appropriate settings
 * 2. Call start method once to begin a charge cycle
 * 3. Call run method periodically (100 ms) intervals
 * 4. Charge cycle continues indefinitely unless a TIMEOUT period is
 *   specified. Exits only if a timeout or error condition is
 *   detected
 * 
 * Hardware timer resources:
 * 1. Charging timer (charge_timer_id)
 *    Counts down from the specified timeout_ms time interval, which
 *    is the maximum amount of time allowed for a fast charge to
 *    to successfully complete.
 *    
 */
#ifndef _TRICKLE_CHARGE_H_
#define _TRICKLE_CHARGE_H_

#include "obcharger.h"
#include "cycle.h"
#include "regulator.h"
#include "battery.h"
#include "utility.h"
#include <stm32_time.h>

/**
 * @brief Trickle charge cycle class
 * 
 * @details Derived from the Charge_Cycle base object, with the following
 * methods overriden to support application-specific functionality:
 * 
 * * run()
 * * status_message()
 * 
 */
class Trickle_Charger : public Charge_Cycle {

public:

    /// @brief Default constructor
    Trickle_Charger();

    /// @brief Constructor with initialization
    /// @param p: Charging parameters structure
    Trickle_Charger(charge_parm_t &p);

    /// @brief Destructor (best practice)
    ~Trickle_Charger();

    /// @brief Run-time handler to manage charging cycle
    /// @returns Charging state
    cycle_state_t run(void);
 
private:

};

#endif