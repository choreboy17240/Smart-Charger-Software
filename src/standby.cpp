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

    // Update any attached OLED displays
    if (millis() - display_timer >= display_period) {
        display_timer = millis();
        status_message(DISPLAY_OLED);
    }

    // Update serial console
    if (millis() - message_timer >= message_period) {
        message_timer = millis();
        status_message(DISPLAY_CONSOLE);
    }

    // Normal exit
    return state_code;
}

/*
 * Write status information for standby mode to the targeted display device.
 * We're overriding the base class method to allow for customized messaging
 * for the standby mode.
 *
 * Console message format:
 *
 *  <name_str>, HH:MM:SS, xx.x
 *
 * OLED display message format, sized to simulate a 16x2 character display:
 * 
 *  0123456789012345
 *  TTTTTT  HH:MM:SS
 *  xx.x V
 *
 *  TTTTTT = Charge cycle title to be displayed
 */
void Standby_Charger::status_message(display_t device) {
    // Retrieve battery voltage
    voltage_mv_t battery_voltage_mV = battery.get_voltage_average_mV();
    
    // Get elapsed time as a string (HH:MM:SS)
    ms_to_hms_str(charging_time_elapsed(), hms_str);

    // Get battery voltage as a string (xx.x)
    milliunits_to_string(battery_voltage_mV, 1, bv_str, sizeof(bv_str));

    switch (device) {
        case DISPLAY_NONE:      // No display present
            break;
        case DISPLAY_CONSOLE:   // Serial console
            Serial.printf("%s, %s, %s\n", name_str, hms_str, bv_str);
            break;
        case DISPLAY_OLED:      // OLED display
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
            } else {
                Serial.printf("Error: OLED status was requested, but display not present\n");
            }
            break;
        default:                // Unknown device
            Serial.printf("Error: Unknown display device %d\n", int(device));
    }
}

