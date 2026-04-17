#ifndef __APP_KEY_H__
#define __APP_KEY_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

typedef enum
{
	UP = 0x00001,
	ENTER = 0x00020,
	DOWN = 0x00400
}KEY_Dev;

//按键触发事件
typedef enum
{
	Press_Down = 0,       	//检测到触发信号
	Short_Press_Once,		//短按一次事件
	Short_Press_Repeat,		//短按连续事件
	Long_Press_Once,		//长按一次事件
	Long_Press_Repeat,		//长按连续事件
	Press_END,		        //触发结束事件
	Press_Error
}KEY_Event;

void KEY_Init(void);
void KEY_Scan(void);
uint8_t KEY_ReadEvent(KEY_Dev dev, KEY_Event event, uint8_t clr);


#ifdef __cplusplus
}
#endif

#endif

