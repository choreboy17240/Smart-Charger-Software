/**
 *  @file regulator.h
 *  @brief Adjustable voltage regulator class
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 *
 *  @details
 *  Supports On-board Battery Charger Rev 1 hardware design with following features:
 *  @li XL6008 switching regulator with MCP4726 DAC to set voltage level
 *  @li Current and bus voltage measurement provided by INA219x sensor
 *  @li Battery/output voltage read directly via A/D channel
 *  
 *  The voltage setting is adjusted by the MCP4726 DAC output level, with a linear
 *  relationship between the DAC count and the resulting regulator output voltage.
 */
#ifndef _REGULATOR_H_
#define _REGULATOR_H_

#include <Arduino.h>
#include "obcharger.h"
#include <ina219.h>
#include <mcp4726.h>

/// @brief Adjustable voltage regulator class
class Vreg {
public:
    /// @brief Default constructor
    Vreg(void);

    /**
     * @brief Constructor with initialization
     * @param control_pin: Pin used to enable/disable regulator
     * @param sensor: INA219 current sensor object
     * @param dac: MCP4726 DAC sensor object
     */ 
    Vreg(PinNumber control_pin, INA219 *sensor, MCP4726 *dac);

    /**
     * @brief Initialize voltage regulator
     * @param control_pin: Pin used to enable/disable regulator
     * @param sensor: INA219 current sensor object
     * @param dac: MCP4726 DAC sensor object
     * @returns Nothing
     */
    void begin(PinNumber control_pin, INA219 *sensor, MCP4726 *dac);

    /**
     * @brief Get output voltage level
     * @returns Voltage in millivolts
     */
    voltage_mv_t get_voltage_mV(void);

    /**
     * @brief Set output voltage level
     * @param voltage: Voltage in millivolts
     * @returns Nothing
     */
    void set_voltage_mV(voltage_mv_t voltage);

    /**
     * @brief Get output current
     * @returns Output current in milliamps
     */
    current_ma_t get_current_mA(void);

    /**
     * @brief Get average output current
     * @returns Output current in milliamps
     */
    current_ma_t get_current_average_mA(void);

    /**
     * @brief Turn voltage regulator on
     * @returns Nothing
     */
    void on(void);

    /** 
     * @brief Turn voltage regulator off
     * @returns Nothing
     */
    void off(void);

    /**
     * @brief Check if voltage regulator is on
     * @returns true=Regulator is on, false=Regulator is off
     */
    bool is_on(void);

protected:
    PinNumber enable_port;          ///< Voltage regulator enable GPIO pin (low=disabled, high=enabled)
    INA219 *sensor = nullptr;       ///< INA219x sensor object associated with the regulator
    MCP4726 *dac = nullptr;         ///< MCP4726 DAC object associated with the regulator

    /**
     * @brief Calculate the DAC value to achieve a targeted voltage output
     * @param voltage: Target voltage setting in millivolts
     * @returns DAC value to achieve target voltage setting
     * @note Assumes a linear relationship between the DAC value and
     *       the resulting output voltage.
     */ 
    uint16_t calc_dac(voltage_mv_t voltage);
};

#endif