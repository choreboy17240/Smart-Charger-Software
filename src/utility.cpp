/** 
 *  @file utility.cpp
 *  @brief Utility functions for time period representation
 */
#include "utility.h"
#include <stdio.h>

// Converts time period in ms to an [hours,minutes,seconds] struct
hms_time_t ms_to_hms_time(time_ms_t period_ms) {
    uint hours = period_ms / HOUR_MS;
    uint mins  = (period_ms - hours * HOUR_MS) / MINUTE_MS;
    uint secs  = (period_ms - hours * HOUR_MS - mins * MINUTE_MS) / SECOND_MS;
    return hms_time_t { hours, mins, secs };
}


// Converts timer period in ms to a 'HH:MM:SS' or 'HHH:MM' string,
// depending on the length of the period.
void ms_to_hms_str(time_ms_t period_ms, char *buffer) {
    hms_time_t hms = ms_to_hms_time(period_ms);
    if (hms.hours < 100) {
        // High-resolution time period (HH:MM:SS)
        int n = sprintf(buffer, "%02u:%02u:%02u", hms.hours, hms.mins, hms.secs);
    } else {
        // Low-resolution time period (HHH:MM)
        int n = sprintf(buffer, "%03u:%02u", hms.hours, hms.mins);
    }
}

