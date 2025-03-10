# MCP4726 12-Bit I2C DAC Class Library for Arduino

Arduino library providing support for the Microchip 12-bit I2C 
digital-to-analog (DAC) with I2C interface.

### Details

This library provides support for interfacing with the Microchip MCP4726
12-bit DAC with I2C interface and NVM storage for configuration and value
settings.

Microchip also offers the MCP4706 (8-bit) and MCP4716 (10-bit) DACs,
which use a very similar programming interface.  Although focused
specifically on the MCP4726, this library could be readily adapted to those
part numbers if desired.

The `i2c_bus` library is used for I2C bus communications, as it provides
a higher-level, simpler approach to communications with I2C devices.

Support for library version checking is provided by the `version()` and
`reldate()` methods.

### Example Usage

    #include <Arduino.h>
    #include <mcp4726.h>

    // Global I2C bus object
    I2C main_i2c_bus = I2C(&Wire, I2C0_SCL_GPIO, I2C0_SDA_GPIO, I2C0_BAUDRATE);

    // Global MCP4726 DAC object
    MCP4726 dac;

    // DAC output level setting
    uint16_t dac_level = 0;

    void setup() {
      // Initialize DAC object
      dac.init(&main_i2c_bus, DAC_I2C_ADDRESS);

      // Check I2C connection to DAC
      if (!dac.connected()) {
        // Fatal error
        Serial.printf("Error: MCP4726 DAC not detected!\n");
        while (1);
      }

      // Configure MCP4726 settings
      if (!dac->begin(MCP4726_AWAKE | MCP4726_VREF_VDD | MCP4726_GAIN_1X)) {
        // Fatal error
        Serial.printf("Error: MC4726 configuration error!\n");
        while (1);
      }

      // Set the initial output level
      if (!dac->set_level(dac_level)) {
        // Fatal error
        Serial.printf("Error: MC4726 comm issue while setting output level!\n");
        while (1);
      }
    }

    void loop() {
      // Set output DAC level (initially set to 0)
      if (dac->set_level(dac_level)) {
        Serial.printf("DAC output level set to %u", dac_level);
      } else {
        // Communication error, report and try to continue
        Serial.printf("Error: MC4726 comm issue while setting output level!\n");
      }

      // Increment level, rolling over at maximum DAC value
      if (++dac_level > 4095) {
        dac_level = 0;
      }

      delay(100);
    }

### Revision History

* 1.0   7/16/2024
        - Initial release for Arduino platform.

* 1.1   11/21/2024
        - Updated Doxgen documentation to latest standards.
        - Added README.md file with usage information and example.
        - Added `version()` and `reldate()` methods to support
          class version checking.




