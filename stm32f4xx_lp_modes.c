/**
  ******************************************************************************
  * @file    PWR/PWR_CurrentConsumption/stm32f4xx_lp_modes.c
  * @author  MCD Application Team
  * @brief   This file provides firmware functions to manage the following
  *          functionalities of the STM32F4xx Low Power Modes:
  *           - Sleep Mode
  *           - STOP mode with RTC
  *           - Under-Drive STOP mode with RTC
  *           - STANDBY mode without RTC and BKPSRAM
  *           - STANDBY mode with RTC
  *           - STANDBY mode with RTC and BKPSRAM
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "RTC.h"
#include "stm32f4xx_lp_modes.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_CurrentConsumption
  * @{
  */

/* Private function prototypes -----------------------------------------------*/
static void SYSCLKConfig_STOP(void);
static uint16_t LP_CalcWakeupCounter(uint32_t seconds);
static int LP_ConfigWakeupTimer(RTC_HandleTypeDef *hrtc, uint32_t seconds);
static void LP_IrqSaveRaw(void);
static void LP_IrqDisableAll(void);

#define NVIC_ISER_COUNT (sizeof(NVIC->ISER) / sizeof(NVIC->ISER[0]))
static uint32_t s_nvic_iser[NVIC_ISER_COUNT];

/* Private functions ---------------------------------------------------------*/

void SleepMode_Measure(void)
{
  LP_SleepEnter();
}

void StopMode_Measure(void)
{
  (void)LP_StopEnter(20U);
}

void StopUnderDriveMode_Measure(void)
{
  (void)LP_StopEnter(20U);
}

void StandbyMode_Measure(void)
{
  (void)LP_StandbyEnter(0U);
}

void StandbyRTCMode_Measure(void)
{
  (void)LP_StandbyEnter(20U);
}

void StandbyRTCBKPSRAMMode_Measure(void)
{
  (void)LP_StandbyEnter(20U);
}

void LP_SleepEnter(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  HAL_ResumeTick();
}

void LP_SleepUntilFlag(volatile uint8_t *wake_flag)
{
  if (wake_flag == NULL) {
    LP_SleepEnter();
    return;
  }

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_SuspendTick();
  while (*wake_flag == 0U) {
    __WFI();
  }
  HAL_ResumeTick();
}

void LP_IrqSaveDisableAllExcept(IRQn_Type keep_irq)
{
  LP_IrqSaveRaw();
  LP_IrqDisableAll();
  NVIC_EnableIRQ(keep_irq);
}

void LP_IrqRestore(void)
{
  uint32_t i;

  LP_IrqDisableAll();
  for (i = 0U; i < NVIC_ISER_COUNT; i++) {
    NVIC->ISER[i] = s_nvic_iser[i];
  }
}

int LP_StopEnter(uint32_t wakeup_seconds)
{
  RTC_HandleTypeDef *hrtc = RTC_GetHandle();

  if ((hrtc == NULL) || (hrtc->Instance == NULL)) {
    return -1;
  }

  __HAL_RCC_PWR_CLK_ENABLE();

  if (wakeup_seconds > 0U) {
    if (LP_ConfigWakeupTimer(hrtc, wakeup_seconds) != 0) {
      return -2;
    }
  }

  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  HAL_ResumeTick();

  SYSCLKConfig_STOP();
  SystemCoreClockUpdate();

  if (wakeup_seconds > 0U) {
    HAL_RTCEx_DeactivateWakeUpTimer(hrtc);
  }

  return 0;
}

int LP_StandbyEnter(uint32_t wakeup_seconds)
{
  RTC_HandleTypeDef *hrtc = RTC_GetHandle();

  if ((hrtc == NULL) || (hrtc->Instance == NULL)) {
    return -1;
  }

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  if (wakeup_seconds > 0U) {
    if (LP_ConfigWakeupTimer(hrtc, wakeup_seconds) != 0) {
      return -2;
    }
  }

  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  HAL_PWR_EnterSTANDBYMode();

  return 0;
}

static uint16_t LP_CalcWakeupCounter(uint32_t seconds)
{
  uint64_t count;

  if (seconds == 0U) {
    return 0U;
  }

  count = ((uint64_t)seconds * 32768U) / 16U;
  if (count == 0U) {
    count = 1U;
  }
  if (count > 0xFFFFU) {
    count = 0xFFFFU;
  }

  return (uint16_t)count;
}

static int LP_ConfigWakeupTimer(RTC_HandleTypeDef *hrtc, uint32_t seconds)
{
  uint16_t counter;

  counter = LP_CalcWakeupCounter(seconds);
  if (counter == 0U) {
    return -1;
  }

  HAL_RTCEx_DeactivateWakeUpTimer(hrtc);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(hrtc, RTC_FLAG_WUTF);

  HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

  if (HAL_RTCEx_SetWakeUpTimer_IT(hrtc, counter, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK) {
    return -1;
  }

  return 0;
}

static void LP_IrqSaveRaw(void)
{
  uint32_t i;

  for (i = 0U; i < NVIC_ISER_COUNT; i++) {
    s_nvic_iser[i] = NVIC->ISER[i];
  }
}

static void LP_IrqDisableAll(void)
{
  uint32_t i;

  for (i = 0U; i < NVIC_ISER_COUNT; i++) {
    NVIC->ICER[i] = 0xFFFFFFFFU;
  }
}

/**
  * @brief  Configures system clock after wake-up from STOP: enable HSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
static void SYSCLKConfig_STOP(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  uint32_t pFLatency = 0U;

  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25U;
  RCC_OscInitStruct.PLL.PLLN = 336U;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7U;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @}
  */

/**
  * @}
  */
