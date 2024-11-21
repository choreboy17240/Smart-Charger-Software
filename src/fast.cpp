/**
 * @file fast.cpp
 * @brief Fast charging cycle handler for SLA batteries
 */
#include "fast.h"

//
// Global variables
//
extern Alarm_Pool timer_pool;               // Hardware timers
extern Vreg vreg;                           // Voltage regulator
extern Battery battery;                     // Battery

// Default constructor
Fast_Charger::Fast_Charger() : Charge_Cycle() {
}

// Constructor with initialization
Fast_Charger::Fast_Charger(charge_parm_t &p) : Charge_Cycle(p) {
    init(p);
}

// Destructor (best practice)
Fast_Charger::~Fast_Charger() {};

// Run-time handler to manage charge cycle
cycle_state_t Fast_Charger::run() {
    // Are we still in startup state?
    if (startup_time_remaining())
        state_code = CYCLE_STARTUP;
    else 
        state_code = CYCLE_RUNNING;

    // Has charging cycle timed-out?
    if (!charging_time_remaining()) {
        // Yes, terminate charging cycle
        stop();
        Serial.printf("Fast charge cycle timed out!\n");
        state_code = CYCLE_TIMEOUT;
        return state_code;
    }

    // Get voltage and current readings
    current_ma_t charging_current = vreg.get_current_mA();
    voltage_mv_t battery_voltage = battery.get_voltage_mV();

    // Check for excessive set voltage level
    if (set_voltage > VREG_VOLTAGE_MAX) {
        Serial.printf("Error: Set voltage level at %u millivolts\n", set_voltage);
        set_voltage = VREG_VOLTAGE_MAX;
        Serial.printf("Cutting set voltage back to %u millivolts now!\n", set_voltage);
        vreg.set_voltage(set_voltage);
    }

    // Fast charging cycle is complete if:
    // (1) the target voltage has been reached, and
    // (2) we've passed the startup delay period
    // The startup delay prevents premature completion due to surface charge
    // present on the battery when the cycle starts.
    if ((state_code != CYCLE_STARTUP) && (battery_voltage >= target_voltage)) {
        // Yes, turn regulator off and return
        stop();
        state_code = CYCLE_DONE;
        return state_code;
    }

    // Target voltage not reached, check and adjust voltage as needed
    // Don't allow the battery voltage to exceed the target voltage, even
    // if we're in the startup period.
    if (charging_current > max_current) {
        // Charging current is too high, drop voltage
        set_voltage -= step_voltage;
    } else {
        // Charging current below max, keep it above the target current
        if (charging_current < target_current) {
            // Charging current below the target, tweak voltage up
            // as long as we're below the target voltage
            if (battery_voltage < target_voltage) {
                // Voltage is ok, tweak it up
                set_voltage += step_voltage;
            } else {
                // Voltage is above target, tweak it back
                set_voltage -= step_voltage;
            }
        }
    }

    // Ensure set voltage is below the maximum limit
    if (set_voltage > VREG_VOLTAGE_MAX) {
        set_voltage = VREG_VOLTAGE_MAX;
    }

    // Set the regulator voltage
    vreg.set_voltage(set_voltage);

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


