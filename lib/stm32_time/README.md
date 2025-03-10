# STM32 Hardware Timer-Based Alarms and Callbacks for Arduino

An Arduino library for STM32 micrcontrollers that provides alarm 
objects for scheduling future execution using the hardware timer
infrastructure.  Alarms are added to an alarm pool, which may hold up
to `STM32_TIME_MAX_ALARMS` (default is 16) active alarms.

### Details

An individual alarm can be used as a simple count-down timer for a singular
event, or can be used as repetitive timer to generate periodic callbacks.
The return code from the callback handler determines the mode.

The alarm pool is provided by the `Alarm_Pool` class implemented in this
library, which should be instantiated by the user and will linked to the 
`STM32_TIME_HW_TIME` hardware timer (default is TIM3).

 Warning: Check for potential conflicts with timer usage from the following
 Arduino subsystems that also allocate timer resources if used:
 + analogWrite (see the `PeripheralPins.c` file for the target variant)
 + Tone (see `TIMER_TONE`)
 + Servo (see `TIMER_SERVO`)
 + SoftSerial (see `TIMER_SERIAL`)

Support for library version checking is provided by the `version()` 
and `reldate()` methods.

### Example Usage

    #include <Arduino.h>
    #include <stm32_time.h>

    // On-board LED GPIO number
    #define led_gpio  PC15

    // Time period for LED on/off
    #define LED_PERIOD_MS   1000

    Alarm_Pool pool;

    int led_handler(alarm_id_t id, void *user_data) {
      // Toggle LED state
      digitalWrite(led_gpio, !digitalRead(led_gpio));
      // Return eschedule to run again
      return 1;
    }

    void setup() {
      // Configure pin in output mode
      pinMode(led_gpio, OUTPUT);
      digitalWrite(led_gpio, 1);

      // Create alarm
      int id = pool.add(LED_PERIOD_MS, led_handler);

      if (id == 0) {
        Serial.printf("Warning: Alarm pool returned '0' ID value\n");
      } else if (id < 0) {
        Serial.printf("Error: Unable to create alarm\n");
      } else if (id > 0) {
        Serial.printf("Alarm pool assigned ID %i\n", id);
      }
    } 

    void loop() {
      // Nothing to do since all is done by the hardware timer interrupts
    }

### Revision History

* 0.1   06/28/2024
        - Initial commit with basic alarm functionality implemented.

* 0.2   06/28/2024
        - Streamlined alarm infrastructure.

* 0.3   10/28/2024
        - Changed default timer from TIM1 to TIM3 to avoid conflicts.
        - Minor cleanup of source code.
        - Added Doxyfile and updated comments to support Doxygen documentation.

* 1.0   11/22/2024
        - Updated Doxgen documentation to latest standards.
        - Added README.md file with usage information and example.
        - Added `version()` and `reldate()` methods to support
          class version checking.


