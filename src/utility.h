/// @file utility.h
//
// Utility functions
//
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "obcharger.h"

typedef struct {
    uint hours;
    uint mins;
    uint secs;
} hms_time_t;

/// @brief Converts time period in ms to an [hours,minutes,seconds] struct
/// @param period: Time period to convert (ms)
/// @returns hms structure with hours, mins, secs fields
hms_time_t ms_to_hms_time(time_ms_t period_ms);

/// @brief Converts timer period in ms to an 'HH:MM:SS' string
/// @param period: Timer period to convert (ms)
/// @param buffer: Character buffer large enough to hold results
/// @returns ASCIIZ string
void ms_to_hms_str(time_ms_t period_ms, char *buffer);

#endif