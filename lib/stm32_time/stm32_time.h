/**
 *  @file stm32_time.h
 *  @brief STM32 Hardware Timer-Based Alarms and Callbacks for Arduino
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 * 
 *  @details Provides alarm objects for scheduling future execution.  Alarms
 *           are added to an alarm pool, which may hold up to 'STM32_TIME_MAX_ALARMS' 
 *           (default is 16) active alarms.
 * 
 *  See README.md file for revision history.
 */

#ifndef _STM32_TIME_H_
#define _STM32_TIME_H_

#include <Arduino.h>

#define STM32_TIME_HW_TIMER         TIM3    ///< Hardware timer used by Alarm_Pool

#define STM32_TIME_MAX_ALARMS       16      ///< Maximum number of alarms in the pool

//
// Alarm functions for scheduling future execution
// Supports single pool of up to STM32_TIME_MAX_ALARMS
// Hardware timer is utilized to ensure consistent 1 ms. timebase is used
//

/**
 *  @brief The identifier for an individual alarm within the pool
 *  @ingroup alarm
 *  @note This identifier is signed because -1 is used as an error condition when creating alarms
 */
typedef int alarm_id_t;

/**
 *  @brief User alarm callback handler
 *  @param id the alarm_id as returned when the alarm was added
 *  @param user_data the user data passed when the alarm was added
 *  @returns < 0 to reschedule the same alarm from the time when it was triggered
 *  @returns > 0 to reschedule the same alarm from the time this method returns
 *  @returns 0 to not reschedule the alarm
 *  @note A class member function can't be used as an interrupt callback, since
 *        the implicit 'this' reference that tells the member function which
 *        instantation it's associated with.
 */
typedef int (*alarm_callback_t)(alarm_id_t id, void *user_data);

/**
 *  @brief Individual alarm instance created by the alarm pool.
 */
class Alarm {
public:
    /**
     *  @brief Default constructor
     */
    Alarm(void);

    /**  
     *  @brief Constructor with initialization
     *  @param user_id: ID assigned to this alarm by the pool
     *  @param period_ms: Time period (ms)
     *  @param handler: User callback handler called at the alarm time
     *  @param user_data: Optional data to be sent to user handler
     */
    Alarm(alarm_id_t user_id, uint32_t period_ms, alarm_callback_t handler, 
          void *user_data=nullptr);

    /**
     *  @brief Cancel an alarm
     *  @note Sets timer and period to zero, but alarm entry remains
     *        in the pool.  Does not call the user handler when setting
     *        the timer to zero.
     */
    void cancel(void);

    /**
     *  @brief Decrement timer count
     *  @note If it a user callback handler was provided when the alarm
     *        was created, it will be called by this method when the
     *        timer reaches zero.
     */
    void dec(void);

    /**
     *  @brief Get time remaining before alarm is triggered
     *  @returns Current time period (ms)
     */
    uint32_t get(void);

    /** 
     *  @brief Get elapsed time since the alarm was set
     *  @returns Elapsed time period (ms)
     */
    uint32_t elapsed(void);

    /**
     *  @brief Set timer period (ms)
     *  @param period_ms: Time period (ms)
     */
    void set(uint32_t period_ms);

    /**
     * @brief Set alarm ID
     * @param alarm_id: ID to be assigned by the alarm pool
     */
    void set_id(alarm_id_t alarm_id);

    /**
     *  @brief Get alarm ID
     *  @returns ID assigned by the alarm pool
     */
    alarm_id_t get_id(void);

private:
    alarm_id_t id;                  ///< ID assigned by alarm pool
    uint32_t period;                ///< Requested time period
    uint32_t timer;                 ///< Current time countdown
    alarm_callback_t handler;       ///< Callback handler
    void *data;                     ///< Optional user data
};

/**
 *  @brief Pool of of up STM32_TIME_MAX_ALARMS alarm instances
 */
class Alarm_Pool {
public:
    /** 
     *  @brief Default constructor
     */
    Alarm_Pool();

    /**
     *  @brief Alarm_Pool setup
     *  @param timx: Timer instance to be used by HardwareTimer (e.g. TIM1)
     *  @param handler: External interrupt handler to be used by HardwareTimer
     *  @note A class member function can't be used as an interrupt callback, since
     *        the implicit 'this' reference that tells the member function which
     *        instantation it's associated with.
     */
    void setup(TIM_TypeDef *timx, callback_function_t handler);

    /** 
     *  @brief Add new alarm to the pool
     *  @param period_ms: Time period (ms)
     *  @param alarm_handler: Optional handler to call when count reaches zero
     *  @param user_data: Optional data to be passed to user handler
     *  @returns >0 the alarm ID
     *  @returns -1 if there are no alarm slots available
     */
    alarm_id_t add(uint32_t period_ms, alarm_callback_t alarm_handler,
                          void *user_data=nullptr);

    /**
     *  @brief Get number of active entries in the pool
     *  @returns Number of active entries
     */
    uint32_t len(void);

    /**
     *  @brief Fetch alarm object
     *  @param id: ID number used to access alarm
     *  @returns Reference to alarm object
     *  @note Return by reference allows user to manipulate an individual
     *        alarm object easily.  Fatal error will be generated
     *        if the call tries to fetch a non-existent alarm.
     */
    Alarm& get_alarm(alarm_id_t id);

    /** 
     *  @brief Get time remaining before alarm is triggered
     *  @param id: ID number used to access alarm
     *  @returns Current time period (ms)
     */
    uint32_t get(alarm_id_t id);

    /**
     *  @brief Get elapsed time since the alarm was set
     *  @param id: ID number used to access alarm
     *  @returns Elapsed time period (ms)
     */
    uint32_t elapsed(alarm_id_t id);

    /**
     *  @brief Set timer period (ms)
     *  @param id: ID number used to access alarm
     *  @param period_ms: Time period (ms)
     */
    void set(alarm_id_t id, uint32_t period_ms);

    /**
     *  @brief Decrement all active alarm timer counts
     *  @note Used by the external interrupt handler to perform timer
     *        count-down handling.
     */
    void dec(void);

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
    bool initialized = false;            ///< Set to true by setup() method
    uint32_t entries;                    ///< Number of alarms in the pool
    Alarm pool[STM32_TIME_MAX_ALARMS];   ///< Pool of alarms
    TIM_TypeDef *hwinstance;             ///< Timer instance used by hwtimer
    HardwareTimer *hwtimer;              ///< Hardware timer object

    /** 
     *  @brief External alarm hardware timer interrupt handler function
     *  @note Calls the Alarm_Pool dec() method to decrement all active
     *        active timers.
     */
    callback_function_t hwtimer_int_handler;
};

#endif
