/** 
 *  @file utility.h
 *  @brief Utility functions for time period representation
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "obcharger.h"

/**
 *  @brief Time data structure (hours/minutes/seconds).
 */
typedef struct {
    uint hours;     ///< Hours
    uint mins;      ///< Minutes
    uint secs;      ///< Seconds
} hms_time_t;

/**
 *  @brief Convert a time period in ms to time in hours,minutes,seconds format.
 *  @param period_ms: Time period to convert (ms)
 *  @returns hms structure with hours, mins, secs fields
 */
hms_time_t ms_to_hms_time(time_ms_t period_ms);

/** 
 *  @brief Convert a time period in ms to an ASCII string representation with 
 *         'appropriate' resolution.
 *  @param period_ms: Time period to convert (ms).
 *  @param buffer: Character buffer large enough to hold resulting ASCIIZ string.
 *  @returns ASCIIZ string representation of the time period
 *  @note
 *  If the time period is less than 100 hours, the time period will be displayed
 *  in `HH:MM:SS` format. If at or more than 100 hours, the resolution will be
 *  reduced to minutes, and the time period will be displayed in `HHH:MM  `
 *  format. This ensures that we have adequate space on fixed-width displays
 *  to show the elapsed time.  Since 100 hours is 41 days, this resolution
 *  should be sufficient for this application.
 */
void ms_to_hms_str(time_ms_t period_ms, char *buffer);

#endif