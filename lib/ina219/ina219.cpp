/**
 *  @file ina219.cpp
 *  @brief INA219 Current/Power Sensor Class Library
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */
#include "ina219.h"

#define VERSION		"1.1"          ///< Software revision number (x.x)
#define RELDATE		"11/20/2024"   ///< Software revision date (MM/DD/YYYY)

// Default constructor
INA219::INA219() {
}

// Initializator
bool INA219::init(I2C *i2c_bus, uint8_t address) {
	INA219::i2c_bus = i2c_bus;
	INA219::i2c_addr = address;

	// Check I2c connection
	if (connected()) {
		// Initialize calibration register
		set_calibration(INA219_CAL);
		return true;
	} else {
		// Couldn't connect to sensor
		return false;
	}
}

// Check I2C connection to the INA219 sensor
bool INA219::connected() {
	uint8_t rxdata;	// Read buffer

	// Perform 1-byte dummy read from the sensor
 	if (i2c_bus->readfrom(i2c_addr, &rxdata, 1, false))
		return true;
	else
		return false;
}

// Performs soft reset of the INA219 sensor
void INA219::reset() {
	write_register(INA219_CONFIG_REG, INA219_RESET);
}

// Read the overflow flag from the bus voltage A/D register
bool INA219::overflow() {
	uint16_t val = read_register(INA219_BUS_REG);
	return (val & 1);
}

// Set the max range for bus voltage A/D
void INA219::set_bus_range(INA219_BUSVRANGE range) {
	uint16_t current_config_reg = read_register(INA219_CONFIG_REG);
	current_config_reg &= ~(INA219_CONFIG_BVOLTAGERANGE_MASK);
	current_config_reg |= range;
	write_register(INA219_CONFIG_REG, current_config_reg);
	_range = range;
}

// Set programmable gain level (1/2/4/8X)
void INA219::set_PGA_gain(INA219_PGA_GAIN gain) {
	uint16_t current_config_reg = read_register(INA219_CONFIG_REG);
	current_config_reg &= ~(INA219_PGA_GAIN_MASK);
	current_config_reg |= gain;
	write_register(INA219_CONFIG_REG, current_config_reg);
	_gain = gain;
}

// Set the operation mode to continuous or triggered
// for shunt voltage, bus voltage or both
void INA219::set_operation_mode(INA219_OPERATION_MODE mode) {
	uint16_t current_config_reg = read_register(INA219_CONFIG_REG);
	current_config_reg &= ~(INA219_CONFIG_MODE_MASK);
	current_config_reg |= mode;
	write_register(INA219_CONFIG_REG, current_config_reg);
	_opmode = mode;
}

// Start single measurement cycle
// Blocks until completion of the measurement (conversion bit set)
void INA219::start_single_measurement() {
	set_operation_mode(SANDBVOLT_TRIGGERED);
	uint16_t conversion_ready = 0x0000;
	while (!conversion_ready) {
		// Measurement is ready when conversion bit is set
		conversion_ready = ((read_register(INA219_BUS_REG)) & 0x0002);
	}
}

// Get the calibration value for the current and power register
uint16_t INA219::get_calibration(void) {
	_calibration = read_register(INA219_CALIBRATION_REG);
	return _calibration;
}

// Set the calibration value for the current and power register
void INA219::set_calibration(uint16_t calibration_value,
											 				uint16_t i_lsb,
											 				uint16_t pwr_lsb) {
	// Program calibration register and cache value
	write_register(INA219_CALIBRATION_REG, calibration_value);
	_calibration = calibration_value;

	// Set scaling factors for easy conversion
	_current_divider_mA = 1000U/i_lsb;
	_power_multiplier_uW = pwr_lsb;
}

// Set the resolution or the number of samples averaged
// for the bus voltage register
void INA219::set_bus_ADC_resolution(INA219_BUS_ADC_RES resolution) {
	uint16_t current_config_reg = read_register(INA219_CONFIG_REG);
	current_config_reg &= ~(INA219_BADCRES_MASK);
	current_config_reg |= resolution;
	write_register(INA219_CONFIG_REG, current_config_reg);
	_bus_res = resolution;
}

// Set the resolution or the number of samples averaged
// for the shunt voltage register
void INA219::set_shunt_ADC_resolution(INA219_SHUNT_ADC_RES resolution) {
	uint16_t current_config_reg = read_register(INA219_CONFIG_REG);
	current_config_reg &= ~(INA219_CONFIG_SADCRES_MASK);
	current_config_reg |= resolution;
	write_register(INA219_CONFIG_REG, current_config_reg);
	_shunt_res = resolution;
}

// Get bus voltage register value (LSB = 4 mV)
uint16_t INA219::get_bus_voltage_raw() {
	uint16_t current_reg_value = read_register(INA219_BUS_REG);
	// Shift register contents right by 3 bits to drop  the CNVR 
	// (conversion ready) and OVF (Overflow) bits
	return (current_reg_value >> 3);
}
 
// Get bus voltage in millivolts
uint32_t INA219::get_bus_voltage_mV() {
	// Multiply by LSB value (LSB = 4 mV)
	return (get_bus_voltage_raw() * 4);
}

// Get shunt voltage register value (LSB = 10uV)
int16_t INA219::get_shunt_voltage_raw() {
	return read_register(INA219_SHUNT_REG);
}

// Read shunt voltage in microvolts
uint32_t INA219::get_shunt_voltage_uV() {
	return (get_shunt_voltage_raw()*10);
}

// Read shunt voltage in millivolts
uint32_t INA219::get_shunt_voltage_mV() {
	return (get_shunt_voltage_raw()/100);
}

// Read shunt current register value
// Returns zero if the calibration register value has not been set
uint16_t INA219::get_current_raw() {
	uint16_t current_raw;

	// Handle missing calibration value
	if (_calibration  == 0) {
		Serial.printf("Warning: Calibration value has not been set (value=0)\n");
		return 0;
	}

	// To avoid risk of device reset during sharp load, re-apply
	// the calibration as a precaution
	write_register(INA219_CALIBRATION_REG, _calibration);

	current_raw = read_register(INA219_CURRENT_REG);
	return current_raw;
}

// Get current in milliamperes (mA)
// Current is calculated based on LSB and the calibration value
// Returns value of 0 if the calibration register has not been set
uint32_t INA219::get_current_mA() {
	// Read current register and calculate current value
	uint16_t current_reg_value = get_current_raw();
	if (current_reg_value) {
		return current_reg_value/_current_divider_mA;
	} else {
		// Calibration register has not been set
		return 0;
	}
}

// Get current in microamperes (uA)
// Current is calculated based on LSB and the  calibration value
// Returns value of 0 if calibration register has not been set
uint32_t INA219::get_current_uA() {
	return get_current_mA() * 1000;
}

// Read calculated power in milliwatts
// Calculated based on LSB and calibration value
// Returns 0 if the calibration register has not been set
uint32_t INA219::get_power_mW() {
	// Handle missing calibration
	if (!_calibration) {
		return 0;
	}

	// Risk of device reset during sharp load, re-apply the calibration by precaution
	write_register(INA219_CALIBRATION_REG, _calibration);

	// Read power register and calculate power value
	return (read_register(INA219_POWER_REG) * _power_multiplier_uW)/1000U;
}

// Get software revision number
void INA219::version(char *buffer, size_t buffer_size) {
  strncpy(buffer, VERSION, buffer_size);
}

// Get software revision date
void INA219::reldate(char *buffer, size_t buffer_size) {
  strncpy(buffer, RELDATE, buffer_size);
}

///==== Private methods =======================================================

// Reads 16-bit register value from sensor register
// Value is read as two bytes in MSB, LSB order
uint16_t INA219::read_register(uint8_t reg_addr) {
	uint8_t buffer_in[2];
	uint8_t buffer_out[1];

	// Set register pointer
	buffer_out[0] = reg_addr;
	i2c_bus->writeto(i2c_addr, buffer_out, 1, false);

	// Read two-byte register
	i2c_bus->readfrom(i2c_addr, buffer_in, 2, false);
	return ((buffer_in[0] << 8) + buffer_in[1]);
}


// Writes 16-bit value to sensor register
// Value is transmitted as two bytes in MSB, LSB order
void INA219::write_register(uint8_t reg_addr, uint16_t val) {
	uint8_t buffer_out[3] = { reg_addr, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };

	// Write to indicated register pointer
	i2c_bus->writeto(i2c_addr, buffer_out, 3, false);
}
