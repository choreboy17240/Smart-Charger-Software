/** 
 * @file mc4726.cpp
 * @brief Arduino library for Microchip MCP4726 I2C DACs
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */
#include "mcp4726.h"

#define VERSION		"1.1"          ///< Software revision number (x.x)
#define RELDATE		"11/21/2024"   ///< Software revision date (MM/DD/YYYY)

// Default constructor
MCP4726::MCP4726() {
}

// Constructor with specific bus and address information
MCP4726::MCP4726(I2C *bus, uint8_t address) {
  init(bus, address);
}

// Initialize bus and address information
void MCP4726::init(I2C *bus, uint8_t address) {
  i2c_bus = bus; 
  i2c_addr = address;
}

// Initialize MCP4726 device with stored NVM settings
// Automatically wakes-up the DAC if in power-down mode
bool MCP4726::begin() {
  uint8_t buffer[3];
  dacmem_t m;

  if (read_memory(m)) {
    // Got current DAC NVM settings
    // Use them to build I2C command to copy them to volatile memory and clear pwrdn bits
    buffer[0] = (uint8_t)((m.config_nvm & MCP4726_PWRDN_MASK & MCP4726_CMD_MASK) | MCP4726_CMD_VOLALL);
    buffer[1] = (uint8_t)((m.level_nvm >> 4) & 0xFF);
    buffer[2] = (uint8_t)((m.level_nvm << 4) & 0xF0);
    return i2c_bus->writeto(i2c_addr, buffer, sizeof(buffer));
  } else {
    return false;
  }
}

// Initialize MCP4726 device with the specified settings
bool MCP4726::begin(uint8_t config) {
  return write_config(config);
}

// Check connection to the MCP4726 device
bool MCP4726::connected(void) {
  return i2c_bus->connected(i2c_addr);
}

// Check if NVM program memory is busy (i.e. in the programming cycle)
// B7 of the status/configuration byte read from the device indicates
// the NVM state (1=busy, 0=not busy)
bool MCP4726::busy(void) {
  return (read_config() & 0x80);
}

// Saves the current MCP4726 settings to NVM as the default
bool MCP4726::save_settings(void) {
  uint8_t buffer[3];
  dacmem_t m;

  // Wait for any in-process NVM writes to finish
  while (busy());

  if (read_memory(m)) {
    // Got current DAC volatile settings
    // Use them to build I2C command to write ALL memory (volatile and NVM)
    buffer[0] = (m.config_vol & MCP4726_CMD_MASK) | MCP4726_CMD_ALL;
    buffer[1] = uint8_t((m.level_vol >> 4) & 0xFF);
    buffer[2] = uint8_t((m.level_vol << 4) & 0xF0);
    return i2c_bus->writeto(i2c_addr, buffer, sizeof(buffer));
  } else {
    // Error getting DAC volatile settings
    return false;
  }
}

// Set DAC output level value (0-4095)
// Automatically wakes-up DAC if in the power-down state
bool MCP4726::set_level(uint16_t level) {
  uint8_t buffer[2];

  // Use Write Volatile DAC Register command to avoid changing configuration bits
  buffer[0] = (uint8_t)(MCP4726_CMD_VOLDAC | MCP4726_AWAKE | ((level >> 8) & 0xF));
  buffer[1] = (uint8_t)(level & 0xFF);
  return i2c_bus->writeto(i2c_addr, buffer, sizeof(buffer));
}

// Power-down the DAC and set VOUT pull-down resistor level
bool MCP4726::power_down(uint8_t pwrdn) {
    // Update the pwrdn bits in the volatile config register
    return write_config((read_config() & MCP4726_PWRDN_MASK) | (pwrdn & !MCP4726_PWRDN_MASK));
}

// Read all of the device memory
bool MCP4726::read_memory(dacmem_t &m) {
  uint8_t buffer[6];   // Read buffer

  if (i2c_bus->readfrom(i2c_addr, buffer, 6)) {
    // Read ok, parse buffer contents
    m.config_vol = buffer[0];
    m.level_vol = ((buffer[1] << 8) + buffer[2]) >> 4;
    m.config_nvm = buffer[3] & MCP4726_CMD_MASK;
    m.level_nvm = ((buffer[4] << 8) + buffer[5]) >> 4;
    return true;
  } else {
    // Read error
    return false;
  }
}

// Write to device config register
// Config value = 0bxxxVVPPG
bool MCP4726::write_config(uint8_t config) {
  uint8_t buffer[1];
  
  buffer[0] = (uint8_t)((config & MCP4726_CMD_MASK) | MCP4726_CMD_VOLCONFIG);
  return i2c_bus->writeto(i2c_addr, buffer, 1);
}

// Read device config register
uint8_t MCP4726::read_config(void) {
  byte buffer[6];   // Read buffer

  if (i2c_bus->readfrom(i2c_addr, buffer, 6)) {
    // Config register contents
    return buffer[0];
  } else {
    // Read error
    return 0;
  }
}

// Get software revision number
void MCP4726::version(char *buffer, size_t buffer_size) {
  strncpy(buffer, VERSION, buffer_size);
}

// Get software revision date
void MCP4726::reldate(char *buffer, size_t buffer_size) {
  strncpy(buffer, RELDATE, buffer_size);
}