/**
 * @file i2c_busio.cpp
 * @brief I2C bus I/O class
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 */

#include "i2c_busio.h"

#define VERSION         "1.1"          ///< Software revision number (x.x)
#define RELDATE         "11/22/2024"   ///< Software revision date (MM/DD/YYYY)

// Constructor with initialization
I2C::I2C(TwoWire *tw, PinNumber scl, PinNumber sda, uint32_t clock) {
    i2c = tw;
    scl_gpio = scl;
    sda_gpio = sda;
    clock_freq = clock;

    // Configure the underlying TwoWire device
    i2c->setClock(clock);
    i2c->setSCL(scl);
    i2c->setSDA(sda);
    i2c->begin();
}

// Deinitializes I2C bus I/O class
void I2C::deinit(void) {
    i2c->end();
};

// Indentify I2C reserved addresses
bool I2C::reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
};

// Check connection to a device
bool I2C::connected(uint8_t address) {
    i2c->beginTransmission(address);
    return (i2c->endTransmission() == 0);
}

// Scan I2C bus for attached devices
uint I2C::scan(bool *addresses_found, bool verbose) {
    uint number_found = 0; // Number of addresses found

    for (uint8_t addr = 0; addr < 128; addr++) {
        // Skip over any reserved addresses.
        if (reserved_addr(addr)) {
            if (verbose)
                Serial.printf("Skipping reserved I2C address 0x%02X\n", addr);
            addresses_found[addr] = false;
        } else {
            // Use return code from endTransmission() to determine
            // whether a device acknowledged at this address
            i2c->beginTransmission(addr);
            uint8_t rv = i2c->endTransmission(true);
            if (rv == 0) {
                // Found device
                if (verbose)
                    Serial.printf("0x%02X: Found device\n", addr);
                addresses_found[addr] = true;
                number_found++;
            } else if (rv == 4) {
                if (verbose)
                    Serial.printf("0x%02X: Unknown error\n", addr);
                addresses_found[addr] = false;
            } else {
                // No device found
                if (verbose)
                    Serial.printf("0x%02X: No device detected\n", addr);
                addresses_found[addr] = false;
            }
        };
    };

    return number_found;
};

// Read from a device at specified address into a buffer
uint I2C::readfrom(uint8_t address, uint8_t *buffer, size_t len, bool nostop) {
    int bytes_coming;
    int bytes_read = 0;;
    
    // Request bytes from device
    bytes_coming = i2c->requestFrom(address, len, !nostop);

    // Read the bytes returned by the device
    for (int i=0; i < bytes_coming; i++) {
        if (i2c->available()) {
            buffer[i] = i2c->read();
            bytes_read++;
        }
    }

    return bytes_read;
};

// Write to a device at specified address from a buffer
uint I2C::writeto(uint8_t address, uint8_t *buffer, size_t len, bool nostop) {
    int bytes_written;
    int rv;

    i2c->beginTransmission(address);
    bytes_written = i2c->write(buffer, len);
    rv = i2c->endTransmission(!nostop);

    if (rv == 0) {
        return bytes_written;
    } else {
        return 0;
    }
};

// Write to a device at specified address from a buffer, and then
// immediately read from the same device into a buffer
uint I2C::writeto_then_readfrom(uint8_t address,
                               uint8_t *out_buffer,
                               size_t out_len,
                               uint8_t *in_buffer,
                               size_t in_len) {
    int bytes_written;
    int bytes_coming;
    int bytes_read;
    int rv;

    // Write with no stop bit to keep control of the bus
    i2c->beginTransmission(address);
    bytes_written = i2c->write(out_buffer, out_len);
    rv = i2c->endTransmission(false);

    if (rv != 0) {
        // Transmission error, bail out
        return 0;
    }

    // Read with stop bit to complete transaction
    bytes_coming = i2c->requestFrom(address, in_len, true);

    // Read the bytes returned by the device
    for (int i=0; i < bytes_coming; i++) {
        if (i2c->available()) {
            in_buffer[i] = i2c->read();
            bytes_read++;
        }
    }

    return bytes_read;
};

// Get software revision number
void I2C::version(char *buffer, size_t buffer_size) {
  strncpy(buffer, VERSION, buffer_size);
}

// Get software revision date
void I2C::reldate(char *buffer, size_t buffer_size) {
  strncpy(buffer, RELDATE, buffer_size);
}
