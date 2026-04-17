#ifndef __APP_PARAMINPUT_H__
#define __APP_PARAMINPUT_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

#define ParamUnit_Kpa (0)
#define ParamUnit_Mpa (1)

//参数设置任务接口
typedef struct
{
	uint8_t Mode;					//1,菜单界面参数设置模式;0,主界面参数设置模式
	uint8_t RefreshFlag;			//显示刷新一次
	uint8_t StartPos_X;				//参数显示的起始横位置
	uint8_t StartPos_Y;				//参数显示的起始纵位置
	uint8_t DecimalDisplayEnable;	//浮点数显示开启
	uint8_t ParamLen;			//参数/浮点数长度
	uint8_t FontSize;			//字体大小
	uint16_t FontColour;		//字体颜色
	uint16_t BackColour;		//背景颜色
	int32_t ParamValue;			//参数值
	int32_t ParamHighLimit;		//参数上限
	int32_t ParamLowLimit;		//参数下限
}ParamSetTaskFormatTypeDef;

void ParamSetTaskCreate(void *param);
void ParamSetTaskStart(uint8_t OnOff);
uint8_t ParamSetTaskReady(void);
uint8_t ParamSetTaskIdleQuery(void);
void ParamSetTaskDelete(void);

void PressureParamIntputEnable(uint16_t x, uint16_t y);
uint8_t PressureParamIntputUnitRead(void);


#ifdef __cplusplus
}
#endif

#endif

