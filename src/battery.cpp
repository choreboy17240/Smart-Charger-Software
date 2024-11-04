/**
 * @file battery.cpp
 *
 * @brief Battery-related utility functions
 */

#include "battery.h"

// Conversion from ADC counts to actual measured voltage at battery terminal
// BATTERY_ADC_TO_MV = (AN_REF_VOLTAGE*(R_BATT_LO+R_BATT_HI)/(AN_READ_MAX*R_BATT_LO))
// Results rounded-up, since integer division truncates the result down.
// ADJUSTED VALUE TO CALIBRATE VOLTAGE READING WITH MEASUREMENT ACROSS
// BATTERY TERMINALS.
const uint32_t BATTERY_ADC_TO_MV = 395;

// Default constructor (does nothing)
Battery::Battery(void) { }

// Get current battery voltage in millivolts
voltage_mv_t Battery::get_voltage_mV(void) {
    uint16_t adc_battery = analogRead(GP_AN_BATTERY);
    return ((adc_battery*BATTERY_ADC_TO_MV)/100);
}


// Get current battery voltage in Q16.16 fixed-point format
// FIXME: WILL COMPILER USE COPY CONSTRUCTOR TO "PASS" FXPTQ1616 BACK?
FXPTQ1616 Battery::get_voltage(void) {
    uint32_t voltage_mV = get_voltage_mV();
    return FXPTQ1616(voltage_mV);
}
