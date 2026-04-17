#include "App_Key.h"
#include "stdint.h"
#include "string.h"

#define Debounce_Time (15)    	//消抖时间计数
#define Sampling_Sum (8)      	//采样次数(Max 15)
#define Long_Press_Time (1000)	//长按进入时间
#define Continue_Cycle (150)		//进入连续事件周期

//按键参数相关定义
typedef struct KEY_Handle
{
	uint8_t key_status :4;   	//按键状态
	uint8_t event_sign :4;     	//事件标志
	uint8_t sampling_cnt :4;   	//信号采样计数
	uint8_t key_trigger :1;    	//按键触发值（高电平为1，低电平为0）
	uint16_t ticks;    			//当前按键计数
	uint8_t (*Key_Value)(void);	//按键当前值
	void (*KeyCallback[Press_Error])(void *param);	//事件回调
	struct KEY_Handle* next;			//节点
}KEY_HandleTypedef;

//按键头指针
static KEY_HandleTypedef *KEY_HEAD = NULL;		
//按键对象
static KEY_HandleTypedef KEY_UP;
static KEY_HandleTypedef KEY_ENTER;
static KEY_HandleTypedef KEY_DOWN;
//按键状态
volatile static uint32_t KEY_Status = 0;

#define EVENT_CALL(ev)   if(handle->KeyCallback[ev])handle->KeyCallback[ev]((KEY_HandleTypedef*)handle)		//回调函数执行


/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void KEY_Config(KEY_HandleTypedef *handle, uint8_t (*pin_value)(void), uint8_t trigger_value)
{
	memset(handle, 0, sizeof(KEY_HandleTypedef));
	handle->event_sign = Press_END;
	handle->Key_Value = pin_value;
	handle->key_trigger = trigger_value;
}
	
/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void KEY_AddWrok(KEY_HandleTypedef *handle)
{
	KEY_HandleTypedef *target = KEY_HEAD; 
	while(target)
	{
		if(target == handle)
			return;
		target = target->next;
	}
	handle->next = KEY_HEAD;
	KEY_HEAD = handle;
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void KEY_DeleteWrok(KEY_HandleTypedef *handle)
{
	KEY_HandleTypedef **curr;
	
	for(curr = &KEY_HEAD; *curr;)
	{
		KEY_HandleTypedef *entry = *curr;
		if(entry == handle)
		{
			*curr = entry->next;
		} 
		else
			curr = &entry->next;
	}
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void Button_ADD_EventMsp(KEY_HandleTypedef* handle, KEY_Event event, void(*ev_msp)(void *param))
{
	handle->KeyCallback[event] = ev_msp;
	
	if(event == Short_Press_Repeat)
	{
		if(handle->KeyCallback[Long_Press_Once])
			handle->KeyCallback[Long_Press_Once] = NULL;
		if(handle->KeyCallback[Long_Press_Repeat])
			handle->KeyCallback[Long_Press_Repeat] = NULL;
	}
	else if(event == Long_Press_Repeat)
	{
		if(handle->KeyCallback[Short_Press_Repeat])
			handle->KeyCallback[Short_Press_Repeat] = NULL;
	}
	else if(event == Long_Press_Once)
	{
		if(handle->KeyCallback[Short_Press_Repeat])
			handle->KeyCallback[Short_Press_Repeat] = NULL;
	}
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void Button_StatusMachine(KEY_HandleTypedef *handle)
{
	uint8_t key_level = handle->Key_Value();
	
	if(handle->key_status > 0)		//有事件触发，消抖延时开启
		handle->ticks++;
	
	if(key_level == handle->key_trigger)	//检测到电平变化
	{		
		if (handle->sampling_cnt < Sampling_Sum)
			handle->sampling_cnt ++;
		else if (handle->sampling_cnt >= Sampling_Sum && handle->event_sign == Press_END)	
			handle->event_sign = Press_Down;	//电平较稳定了，标记被按下
	}
	else		//采样电平不为连续有效则清零
		handle->sampling_cnt = 0;
	
	switch(handle->key_status)
	{
		case 0:
			if (handle->event_sign == Press_Down)
			{
				handle->key_status = 1;
				handle->ticks = 0;
			}
			else
				handle->event_sign = Press_END;
		break;
			
		case 1:
			if(handle->ticks >= Debounce_Time)
			{			
				handle->event_sign = Short_Press_Once;		//短按一次事件
				if (key_level == handle->key_trigger)		//再次检测按键是否有效
				{
					EVENT_CALL(handle->event_sign);	
					handle->ticks = 0;
					handle->key_status = 2;
				}
				else
				{
					handle->key_status = 0;
					handle->event_sign = Press_END;
					EVENT_CALL(Press_END);
				}	
			}
			break;
			
		case 2:
			if(key_level == handle->key_trigger)		
			{
				if(handle->ticks >= Continue_Cycle && handle->KeyCallback[Short_Press_Repeat])
				{
					EVENT_CALL(Short_Press_Repeat);		//短按连续事件
					handle->ticks = 0;
				}
				if(handle->event_sign == Long_Press_Once)
					handle->key_status = 2;
				else
					handle->key_status = 3;
			}
			else
			{
				handle->key_status = 0;
				handle->event_sign = Press_END;
				EVENT_CALL(Press_END);
			}
		break;
		
		case 3:
			if(handle->ticks >= Long_Press_Time)		//长按时间到
			{			
				handle->event_sign = Long_Press_Once;	//长按一次事件
				if(key_level == handle->key_trigger)	
				{		
					EVENT_CALL(handle->event_sign);	
					handle->key_status = 4;
					handle->ticks = 0;
				}
				else
				{
					handle->key_status = 0;
					handle->event_sign = Press_END;
					EVENT_CALL(Press_END);
				}
			}
			else
				handle->key_status = 2;
		break;
			
		case 4:	
			if(key_level == handle->key_trigger)
			{		
				if(handle->ticks >= Continue_Cycle && handle->KeyCallback[Long_Press_Repeat])
				{
					EVENT_CALL(Long_Press_Repeat);			//长按连续事件
					handle->ticks = 0;
				}
				if(handle->KeyCallback[Long_Press_Repeat] || (!handle->KeyCallback[Short_Press_Repeat]))	
					handle->key_status = 4;
				else
					handle->key_status = 2;
			}
			else
			{
				handle->key_status = 0;
				handle->event_sign = Press_END;
				EVENT_CALL(Press_END);
			}
		break;
	}	
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void KEY_Scan(void)
{
	KEY_HandleTypedef* target; 
	
	for (target = KEY_HEAD; target; target = target->next)
		Button_StatusMachine(target);
}

/*******************按键信号输入接口*******************/
uint8_t KEY_UP_Read(void)
{
	return ((GPIOB->IDR & GPIO_PIN_0) ? 1 : 0);
}
uint8_t KEY_ENTER_Read(void)
{
	return ((GPIOB->IDR & GPIO_PIN_1) ? 1 : 0);
}
uint8_t KEY_DOWN_Read(void)
{
	return ((GPIOA->IDR & GPIO_PIN_4) ? 1 : 0);
}
/*******************按键事件回调接口*******************/
//短按事件
void KEY_UP_Short_Press_Once_Msp(void *spare)
{
	KEY_Status |= (uint32_t)(UP<<(Short_Press_Once - 1));
}
void KEY_ENTER_Short_Press_Once_Msp(void *spare)
{
	KEY_Status |= (uint32_t)(ENTER<<(Short_Press_Once - 1));
}
void KEY_DOWN_Short_Press_Once_Msp(void *spare)
{
	KEY_Status |= (uint32_t)(DOWN<<(Short_Press_Once - 1));
}
//短按连续事件
void KEY_UP_Short_Press_Repeat_Msp(void *spare)
{
	KEY_Status |= (uint32_t)(UP<<(Short_Press_Repeat - 1));
}
void KEY_DOWN_Short_Press_Repeat_Msp(void *spare)
{
	KEY_Status |= (uint32_t)(DOWN<<(Short_Press_Repeat - 1));
}
//长按事件
void KEY_ENTER_Long_Press_Once_Msp(void *spare)
{
	KEY_Status |= (uint32_t)(ENTER<<(Long_Press_Once - 1));
}
//结束事件
void KEY_UP_Press_END_Msp(void *spare)
{
	KEY_Status &= 0xFFFFFFF0;
	KEY_Status |= (uint32_t)(UP<<(Press_END - 1));
}
void KEY_ENTER_Press_END_Msp(void *spare)
{
	KEY_Status &= 0xFFFFFF0F;
	KEY_Status |= (uint32_t)(ENTER<<(Press_END - 1));
}
void KEY_DOWN_Press_END_Msp(void *spare)
{
	KEY_Status &= 0xFFFFF0FF;
	KEY_Status |= (uint32_t)(DOWN<<(Press_END - 1));
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
uint8_t KEY_ReadEvent(KEY_Dev dev, KEY_Event event, uint8_t clr)
{
	if(clr)
	{
		if(KEY_Status & (dev<<(event - 1)))
		{
			KEY_Status &= ~(uint32_t)(dev<<(event - 1));
			return 1;
		}
	}
	else
	{
		if(KEY_Status & (dev<<(event - 1)))
		{
			return 1;
		}
	}
	return 0;
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	//PB0 UP   PB1 SET
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	//PA4 DOWN
	GPIO_InitStruct.Pin = GPIO_PIN_4;		
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	KEY_Config(&KEY_UP, KEY_UP_Read, 0);
	KEY_Config(&KEY_ENTER, KEY_ENTER_Read, 0);
	KEY_Config(&KEY_DOWN, KEY_DOWN_Read, 0);
	
	Button_ADD_EventMsp(&KEY_UP, Short_Press_Once, KEY_UP_Short_Press_Once_Msp);
	Button_ADD_EventMsp(&KEY_ENTER, Short_Press_Once, KEY_ENTER_Short_Press_Once_Msp);
	Button_ADD_EventMsp(&KEY_DOWN, Short_Press_Once, KEY_DOWN_Short_Press_Once_Msp);
	
	Button_ADD_EventMsp(&KEY_UP, Short_Press_Repeat, KEY_UP_Short_Press_Repeat_Msp);
	Button_ADD_EventMsp(&KEY_DOWN, Short_Press_Repeat, KEY_DOWN_Short_Press_Repeat_Msp);
	
	Button_ADD_EventMsp(&KEY_ENTER, Long_Press_Once, KEY_ENTER_Long_Press_Once_Msp);
	
	Button_ADD_EventMsp(&KEY_UP, Press_END, KEY_UP_Press_END_Msp);
	Button_ADD_EventMsp(&KEY_ENTER, Press_END, KEY_ENTER_Press_END_Msp);
	Button_ADD_EventMsp(&KEY_DOWN, Press_END, KEY_DOWN_Press_END_Msp);
	
	KEY_AddWrok(&KEY_UP);
	KEY_AddWrok(&KEY_DOWN);
	KEY_AddWrok(&KEY_ENTER);
}

