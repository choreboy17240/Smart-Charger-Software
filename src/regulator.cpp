/**
 *  @file regulator.cpp
 *  @brief Adjustable voltage regulator class
 */

#include "regulator.h"
#include "battery.h"

// Global variables
extern Battery battery;

// Default constructor
Vreg::Vreg(void) {
}

// Constructor with initialization
Vreg::Vreg(PinNumber control_pin, INA219 *sensor, MCP4726 *dac) {
    // Initialize regulator, saves the interface objects
    begin(control_pin, sensor, dac);
}

// Initialize voltage regulator
void Vreg::begin(PinNumber control_pin, INA219 *sensor, MCP4726 *dac) {

    // Save underlying current sensor and DAC objects
    Vreg::sensor = sensor;
    Vreg::dac = dac;

    // Configure GPIO port to enable/disable voltage regulator (default=off)
    // Low: regulator off, High: regulator on
    Vreg::enable_port = control_pin;
    digitalWrite(control_pin, LOW);
    pinMode(control_pin, OUTPUT);

    // Configure INA219 voltage/current sensor
    if (sensor->connected()) {
        // Complete setup of the sensor
        sensor->reset();
        sensor->set_bus_range(RANGE_32V);
        sensor->set_bus_ADC_resolution(BUS_RES_12BIT);
        sensor->set_shunt_ADC_resolution(SHUNT_RES_12BIT);
        sensor->set_PGA_gain(GAIN_8_320MV);
        sensor->set_calibration(INA219_CAL, INA219_ILSB, INA219_PLSB);
        sensor->set_operation_mode(SANDBVOLT_CONTINUOUS);
    } else {
        // Fatal error
        Serial.printf("Error: INA219B sensor is not responding!\n");
        while (1);
    }

    // Configure MCP4726 DAC
    if (dac->connected()) {
        dac->begin(MCP4726_AWAKE | MCP4726_VREF_VDD | MCP4726_GAIN_1X);
        dac->set_level(4095);  // Minimum voltage level
    } else {
        // Fatal error
        Serial.printf("Error: MCP4726 DAC is not responding!\n");
        while (1);
    }
}

// Get output voltage level
voltage_mv_t Vreg::get_voltage(void) {
    // Output voltage is only meaningful if the voltage regulator is on.
    // Returns 0 to avoid reading bogus voltage levels.
    if (is_on())
        return (voltage_mv_t)(sensor->get_bus_voltage_mV());
    else
        return 0;
}

// Set output voltage level
void Vreg::set_voltage(voltage_mv_t voltage) {
    voltage_mv_t sv;

    // Apply voltage limits to requested voltage
    if (voltage < VREG_VOLTAGE_MIN) {
        sv = VREG_VOLTAGE_MIN;
    } else if (voltage > VREG_VOLTAGE_MAX) {
        sv = VREG_VOLTAGE_MAX;
    } else {
        sv = voltage;
    };

    // Set DAC level to achieve requested voltage
    // USING CALCULATION SINCE LINEAR RELATIONSHIP EXISTS
    uint16_t dac_setting = calc_dac(sv);
    dac->set_level(dac_setting);
}

// Get output current
current_ma_t Vreg::get_current_mA(void) {
    if (sensor->get_bus_voltage_mV() > (battery.get_voltage_average_mV() + 250)) {
        // Normal condition
        return sensor->get_current_mA();
    } else {
        // If the regulator output voltage is not 300-400 mV or more above the
        // battery voltage (i.e. the drop across the schottky diode), the 
        // charging current will be zero.  Force that result to avoid returning
        // spurious current readings caused by noise at the INA219 inputs.
        return 0;
    }
}

/**
 *  @brief Number of consecutive readings to calculate an average from
 *  @note Should be a power of 2 to avoid binary division errors
 */
#define AVG_READINGS 4

// Get average output current over AVG_READINGS
current_ma_t Vreg::get_current_average_mA(void) {
    uint32_t reading[AVG_READINGS];
    uint32_t min = 0xFFFFFFFF;
    uint32_t max = 0;
    uint32_t sum = 0;

    // Take consecutive voltage readings
    for (int i=0; i < AVG_READINGS; i++) {
        reading[i] = get_current_mA();
        if (reading[i] < min) {
            min = reading[i];
        }
        if (reading[i] > max) {
            max = reading[i];
        }
        sum += reading[i];
    }

    // Debugging information
    // Serial.printf("Charging mA: Avg %u, Min %u, Max %u\n", (sum/AVG_READINGS), min, max);

    // Return calculated average
    return (current_ma_t)(sum/AVG_READINGS);
}

// Turn voltage regulator on
void Vreg::on(void) {
    digitalWrite(enable_port, HIGH);
}

// Turn voltage regulator off
void Vreg::off(void) {
    digitalWrite(enable_port, LOW);
}

// Check if voltage regulator is on
bool Vreg::is_on(void) {
    if (digitalRead(enable_port)) {
        return true;
    } else {
        return false;
    }
}

// Get DAC value to achieve targeted voltage output
uint16_t Vreg::calc_dac(voltage_mv_t voltage) {
    // Inverse linear relationship between voltage and DAC value
    return map(voltage, VREG_VOLTAGE_MIN, VREG_VOLTAGE_MAX, MCP4726_DAC_MAX, MCP4726_DAC_MIN);
}
