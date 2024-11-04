/**
 * @file topping.cpp
 *
 * @brief Topping charging handler
 */
#include "topping.h"

//
// Global variables
//
extern Alarm_Pool timer_pool;               // Hardware timers
extern Vreg vreg;                           // Voltage regulator
extern Battery battery;                     // Battery

// Default constructor
Topping_Charger::Topping_Charger() : Charge_Cycle() {
}

// Destructor (best practice)
Topping_Charger::~Topping_Charger() {};

// Constructor with initialization
Topping_Charger::Topping_Charger(charge_parm_t &p) : Charge_Cycle(p) {
    init(p);
}

//  Run-time handler to manage charging cycle
cycle_state_t Topping_Charger::run() {
    // Are we still in startup state?
    if (startup_time_remaining())
        state_code = CYCLE_STARTUP;
    else 
        state_code = CYCLE_RUNNING;

    // Has charging cycle timed-out?
    if (!charging_time_remaining()) {
        // Yes, terminate charging cycle
        stop();
        Serial.printf("Topping charge cycle timed out!\n");
        state_code = CYCLE_TIMEOUT;
        return state_code;
    }

    // Get voltage and current readings
    current_ma_t charging_current = vreg.get_current_mA();
    voltage_mv_t battery_voltage = battery.get_voltage_mV();

    // Has target been reached?
    if ((state_code != CYCLE_STARTUP) && (charging_current <= target_current)) {
        // Yes, turn regulator off and return
        stop();
        state_code = CYCLE_DONE;
        return state_code;
    }

    // Target voltage not reached, check and adjust voltage as needed
    if (charging_current > max_current) {
        // Avoid excess current
        set_voltage -= step_voltage;
        vreg.set_voltage(set_voltage);
    } else {
        // Normal regulation to keep voltage stable
        if (battery_voltage > target_voltage + VOLTS_HYSTERESIS) {
            set_voltage -= step_voltage;
            vreg.set_voltage(set_voltage);
        } else {
            if (battery_voltage < target_voltage - VOLTS_HYSTERESIS) {
                set_voltage += step_voltage;
                vreg.set_voltage(set_voltage);
            }
        }
    }

    // Update RGB LED status as needed
    status_led();

    // Update console and any attached displays periodically
    if (millis() - message_timer >= message_period) {
        // Time to update console and any attached displays
        message_timer = millis();
        status_message();
    }

    // Normal exit
    return state_code;
}


