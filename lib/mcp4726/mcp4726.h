/** 
 * @file mcp4726.h
 * @brief Arduino library for Microchip MCP4726 I2C DACs
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 * 
 * @details This library provides support for the MCP4726 12-bit DAC 
 * with I2C interface and NVM storage for configuration and value settings.
 * 
 * For revision history, see the README.md file.
 */

#ifndef _MCP4726_H_
#define _MCP4726_H_

#include <Arduino.h>
#include <i2c_busio.h>

/**
 * @brief I2C addresses for MCP4726xx part numbers
 * @details Part numbers are factory programmed to range from 0x60 to 0x67, 
 *          using suffixes ranging from A0 to A7.
 */ 
const uint8_t MCP4726_ADDRESS_A0 = 0x60;
const uint8_t MCP4726_ADDRESS_A1 = 0x61;
const uint8_t MCP4726_ADDRESS_A2 = 0x62;
const uint8_t MCP4726_ADDRESS_A3 = 0x63;
const uint8_t MCP4726_ADDRESS_A4 = 0x64;
const uint8_t MCP4726_ADDRESS_A5 = 0x65;
const uint8_t MCP4726_ADDRESS_A6 = 0x66;
const uint8_t MCP4726_ADDRESS_A7 = 0x67;

const uint8_t MCP4726_DEFAULT = MCP4726_ADDRESS_A0;

const uint8_t  MCP4726_DAC_BITS = 12;
const uint16_t MCP4726_DAC_MIN = 0;
const uint16_t MCP4726_DAC_MAX = (2 << MCP4726_DAC_BITS-1)-1; 

/**
 * @brief DAC memory contents
 */
struct dacmem_t {
    uint8_t config_vol;                     ///< Volatile config register
    uint16_t level_vol;                     ///< Volatile output level (12-bits)
    uint8_t config_nvm;                     ///< NVM config register values
    uint16_t level_nvm;                     ///< NVM output level (12-bits)
};

/**
 * Configuration register = 0bCCCVVPPG
 *
 * CCC = Command bits
 * VV  = Reference voltage control bits
 * PP  = Power-down mode control bits
 * G   = Programmable gain control bit 
 */

/// @brief Programmable Gain bit definitions
enum gain_t {
    MCP4726_GAIN_1X = 0x00,
    MCP4726_GAIN_2X = 0x01,
    MCP4726_GAIN_MASK = 0xFE
};

/// @brief Power Down Mode bit definitions
enum pwrdn_t {
    MCP4726_AWAKE = 0x00,
    MCP4726_PWRDN_1K = 0x02,
    MCP4726_PWRDN_100K = 0x04,
    MCP4726_PWRDN_500K = 0x06,
    MCP4726_PWRDN_MASK = 0xF9
};

/// @brief Reference voltage bit definitions
enum vref_t {
    MCP4726_VREF_VDD = 0x00,                ///< Vref = VDD
    MCP4726_VREF_VREFPIN = 0x10,            ///< Vref = Vref with buffering
    MCP4726_VREF_VREFPIN_BUFFERED = 0x18,   ///< Vref = Vref w/o buffering
    MCP4726_VREF_MASK = 0xE7                ///< Mask to isolate ref voltage bits
};

/// @brief Command bit definitions
enum cmd_t {
    MCP4726_CMD_VOLDAC = 0x00,              ///< Write volatile DAC register (includes power-down bits)
    MCP4726_CMD_VOLALL = 0x40,              ///< Write all volatile memory
    MCP4726_CMD_VOLCONFIG = 0x80,           ///< Write volatile configuration register
    MCP4726_CMD_ALL = 0x60,                 ///< Write all memory (volatile and EEPROM)
    MCP4726_CMD_MASK = 0x1F                 ///< Mask to isolate command bits
};

/**
 * @brief Microchip MCP4726 I2C DAC Support Library
 */
class MCP4726 {
public:
    /**
     * @brief Default constructor
     */
    MCP4726();

    /**
     * @brief Constructor with specific bus and address information
     * @param bus: I2C bus I/O object
	 * @param address: I2C bus address for MCP4726 device
     */
    MCP4726(I2C *bus, uint8_t address);

    /**
     * @brief Initialize I2C bus and address
     * @param bus: I2C bus I/O object
	 * @param address: I2C bus address for MCP4726 device
     */
    void init(I2C *bus, uint8_t address);

    /**
     * @brief Initialize MCP4726 device with stored NVM settings
     * @note We read the stored NVM settings from the MCP4726 and reinitialize
     *       the device, since a microcontroller reset doesn't necessarily
     *       restart the device.
     */
    bool begin(void);

    /**
     * @brief Initialize MCP4726 device with specified settings
     * @param config: Configuration register settings
     * @returns true=Config setting successfull, false=Error occurred
     */  
    bool begin(uint8_t config);

    /**
      * @brief Check connection to the MCP4726 device
      * @details Verify that the device is connected and responds as expected.
      * @returns true if connection is valid, false otherwise
      */
    bool connected(void);

    /**
     * @brief Check if NVM program memory is busy (i.e. in the programming cycle)
     * @returns true=Busy, false=Not busy
     */
    bool busy(void);

    /**
     * @brief Saves the current MCP4726 settings to NVM as the default
     * @returns true=Settings saved successfully, false=Error occurred
     */
    bool save_settings(void);

    /**
     * @brief Set the DAC output level
     * @param Output level (0-4095)
     * @note Setting the output level will automatically awaken the
     *       device if powered-down.
     */
    bool set_level(uint16_t level);

    /**
     * @brief Power-down the DAC and set VOUT pull-down resistor level
     * @param pwrdn: Power-down selection bit setting
     * @note Power-down bit settings:
     * - 01 = Vout loaded with 1K resistor to ground
     * - 10 = Vout loaded with 100K resistor to ground
     * - 11 = Vout loaded with 500K resistor to ground
     */
    bool power_down(uint8_t pwrdn);

    /**
     * @brief Read all of the device memory
     * @returns DAC memory structure
     * @see dacmem_t for fields
     */
    bool read_memory(dacmem_t &m);

    /**
     * @brief Write configuration to the device's volatile config register
     * @param config: Configuration data to be written
     * @returns true=Config setting successfull, false=Error occurred
     * @note This method ONLY modifies the configuration register, the DAC output
     *       level is NOT changed.
     */
    bool write_config(uint8_t config);

    /**
     * @brief Read configuration from the device's volatile config register
     * @returns Configuration data from the config register
     */
    uint8_t read_config(void);

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
	I2C *i2c_bus;		                    ///< I2C bus connected to device
	uint8_t i2c_addr;	                    ///< I2C address for device
};

#endif // _MCP4726_H_
