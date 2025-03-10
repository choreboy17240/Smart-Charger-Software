# I2C Bus Class Library for Arduino

An Arduino library that provides an I2C bus class with a simpler higher-level
interface for accessing the I2C buses available on the embedded controller.

### Details

This library was originally developed to provide a higher-level I2C
interface to the SDK for the Raspberry Pi Pico microcontrollers.
This Arduino version simplifies the task of porting other libraries
from the Pico environment to Arduino.   For Arduino, the `TwoWire` 
class used by the default `Wire` object will be used.  For Pico, 
the SDK provides the low-level interface to the I2C bus.  In both 
environments, this library provides a wrapper with consistent 
interface to the underlying I2C bus. 

The class constructor is used to create and initialize the I2C
instance, specifying the underlying `TwoWire` object to be used
for bus access, GPIO pins for the SCL and SDA signals, and 
(optionally) the desired bus speed.  The default bus speed is 100 kHz
if not specified in the constructor parameters.

Support for library version checking is provided by the `version()` 
and `reldate()` methods.

### Example Usage

    #include <Arduino.h>
    #include <i2c_busio.h>

    // Outgoing data
    uint8_t obuffer[] = { 0, 1, 2, 3 };
    
    // Incoming buffer
    uint8_t ibuffer[8];       

    // Number of bytes written or read
    uint n;

    const uint8_t   I2C_ADDRESS = 0x40;         // I2C address for target device
    const PinNumber I2C0_SCL_GPIO = PA11;       // I2C0 SCL pin
    const PinNumber I2C0_SDA_GPIO = PA12;       // I2C0 SDA pin
    const uint32_t  I2C0_BAUDRATE = 100000;     // I2C0 default clock rate (100khz)

    // Global I2C bus object
    I2C main_i2c_bus = I2C(&Wire, I2C0_SCL_GPIO, I2C0_SDA_GPIO, I2C0_BAUDRATE);

    void setup() {
      // Scan the I2C bus for active devices and display the results
      // Using dynamic array to hold results, memory is released below
      Serial.printf("Scanning I2C Wire bus... ");
      bool *addresses_found = new bool[128];
      int number_found = main_i2c_bus.scan(addresses_found, false);
      Serial.printf("Done!\n");
      Serial.printf("Found %u devices on Wire I2C bus \n", number_found);
      Serial.printf("\n");
      Serial.printf("Results of the I2C scan:\n");
      i2c_map(addresses_found);
      Serial.printf("\n");
      delete [] addresses_found;  // Release memory

      // Check if a specific target device is present
      if (main_i2c_bus.connected(I2C_ADDRESS)) {
        Serial.printf("Found the targeted I2C device at %u\n", I2C_ADDRESS);
      } else {
        // Fatal error
        Serial.printf("Error: Can't find the targeted device at %u\n", I2C_ADDRESS);
        while (1);
      }

      // Write some data to the device (with stop bit)
      n = writeto(I2C_ADDRESS, obuffer, sizeof(obuffer), false);
      if (n == sizeof(obuffer)) {
        Serial.printf("Successfully wrote %u bytes to the targeted device at %u\n", n, I2C_ADDRESS);
      } else {
        Serial.printf("Error writing to the targeted device at %u\n", I2C_ADDRESS);
      }
   
      // Read some data from the device (with stop bit)
      n = writeto(I2C_ADDRESS, ibuffer, sizeof(ibuffer), false);
      if (n > 0) {
        Serial.printf("Successfully read %u bytes from the targeted device at %u\n", n, I2C_ADDRESS);
      } else {
        Serial.printf("Error reading from the targeted device at %u\n", I2C_ADDRESS);
      }

      // Write and then immediately read from the device
      // Typical operation used to retrieve register contents from an I2C device
      n = writeto_then_readfrom(I2C_ADDRESS, obuffer, sizeof(obuffer),
                                             ibuffer, sizeof(ibuffer));
      if (n > 0) {
        Serial.printf("Successfully wrote to and then read %u bytes from the targeted device at %u\n", n, I2C_ADDRESS);
      } else {
        Serial.printf("Error writing to and then reading from the targeted device at %u\n", I2C_ADDRESS);
      }                                             
    }
    
    void loop() {
      // Nothing to do
    }

### Revision History

* 1.0   11/23/2021
        - Initial release with base functionality.
        - Ported from the Raspberry Pi Pico library of the same name.

* 1.1   11/22/2024
        - Updated Doxgen documentation to latest standards.
        - Added `version()` and `reldate()` methods to support
          class version checking.


