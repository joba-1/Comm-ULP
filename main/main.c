/*
 * JoBa1 ESP32 ULP comm example
 *
 * Load and start an ULP programm, then read data stream from ULP
 * Maybe later: go to sleep if no new messages and get woken up by ULP if comm buffer fills up
 */

#include <stdio.h>         // printf(), fflush()
#include <unistd.h>        // usleep()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" // for task and timing
#include "esp_sleep.h"     // esp_sleep_enable_ulp_wakeup(), esp_deep_sleep_start()

#include "ulp-util.h"      // my ulp_init(), ulp_start(), ulp_get() and .globals


// Helper routine to convert 16-bit value to a bit string with dot separators
// Not required for comm, just for nicer prints of data from the ULP.

char *b2s( uint16_t val, char *buf, uint16_t dots ) {
  char *ptr = buf;
  for( uint16_t mask=1<<(16-1); mask; mask>>=1 ) {
    *(ptr++) = val & mask ? '1' : '0';
    if( dots & mask ) *(ptr++) = '.';
  }
  *ptr = '\0';
  return buf;
}


uint32_t *buffer = &ulp_buffer;
uint16_t size;

/* only done by ULP
void init(void) {
  head = tail = buffer;
}
*/

void inc( uint16_t *index ) {
  if( ++(*index) == size ) *index = 0;
}

/* only done by ULP
void put( uint16_t item ) {
  *head = item;
  inc(head);
  while( head == tail ) { // overflow!
    // inc(tail); // discard tail (would need access semaphore for tail)
    sleep(1)      // wait until tail processed
  }
}
*/

bool get( uint16_t *item ) {
  uint16_t tail = ulp_get(ulp_tail);
  uint16_t head = ulp_get(ulp_head);
  if( tail == head ) return false; // empty buffer!
  *item = ulp_get(buffer[ulp_get(tail)]);
  inc(&tail);
  ulp_tail = tail;
  return true;
}


void read_ulp() {
  size = ulp_get(ulp_size); // size of buffer in 32bit words for inc()

  static uint16_t value = 0;

  uint16_t val;
  uint16_t item;

  for(;;) {
    val = ulp_get(ulp_value);
    if( val != value ) {
      value = val;
      printf("expected: %04x\n", value);
    }

    if( get(&item) ) {
      putchar(item);
      if(  item == '\n' ) fflush(stdout);
    }
    else {
      xTaskDelay(0); // wait a bit (maybe helps ulp to access shared memory)
    }
  }
}


void app_main()
{
  // uint16_t dots = BIT(4) + BIT(8) + BIT(12); // where b2s() places separator dots in the binary string
  // char bins[16+3+1];                         // buffer for a 16 bit binary string with three dots

  printf("Hello Comm ULP!\n");

  ulp_init(NULL, 0); // load ULP and use standard ULP sleep interval
  ulp_start();

  printf("Started ULP.\n");

  // in this example the ULP simply increments a counter when it wakes up,
  // writes the value as hex string into the ringbuffer and goes to sleep.
  xTaskCreate(read_ulp, "read_ulp", 4*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

  /*
  printf("Going to sleep now.\n");
  fflush(stdout);

  ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup() );
  esp_deep_sleep_start();
  */
}
