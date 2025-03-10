/**
 *  @file trickle.h
 *  @brief Trickle charging cycle handler for SLA batteries
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 * 
 *  @details
 *  Called by the exec supervisor to manage the trickle charging cycle from
 *  start to finish.  The trickle charging algorithm used to charge the battery
 *  applies a constant `VOLTS_TARGET` voltage level to the battery indefinitely.
 * 
 *  Charging relies on a global `Vreg` voltage regulator object that
 *  is declared in `globals.h` and defined in `globals.cpp`.
 * 
 *  Unlike the more aggressive charging cycles, the start-up time doesn't
 *  play a factor in the trickle charging algorithm.
 * 
 *  A maximum charging time is specified to prevent over-charging the
 *  battery. A `TIMEOUT` condition is a normal exit in this case.
 *
 * These parameters are configurable and are set when the handler
 * is initialized using the `init()` method.  See the documentation
 * for the `charge_parm_t` structure for details on the configuration
 * parameters for this handler.
 * 
 * See the documentation for the `Charge_Cycle` base class and the 
 * README file for more general information on the charging cycle handle
 * infrastructure.
 * 
 * Hardware timer resources:
 * 1. Charging timer (charge_timer_id)
 *    Counts down from the specified timeout_ms time interval, which
 *    is the maximum amount of time allowed for a fast charge to
 *    to successfully complete.
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
 * @brief Trickle charging cycle handler for SLA batteries
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
 * These parameters are configurable and are set when the handler is 
 * initialized using the init() method.  See the documentation for the 
 * `charge_parm_t` structure for details on the configuration parameters for
 * this handler (and others derived from the `Charge_Cycle` base class).
 * 
 * As noted below, there is no charging "goal" when trickle charging the
 * battery. The normal exit will occur in the form of a "timeout" condition
 * (i.e. returning `CYCLE_TIMEOUT` state) rather than a "completed" condition
 * (i.e. returning `CYCLE_DONE` state).
 * 
 * See the documentation for the `Charge_Cycle` base class for more
 * general information on the charging cycle handle infrastructure.
 */
class Trickle_Charger : public Charge_Cycle {

public:

    /// @brief Default constructor
    Trickle_Charger();

    /**
     * @brief Constructor with initialization
     * @param p: Charging parameters structure
     */
    Trickle_Charger(charge_parm_t &p);

    /// @brief Destructor (best practice)
    ~Trickle_Charger();

    /**
     * @brief Run-time handler to manage charging cycle
     * @returns Charging state
     * @note
     * Unlike other charging cycles, there is no charging goal when trickle
     * charging the battery. The normal exit will occur in the form of a
     * "timeout" condition (i.e. returning `CYCLE_TIMEOUT` state) rather than
     * a "completed" condition (i.e. returning `CYCLE_DONE` state).
     */
    cycle_state_t run(void);
 
private:

};

#endif