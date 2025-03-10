/**
 * @file ringbuffer.h
 * @brief Arduino ring buffer class
 *
 * Copyright(c) 2025  John Glynn
 * 
 * This code is licensed under the MIT License.
 * See the LICENSE file for the full license text.
 * 
 * @details This library provides support for circular (aka ring) buffers
 * that store data in a continuous loop for up to the maximum allocated
 * size of the buffer.  See the README.md file for additional details.
 *
 * See the README.md file for revision history.
 */
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#ifdef linux
#include "Arduino.h"
#else
#include <Arduino.h>
#endif
#include <assert.h>
#include <string.h>

/**
 *  @brief Ring buffer class for holding 16-bit unsigned integer entries.
 */
class RingBuffer16 {
public:
    /**
     *  @brief Default constructor
     */
    RingBuffer16(void);

    /**
     *  @brief Constructor with initialization
     *  @param entries: Number of entries in the ring buffer
     */
    RingBuffer16(uint entries);

    /**
     * @brief Default destructor
     */
    ~RingBuffer16(void);


    /**
     *  @brief Initialize the ring buffer object
     *  @param entries: Number of entries in the ring buffer
     */
    void init(uint entries);

    /**
     *  @brief Get the number of elements in the ring buffer
     *         available for getting or peeking at.
     *  @returns Number of elements in the ring buffer
     */
    uint available(void);

    /**
     *  @brief Append an entry to the ring buffer
     *  @param entry: Entry to be appended to end of the buffer
     */
    void append(uint16_t entry);

    /**
     *  @brief Get the oldest entry from the ring buffer
     *  @returns Entry retrieved from the ring buffer
     *  @note The entry will be removed from the buffer.  To examine
     *        buffer entries without modifying it, see the peek()
     *        method.
     */
    uint16_t get(void);

    /**
     *  @brief Peek at the oldest entry in the ring buffer
     *  @returns Entry retrieved from the ring buffer
     *  @note The buffer is not modified by this method.  To fetch and remove
     *        the entry from the buffer, see the get() method.
     *        If there is no data in the buffer, the method will return a
     *        zero value.
     */
    uint16_t peek(void);

    /**
     *  @brief Copies the ring buffer entries to an external buffer in order
     *  @param outbuffer: Output buffer to receive the ring buffer data
     *  @param outbuffer_size: Size of the output buffer in entries
     *  @returns Number of entries copied into the external buffer
     *  @note The ring buffer is not modified by this method.  Entries are 
     *        copied to the external buffer from oldest to newest order.
     */
    size_t copy(uint16_t *outbuffer, size_t outbuffer_size);

    /**
     *  @brief Check and clear the status of buffer overflow flag
     *  @returns true: Overflow has occurred, false: No overflow has occurred
     */
    bool overflow(void);

    /**
     *  @brief Get the average of all elements in the ring buffer
     *  @returns Average value
     */
    uint16_t average(void);

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
  uint head;                ///< Buffer offset pointing to empty slot
  uint tail;                ///< Buffer offset pointing to oldest data
  uint buffer_size;         ///< Size of the buffer in elements
  uint16_t *buffer;         ///< Dynamically allocated buffer
  bool buffer_overflow;     ///< Overflow flag indicating data has been overwritten
};

#endif
