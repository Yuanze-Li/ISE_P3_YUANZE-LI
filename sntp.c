#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "rl_net.h"
#include <stdio.h>
#include "led.h"
#include "time.h"
#include "RTC.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
  const NET_ADDR4 ntp_server_1 = { NET_ADDR_IP4 , NULL, 213,161,194,93 };
 
 const NET_ADDR4 ntp_server_2 = { NET_ADDR_IP4 , NULL, 130, 206,   3, 166 };
 
osThreadId_t tid_SNTP;                        // thread id
static void time_callback (uint32_t seconds, uint32_t seconds_fraction);
void get_time (void);
void ThSNTP (void *argument);                   // thread function
 void unix_to_datetime_rtc(uint32_t unix_time);
 
 	RTC_DateTypeDef sdatest_ntp;
 RTC_TimeTypeDef stimestr_ntp;

 
uint8_t decimal_to_bcd(uint8_t decimal);
  
int Init_SNTP (void) {
 
  tid_SNTP = osThreadNew(ThSNTP, NULL, NULL);
  if (tid_SNTP == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThSNTP (void *argument) {
 
  while (1) {
    get_time();
    osDelay(180000);
    osThreadYield();                            // suspend thread
  }
}


void get_time (void) {
  if (netSNTPc_GetTime ((NET_ADDR *)&ntp_server_1 , time_callback) == netOK) {
    printf ("SNTP request sent.\n");
    
  }
  else {
   printf ("SNTP not ready or bad parameters.\n");

  }
}
 
static void time_callback (uint32_t seconds, uint32_t seconds_fraction) {
  if (seconds == 0) {
    //unix_to_datetime_rtc(seconds);
    printf ("Server not responding or bad response.\n");
  }
  else {
    
    unix_to_datetime_rtc(seconds);
    for(int i = 0; i < 20; i++) {
      LED_On(2);
      osDelay(100);
      LED_Off(2);
      osDelay(100);
    }

  }
}


void unix_to_datetime_rtc(uint32_t unix_time) {
    struct tm *dt;
    time_t raw_time = unix_time;
    
    dt = localtime(&raw_time);  // Convert to local UTC time struct

  //  printf("Date: %04d-%02d-%02d Time: %02d:%02d:%02d UTC\n", 
  //         dt->tm_year + 1900, dt->tm_mon + 1, dt->tm_mday,
    //       dt->tm_hour, dt->tm_min, dt->tm_sec);
  

  sdatest_ntp.Year = decimal_to_bcd(dt->tm_year-100) ;
  sdatest_ntp.Month = decimal_to_bcd(dt->tm_mon + 1);
  sdatest_ntp.Date = decimal_to_bcd(dt->tm_mday);
  sdatest_ntp.WeekDay =dt->tm_wday+1;
  

  /*##-2- Configure the Time #################################################*/

  stimestr_ntp.Hours = decimal_to_bcd(dt->tm_hour+1);
  stimestr_ntp.Minutes = decimal_to_bcd(dt->tm_min);
  stimestr_ntp.Seconds = decimal_to_bcd(dt->tm_sec);
  stimestr_ntp.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestr_ntp.DayLightSaving = RTC_DAYLIGHTSAVING_ADD1H ;
  stimestr_ntp.StoreOperation = RTC_STOREOPERATION_RESET;

   RTC_CalendarConfig(sdatest_ntp, stimestr_ntp);
   
  
}

uint8_t decimal_to_bcd(uint8_t decimal) {
    return ((decimal / 10) << 4) | (decimal % 10);
}

