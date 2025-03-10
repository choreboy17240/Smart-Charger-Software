/**
 * @file topping.cpp
 * @brief Topping charging cycle handler for SLA batteries
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 */
#include "topping.h"

//
// Global variables
//
extern Alarm_Pool timer_pool;               // Hardware timers
extern Vreg vreg;                           // Voltage regulator
extern Battery battery;                     // Battery
extern bool oled_found;                     // OLED display found at startup in main()?

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
        vreg.set_voltage_mV(set_voltage);
    } else {
        // Normal regulation to keep voltage stable
        if (battery_voltage > target_voltage + VOLTS_HYSTERESIS) {
            set_voltage -= step_voltage;
            vreg.set_voltage_mV(set_voltage);
        } else {
            if (battery_voltage < target_voltage - VOLTS_HYSTERESIS) {
                set_voltage += step_voltage;
                vreg.set_voltage_mV(set_voltage);
            }
        }
    }

    // Update RGB LED status as needed
    status_led();

    // Update any attached OLED displays
    if (millis() - display_timer >= display_period) {
        display_timer = millis();
        if (oled_found) {
            status_message(DISPLAY_OLED);
        }
    }

    // Update serial console
    if (millis() - message_timer >= message_period) {
        message_timer = millis();
        status_message(DISPLAY_CONSOLE);
    }

    // Normal exit
    return state_code;
}


