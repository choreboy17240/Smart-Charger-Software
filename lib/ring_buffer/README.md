# Ring Buffer Class Library for Arduino

Ring buffer class for implementing circular queues or free-running
caching of data.

### Details

This library provides support for circular (aka ring) buffers
that store data in a continuous loop for up to the maximum allocated
size of the buffer (defined during initialization of the RingBuffer16
object).

The ring buffer object is created using either (1) the default constructor
or (2) the constructor with initialization.  When using the default 
constructor, the `init()` method must be called before using the ring
buffer, in order to define the buffer size and allocate the associated 
memory from the heap.  These steps are performed automatically by the 
constructor with initialization.

Once initialized, data can be added to the buffer using the `append()`
method, and retrieved/removed from the buffer in a FIFO manner using the
`get()` method.  Similar to the Arduino Serial object, an `available()`
method is provided to check how many entries are currently in the 
buffer.

Buffer data can be examined without modifying the contents using
the `peek()` and `copy()` methods.  The `peek()` method retrieves the 
oldest data in the buffer without removing it from the queue.  The `copy()`
method copies the buffer contents to a buffer provided by the caller.

The ring buffer can be used as a queue or as a free-running cache of
the last 'N' data points (N = ring buffer size).  When used as a queue,
the `overflow()` method is provided to check whether data in the buffer
has been overwritten by new entries prior to being retrieved.  When
used as a free-running cache, the overflow() method can be ignored.

The `average()` method is provided as a convenience for use with
free-running caches.  The average of all values in the buffer is 
calculated and returned.

Finally, support for library version checking is provided by the
`version()` and `reldate()` methods.

### Example Usage

    #include <Arduino.h>
    #include <ringbuffer.h>

    #define RING_BUFFER_SIZE  10

    // Global ring buffer for storing queue data
    RingBuffer16 myqueue(RB_BUFFER_SIZE);

    int main() {
      // Add data to ring buffer
      myqueue.append(1);
      myqueue.append(2);
      myqueue.append(3);
      myqueue.append(4);

      // Check how many entries in the buffer (shows '4' in this case)
      printf("There are %u entries in the queue\n", myqueue.available());

      // Peek at oldest data in queue (returns '1' in this case)
      // Buffer contents are unchanged, so multiple peeks are ok.
      uint16_t value = myqueue.peek();
      printf("The oldest entry is %u\n", myqueue.peek());

      // Retrieve and remove an entry from the queue (returns '1' in this case)
      // Three entries (2, 3, 4) will be left in the queue after the get().
      uint16_t entry = myqueue.get();

      // Copy data from the queue to an external buffer.  Data is copied
      // in FIFO order and the queue contents are unchanged.
      // Three entries (2, 3, 4) are copied to new_buffer in this case.
      uint16_t new_buffer[3];
      uint16_t new_buffer_entries = myqueue.copy(new_buffer, sizeof(new_buffer)/sizeof(uint16_t));

      // Check for overflow condition (does not exist in this case)
      if (myqueue.overflow()) {
        printf("Buffer overflow occurred\n");
      } else {
        printf("No buffer overflow has occurred\n");
      }
    }


### Revision History

* 1.0  11/20/2024
	- Initial release with a RingBuffer16 class for managing 
      `uint16_t` data entries.
	- Created `version()` and `reldate()` methods to support checking
      library version information.
    - Doxygen documentation generated, including a `README.md` file
      for main page documentation.

* 1.1	01/17/2025
	- Updates to improve robustness after ChatGPT review of code.

* 1.2 01/20/2025
  - Updated to add destructor after ChatGPT review of code.



