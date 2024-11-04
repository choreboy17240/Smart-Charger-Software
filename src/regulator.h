/// @file regulator.h
//
/// @mainpage Voltage regulator class
/// Supports On-board Battery Charger Rev 1 hardware design with following features:
/// @li XL6008 switching regulator with MCP4726 DAC to set voltage level
/// @li Current and bus voltage measurement provided by INA219B sensor
/// @li Battery/output voltage read directly via A/D channel
/// 
/// Voltage control formula:
/// <<< DOCUMENT THIS HERE OR IN CONOPS DOCUMENT >>>
///
#ifndef _REGULATOR_H_
#define _REGULATOR_H_

#include <Arduino.h>
#include "obcharger.h"
#include <ina219.h>
#include <mcp4726.h>

/// @brief Voltage regulator class
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
     */
    void begin(PinNumber control_pin, INA219 *sensor, MCP4726 *dac);

    /**
     * @brief Get output voltage level
     * @returns Voltage in millivolts
     */
    voltage_mv_t get_voltage(void);

    /**
     * @brief Set output voltage level
     * @param voltage: Voltage in millivolts
     */
    void set_voltage(voltage_mv_t voltage);

    /**
     * @brief Get output current
     * @returns Output current in milliamps
     */
    current_ma_t get_current_mA(void);

    /**
     * @brief Turn voltage regulator on
     */
    void on(void);

    /** 
     * @brief Turn voltage regulator off
     */
    void off(void);

    /**
     * @brief Check if voltage regulator is on
     * @returns true=Regulator is on, false=Regulator is off
     */
    bool is_on(void);

private:
    PinNumber enable_port;
    INA219 *sensor = nullptr;
    MCP4726 *dac = nullptr;

    /**
     * @brief Calculate the DAC value to achieve a targeted voltage output
     * @param voltage: Target voltage setting in millivolts
     * @returns DAC value to achieve target voltage setting
     * @note Assumes a linear relationship between the DAC value and
     *       the resulting output voltage.  Use lookup_dac with a table
     *       if non-linear relationship exists.
     */ 
    uint16_t calc_dac(voltage_mv_t voltage);

    // Regulator output voltage may be non-linear
    // Use an interpolation table with measured values to approximate the 
    // set points requested by user
    // #define ITABLE_SIZE     13U
    // uint16_t const vout_targets[ITABLE_SIZE] = { 10000, 10500,
    //                               11000, 11500,
    //                               12000, 12500, 
    //                               13000, 13500, 
    //                               14000, 14500,
    //                               15000, 15500,
    //                               16000 };
    // uint16_t const dac_values[ITABLE_SIZE] = { 5000, 4540,
    //                               4010, 3540, 
    //                               3120, 2765, 
    //                               2440, 2140, 
    //                               1880, 1640, 
    //                               1420, 1220, 
    //                               1040 };

    // /**
    //  * @brief Get DAC value to achieve a targeted voltage output
    //  * @param voltage: Target voltage setting in millivolts
    //  * @returns DAC value to achieve target voltage setting
    //  * @note This is a more complicated table-driven approach to deriving
    //  *       the output voltage, for cases where a non-linear relationship
    //  *       exists between the DAC value and resulting output voltage.
    //  *       Use calc_dac() when possible as a simpler solution. 
    //  */
    // uint16_t lookup_dac(uint16_t voltage);
};

#endif