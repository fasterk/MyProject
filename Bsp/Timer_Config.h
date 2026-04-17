#ifndef __TIMER_CONFIG_H__
#define __TIMER_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"
#define NowTickCount GetSystemTick()


void Timer_Init(void);
uint32_t GetSystemTick(void);
int32_t TimerTaskCreate(uint32_t tick, void (*task_funtion)(void *param), void *param);
void TimerTaskDelete(int32_t task_num);
void TimerTaskTickSet(int32_t task_num, uint32_t tick);
void TimerTaskHandle(void);


#ifdef __cplusplus
}
#endif

#endif

