#include "main.h"
#include "buttom.h"

GPIO_InitTypeDef GPIO_InitStruct;

void Init_Buttom() {
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
