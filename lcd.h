#ifndef __LCD_H
#define __LCD_H
#include "cmsis_os2.h"

#include "RTE_Components.h"
#include <stdint.h>
#include "Driver_SPI.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

uint8_t LCD_sel(char text[], uint8_t sel);
uint8_t LCD_center(char text[]);
void resetPins(void);
void LCD_write(uint8_t columna, uint8_t pagina, uint8_t valor);
void LCD_update(void);
void LCD_init(void);
void LCD_reset(void);
void LCD_symbolToLocalBuffer(uint8_t line,uint8_t symbol, uint8_t reset, uint16_t posicionL);
void clk_enable(void);

  #define QUEUE_LCD_SIZE 2
  typedef struct {
  
  char Buffer[21];
  uint8_t Linea;
  uint8_t Reset;
  uint8_t selec;
}MSG_LCD;
  
  extern osMessageQueueId_t queueLCD;

  int Init_LCD (void);
#endif
