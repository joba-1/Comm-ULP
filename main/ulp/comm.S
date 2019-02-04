/*
JoBa1 ESP32 ULP communication example

Simple ULP program that writes messages in a ring buffer that are read by the main cores
*/

// Declare data visible to main cores
.global entry                // ulp start address used by main core program

.global buffer               // Start address of ring buffer
.global size                 // Size of ring buffer
.global item                 // Size of one buffer item
.global head                 // Write position of ringbuffer
.global tail                 // Read position of ring buffer

.bss

// Circular buffer, old data not yet read will be overwritten
data:
buffer: .long 0
        .skip 128
buf_end:

size:   .long 0
item:   .long 0
head:   .long 0
tail:   .long 0
full:   .long 0

value:  .long 0              // Internal/sensor value to push to ring buffer
value_end:                   // Calculate buffer item size with this
.set    item_size, (value_end-value)

// Offsets to variables relative to data (which I try to keep in r2)
.set    o_buffer, (buffer-data)
.set    o_size,   (size-data)
.set    o_item,   (item-data)
.set    o_head,   (head-data)
.set    o_tail,   (tail-data)
.set    o_full,   (full-data)
.set    o_value,  (value-data)

.text

entry:

// Check if we just restart and data is already initialized
        move  r2, data       // constant data frame pointer
        ld    r0, r2, o_size
        jumpr init_done, ge  // size > 0: already initialized

// Ringbuffer is not yet initialized. Do it now.
        ld    r0, buffer
        st    r0, r2, o_head // head = buffer  
        st    r0, r2, o_tail // tail = buffer == head -> empty
        move  r0, (item_size)
        st    r0, r2, o_item
        move  r1, (buf_end-buffer)
        sub   r0, r1, r0
        st    r0, r2, o_size
init_done:

// Now the "important" background task of the ULP: counting (later probably ADC/I2C reading or similar...)
        ld    r0, r2, o_value // read old value to r0
        add   r0, r0, 1       // and "calculate" a new value
        st    r0, r2, o_value // store new incremented value

// Finally tell the main core what we have done (copy new value into ringbuffer)
        move  r0, value       // points to current source word
        move  r1, head        // points to current ringbuffer head
loop:   ld    r3, r0, 0       // first word of value
        st    r3, r1, 0       // copied to first word of head
        add   r1, r1, 4       // move head to next word (or should I add only 1?)
        move  r3, buf_end     // Check if head meved beyond buffer
        sub   r3, r3, r1      // (buf_end - head) == 0 if beyond buffer
        jump  wrap, eq
wrapped:
        ld    r3, r2, o_tail  // Check if we have to discard an old value from tail
        ...
wrap:
        move  r1, buffer
        jump  wrapped

