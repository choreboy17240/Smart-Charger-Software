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

Version number is encoded in the `obcharger.h` file, which defines the following
macros with the version number information (e.g. VERSION_MAJOR.VERSION_MINOR):
* VERSION_MAJOR - Major revision number
* VERSION_MINOR - Minor revision number

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



      