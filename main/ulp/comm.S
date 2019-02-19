/*
JoBa1 ESP32 ULP communication example

Simple ULP program that writes messages in a ring buffer that are read by the main cores
*/

#include "soc/soc_ulp.h"      // for READ_RTC_FIELD()
#include "soc/rtc_cntl_reg.h" // for RTC_CNTL_LOW_POWER_ST_REG


// Declare data visible to main cores
.global entry                 // ulp start address used by main core program

.global offset                // tell main cores our idea of buffer address
.global head                  // write position of ringbuffer
.global tail                  // read position of ring buffer
.global buffer                // start address of ring buffer
.global buffer_end            // adress behind buffer


.bss

data:                         // data frame pointer

offset: .long 0               // address of buffer for main core
head:   .long 0
tail:   .long 0
value:  .long 0               // internal/sensor value to push to ring buffer
buffer: .long 0               // circular buffer tail-head-gap
        .skip 124             // and 31 long buffer items
buffer_end:


// Offsets to variables relative to data (which I try to keep in r2 for ld/st)
.set    o_offset,     (offset-data)
.set    o_head,       (head-data)
.set    o_tail,       (tail-data)
.set    o_value,      (value-data)


.text

entry:
        move  r2, data        // constant data frame pointer
        ld    r0, r2, o_head
        jumpr init_done, 1, ge // head > 0: already initialized

// Ringbuffer is not initialized by previous runs. Do it now.
        move  r0, buffer
        st    r0, r2, o_head  // head = buffer
        st    r0, r2, o_tail  // tail = buffer == head -> empty
        st    r0, r2, o_offset // offset = buffer (for main cores)
init_done:

// Now the "important" background task of the ULP: counting (later probably ADC/I2C reading or similar...)
        ld    r0, r2, o_value // read old value to r0
        add   r0, r0, 1       // and "calculate" a new value (value = value + 1)
        st    r0, r2, o_value // store new incremented value

// Convert new value to hex chars and put them in ringbuffer
        ld    r0, r2, o_value // value in r0
        rsh   r0, r0, 12      // high nibble 3 in r0
        move  r3, done_nibble_3
        jump  put_hex         // put r0 as hex in ring buffer then go to r3
done_nibble_3:
        ld    r0, r2, o_value // value in r0
        rsh   r0, r0, 8       // nibbles 3, 2 in r0
        move  r3, done_nibble_2
        jump  put_hex
done_nibble_2:
        ld    r0, r2, o_value // value in r0
        rsh   r0, r0, 4       // nibbles 3, 2, 1 in r0
        move  r3, done_nibble_1
        jump  put_hex
done_nibble_1:
        ld    r0, r2, o_value // value in r0
        move  r3, done_nibble_0
        jump  put_hex
done_nibble_0:
        move  r0, '\n'
        move  r3, done
        jump  put
done:   halt

put_hex:
        and   r0, r0, 0xf     // only use low nibble
        jumpr hex, 10, ge     // jump if a-f
        add   r0, r0, '0'     // 0-9
        jump  put
hex:    add   r0, r0, ('a'-10)

// Finally actually put r0 byte (0-9, a-f) to ringbuffer
put:    ld    r1, r2, o_head  // r1 points to buffer at head position
        st    r0, r1, 0       // store byte in buffer at head position
        add   r1, r1, 1       // head++
        move  r0, buffer_end
        sub   r0, r0, r1      // 0 means head beyond buffer
        jumpr check_tail, 1, ge
        move  r1, buffer      // set head at start of buffer
check_tail:
        ld    r0, r2, o_tail
        sub   r0, r0, r1
        jump  overflow, eq    // need to wake up main cores
have_space:
        st    r1, r2, o_head  // new head saved
        jump  r3
overflow:
        READ_RTC_FIELD(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP)
        and r0, r0, 1
        jump overflow, eq     // Retry until the bit is set
        wake                  // Trigger wake up
check_again:
        wait  10              // wait some ticks, so main core can update tail
        ld    r0, r2, o_tail
        sub   r0, r0, r1
        jump  check_again, eq // still need to wait for main cores
        jump  have_space
