/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"

#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "string.h"

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "LED.h"                  // ::Board Support:LED
#include "LCD.h"
#include "ADC.h"
#include "RTC.h"
#include "SNTP.h"
#include "buttom.h"
//#include "Board_Buttons.h"              // ::Board Support:Buttons
//#include "Board_ADC.h"                  // ::Board Support:A/D Converter
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD


// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

//extern GLCD_FONT GLCD_Font_6x8;
//extern GLCD_FONT GLCD_Font_16x24;


uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0};

extern uint16_t AD_in          (uint32_t ch);
extern uint8_t  get_button     (void);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

extern bool LEDrun;
extern char lcd_text[2][20+1];

extern osThreadId_t TID_Display;
extern osThreadId_t TID_Led;

static MSG_LCD msgLcd;
bool LEDrun;
char lcd_text[2][20+1] = { "LCD line 1",
                           "LCD line 2" };

/* Thread IDs */
osThreadId_t TID_Display;
osThreadId_t TID_Led;
osThreadId_t TID_RTC;

/* Thread declarations */
static void BlinkLed (void *arg);
static void Display  (void *arg);

__NO_RETURN void app_main (void *arg);

/* Read analog inputs */
uint16_t AD_in (uint32_t ch) {
 static uint32_t val = 0;

  ADC_HandleTypeDef adchandle;
  
  if (ch == 0) {
    
    
    ADC_Init_Single_Conversion(&adchandle , ADC1);
    //while (ADC_ConversionDone () < 0);
   // val = ADC_getVoltage();
  }
  val=ADC_getVoltage(&adchandle , 10 );
  //value = (uint16_t)val;
  //val=ADC_getVoltage(&adchandle , 13 );
  return (uint16_t)(val);
}

/* Read digital inputs */
uint8_t get_button (void) {
//  return ((uint8_t)Buttons_GetState ());
  return 0;
}

/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
    osThreadFlagsSet (TID_Display, 0x01);
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler This tread is just to check if the SPI is well configured
 *---------------------------------------------------------------------------*/
static __NO_RETURN void Display (void *arg) {
  

  (void)arg;

  while(1){		/*##-3- Display the updated Time and Date ################################*/
    RTC_CalendarShow(aShowTime, aShowDate);
    
    msgLcd.Reset=0;

    strcpy(msgLcd.Buffer, (char *)aShowTime);
    msgLcd.Linea = 0;
    osMessageQueuePut(queueLCD, &msgLcd, NULL, 100);
    
    
    
    strcpy(msgLcd.Buffer,(char *)aShowDate);
    msgLcd.Linea = 1;
    osMessageQueuePut(queueLCD, &msgLcd, NULL, 100); 
    osDelay(500);
    osThreadYield();  
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static __NO_RETURN void BlinkLed (void *arg) {
  const uint8_t led_val[16] = { 0x48,0x88,0x84,0x44,0x42,0x22,0x21,0x11,
                                0x12,0x0A,0x0C,0x14,0x18,0x28,0x30,0x50 };
  uint32_t cnt = 0U;

  (void)arg;

  LEDrun = false;
  while(1) {
    /* Every 100 ms */
    if (LEDrun == true) {
      LED_SetOut (led_val[cnt]);
      if (++cnt >= sizeof(led_val)) {
        cnt = 0U;
      }
    }
    osDelay (100);
  }
}



/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;
  


  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
  
  sdatestructure.Date= 0x10;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  sdatestructure.Year = 0x18;


  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x05;
  stimestructure.Minutes = 0x30;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;


  
  LED_Initialize();
  ADC1_pins_F429ZI_config();
  
  Init_LCD ();
  RTC_Config();
  RTC_CalendarConfig(sdatestructure,stimestructure);
  RTC_Alarm_Config();
  Init_Alarma();
  Init_Buttom();
  
  netInitialize ();
  //LED_Initialize();
  
  osDelay(5000);
  Init_SNTP();
  
  
  //TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
  TID_Display = osThreadNew (Display,  NULL, NULL);
//  
//  

  

  
  osThreadExit();
}
