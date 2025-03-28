/**
 *  @file battery.cpp
 *  @brief Battery class with methods to support voltage readings
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */

#include "battery.h"

/**
 *  @brief Conversion from ADC counts to actual measured voltage at battery terminal
 *  @details
 *  BATTERY_ADC_TO_MV = (AN_REF_VOLTAGE*(R_BATT_LO+R_BATT_HI)/(AN_READ_MAX*R_BATT_LO))
 * 
 *  Results will be rounded-up, since integer division truncates the result down.
 *  @note
 *  ADJUSTED VALUE TO CALIBRATE VOLTAGE READING WITH MEASUREMENT ACROSS
 *  BATTERY TERMINALS.
 */
const uint32_t BATTERY_ADC_TO_MV = 395;

// Default constructor (does nothing)
Battery::Battery(void) { }

// Get current battery voltage in millivolts
voltage_mv_t Battery::get_voltage_mV(void) {
    uint16_t adc_battery = analogRead(GP_AN_BATTERY);
    return ((adc_battery*BATTERY_ADC_TO_MV)/100);
}

/**
 *  @brief Number of consecutive readings to calculate an average from
 *  @note Should be a power of 2 to avoid binary division errors
 */
#define AVG_READINGS 4

voltage_mv_t Battery::get_voltage_average_mV(void) {
    uint32_t reading[AVG_READINGS];
    uint16_t min = 0xFFFF;
    uint16_t max = 0;
    uint32_t sum = 0;

    // Take consecutive voltage readings
    for (int i=0; i < AVG_READINGS; i++) {
        reading[i] = get_voltage_mV();
        if (reading[i] < min) {
            min = reading[i];
        }
        if (reading[i] > max) {
            max = reading[i];
        }
        sum += reading[i];
    }

    // Debugging information
    // Serial.printf("Battery mV: Avg %u, Min %u, Max %u\n", (sum/AVG_READINGS), min, max);

    // Return calculated average
    return (voltage_mv_t)(sum/AVG_READINGS);
}
