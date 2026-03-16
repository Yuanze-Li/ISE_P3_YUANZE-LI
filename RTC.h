#ifndef __RTC_H
#define __RTC_H
#include "cmsis_os2.h"
#include "RTE_Components.h"
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"


#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

void RTC_Config(void);
void RTC_CalendarConfig(RTC_DateTypeDef sdatestructure, RTC_TimeTypeDef stimestructure);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void RTC_Alarm_Config(void);
int  Init_Alarma(void);
void SNTP_Date_Req(void);

int Init_RTC(void);
RTC_HandleTypeDef *RTC_GetHandle(void);
#endif
