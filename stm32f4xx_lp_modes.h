#ifndef __STM32F4XX_LP_MODES_H
#define __STM32F4XX_LP_MODES_H

#include <stdint.h>

void SleepMode_Measure(void);
void StopMode_Measure(void);
void StopUnderDriveMode_Measure(void);
void StandbyMode_Measure(void);
void StandbyRTCMode_Measure(void);
void StandbyRTCBKPSRAMMode_Measure(void);

void LP_SleepEnter(void);
void LP_SleepUntilFlag(volatile uint8_t *wake_flag);
void LP_IrqSaveDisableAllExcept(IRQn_Type keep_irq);
void LP_IrqRestore(void);
int  LP_StopEnter(uint32_t wakeup_seconds);
int  LP_StandbyEnter(uint32_t wakeup_seconds);

#endif
