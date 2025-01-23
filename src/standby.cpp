/**
 *  @file standby.cpp
 *  @brief Standby mode charging cycle handler for SLA batteries
 */
#include "standby.h"

//
// Global variables
//
extern Alarm_Pool timer_pool;               ///< Hardware timers
extern Vreg vreg;                           ///< Voltage regulator
extern Battery battery;                     ///< Battery
extern SSD1306PrintDevice oled;             ///< OLED display object
extern bool oled_found;                     ///< OLED display found at startup in main()?

// Default constructor
Standby_Charger::Standby_Charger() : Charge_Cycle() {
}

// Constructor with initialization
Standby_Charger::Standby_Charger(charge_parm_t &p) : Charge_Cycle(p) {
    init(p);
}

// Destructor (best practice)
Standby_Charger::~Standby_Charger() {
}

// Run-time handler to manage charge cycle
cycle_state_t Standby_Charger::run() {

    // No startup time
    state_code = CYCLE_RUNNING;

    // Make sure voltage regulator is off
    vreg.off();

    // Has charging cycle timed-out?
    if (!charging_time_remaining()) {
        // Yes, terminate charging cycle
        stop();
        state_code = CYCLE_TIMEOUT;
        return state_code;
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

//
// Write status information for standby mode to console and any attached displays
//
// Console message format:
// <name_str> HH:MM:SS Battery=xx.x V
//
// OLED display message format:
// 0123456789012345
// TTTTTT  HH:MM:SS
// xx.x V
//
// OLED display message sized to fit 16x2 character display.
// TTTTTT = Charge cycle title to be displayed
//
void Standby_Charger::status_message() {
    if ((state_code == CYCLE_STARTUP) || (state_code == CYCLE_RUNNING)) {
        // Display charging status
        voltage_mv_t battery_voltage_mV = battery.get_voltage_average_mV();

        // Create elapsed time string (HH:MM:SS) in hms_str buffer
        // ms_to_hms_str(millis()-start_time, hms_str);
        ms_to_hms_str(charging_time_elapsed(), hms_str);

        // Create battery voltage string (xx.x) in bv_str buffer
        milliunits_to_string(battery_voltage_mV, 1, bv_str, sizeof(bv_str));

        // Write message to OLED display if present
        // Assumes display is configured for the default 8x16 proportional font
        // Note: OLED display is cleared at the start of charging cycle
        if (oled_found) {
            oled.clear();
            oled.setCursor(0,0);
            oled.printf("%s", title_str);
            oled.setCursor(64,0);
            oled.printf("%s", hms_str);
            oled.setCursor(0,2);
            oled.printf("%s V", bv_str);
            oled.switchFrame();
        }

        // Write console message
        Serial.printf("%s, %s, %s\n", name_str, hms_str, bv_str);
    }
}

