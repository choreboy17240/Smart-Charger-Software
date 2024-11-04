/**
 * @file battery.h
 *
 * @brief Battery-related utility functions
 */
#ifndef _BATTERY_H_
#define _BATTERY_H_

#include "obcharger.h"

/* 
 * Define constant ratio to allow conversion of battery A/D count to voltage
 * in microvolts.
 *
 * Note: Using this microvolt conversion constant and dividing the result by 
 * 100 to get millivolts helps preserve the accuracy of the integer math.
 * 
 * Example for battery voltage in millivolts:
 * 
 *   Vbatt = ((ADC Count)*BATTERY_ADC_TO_UV)/100;
 */

/// @brief Battery class
class Battery {
public:
    /// @brief Default constructor
    Battery(void);

    /// @brief Get battery voltage (mV)
    /// @returns Battery voltage in mV
    voltage_mv_t get_voltage_mV(void);

    /// @brief Get battery voltage (V)
    /// @returns Voltage in Q16.16 fixed-point format
    FXPTQ1616 get_voltage(void);

private:

};

#endif