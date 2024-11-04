/**
 * @file cycle.cpp
 *
 * @brief Battery charging cycle base object
 */

#include "obcharger.h"
#include "cycle.h"

// OLED display support
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//
// Global variables
//
extern Alarm_Pool timer_pool;               // Hardware timers
extern Vreg vreg;                           // Voltage regulator
extern Battery battery;                     // Battery
extern RGB_LED rgb_led;                     // RGB status LED
extern bool oled_found;                     // OLED display found at startup in main()?
extern Adafruit_SSD1306 display;            // OLED display device

// Default constructor
Charge_Cycle::Charge_Cycle() {
}

// Constructor with initialization
Charge_Cycle::Charge_Cycle(charge_parm_t &p) {
    // Initialize charging cycle handler
    init(p);
}

// Initialize charging cycle handler
void Charge_Cycle::init(const charge_parm_t &p) {
    // Initialize charge state
    state_code = CYCLE_INIT;

    // Set global voltage regulator to off
    vreg.off();
    set_voltage = 0;

    // Save charging parameters
    target_voltage = p.voltage_target;
    step_voltage = p.voltage_step;
    target_current = p.current_target;
    max_current = p.current_max;

    // Save timer values
    charge_period_max = p.charge_period_max;
    startup_period = p.startup_period;
    idle_period = p.idle_period;

    // Save status LED parameters
    led_off_period = p.led_off_period;
    led_on_period = p.led_on_period;
    led_color = p.led_color;

    // Save status message strings
    title_str = p.title_str;
    name_str = p.name_str;
}

// Destructor (best practice)
Charge_Cycle::~Charge_Cycle() {};

// Startup initialization for new charge cycle
void Charge_Cycle::start() {
    state_code = CYCLE_STARTUP;

    // Set the global voltage regulator output at 500 mV below the current
    // battery voltage to provide a "soft start" that avoids overloading the
    // regulator when we turn it on!
    uint32_t battery_voltage = battery.get_voltage_mV();
    if (battery_voltage < VREG_VOLTAGE_MIN) {
        // Start at minimum regulator voltage
        set_voltage = VREG_VOLTAGE_MIN;
    } else if (battery_voltage > VREG_VOLTAGE_MAX) {
        // Shouldn't really happen, issue a warning
        Serial.printf("Warning: Battery voltage above %u mV!\n", VREG_VOLTAGE_MAX);
        set_voltage = VREG_VOLTAGE_MAX;
    } else {
        // Start just below battery voltage
        set_voltage = battery_voltage - 100;
    }
    vreg.set_voltage(set_voltage);
    vreg.on();

    // Start the hardware alarm timers
    charge_timer_id = timer_pool.add(charge_period_max, nullptr);

    // Store the system time when charging cycle starts
    // and initialize the software status message timer
    start_time = millis();
    message_timer = start_time;

    // Initialize LED to the 'on' state, with the specified color for
    // the current charge cycle.  The cycle-specific run() handlers will
    // call the status_led() method below to control the LED state.
    rgb_led.begin(GP_LEDR, GP_LEDG, GP_LEDB, led_color);
    led_state = true;
    led_timer = start_time;

    // Display startup message to serial console only
    Serial.printf("Starting %s charging cycle\n", name_str);

    // Clear OLED display to prepare for new charging cycle messages
    if (oled_found) {
        display.clearDisplay();
        display.setTextSize(1);      // Default 6x8 character size
        display.setTextColor(SSD1306_WHITE); // Draw white text
        display.cp437(true);         // Use full 256 char 'Code Page 437' font
        display.display();
    }
}

// Run-time handler called to manage charging cycle
// Virtual function, overriden by derived classes
cycle_state_t Charge_Cycle::run() {
    // Abnormal exit, as base class method should never by called
    return CYCLE_ERROR;
}

// Stop charging cycle
void Charge_Cycle::stop() {
    vreg.off();
}

// Get charging state code
cycle_state_t Charge_Cycle::state() {
    return state_code;
}

// Get remaining startup time
// Startup time must always be shorter than the charging timeout!
uint32_t Charge_Cycle::startup_time_remaining(void) {
    // FIXME: Created variable for debugging purposes
    uint32_t charging_elapsed = charging_time_elapsed();

    if (charging_elapsed >= startup_period) {
        return 0;
    } else {
        return (startup_period - charging_elapsed);
    }
}

// Get remaining charging time
uint32_t Charge_Cycle::charging_time_remaining(void) {
    // FIXME: Created variable for debugging purposes
    uint32_t charging_timer = timer_pool.get(charge_timer_id);
    return charging_timer;
}

// Get elapsed charging time
uint32_t Charge_Cycle::charging_time_elapsed(void) {
    // FIXME: Created variable for debugging purposes
    uint32_t elapsed_time = timer_pool.elapsed(charge_timer_id);
    return elapsed_time;
}

// Update RGB LED status
// Uses software timer 'led_timer' for managing on/off times
void Charge_Cycle::status_led() {
    if (led_state == LED_ON) {
        // RGB LED is currently on
        if (millis() - led_timer >= led_on_period) {
            // Time to turn if off
            rgb_led.color(LED_BLK);
            led_state = false;
            led_timer = millis();
        }
    } else {
        // RGB LED is currently off
        if (millis() - led_timer >= led_off_period) {
            // Time to turn it on
            rgb_led.color(led_color);
            led_state = true;
            led_timer = millis();
        }
    }
}

// Write status information to serial console and any attached displays
//
// Console message format:
// <title_str> HH:MM:SS Battery=xx.x V, xxxx mA
//
// OLED display message format:
// 0123456789012345
// TTTTTT  HH:MM:SS
// xx.x V   xxxx mA
//
// OLED display message sized to fit 16x2 character display.
// TTTTTT = Charge cycle title to be displayed
//
void Charge_Cycle::status_message() {
    if ((state_code == CYCLE_STARTUP) || (state_code == CYCLE_RUNNING)) {
        // Display charging status

        // Get current charging parameters
        current_ma_t charging_current = vreg.get_current_mA();
        FXPTQ1616 battery_voltage = battery.get_voltage();

        // Create elapsed time string (HH:MM:SS) in hms_str buffer
        // ms_to_hms_str(millis()-start_time, hms_str);
        ms_to_hms_str(charging_time_elapsed(), hms_str);

        // Create battery voltage string (xx.x) in bv_str buffer
        battery_voltage.to_string(bv_str, sizeof(bv_str), "%2.1f");

        // Write message to OLED display if present
        // Assumes display is configured for the default 5x7 font and 1X size
        // Note: OLED display is cleared at the start of charging cycle
        if (oled_found) {
            display.clearDisplay();
            display.setCursor(0,0);
            display.printf("%s %s", title_str, hms_str);
            display.setCursor(0,10);
            display.printf("%s V %4u mA", bv_str, charging_current);
            display.display();
        }

        // Write console message
        Serial.printf("%s %s Battery @ %s V, %04u mA\n", title_str, hms_str, bv_str, charging_current);
    }
}
