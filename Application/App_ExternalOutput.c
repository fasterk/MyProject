#include "App_ExternalOutput.h"
#include <string.h>
#include "Timer_Config.h"
#include "GPIO_Config.h"
#include "Flash_Config.h"
#include "LCD.h"
#include "App_System.h"

typedef struct
{
	uint8_t State;
	//bit7,1:工作使能
	//bit6,1:满足输出条件
	//bit5,1:不满足输出条件
	//
	//bit3,1:开启计时
	//bit2,1：
	//
	uint8_t Mode;
	uint8_t WorkVal;
	int32_t ThresholdVal;
	int32_t PeakVal;
	int32_t ValleyVal;
	int32_t HysteresisVal;
	int32_t ResponseTimeVal;
	uint32_t Tick;
	void (*OutputCallback)(uint8_t OnOff);
}ExternalOutputHandle_Typedef;

static void OutputChannelCtrl_1(uint8_t OnOff);
static void OutputChannelCtrl_2(uint8_t OnOff);

ExternalOutputHandle_Typedef ExternalOutObj[OutputChannelMax] =
{
	{0,0,0,0,0,0,0,0,0,OutputChannelCtrl_1},
	{0,0,0,0,0,0,0,0,0,OutputChannelCtrl_2},
};


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void OutputChannelCtrl_1(uint8_t OnOff)
{
	if(OnOff == ExternalOutObj[OutChannel_1].WorkVal)
		NpnOut_Ctrl(NPN1, 1);
	else
		NpnOut_Ctrl(NPN1, 0);
	//控件显示
	if(OnOff)
	{
//		LCD_Draw_Filled_Ellipse(50,71,14,7,RED);
//		LCD_ShowString(39,66,(uint8_t *)"OUT1",BLACK,BLACK,12,1);
		
		LCD_ShowChinese(34,64,RED,BLACK,16,0,111);
		LCD_ShowChinese(50,64,RED,BLACK,16,0,112);
		LCD_ShowString(38,66,(uint8_t *)"OUT1",BLACK,BLACK,12,1);
	}
	else
	{
//		LCD_Draw_Filled_Ellipse(50,71,14,7,AGREEN);
//		LCD_ShowString(39,66,(uint8_t *)"OUT1",BLACK,BLACK,12,1);
		
		LCD_ShowChinese(34,64,AGREEN,BLACK,16,0,111);
		LCD_ShowChinese(50,64,AGREEN,BLACK,16,0,112);
		LCD_ShowString(38,66,(uint8_t *)"OUT1",BLACK,BLACK,12,1);
	}
	LED_Ctrl(LED1, OnOff);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void OutputChannelCtrl_2(uint8_t OnOff)
{
	if(OnOff == ExternalOutObj[OutChannel_2].WorkVal)
		NpnOut_Ctrl(NPN2, 1);
	else
		NpnOut_Ctrl(NPN2, 0);
	//控件显示
	if(OnOff)
	{
//		LCD_Draw_Filled_Ellipse(93,71,14,7,RED);
//		LCD_ShowString(82,66,(uint8_t *)"OUT2",BLACK,BLACK,12,1);
		
		LCD_ShowChinese(79,64,RED,BLACK,16,0,111);
		LCD_ShowChinese(95,64,RED,BLACK,16,0,112);
		LCD_ShowString(83,66,(uint8_t *)"OUT2",BLACK,BLACK,12,1);
	}
	else
	{
//		LCD_Draw_Filled_Ellipse(93,71,14,7,AGREEN);
//		LCD_ShowString(82,66,(uint8_t *)"OUT2",BLACK,BLACK,12,1);
		
		LCD_ShowChinese(79,64,AGREEN,BLACK,16,0,111);
		LCD_ShowChinese(95,64,AGREEN,BLACK,16,0,112);
		LCD_ShowString(83,66,(uint8_t *)"OUT2",BLACK,BLACK,12,1);
	}
	LED_Ctrl(LED2, OnOff);
}

///****************************************
// *函数名称：
// *功能：
// *参数说明：
// ****************************************/
//static void OutputChannelCtrl_1(uint8_t OnOff)
//{
//	if(OnOff == ExternalOutObj[OutChannel_1].WorkVal)
//		NpnOut_Ctrl(NPN1, 1);
//	else
//		NpnOut_Ctrl(NPN1, 0);
//	//控件显示
//	if(OnOff)
//	{
//		LCD_Draw_Filled_Ellipse(55,71,14,7,RED);
//		LCD_ShowString(44,66,(uint8_t *)"OUT1",BLACK,BLACK,12,1);
//	}
//	else
//	{
//		LCD_Draw_Filled_Ellipse(55,71,14,7,AGREEN);
//		LCD_ShowString(44,66,(uint8_t *)"OUT1",BLACK,BLACK,12,1);
//	}
//	LED_Ctrl(LED1, OnOff);
//}

///****************************************
// *函数名称：
// *功能：
// *参数说明：
// ****************************************/
//static void OutputChannelCtrl_2(uint8_t OnOff)
//{
//	if(OnOff == ExternalOutObj[OutChannel_2].WorkVal)
//		NpnOut_Ctrl(NPN2, 1);
//	else
//		NpnOut_Ctrl(NPN2, 0);
//	//控件显示
//	if(OnOff)
//	{
//		LCD_Draw_Filled_Ellipse(101,71,14,7,RED);
//		LCD_ShowString(90,66,(uint8_t *)"OUT2",BLACK,BLACK,12,1);
//	}
//	else
//	{
//		LCD_Draw_Filled_Ellipse(101,71,14,7,AGREEN);
//		LCD_ShowString(90,66,(uint8_t *)"OUT2",BLACK,BLACK,12,1);
//	}
//	LED_Ctrl(LED2, OnOff);
//}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ExternalOutput_Init(void)
{
	ExternalOutObj[OutChannel_1].Mode = AppDataRead(APP_Out1Mode);
	ExternalOutObj[OutChannel_1].ThresholdVal = AppDataRead(APP_Out1ThresholdVal);
	ExternalOutObj[OutChannel_1].PeakVal = AppDataRead(APP_Out1PeakVal);
	ExternalOutObj[OutChannel_1].ValleyVal = AppDataRead(APP_Out1ValleyVal);
	ExternalOutObj[OutChannel_1].HysteresisVal = AppDataRead(APP_Out1LagVal);
	ExternalOutObj[OutChannel_1].ResponseTimeVal = AppDataRead(APP_Out1ResponseTimeVal);
	if(AppDataRead(APP_Out1Way) == SystemOutWay_NoReverse)
		ExternalOutObj[OutChannel_1].WorkVal = 1;
	else
		ExternalOutObj[OutChannel_1].WorkVal = 0;
	
	ExternalOutObj[OutChannel_2].Mode = AppDataRead(APP_Out2Mode);
	ExternalOutObj[OutChannel_2].ThresholdVal = AppDataRead(APP_Out2ThresholdVal);
	ExternalOutObj[OutChannel_2].PeakVal = AppDataRead(APP_Out2PeakVal);
	ExternalOutObj[OutChannel_2].ValleyVal = AppDataRead(APP_Out2ValleyVal);
	ExternalOutObj[OutChannel_2].HysteresisVal = AppDataRead(APP_Out2LagVal);
	ExternalOutObj[OutChannel_2].ResponseTimeVal = AppDataRead(APP_Out2ResponseTimeVal);
	if(AppDataRead(APP_Out2Way) == SystemOutWay_NoReverse)
		ExternalOutObj[OutChannel_2].WorkVal = 1;
	else
		ExternalOutObj[OutChannel_2].WorkVal = 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ExternalOutputChannelEnable(OutputChannel_Dev Dev, uint8_t sta)
{
	if(Dev >= OutputChannelMax) return;
	
	if(sta)
		ExternalOutObj[Dev].State |= 0x80;
	else
		ExternalOutObj[Dev].State = 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ExternalOutputImageRef(OutputChannel_Dev Dev)
{
	if(Dev >= OutputChannelMax) return;
	
//	ExternalOutObj[Dev].State |= 0x01;
	
	if(ExternalOutObj[Dev].State & 0x10)
	{
		ExternalOutObj[Dev].OutputCallback(1);
	}
	else
	{
		ExternalOutObj[Dev].OutputCallback(0);
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ExternalOutputScanTask(void)
{
	uint8_t i;
	int32_t AirPressure = 0;
	
	for(i=OutChannel_1; i<OutputChannelMax; i++)
	{
		if(ExternalOutObj[i].State & 0x80)
		{
			AirPressure = GetAirPressureVal(0);		
			switch(ExternalOutObj[i].Mode)
			{
				case SystemChannelMode_Simple:
					if(AirPressure > (ExternalOutObj[i].ThresholdVal + ExternalOutObj[i].HysteresisVal))
					{
						ExternalOutObj[i].State |= 0x40;
					}
					else if(AirPressure < ExternalOutObj[i].ThresholdVal)
					{
						ExternalOutObj[i].State &= ~0x70;
						ExternalOutObj[i].State |= 0x01;
//						ExternalOutObj[i].OutputCallback(0);
					}
				break;
				
				case SystemChannelMode_Lag:
					if(AirPressure > ExternalOutObj[i].PeakVal)
					{
						ExternalOutObj[i].State |= 0x40;
					}
					else if(AirPressure < ExternalOutObj[i].ValleyVal)
					{
						ExternalOutObj[i].State &= ~0x70;
						ExternalOutObj[i].State |= 0x01;
//						ExternalOutObj[i].OutputCallback(0);
					}
				break;
				
				case SystemChannelMode_Window:
					if((ExternalOutObj[i].State & 0x08) == 0x00)
					{
						if(AirPressure > (ExternalOutObj[i].PeakVal + ExternalOutObj[i].HysteresisVal))
						{
							ExternalOutObj[i].State &= ~0x70;
							ExternalOutObj[i].State |= 0x09;
//							ExternalOutObj[i].OutputCallback(0);
						}
						else if(AirPressure > (ExternalOutObj[i].ValleyVal + ExternalOutObj[i].HysteresisVal))
						{
							ExternalOutObj[i].State |= 0x40;
						}
						else if(AirPressure < ExternalOutObj[i].ValleyVal)
						{
							ExternalOutObj[i].State &= ~0x78;
							ExternalOutObj[i].State |= 0x01;
//							ExternalOutObj[i].OutputCallback(0);
						}
					}
					if(ExternalOutObj[i].State & 0x08)
					{
						if((AirPressure < ExternalOutObj[i].PeakVal) && (AirPressure > ExternalOutObj[i].ValleyVal))
						{
							ExternalOutObj[i].State |= 0x40;
						}
						else if(AirPressure > (ExternalOutObj[i].PeakVal + ExternalOutObj[i].HysteresisVal))
						{
							ExternalOutObj[i].State &= ~0x70;
							ExternalOutObj[i].State |= 0x01;
//							ExternalOutObj[i].OutputCallback(0);
						}
						else if(AirPressure < ExternalOutObj[i].ValleyVal)
						{
							ExternalOutObj[i].State &= ~0x78;
							ExternalOutObj[i].State |= 0x01;
//							ExternalOutObj[i].OutputCallback(0);
						}
					}
				break;
				
				default: break;
			}
			//停止检测
			ExternalOutObj[i].State &= ~0x80;
		}
		//当前未处于输出状态
		if((ExternalOutObj[i].State & 0x10) == 0x00)
		{
			//检测是否满足输出
			if(((ExternalOutObj[i].State & 0x20) == 0x00) && (ExternalOutObj[i].State & 0x40))
			{
				ExternalOutObj[i].State |= 0x20;
				ExternalOutObj[i].State &= ~0x40;
				ExternalOutObj[i].Tick = GetSystemTick();
			}
			//满足输出等待响应时间到
			if((ExternalOutObj[i].State & 0x20) && (GetSystemTick() - ExternalOutObj[i].Tick > ExternalOutObj[i].ResponseTimeVal))
			{
				ExternalOutObj[i].State |= 0x10;
				ExternalOutObj[i].State &= ~0x03;
				ExternalOutObj[i].OutputCallback(1);
			}
		}
		//
		if((ExternalOutObj[i].State & 0x03) == 0x01)
		{
			ExternalOutObj[i].OutputCallback(0);
			ExternalOutObj[i].State |= 0x02;
		}
	}
}

