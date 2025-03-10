/**
 * @file battery.h
 * @brief Battery class with methods to support voltage readings
 * 
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
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


/**
 *  @brief Battery class with methods to support voltage readings
 */
class Battery {
public:
    /// @brief Default constructor
    Battery(void);

    /**
     *  @brief Get battery voltage (mV)
     *  @returns Battery voltage in mV
     */
    voltage_mv_t get_voltage_mV(void);

    /**
     *  @brief Get average battery voltage (mV)
     *  @returns Battery voltage in mV
     *  @note Takes four consecutive readings and returns the average
     *        of those readings to smooth-out fluctuations.
     */
    voltage_mv_t get_voltage_average_mV(void);

};

#endif