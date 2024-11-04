#include <Arduino.h>

#include "obcharger.h"
#include "rgbled.h"
#include "oled.h"
#include "battery.h"
#include "fast.h"
#include "topping.h"
#include "trickle.h"

// Libraries
#include <i2c_busio.h>
#include <ina219.h>

// OLED display support
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//=============================================================================
// Global variables
//=============================================================================

// Master start time (beginning of program)
time_ms_t start_time;

// Loop timer
time_ms_t loop_timer;

// Master charger state
charger_state_t charger_state;

// Timer support
Alarm_Pool timer_pool;

// Timer pool interrupt handler
void timer_pool_handler(void) {
    timer_pool.dec();
}

// Global charging task handlers
Fast_Charger fast_charger;
Topping_Charger topping_charger;
Trickle_Charger trickle_charger;

// I2C bus
I2C main_i2c_bus = I2C(&Wire, I2C0_SCL_GPIO, I2C0_SDA_GPIO, I2C0_BAUDRATE);

// Battery
Battery battery;

// Current sensor
INA219 sensor;

// DAC
MCP4726 dac;

// Voltage regulator
Vreg vreg;

// RGB LED
RGB_LED rgb_led;

// Optional OLED display
bool oled_found = false;    // Indicate whether OLED display was detected
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET);

// General ASCIIZ string message buffer
char buffer[81];

//=============================================================================
// Utility functions
//=============================================================================

// Print-out I2C scan results
void i2c_map(bool *addresses_found) {
    Serial.printf("    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
 
    for (int addr = 0; addr < 128; ++addr) {
        if (addr % 16 == 0) {
            Serial.printf("%02x  ", addr);
        }
 
        if (addresses_found[addr] == 1)
            Serial.printf("X");
        else
            Serial.printf(".");
        Serial.printf(addr % 16 == 15 ? "\n" : "  ");
    }
}


//=============================================================================
// Setup
//=============================================================================
void setup() {

    // Configure Serial port
    // Default Serial uses UART2 with PA2 (TX) and PA3 (RX)
    Serial.begin(115200);
    
    // Configure I2C bus
    Wire.setSCL(PA11);
    Wire.setSDA(PA12);
    Wire.begin();

    // Record system start-up time
    Serial.printf("Starting initialization now\n");
    start_time = millis();

    // Scan I2C buses
    // Use dynamic array to hold results, memory is released below
    Serial.printf("Scanning I2C Wire bus...\n");
    bool *addresses_found = new bool[128];
    int number_found = main_i2c_bus.scan(addresses_found, false);
    Serial.printf("Done!\n");

    // Display results
    Serial.printf("Found %u devices on Wire I2C bus \n", number_found);
    Serial.printf("Results of the I2C scan:\n");
    i2c_map(addresses_found);

    // Check if the optional OLED I2C display is installed
    // Initialize it if successfully detected on the I2C bus
    Serial.printf("Checking for OLED display on I2C bus ");
    oled_found = addresses_found[ADDRESS_128x32];
    delete [] addresses_found;  // Release memory
    if (oled_found) {
        Serial.printf("- found at address 0x%x\n", ADDRESS_128x32);
        Serial.printf("Initializing OLED display ");
        if (!display.begin(SSD1306_SWITCHCAPVCC, ADDRESS_128x32, NO_HARD_RESET, NO_WIRE_INIT)) {
            // OLED display initialization issue
            // Report error and disable the OLED display usage
            Serial.printf("- Error: SSD1306 allocation failed\n");
            oled_found = false;
        } else {
            display.clearDisplay();
            display.setTextSize(2);      // Default 6x8 character size
            display.setTextColor(SSD1306_WHITE); // Draw white text
            display.cp437(true);         // Use full 256 char 'Code Page 437' font
            display.display();
            Serial.printf("- Done\n");
        };
    } else {
        Serial.printf("- NOT found at address 0x%x\n", ADDRESS_128x32);
    };

    //
    // Initialize and test I/O drivers
    //
    Serial.printf("Initializing voltage regulator (off) ");
    digitalWrite(GP_VREG_ENABLE, LOW);
    pinMode(GP_VREG_ENABLE, OUTPUT);
    sensor.init(&main_i2c_bus, INA219B_I2C_ADDRESS); 
    dac.init(&main_i2c_bus, DAC_I2C_ADDRESS);
    vreg.begin(GP_VREG_ENABLE, &sensor, &dac);
    Serial.printf("- Done\n");

    Serial.printf("Initializing RGB LED (off) ");
    rgb_led.begin(GP_LEDR, GP_LEDG, GP_LEDB, LED_BLK);
    Serial.printf("- Done\n"); 

    // Initialize A/D converter support to 12-bit resolution
    analogReadResolution(12);

    // Initialize the alarm pool
    Serial.printf("Initializing the timer pool ");
    timer_pool.setup(TIM3, timer_pool_handler);
    Serial.printf("- Done\n");

    // Initialize the charging cycle handlers
    Serial.printf("Initializing charging cycle handlers ");
    fast_charger.init(FAST_PARMS);
    topping_charger.init(TOP_PARMS);
    trickle_charger.init(TRCKL_PARMS);
    Serial.printf("- Done\n");

    // FIXME: ADD SUPPORT FOR STORAGE CHARGING CYCLE
    
    // Initial state of charger
    charger_state = CHARGER_STARTUP;

    // Initialize loop timer
    loop_timer = millis();
}

//=============================================================================
// Main loop
//=============================================================================
void loop() {

    // Former charging supervisor routine moved to loop() to be consistent
    // with the usual Arduino approach
    if ((millis() - loop_timer) >= LOOP_DELAY) {
        // Run charging supervisor
        loop_timer = millis();

        switch (charger_state) {
            case CHARGER_STARTUP: {
                // Starting-up initialization
                Serial.printf("Entering startup initialization state\n");

                // Check initial battery voltage to determine the appropriate
                // charging cycle. Fast if discharged heavily, topping otherwise.
                voltage_mv_t battery_voltage = battery.get_voltage_mV();
                if (battery_voltage <= BATTERY_DISCHARGED_MV) {
                    Serial.printf("Battery voltage @ %u.%u volts, initiating fast charge\n", 
                                   battery_voltage/1000, battery_voltage%1000);
                    charger_state = CHARGER_FAST;
                    fast_charger.start();
                } else {
                    Serial.printf("Battery voltage @ %u.%u volts, initiating topping charge\n", 
                                   battery_voltage/1000, battery_voltage%1000);
                    charger_state = CHARGER_TOPPING;
                    topping_charger.start();
                }
            }
                break;

            case CHARGER_FAST: {
                // Fast-charging is in process
                switch (fast_charger.run()) {
                    case CYCLE_STARTUP:
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_DONE:
                        // Charging done, move to topping charge
                        Serial.printf("Fast charging cycle completed!\n");
                        Serial.printf("Initiating topping charge cycle\n");
                        charger_state = CHARGER_TOPPING;
                        topping_charger.start();
                        break;
                    case CYCLE_TIMEOUT:
                        // Charging timed-out, something's not right
                        Serial.printf("Fast charging cycle timed-out!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    case CYCLE_ERROR:
                        // Hardware error detected
                        Serial.printf("Fast charging cycle aborted by error condition!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    default:
                        Serial.printf("Fast charging cycle returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(fast)
            }
                break;

            case CHARGER_TOPPING: {
                // Topping charging is in process
                switch (topping_charger.run()) {
                    case CYCLE_STARTUP:
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_DONE:
                        // Charge done, move to topping charge
                        Serial.printf("Topping charging cycle completed!\n");
                        Serial.printf("Initiating trickle charge cycle\n");
                        charger_state = CHARGER_TRICKLE;
                        trickle_charger.start();
                        break;
                    case CYCLE_TIMEOUT:
                        // Charge timed-out, something's not right
                        Serial.printf("Topping charging cycle timed-out!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    case CYCLE_ERROR:
                        // Hardware error detected
                        Serial.printf("Topping charging cycle aborted by error condition!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    default:
                        Serial.printf("Topping charging cycle returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(topping)
            }
                break;

            case CHARGER_TRICKLE: {
                // Trickle charge is in process
                switch (trickle_charger.run()) {
                    case CYCLE_STARTUP:
                    case CYCLE_RUNNING:
                        break;
                    case CYCLE_DONE:
                        // Charge done, move to topping charge
                        Serial.printf("Trickle charging cycle completed!\n");
                        charger_state = CHARGER_TRICKLE;
                        break;
                    case CYCLE_TIMEOUT:
                        // Charge timed-out, something's not right
                        Serial.printf("Trickle charging cycle timed-out!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    case CYCLE_ERROR:
                        // Hardware error detected
                        Serial.printf("Trickle charging cycle aborted by error condition!\n");
                        charger_state = CHARGER_SHUTDOWN;
                        break;
                    default:
                        Serial.printf("Trickle charging cycle returned unknown status!\n");
                        charger_state = CHARGER_SHUTDOWN;
                } // switch(trickle)
            }
                break;

            case CHARGER_SHUTDOWN:
                break;

            case CHARGER_LOAD_TEST:
                // Battery load test not implemented
                Serial.printf("Battery load test not implemented\n");
                break;

            default:
                // Fatal error - we should never get here!
                Serial.printf("Fatal error: Invalid charger state code '%u'!", charger_state);
                while (1);
        } // switch(charger_state)
    }  // charging supervisor

}  // loop()
