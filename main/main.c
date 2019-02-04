/*
 * JoBa1 ESP32 ULP comm example
 *
 * Load and start an ULP programm, then read data stream from ULP
 * Maybe later: go to sleep if no new messages and get woken up by ULP if comm buffer fills up
 */

#include <stdio.h>         // printf(), fflush()
#include <unistd.h>        // usleep()
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


void app_main()
{
  uint16_t dots = BIT(4) + BIT(8) + BIT(12); // where b2s() places separator dots in the binary string
  char bins[16+3+1];                         // buffer for a 16 bit binary string with three dots 

  printf("Hello Comm ULP!\n");

  ulp_init(NULL, 0); // load ULP and use standard ULP sleep interval
  ulp_start();

  printf("Started ULP.\n");

  {
    /*
    Demo of ULP copro writing values in a ring buffer and we read them here
    */
  }

  printf("Going to sleep now.\n");
  fflush(stdout);

  ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup() );
  esp_deep_sleep_start();
}

