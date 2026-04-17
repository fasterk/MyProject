#include "Timer_Config.h"
#include <string.h>

#define TIMER_TASK_MAX (10)

struct TimerTaskObjTypedef
{
	uint8_t start_flag;
	uint8_t run_flag;
	uint32_t task_period;
	uint32_t task_cnt;
	void *param;
	void (*task_callback)(void *param);
};

struct TimerTaskObjTypedef _gTimerTaskObj[TIMER_TASK_MAX];

volatile static uint32_t SystemTick = 0;


/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void Timer_Init(void)
{
//	systick_clock_config(SYSTICK_CLKSEL_HCLK);
//	if(SysTick_Config(SystemCoreClock/1000U))
//	{
//		while(1)
//		{
//			
//		}
//	}
//	NVIC_SetPriority(SysTick_IRQn, 3);
	
	memset(_gTimerTaskObj, 0, sizeof(_gTimerTaskObj));
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
uint32_t GetSystemTick(void)
{
	return SystemTick;
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
int32_t TimerTaskCreate(uint32_t tick, void (*task_funtion)(void *param), void *param)
{
	uint32_t i;
	
	for(i=0; i<TIMER_TASK_MAX; i++)
	{
		if(_gTimerTaskObj[i].start_flag == 0)
		{
			_gTimerTaskObj[i].start_flag = 1;
			_gTimerTaskObj[i].task_period = tick;
			_gTimerTaskObj[i].task_callback = task_funtion;
			_gTimerTaskObj[i].param = param;
			return i;
		}
	}
	
	return -1;
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void TimerTaskDelete(int32_t task_num)
{
	if((task_num < 0) || (task_num >= TIMER_TASK_MAX)) return;
	
	__disable_irq();
	_gTimerTaskObj[task_num].start_flag = 0;
	_gTimerTaskObj[task_num].run_flag = 0;
	_gTimerTaskObj[task_num].task_callback = 0;
	_gTimerTaskObj[task_num].task_cnt = 0;
	_gTimerTaskObj[task_num].param = 0;
	__enable_irq();
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void TimerTaskTickSet(int32_t task_num, uint32_t tick)
{
	if((task_num < 0) || (task_num >= TIMER_TASK_MAX)) return;
	
	__disable_irq();
	_gTimerTaskObj[task_num].run_flag = 0;
	_gTimerTaskObj[task_num].task_period = tick;
	_gTimerTaskObj[task_num].task_cnt = 0;
	__enable_irq();
}

static uint32_t vTimerTaskScan = 0;
/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void TimerTaskHandle(void)
{
	if(vTimerTaskScan >= TIMER_TASK_MAX)
		vTimerTaskScan = 0;
	
	if(_gTimerTaskObj[vTimerTaskScan].run_flag)
	{
		if(_gTimerTaskObj[vTimerTaskScan].task_callback)
			_gTimerTaskObj[vTimerTaskScan].task_callback(_gTimerTaskObj[vTimerTaskScan].param);
		_gTimerTaskObj[vTimerTaskScan].run_flag = 0;
	}
	
	vTimerTaskScan++;
}

//SysTick中断服务函数
void SysTick_Handler(void)
{
	uint32_t i;
	
	for(i=0; i<TIMER_TASK_MAX; i++)
	{
		if(_gTimerTaskObj[i].start_flag)
		{
			if(_gTimerTaskObj[i].run_flag == 0)
			{
				_gTimerTaskObj[i].task_cnt++;
				if(_gTimerTaskObj[i].task_cnt >= _gTimerTaskObj[i].task_period)
				{
					_gTimerTaskObj[i].task_cnt = 0;
					_gTimerTaskObj[i].run_flag = 1;
				}
			}
		}
	}
	
	++SystemTick;
	HAL_IncTick();
}

