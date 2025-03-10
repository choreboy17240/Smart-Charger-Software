/**
 *  @file i2c_busio.h
 *  @brief I2C bus I/O class
 *  @details Provides a simpler interface to access the I2C buses available
 *           on the embedded controller.
 * 
 *  See the README.me file for revision history.
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */

#ifndef _I2C_BUSIO_
#define _I2C_BUSIO_

#include <Arduino.h>
#include <Wire.h>

typedef uint32_t PinNumber;                 ///< GPIO pin number

/**
 *  @brief The I2C bus I/O class provides a simpler higher-level
 *         interface to access the I2C buses available on an
 *         embedded controller.
 */
class I2C {
public:
    /**
     *  @brief Instantiates and initializes a new I2C bus I/O class
     *  @param tw: Wire device to use for accessing the I2C bus
     *  @param scl: GPIO pin number to be assigned to the SCL signal
     *  @param sda: GPIO pin number to be assigned to the SDA signal
     *  @param clock: Clock frequency (Hz) for the I2C bus (default=100K)
     */
    I2C(TwoWire *tw, PinNumber scl, PinNumber sda, uint32_t clock=100000);

    /** 
     *  @brief Deinitializes the I2C bus I/O class
     */
    void deinit(void);

    /**
     *  @brief Check connection to a device
     *  @param address: I2C address
     *  @returns true=connection is valid, false=otherwise
     *  @details Verifies that the device is connected and responds as expected.
     */
    bool connected(uint8_t address);

    /** 
     *  @brief Scan the I2C bus for attached devices
     *  @param addresses_found: Empty 128 bool array to be used for storing 
     *                          addresses found. Entries are set to true if
     *                          detected, and false otherwise.
     *  @param verbose: true=output details to Serial, false=no output
     *  @returns Number of attached devices found
     *  @note Performs 1-byte dummy read from each 7-bit I2C address. If a 
     *        slave device acknowledges the read, the corresponding array
     *        entry for the address will be set to true.  If the read is 
     *        ignored, the array entry will be set to false. I2C reserved 
     *        addressses are skipped.
     *  @note I2C reserves addresses of the form 0000xxx or 1111xxx for
     *        special purposes.
     */
    uint scan(bool *addresses_found, bool verbose=false);

    /**
     *  @brief Read from a device at the specified address into a buffer.
     *  @param address: I2C address
     *  @param buffer: Input buffer
     *  @param len: Number of bytes to read
     *  @param nostop: true=no stop bit sent, false=stop bit sent
     *  @returns Number of bytes read (0 if no response)
     */
    uint readfrom(uint8_t address,
                 uint8_t *buffer,
                 size_t len,
                 bool nostop=false);

    /**
     *  @brief Write to a device at the specified address from a buffer.
     *  @param address: I2C address
     *  @param buffer: Bytes to be written
     *  @param len: Number of bytes to be written
     *  @param nostop: true=no stop bit sent, false=stop bit sent
     *  @returns Number of bytes written (0 if failed)
     */
    uint writeto(uint8_t address,
                uint8_t *buffer,
                size_t len,
                bool nostop=false);

    /**
     *  @brief Write to a device at the specified address from a buffer, 
     *         and then immediately read from the same device into another
     *         buffer.
     *  @param address: I2C address
     *  @param out_buffer: Bytes to be written
     *  @param out_len: Number of bytes to be written
     *  @param in_buffer: Bytes to be read
     *  @param in_len: Number of bytes to be read
     *  @returns Number of bytes read (0 if failed)
     */
    uint writeto_then_readfrom(uint8_t address,
                              uint8_t *out_buffer,
                              size_t out_len,
                              uint8_t *in_buffer,
                              size_t in_len);

    /**
     *  @brief Retrieve software revision date as a string.
     *  @param buffer: Buffer to copy revision date string into (MM/DD/YYYY)
     *  @param buffer_size: Size of buffer to hold version number string
     *  @note Buffer should be at least eleven characters in size to hold
     *        the full date string (e.g. 'MM/DD/YYYY\0')
     */
    void reldate(char *buffer, size_t buffer_size);

    /**
     *  @brief Retrieve software revision number as a string.
     *  @param buffer: Buffer to copy revision number string into (x.y)
     *  @param buffer_size: Size of buffer to hold version number string
     *  @note Buffer should be at least five characters in size to hold
     *        a typical revision string (e.g. 'xx.x\0').
     */
    void version(char *buffer, size_t buffer_size);


private:
    TwoWire *i2c;                           // I2C bus
    PinNumber sda_gpio;                     // GPIO port for SDA
    PinNumber scl_gpio;                     // GPIO port for SCL
    uint32_t clock_freq;                    // Requested clock frequency

    /** 
     *  @brief Indentify I2C reserved addresses
     *  @param Address to check
     *  @returns True:it's a reserved address, False:otherwise
     *  @note I2C reserves addresses of the form 000 0xxx or 111 1xxx for
     *        special purposes.
     */
    bool reserved_addr(uint8_t addr);
};

#endif