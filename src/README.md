# On-Board Battery Charger

Software support for the STM32-based intelligent battery charger/control module.

### Concept of Operation

From a high-level the sequence of operations performed the charger after 
power-up is as follows:
1.  Perform startup initialization
2.  Check the charge state of the battery
3.  Run the fast charge cycle if battery voltage is low
4.  Run the topping charge cycle to top-off the battery
5.  Run the trickle charge cycle to complete fully charging the battery
6.  Move to standby mode for a calibratable time period (e.g. 1 week)
7.  Return to step 2

The charging cycles are performed by **charge cycle handler** objects that are
derived from the **Charge_Cycle** class.  This base class provides the 
framework for the handlers, which need only to provide a customized run() 
method to provide the functionality needed to manage the specific charging 
process for the target battery.

Within each individual charge cycle, the following steps are applied to the
battery being charged:
1. Initialize the cycle parameters using the Charge_Cycle constructor, or by
   calling the init() method.  This should be done once, typically during the
   charger's startup initialization phase.
2. Start the charge cycle at the appropriate time by calling the start() 
   method of the desired charge cycle handler object.
3. Call the run() method periodically (typically every 100 ms) so the handler
   can manage the charging process for the battery being charged.
4. Monitor the return status from the run() method to determine next step
   in the charging process.
5. Terminate the charging process by calling the stop() method in the event
   that the charging targets are achieved, the charging cycle times-out, or
   a charging error is detected.

In `standby mode`, the power supply is shut down, and the micro "wakes up"
every 60 seconds to check things out and update the RGB indicator and any
attached displays.

The RGB LED indicator is used to show the current charging cycle:
- Fast: Blue pulse every second
- Topping: Yellow pulse every 1.25 seconds)
- Trickle: Green pulse every 3.0 seconds
- Standby: Green pulse every 60 secondss

RGB color and timing parameters are configured for each cycle in the 
charge parameter structures defined in the `cycle.h` file.

### Implementation Notes

#### General

Software revision information is encoded in the `obcharger.h` file, which 
defines the following macros:
* OBC_VERSION - String with revision number (x.x)
* OBC_RELDATE - String with release date (MM/DD/YYYY)

The revision information should be updated for new releases, and will be
displayed to the serial console at startup, along with release information
for major libraries being used.

Global variables are declared in the `main.cpp` file, including the instantations
of classes that contain much of the functionality.

Global typedefs and constants are declared in the `obcharger.h` file.

#### Configuring charge cycle parameters

Charging cycles parameters are configured at compile time by adjusting values
of fields in the `charge_parm_t` structure defined for each charge cyle in the
`cycle.h` file.  These values can be modified to customize each charge cycle
to the battery to be charged.

As of revision v0.5, the `charge_parm_t` structure is defined as follows:

    struct charge_parm_t {
        current_ma_t current_target;            ///< Target charging current
        current_ma_t current_max;               ///< Maximum charging current
        voltage_mv_t voltage_target;            ///< Target battery voltage
        voltage_mv_t voltage_step;              ///< Step size for adjusting voltage
        time_ms_t charge_period_max;            ///< Maximum allowable cycle time
        time_ms_t startup_period;               ///< Startup time period
        time_ms_t led_on_period;                ///< Status LED on time while charging
        time_ms_t led_off_period;               ///< Status LED off time while charging
        rgb_t led_color;                        ///< Status LED color while charging
        const char *title_str;                  ///< Charge cycle title (6 chars) for LCD display (e.g. "FAST  ")
        const char *name_str;                   ///< Charge cycle name for serial output (e.g. 'fast')
        time_ms_t display_period;               ///< Time between OLED display updates
        time_ms_t message_period;               ///< Time between serial console message updates
    };

Here's how each of these fields impact the charging cycles to which they are passed:

* **current_target**: Target charging current (mA) that the handler will try to achieve by adjusting the regulator voltage. This parameter is used in most active charging cycles, except for trickle charging, which is based on a constant voltage algorithm.
* **current_max**: Maximum charging current (mA) which the handler will allow, to avoid damage to the battery being charged.  This parameter is used by all active charging cycles (fast, topping, trickle).
* **voltage_target**: Target battery voltage (mV) that the handler will try to achieve during the charging cycle. This parameter is used by all active charging cycles (fast, topping, trickle).
* **voltage_step**: Step size (mV) to be used for adjusting the regulator voltage. This parameter is used by all active charging cycles (fast, topping, trickle).
* **charge_period_max**: Maximum time (ms) that the handler will allow for the cycle. If the target goal for the cycle is not reached within this time period, the handler will shut-off the regulator to avoid battery damage and return a `CYCLE_TIMEOUT` state to the `loop()` function.  This parameter is used by all charging cycles.
* **startup_period**: Special time period (ms) allowed at the beginning a charge cycle to allow the battery being charged to stabilize (e.g. battery voltage float to dissipate). This parameter forces the handler to delay checking whether the cycle's goals have been achieved until after the startup period has expired, avoiding premature decisions on whether the charging cycle's goal has been achieved.  This parameter is used by all active charging cycles (fast, topping, trickle).
* **led_on_period**: Time period (ms) that the RGB LED will be illuminated during the charging cycle. This parameter is used by all charging cycles.
* **led_off_period**: Time period (ms) that the RGB LED will be turned off during the charging cycle. This parameter is used by all charging cycles.
* **led_color**: Color to be used by the RGB LED when illuminated during the charging cycle. The `rgb_t` structure is used to pass 8-bit RGB values to represent the color.  This parameter is used by all charging cycles.
* **title_str**: A short title (six characters, pad with spaces for shorter titles) for the charge cycle (e.g. "FAST  ") to be displayed on the OLED display for status updates. This parameter is used by all charging cycles.
* **name_str**: A longer descriptive name for the charge cycle (e.g. "Fast") to be displayed in console status messages. This parameter is used by all charging cycles.
* **display_period**: Time period (ms) between updates to any attached OLED display (if present).  This parameter is used by all charging cyles.
* **message_period**: Time period (ms) between status messages sent to the serial console. This parameter is used by all charging cyles.

#### Hardware timer resources used

**Charging cycle timer**

Software timers are used for most events, with the exception of the charging cycle timer. This timer uses a hardware timer provided by the `stm32_time` library. As of revision v0.5, the hardware timer used is defined as `TIM3` in the `stm32_time.h` file in the library directory.

**RGB LED PWM control**

In the STM32G030 microcontroller, the GPIO pins PB6, PB7, and PB8 are associated with specific timers that facilitate PWM functionality. When using the STM32Duino framework, the RGB pins are mapped to the following hardware timers:

- Red (PB8): TIM16, Channel 1
- Green (PB7): TIM17, Channel 1
- Blue (PB6): TIM16, Channel 1

ChatGPT noted that both PB6 and PB8 are associated with TIM16, Channel 1. It warned that configuring PWM on both pins simultaneously requires careful management to avoid conflicts, as they share the same timer channel.

#### Software filtering of current and voltage readings

Charging current readings were found to be a bit volatile and erratic in early
testing. This may indicate a need to add hardware filtering to the INA219 
sensor, but using a software filtering strategy for now. Added the 
`get_current_mA()` method to the `Vreg` class, which takes the average of
four consecutive current readings from the INA219 sensor.  The number of
readings can be configured using the `AVG_READINGS` constant in the 
`regulator.cpp` file.

Even with this change, the values displayed to the console and OLED display
were still a bit wanky.  Added a ring buffer to store the historical current
readings taken within the cycle handler every 100 ms. and am now using that 
rolling average for display purposes. The size of the ring buffer can be
configured using the `RB_CHARGING_CURRENT_SAMPLES` constant defined in the
`obcharger.h` file.  As of v0.3 release, ten samples are used to align the
updates with the one second updates done to the console and OLED display.

Battery voltage readings were also a bit volatile, so added the 
`get_voltage_average_mV()` method to the `Battery` class.  This method
also takes four consecutive readings from the ADC and averages them to
get a somewhat smoothed value.  The number of readings can be configured
using the `AVG_READINGS` constant in the `battery.cpp` file.


### Revision History

* 0.1  11/2/2024
      - Pre-release with base functionality.
      - Limited support for optional OLED display.
      - Fast, topping, and trickle charging cycles are functional.
      - Current and voltage readings are not as stable as desired, 
        need to implement software averaging/filtering.
      - Rough calibration of voltage readings completed, but have
        not confirmed current readings.

* 0.2 11/21/2024
      - Major update to base functionality.
      - Significant improvement in stability of current and
        voltage readings, using multi-read averaging and
        a ring buffer cache to generate a long-term average
        for the charging current reading (i.e. averaging
        10 readings over each second between display updates).
      - Optional OLED display support streamlined and 
        enhanced using the `STM32_4kOLED` library rather
        than Adafruit display libraries. This change provides
        enhanced fonts and a much smaller memory footprint.

* 0.3 01/21/2025
      - Eliminated use of fixed-point numbers throughout the project, keeping
        all voltage and current values in milliunits.
      - Added a simple utility function to the `cycle` module to convert the 
        milliunit values to decimal string representation for OLED and console
        display purposes.
      - Removed fixed-point functions from the `battery` and `vreg` modules,
        now all values are represented in milliunits.
      - Updated naming of the some methods to use the _mV or _mA suffix to
        ensure clarity on the units used.
      - Although the goal was really to streamline the code, the changes has
        a positive impact on memory usage.  The RAM usage increase slightly
        from 2552 bytes to 2564 bytes, but flash usage decreased from 47720
        bytes to 46636 bytes.  It may be possible to optimize RAM usage a
        bit, but we're well below the 8K available on the targeted STM32G030
        micro for this project.
      - Changed the time period between message updates to a configurable
        parameter for individual charge cycles, as less frequent updates are
        needed for trickle charge and standby modes.  Added a `message_period`
        value to the `charge_parm_t` structure to specify the period.

* 0.4 01/23/2025
      - Fixed logic in the `loop()` function to correctly manage the successful
        completion of the trickle charging cycle and transition to the standby
        mode.
      - Minor updates to correct some incorrect comments and superfluous
        timeout status messages in the cycle handlers.
      - Temporarily set cycle times for both trickle and standby cycles to 
        four hours to help speed-up validation testing.  These will need to be
        set to "normal" durations for the official release.

* 0.5 01/24/2025
      - Fixed bug that was causing the standby mode to exit immediately after
        entry. The cycle `start()` method didn't reflect the use of a separate
        configurable parameter for the standby time period. Eliminated the
        separate parameter, now using the existing `charge_period_max` parameter
        to specify the standby period.
      - Revised to allow updates to the OLED display and console status messages
        to occur at independent intervals. The configuration structure now
        provides a `display_period` parameter for OLED display updates, in
        addition to the existing `message_period`, which now only applies
        to the console status messages.

* 0.6 01/24/2025
      - Fixed bug with startup initialization in the setup() function in the
        `main` module. Forgot to add the inialization step for the 
        `standby_charger` handler instance, which caused some strange issues
        with the standby cycle terminating immediately.
      - Fixed bug with hardware timer usage by the base `Charge_Cycle` class
        in the `cycle` module. The `start()` method was allocating a new
        hardware timer from the pool every time, and was also not verifying 
        whether the timer was successfully allocated. Moved the timer
        allocation request to the `init()` method, and updated the `start()`
        method to set the existing hardware timer to the charging cycle
        period.

      