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
6.  Run the battery storage/maintenance charge cycle
7.  Delay for calibratable time period (e.g. 1 week)
8.  Return to step 2

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


### Implementation Notes

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


      