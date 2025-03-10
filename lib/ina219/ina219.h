/**
 *  @file ina219.h
 *  @brief INA219 Current/Power Sensor Class Library
 *
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 * 
 *  @details This library provides support for interfacing with a TI INA219x
 *  current/power sensor through the I2C bus.
 * 
 *  See the README.md file for revision history.
 */

#ifndef _LIB_INA219_
#define _LIB_INA219_

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <i2c_busio.h>

//
// INA219 register addresses
//
#define INA219_CONFIG_REG 			0x00	///< Configuration Register address
#define INA219_SHUNT_REG    		0x01 	///< Shunt Voltage Register address
#define INA219_BUS_REG    			0x02 	///< Bus Voltage Register address
#define INA219_POWER_REG     		0x03 	///< Power Register address
#define INA219_CURRENT_REG 			0x04 	///< Current flowing through Shunt address
#define INA219_CALIBRATION_REG	0x05 	///< Calibration Register address

//
// Default calibration values
// Assumes 0.1 ohm shunt, 32V bus voltage, PGA=/8, 40uA current LSB
//
const uint16_t INA219_CAL = 10240;		///< 0.1 ohm shunt, 32V bus, PGA=/8, 40uA LSB
const uint16_t INA219_ILSB = 40;			///< Shunt current LSB in uAmp
const uint16_t INA219_PLSB = 20*INA219_ILSB;	///< Power LSB in uWatts

//
// Configuration register manipulation
//

/// @brief Bit mask for initiating a soft reset of the INA219 device
#define INA219_RESET   0x8000

/// @brief Bit mask for isolating the bus voltage range bits
#define INA219_CONFIG_BVOLTAGERANGE_MASK 0x2000

/// Values for setting the bus voltage range
enum INA219_BUSVRANGE {
	RANGE_16V = 0x0000, ///< 0-16V Range
	RANGE_32V = 0x2000, ///< 0-32V Range
};

/// @brief Bit mask for isolating the programmable gain bits
#define INA219_PGA_GAIN_MASK 0x1800 // Gain Mask

/// @brief Values for setting the programmable gain bits
enum INA219_PGA_GAIN {
	GAIN_1_40MV = 0x0000,  ///< Gain 1, 40mV Range
	GAIN_2_80MV = 0x0800,  ///< Gain 2, 80mV Range
	GAIN_4_160MV = 0x1000, ///< Gain 4, 160mV Range
	GAIN_8_320MV = 0x1800, ///< Gain 8, 320mV Range
};

/// @brief Bit mask for isolating the bus ADC resolution bits
#define INA219_BADCRES_MASK 0x0780

/// @brief Values for setting bus ADC resolution
enum INA219_BUS_ADC_RES {
	BUS_RES_9BIT = 0x0000, 	///< 9-bit bus res = 0..511
	BUS_RES_10BIT = 0x0080, ///< 10-bit bus res = 0..1023
	BUS_RES_11BIT = 0x0100, ///< 11-bit bus res = 0..2047
	BUS_RES_12BIT = 0x0180, ///< 12-bit bus res = 0..4097
	BUS_RES_2S = 0x0480, 		///< 2 x 12-bit bus samples averaged together
	BUS_RES_4S = 0x0500, 		///< 4 x 12-bit bus samples averaged together
	BUS_RES_8S = 0x0580, 		///< 8 x 12-bit bus samples averaged together
	BUS_RES_16S = 0x0600, 	///< 16 x 12-bit bus samples averaged together
	BUS_RES_32S = 0x0680, 	///< 32 x 12-bit bus samples averaged together
	BUS_RES_64S = 0x0700, 	///< 64 x 12-bit bus samples averaged together
	BUS_RES_128S = 0x0780, 	///< 128 x 12-bit bus samples averaged together
};

/// @brief Bit mask for isolating the shunt ADC resolution and averaging control bits
#define INA219_CONFIG_SADCRES_MASK 0x0078

/// @brief Values for setting shunt ADC resolution
enum INA219_SHUNT_ADC_RES{
	SHUNT_RES_9BIT = 0x0000, 	///< 1 x 9-bit shunt sample
	SHUNT_RES_10BIT = 0x0008, 	///< 1 x 10-bit shunt sample
	SHUNT_RES_11BIT = 0x0010, 	///< 1 x 11-bit shunt sample
	SHUNT_RES_12BIT = 0x0018,	///< 1 x 12-bit shunt sample
	SHUNT_RES_2S = 0x0048, 		///< 2 x 12-bit shunt samples averaged together
	SHUNT_RES_4S = 0x0050, 		///< 4 x 12-bit shunt samples averaged together
	SHUNT_RES_8S = 0x0058, 		///< 8 x 12-bit shunt samples averaged together
	SHUNT_RES_16S = 0x0060, 	///< 16 x 12-bit shunt samples averaged together
	SHUNT_RES_32S = 0x0068, 	///< 32 x 12-bit shunt samples averaged together
	SHUNT_RES_64S = 0x0070, 	///< 64 x 12-bit shunt samples averaged together
	SHUNT_RES_128S = 0x0078, 	///< 128 x 12-bit shunt samples averaged together
};

/// @brief Bit mask for isolating the operating mode bits
#define INA219_CONFIG_MODE_MASK 0x0007

/// @brief Values for checking and configuring the operating mode
enum INA219_OPERATION_MODE {
	POWERDOWN = 0x00, 						///< power down
	SVOLT_TRIGGERED = 0x01, 			///< shunt voltage triggered
	BVOLT_TRIGGERED = 0x02, 			///< bus voltage triggered
	SANDBVOLT_TRIGGERED = 0x03, 	///< shunt and bus voltage triggered
	ADCOFF = 0x04, 								///< ADC off
	SVOLT_CONTINUOUS = 0x05, 			///< shunt voltage continuous
	BVOLT_CONTINUOUS = 0x06, 			///< bus voltage continuous
	SANDBVOLT_CONTINUOUS = 0x07,	///< shunt and bus voltage continuous
};

/**
 *  @brief Class for interfacing with a TI INA219x current/power sensor.
 */
class INA219 {
public:
	/**
	 *  @brief Default constructor
	 */
	INA219();

	//
	// Initialization methods
	//

	/**
	 *  @brief Initialize INA219 object and connect to the physical sensor.
	 *  @param i2c_bus: I2C bus I/O object
	 *  @param address: I2C bus address for physical sensor
	 *  @returns true=initialized successfully, false=initialization failed.
	 *  @sa I2C_BUSIO library.
	 */
	bool init(I2C *i2c_bus, uint8_t address = 0x40);

	/**
	 *  @brief Performs a soft reset of the INA219 sensor.
	 */
	void reset();

	/**
	 *  @brief Check the I2C connection to the INA219 sensor.
	 *  @returns true if connection is valid, false otherwise
	 *  @details Verifies that the device is connected via the I2C bus and
	 *           that it responds as expected.
	 */
	bool connected(void);

	//
	// Configuration methods
  //

	/**
	 *  @brief Set the resolution or the number of samples averaged
	 *        for the bus voltage register
	 *  @param resolution: `INA219_BUS_ADC_RES` enum value.
	 *  @sa INA219_BUS_ADC_RES
	 */
	void set_bus_ADC_resolution(INA219_BUS_ADC_RES resolution);

	/**
	 *  @brief Set the resolution or the number of samples averaged
	 *         for the shunt voltage register
	 *  @param resolution: `INA219_SHUNT_ADC_RES` enum value.
	 *  @sa INA219_SHUNT_ADC_RES enum.
	 */
	void set_shunt_ADC_resolution(INA219_SHUNT_ADC_RES resolution);

	/**
	 *  @brief Set operation mode
	 *  @param mode: `INA219_OPERATION_MODE` enum value.
	 *  @note Settings include continuous or triggered reads
	 *        and can apply to bus voltage, shunt voltage, or both.
	 *  @sa INA219_OPERATION_MODE enum.
	 */
	void set_operation_mode(INA219_OPERATION_MODE mode);

	/**
	 *  @brief Set programmable gain level (1,2,4, or 8x)
	 *  @param gain: `INA219_PGA_GAIN` enum value.
	 *  @sa INA219_PGA_GAIN enum.
	 */
	void set_PGA_gain(INA219_PGA_GAIN gain);

	/**
	 *  @brief Set bus voltage range
	 *  @param range: `INA219_BUSVRANGE` enum value.
	 *  @sa INA219_BUSVRANGE enum.
	 */
	void set_bus_range(INA219_BUSVRANGE range);

	/** 
	 *  @brief Get the calibration value for the current and power register.
	 *  @returns Calibration value read from sensor.
	 */
	uint16_t get_calibration(void);

	/**
	 *  @brief Set the calibration value for the current and power register
	 *  @param calibration_value: Constant value to calibrate sensor readings
	 *  @param i_lsb: Current register LSB in microAmps
	 *  @param pwr_lsb: Power register LSB in microWatts
	 *  @sa INA219_ILSB and INA219_PLSB default values.
	 */
	void set_calibration(uint16_t calibration_value, 
											 uint16_t i_lsb = INA219_ILSB,
											 uint16_t pwr_lsb = INA219_PLSB);

	//
	// Usage methods
	//

	/**
	 *  @brief Get raw bus voltage register value (LSB = 4 mv/count).
	 *  @returns 12-bit A/D register reading for bus voltage.
	 */
	uint16_t get_bus_voltage_raw(void);

	/**
	 *  @brief Get bus voltage level
	 *  @returns Bus voltage in millivolts.
	 */
	uint32_t get_bus_voltage_mV(void);

	/** 
	 *  @brief Get shunt voltage register value (LSB = 0.01mV/count)
	 *  @returns 16-bit shunt voltage register value.
	 */
	int16_t get_shunt_voltage_raw();

	/**
	 *  @brief Get shunt voltage level in microvolts.
	 *  @returns Shunt voltage in microvolts.
	 */
	uint32_t get_shunt_voltage_uV(void);

	/**
	 *  @brief Get shunt voltage level in millivolts.
	 *  @returns Shunt voltage in millivolts.
	 */
	uint32_t get_shunt_voltage_mV(void);

	/**
	 *  @brief Get shunt current level in milliamperes.
	 *  @returns Shunt current in milliamperes.
	 * 	@note The calibration register must be programmed before
	 *        trying to read current or power from the INA219 sensor.
	 */
	uint32_t get_current_mA(void);

	/** 
	 *  @brief Get shunt current level in microamperes.
	 *  @returns Shunt current in microamperes.
	 * 	@note The calibration register must be programmed before
	 *        trying to read current or power from the INA219 sensor.
	 */
	uint32_t get_current_uA(void);

	/** 
	 *  @brief Get shunt current register value.
	 *  @returns Shunt current register reading.
	 */
	uint16_t get_current_raw();

	/** 
	 *  @brief Get power level
	 *  @returns Power in milliwatts (mW)
	 *  @note The calibration register must be programmed before
	 *        trying to read current or power from the INA219 sensor.
	 */
	uint32_t get_power_mW(void);

	/** 
	 *  @brief Read the overflow flag from the bus voltage A/D register.
	 *  @returns true=overflow flag was set, false=overflow flag was clear.
	 */
	bool overflow(void);

	/** 
	 *  @brief Start single measurement cycle of shunt and bus voltages.
	 *  @note This method blocks until completion of the measurement 
	 *        (i.e. when the conversion bit is set).
	 */
	void start_single_measurement(void);

	/** 
	 *  @brief Shunt resistor value in milliohms (default = 100)
	 */
	uint32_t r_shunt = 100;

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
	I2C *i2c_bus;											// I2C bus connected to device
	uint8_t i2c_addr;									// I2C address for device

	// Cached configuration settings
	INA219_OPERATION_MODE _opmode;  	// Cached operating mode
	INA219_BUSVRANGE _range;					// Cached bus voltage range
	INA219_BUS_ADC_RES _bus_res;	  	// Cached bus voltage resolution
	INA219_SHUNT_ADC_RES _shunt_res;	// Cached shunt voltage resolution
	INA219_PGA_GAIN _gain;						// Cached PGA gain

	uint16_t _calibration; 						// Cached calibration register value
	uint16_t _current_divider_mA;			// Divide current register value to get mA
	uint16_t _power_multiplier_uW;		// Multiply power register value to get uW

	/**
	 * @brief Writes 16-bit value to a device register.
	 *        The value is transmitted as two bytes in MSB, LSB order.
	 * @param reg_addr: Register address
	 * @param val: Value to be written
	 */
	void write_register(uint8_t reg_addr, uint16_t val);

	/** @brief Reads 16-bit value from a device register.
	 *  		   The value is read as two bytes in MSB, LSB order.
	 *  @param reg_addr: Register address
	 *  @returns Value read from register
	 */
	uint16_t read_register(uint8_t reg_addr);
};

#endif

