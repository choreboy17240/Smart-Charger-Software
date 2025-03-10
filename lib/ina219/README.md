# INA219 Current/Power Sensor Class Library for Arduino

Arduino library providing support for the INA219x current/power sensor
with I2C interface.

### Details

This library provides support for interfacing with the Texas Instruments
INA219x current/power sensors through an I2C bus interface.

The INA219x sensor should be connected to the shunt resistor to ensure that
the current being measured will be in the positive direction, as the sign
of the shunt voltage is ignored by this library.

The i2c_bus library is used for I2C bus communications, as it provides
a higher-level, simpler approach to communications with I2C devices.

Support for library version checking is provided by the `version()` and
`reldate()` methods.

### Example Usage

    #include <Arduino.h>
    #include <ina219.h>

    const uint8_t  INA219B_I2C_ADDRESS = 0x40;  // INA219 sensor I2C address
    const PinNumber I2C0_SCL_GPIO = PA11;       // I2C0 SCL pin
    const PinNumber I2C0_SDA_GPIO = PA12;       // I2C0 SDA pin
    const uint32_t I2C0_BAUDRATE = 100000;      // I2C0 default clock rate (100khz)
    const uint16_t INA219B_CALIBRATION = 4071;  // Sensor calibration value
    const uint16_t INA219_ILSB = 40;            // Shunt current LSB in uAmp
    const uint16_t INA219_PLSB = 20*INA219_ILSB;  // Power LSB in uWatts

    // Global I2C bus object
    I2C main_i2c_bus = I2C(&Wire, I2C0_SCL_GPIO, I2C0_SDA_GPIO, I2C0_BAUDRATE);

    // Global current sensor object
    INA219 sensor;

    void setup() {
      // Initialize INA219 sensor object and check I2C connection
      if (!sensor.init(&main_i2c_bus, INA219B_I2C_ADDRESS) {
        // Fatal error
        Serial.printf("Error: INA219 sensor not detected!\n");
        while (1);
      }

      // Configure INA219 voltage/current sensor
      if (sensor.connected()) {
          // Complete setup of the sensor
          sensor.reset();
          sensor.set_bus_range(RANGE_32V);
          sensor.set_bus_ADC_resolution(BUS_RES_12BIT);
          sensor.set_shunt_ADC_resolution(SHUNT_RES_12BIT);
          sensor.set_PGA_gain(GAIN_8_320MV);
          sensor.set_calibration(INA219_CAL, INA219_ILSB, INA219_PLSB);
          sensor.set_operation_mode(SANDBVOLT_CONTINUOUS);
      } else {
          // Fatal error
          Serial.printf("Error: INA219B sensor is not responding!\n");
          while (1);
      }
    }

    void loop() {
      current_ma_t current = sensor.get_current_mA();
      voltage_mv_t voltage = sensor.get_bus_voltage_mV();

      Serial.printf("Bus voltage = %u mV, Current = %u mA\n", voltage, current);
      delay(1000);
    }

### Revision History

* 1.0   7/16/2024
        - Initial release for Arduino platform.
        - Ported from the Raspberry Pi Pico library created for the
          `Charger4` battery charger project.
        - Numeric quantities were changed from `float` to `uint32_t`
          integers to avoid the overhead of the floating point math
          library. Voltage and current are provided as mA and mV
          values to fit into meaningful integer values.

* 1.1   11/20/2024
        - Updated Doxgen documentation to latest standards.
        - Added `version()` and `reldate()` methods to support
          class version checking.




