/**
 *  @file stm32_time.cpp
 *  @brief STM32 Hardware Timer-Based Alarms and Callbacks for Arduino
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */
#include "stm32_time.h"

#define VERSION         "1.0"          ///< Software revision number (x.x)
#define RELDATE         "11/22/2024"   ///< Software revision date (MM/DD/YYYY)

//== Alarm ====================================================================

// Default constructor
// ID with -1 value indicates an uninitialized alarm object
Alarm::Alarm(void) {      
    id = -1;             
    period = 0;
    timer = 0;
    handler = nullptr;
    data = nullptr;
}

// Constructor with initialization
Alarm::Alarm(alarm_id_t alarm_id, uint32_t period_ms, alarm_callback_t alarm_handler, 
             void *user_data) {
    id = alarm_id;
    period = period_ms;
    timer = period_ms;
    handler = alarm_handler;
    data = user_data;
}

// Cancel an alarm
// Does not call the user handler
void Alarm::cancel(void) {
    period = 0;
    timer = 0;
}

// Decrement timer count by one
// Calls the user handler if zero is reached
void Alarm::dec(void) {
    if (timer) {
        // Decrement timer
        timer--;
        // Call user handler if we reached zero now
        if ((timer == 0) & (handler != nullptr)) {
            // Capture alarm trigger time 
            uint32_t alarm_triggered = millis();
            int handler_rtn = handler(id, data);
            // Reschedule alarm if handler returns a non-zero response
            if (handler_rtn) {
                if (handler_rtn > 0) {
                    // Pos value - reschedule alarm from now
                    set(period);
                } else {
                   // Neg value - reschedule alarm from trigger time
                   set(period+millis()-alarm_triggered);
                }
            }
        }
    }
}

// Get time remaining before alarm is triggered (ms)
uint32_t Alarm::get(void) {
    return timer;
}

// Get elapsed time since start of alarm (ms)
uint32_t Alarm::elapsed(void) {
    return (period - timer);
}

// Set timer period (ms)
void Alarm::set(uint32_t period_ms) {
    timer = period_ms;
    period = period_ms;
}

// Set alarm ID
void Alarm::set_id(alarm_id_t alarm_id) {
    id = alarm_id;
}

// Get alarm ID
alarm_id_t Alarm::get_id(void) {
    return id;
}

//== Alarm Pool ===============================================================

// Define the static variables used within the Alarm_Pool class
// Required to avoid compilation errors
// uint32_t Alarm_Pool::entries = 0;               // Number of alarms in the pool
// Alarm Alarm_Pool::pool[STM32_TIME_MAX_ALARMS];  // Pool of alarms
// HardwareTimer *Alarm_Pool::hwtimer;             // Hardware timer object

// Default constructor
Alarm_Pool::Alarm_Pool(void) {
    initialized = false;
    entries = 0;
    hwinstance = nullptr;
    hwtimer = nullptr;
    hwtimer_int_handler = nullptr;
}

// Setup Alarm_Pool
void Alarm_Pool::setup(TIM_TypeDef *timx, callback_function_t handler) {

    // Create and initialize hardware timer to generate 1 ms. interrupts
    hwtimer = new HardwareTimer(timx);
    hwtimer->setOverflow(1000, MICROSEC_FORMAT);    // 1 ms.
    hwtimer->attachInterrupt(handler);
    hwtimer->resume();

    // Configure object settings
    hwinstance = timx;
    hwtimer_int_handler = handler;
    initialized = true;
}

// Add new alarm to the pool
alarm_id_t Alarm_Pool::add(uint32_t period_ms, 
                           alarm_callback_t handler, void *user_data) {
    if (entries < STM32_TIME_MAX_ALARMS) {
        // Create alarm object and return entry number as alarm ID
        Alarm *alarm = new Alarm(entries+1, period_ms, handler, user_data);
        pool[entries] = *alarm;
        entries++;
        return entries;
    } else {
        // Error, no alarm slots available in the pool
        return -1;
    }
}

// Get number of active entries in the pool
uint32_t Alarm_Pool::len(void) {
    return entries;
}

// Fetch alarm object
Alarm& Alarm_Pool::get_alarm(alarm_id_t id) {
    Alarm& s = pool[id-1];
    return s;
}

// Get time remaining before alarm is triggered (ms)
uint32_t Alarm_Pool::get(alarm_id_t id) {
    return pool[id-1].get();
}

// Get elapsed time since start of alarm (ms)
uint32_t Alarm_Pool::elapsed(alarm_id_t id) {
    return pool[id-1].elapsed();
}

// Set timer period (ms)
void Alarm_Pool::set(alarm_id_t id, uint32_t period_ms) {
    pool[id-1].set(period_ms);
}

// Decrement all active alarm timer counts
void Alarm_Pool::dec(void) {
    for (uint32_t i=0; i<entries; i++) {
        pool[i].dec();
    }
}

// Get software revision number
void Alarm_Pool::version(char *buffer, size_t buffer_size) {
  strncpy(buffer, VERSION, buffer_size);
}

// Get software revision date
void Alarm_Pool::reldate(char *buffer, size_t buffer_size) {
  strncpy(buffer, RELDATE, buffer_size);
}
