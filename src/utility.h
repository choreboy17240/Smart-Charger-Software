/** 
 *  @file utility.h
 *  @brief Utility functions for time period representation
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
 *  @brief Convert a time period in ms to an 'HH:MM:SS' string
 *  @param period_ms: Time period to convert (ms).
 *  @param buffer: Character buffer large enough to hold resulting ASCIIZ string.
 *  @returns ASCIIZ string representation of the time period in `HH:MM:SS` format.
 */
void ms_to_hms_str(time_ms_t period_ms, char *buffer);

#endif