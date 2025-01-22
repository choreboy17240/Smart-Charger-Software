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

Charging cycles parameters are configured at compile time based on the values
of fields in the `charge_parm_t` structure defined for each charge cyle in the
`cycle.h` file.  These values can be modified to customize each charge cycle
to the battery to be charged.

As of revision v0.3, the `charge_parm_t` structure is defined as follows:

    struct charge_parm_t {
        current_ma_t current_target;            ///< Target charging current
        current_ma_t current_max;               ///< Maximum charging current
        voltage_mv_t voltage_target;            ///< Target battery voltage
        voltage_mv_t voltage_step;              ///< Step size for adjusting voltage
        time_ms_t charge_period_max;            ///< Maximum allowable charging time
        time_ms_t startup_period;               ///< Startup time period
        time_ms_t idle_period;                  ///< Idle time between charges (standby mode only)
        time_ms_t led_on_period;                ///< Status LED on time while charging
        time_ms_t led_off_period;               ///< Status LED off time while charging
        rgb_t led_color;                        ///< Status LED color while charging
        const char *title_str;                  ///< Charge cycle title (6 chars) for LCD display (e.g. "FAST  ")
        const char *name_str;                   ///< Charge cycle name for serial output (e.g. 'Fast')
        time_ms_t message_period;               ///< Time between console and OLED message updates
    };

Here's how each of these fields impact the charging cycles to which they are passed:

* **current_target**: Target charging current (mA) that the handler will try to achieve by adjusting the regulator voltage. This parameter is used in most active charging cycles, except for trickle charging, which is based on a constant voltage algorithm.
* **current_max**: Maximum charging current (mA) which the handler will allow, to avoid damage to the battery being charged.  This parameter is used by all active charging cycles (fast, topping, trickle).
* **voltage_target**: Target battery voltage (mV) that the handler will try to achieve during the charging cycle. This parameter is used by all active charging cycles (fast, topping, trickle).
* **voltage_step**: Step size (mV) to be used for adjusting the regulator voltage. This parameter is used by all active charging cycles (fast, topping, trickle).
* **charge_period_max**: Maximum time (ms) that the handler will allow for the cycle. If the target goal for the cycle is not reached within this time period, the handler will shut-off the regulator to avoid battery damage and return a `CYCLE_TIMEOUT` state to the `loop()` function.  This parameter is used by all active charging cycles (fast, topping, trickle).
* **startup_period**: Special time period (ms) allowed at the beginning a charge cycle to allow the battery being charged to stabilize (e.g. battery voltage float to dissipate). This parameter forces the handler to delay checking whether the cycle's goals have been achieved until after the startup period has expired, avoiding premature decisions on whether the charging cycle's goal has been achieved.  This parameter is used by all active charging cycles (fast, topping, trickle).
* **idle_period**: Time period (ms) used exclusively by the standby mode to determine how long to wait before resuming active charging of the battery. The voltage regulator will be shut-off during standby mode, but displays will be updated periodically (typically at a slower rate) to reflect the standby time and current battery voltage.  This parameter is only used by the standby mode.
* **led_on_period**: Time period (ms) that the RGB LED will be illuminated during the charging cycle. This parameter is used by all charging cycles.
* **led_off_period**: Time period (ms) that the RGB LED will be turned off during the charging cycle. This parameter is used by all charging cycles.
* **led_color**: Color to be used by the RGB LED when illuminated during the charging cycle. The `rgb_t` structure is used to pass 8-bit RGB values to represent the color.  This parameter is used by all charging cycles.
* **title_str**: A short (6 characters) title for the charge cycle (e.g. "FAST  ") to be displayed on the OLED display for status updates. This parameter is used by all charging cycles.
* **name_str**: A longer descriptive name for the charge cycle (e.g. "Fast") to be displayed in console status messages. This parameter is used by all charging cycles.
* **message_period**: Time period (ms) between updates to the console and OLED status messages. The frequency can be varied depending on how volatile the charging cycle is expected to be. Fast charging would likely require frequent updates, while the long standby mode can be much less timely given the limited action. This parameter is used by all charging cycles.

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

      

      