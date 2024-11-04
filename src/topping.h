/// @file topping.h
/// @mainpage Topping charging handler
/// Called by the exec supervisor to manage the topping charging cycle from
/// start to finish.  The topping charging algorithm used to charge the 
/// battery applies constant VOLTS_TARGET voltage level to the battery until
// the charging current drops below the CURRENT_TARGET level.
///
/// Charging relies on a global voltage regulator object 'vreg' that
/// is declared in globals.h and defined in globals.cpp.
///
/// Start-up time is specified to allow the battery voltage and current
/// to stabilize to consistent levels, preventing premature exit from
/// the charging cycle.
///
/// A maximum charging time is specified to prevent over-charging a
/// damaged or defective battery.  If the target current and voltage
/// are not reached by the end of this timer period, the charging cycle
/// will stop automatically and the state will be set to the 'TIMEOUT'
/// value.
///
/// A typical charge cycle would be as follows:
/// 1. Create new Topping_Charger object with appropriate settings
/// 2. Call start method once to begin a charge cycle
/// 3. Call run method periodically (100 ms) intervals
/// 4. Charge cycle continues until current drops below CURRENT_TARGET, or 
///   until an error condition or timeout is detected
//
#ifndef _TOPPING_CHARGE_H_
#define _TOPPING_CHARGE_H_

#include "obcharger.h"
#include "cycle.h"
#include "regulator.h"
#include "battery.h"
#include "utility.h"
#include <stm32_time.h>

/**
 * @brief Topping charge cycle class
 * 
 * @details Derived from the Charge_Cycle base object, with the following
 * methods overriden to support application-specific functionality:
 * 
 * * run()
 * * status_message()
 * 
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