/*
JoBa1 ESP32 ULP comm example

Load and start an ULP programm, then read data stream from ULP
Later: go to sleep if no new messages and get woken up by ULP if comm buffer fills up
*/

#include <stdio.h>         // printf(), putchar();
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" // for task and timing
// #include "esp_sleep.h"  // esp_sleep_enable_ulp_wakeup(), esp_deep_sleep_start()

#include "ulp-util.h"      // my ulp_init(), ulp_start(), ulp_get() and .globals


uint32_t *buffer = &ulp_buffer;
uint16_t buffer_start_ulp; // buffer start in ulp address space
uint16_t buffer_end_ulp;   // buffer end in ulp address space


// Get one 16 bit value from the rinbuffer that is filled by the ULP,
// and increment the tail pointer to make room for more ULP input
bool get( uint16_t *item ) {
  uint16_t tail = ulp_get(ulp_tail);
  uint16_t head = ulp_get(ulp_head);
  if( tail == head ) return false; // empty buffer!
  *item = ulp_get(buffer[tail-buffer_start_ulp]);
  if( ++tail == buffer_end_ulp ) tail = buffer_start_ulp;
  ulp_tail = tail;
  return true; // item contains valid value
}


void read_ulp() {
  uint16_t item;

  while( (buffer_start_ulp = ulp_get(ulp_offset)) == 0 )
    vTaskDelay(1 / portTICK_PERIOD_MS); // wait until ulp is initialized
  buffer_end_ulp = buffer_start_ulp + (&ulp_buffer_end - &ulp_buffer);

  for(;;) {
    if( get(&item) ) {
      putchar(item & 0xff);  // this example uses only the low byte of the value
    }
    else {
      vTaskDelay(1 / portTICK_PERIOD_MS); // wait for ulp providing more data
    }
  }
}


void app_main()
{
  printf("Hello Comm ULP!\n");

  uint32_t wakeups[] = { 2*1000 };
  ulp_init(wakeups, sizeof(wakeups)/sizeof(*wakeups));
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
