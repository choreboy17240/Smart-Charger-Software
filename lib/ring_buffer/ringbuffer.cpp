/**
 *  @file ringbuffer.cpp
 *  @brief Ring buffer classes
 * 
 *  Copyright(c) 2025  John Glynn
 * 
 *  This code is licensed under the MIT License.
 *  See the LICENSE file for the full license text.
 */
#include "ringbuffer.h"

#define VERSION "1.2"             ///< Software revision number (x.x)
#define RELDATE "01/20/2025"      ///< Software revision date (MM/DD/YYYY)

// Default constructor
RingBuffer16::RingBuffer16(void) {
  init(0);
}

// Constructor with initialization
RingBuffer16::RingBuffer16(uint entries) {
  init(entries);
}

// Initialize the ring buffer object
void RingBuffer16::init(uint entries) {
	// Safely release allocated memory if called multiple times
	if (buffer != nullptr) {
		delete[] buffer;
	}

	if (entries != 0) {
	  buffer = (uint16_t *)new uint16_t[entries];
	} else {
		buffer = nullptr;
	}
	buffer_size = entries;
  buffer_overflow = false;
  head = 0;
  tail = 0;
}

// Destructor to release allocated memory
RingBuffer16::~RingBuffer16(void) {
    if (buffer != nullptr) {
        delete[] buffer;
    }
}

// Append an entry to the ring buffer
void RingBuffer16::append(uint16_t entry) {
  // Initialization check
  assert(buffer != nullptr);

  buffer[head] = entry;
  if (++head >= buffer_size) {
    head = 0;
  }
  if (head == tail) {
    // Overflow condition
    // Move tail position to allow overwrite of the oldest data
    // Set overflow flag so user can detect the condition if needed
    if (++tail >= buffer_size) {
      tail = 0;
    }
    buffer_overflow = true;
  }
}

// Get the oldest entry from the ring buffer
uint16_t RingBuffer16::get(void) {
  uint16_t entry;

  // Initialization check
  assert(buffer != nullptr);

  if (available()) {
    entry = buffer[tail++];
    if (tail >= buffer_size) {
      tail = 0;
    }
  } else {
    entry = 0;
  }

  return entry;
}

// Peek at oldest entry in the ring buffer
uint16_t RingBuffer16::peek(void) {
  uint16_t entry;

  // Initialization check
  assert(buffer != nullptr);

  if (available()) {
    entry = buffer[tail];
  } else {
    entry = 0;
  }
  
  return entry;
}

// Copy ring buffer entries into external buffer
size_t RingBuffer16::copy(uint16_t *outbuffer, size_t outbuffer_size) {
  size_t n_copied = 0;	// Number of entries copied

  // Determine the number of valid entries to copy
  size_t n_to_copy = std::min(available(), outbuffer_size);

  // Handle empty buffer case
  if (!n_to_copy) {
    return 0;
  }

  // Copy entries in order until either the output buffer is full or
  // we've copied all of the available entries in the ring buffer.
  for (size_t n = 0; n < n_to_copy; n++) {
  	// Calculate the index in the ring buffer
    uint ptr = (tail + n) % buffer_size;
    // Copy the value to the external buffer
    outbuffer[n] = buffer[ptr];
    // Keep cumulative count
    n_copied++;
  }

  // Return number of entries copied
  return n_copied;
}

// Get the average of all elements in the ring buffer
uint16_t RingBuffer16::average(void) {
  uint32_t total = 0;   // Cumulative total
  uint n = available(); // Number of entries in ring buffer

  // Initialization check
  assert(buffer != nullptr);

  // Handle empty buffer case
  if (!n) {
    return 0;
  }

  // Process all entries in the ring buffer
  for (uint i=0; i < n; i++) {
    uint ptr = (tail+i) % buffer_size;
    total += buffer[ptr];
    if (ptr == head) {
      break;      // No more entries
    }
  }

  // Return average value
  return total/n;
}

// Get the number of elements in the ring buffer
uint RingBuffer16::available(void) {
  // Debugging checks
  assert(buffer != nullptr);
  assert(buffer_size != 0);

  // Handle error cases
  if ((buffer == nullptr) || (buffer_size == 0)) {
  	return 0;
  }

  if (head >= tail) {
    return head-tail;
  } else {
    return buffer_size-(tail-head);
  }
}

// Check and clear the status of buffer overflow flag
bool RingBuffer16::overflow(void) {
  // Initialization check
  assert(buffer != nullptr);

  if (buffer_overflow) {
    buffer_overflow = false;
    return true;
  } else {
    return false;
  }
}

// Get software revision number
void RingBuffer16::version(char *buffer, size_t buffer_size) {
  strncpy(buffer, VERSION, buffer_size);
}

// Get software revision date
void RingBuffer16::reldate(char *buffer, size_t buffer_size) {
  strncpy(buffer, RELDATE, buffer_size);
}
