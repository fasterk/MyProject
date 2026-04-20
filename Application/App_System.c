#include "App_System.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "GPIO_Config.h"
#include "Flash_Config.h"
#include "Uart_Config.h"
#include "Timer_Config.h"
#include "ADC_Config.h"
#include "PWM_Config.h"
#include "LCD.h"
#include "App_Key.h"
#include "MeterInterface.h"
#include "App_ExternalOutput.h"
#include "APP_Sensor.h"
#include "APP_JM1203.h"
#include "PWM_Config.h"

#define EPS 1e-5

#define ADCxFilterCount (512)	//ADC滤波次数

//计算相关
#define Formula_K ((int32_t)500)
#define Formula_Constant ((int32_t)-1500)
	
//显示相关
#define FixedContentRefreshPer (200)	//主界面固定内容刷新周期
extern TypeParam_Config Param_Config;

/****************其他****************/
uint32_t RangeNumber = 0;
uint32_t RangeSet_Flag = 0;

typedef enum
{
	VoltageCalibratePage = 0,
	MainPage,
	MenuDisplayPage
}MainPageSelect;

typedef struct
{
	uint8_t sDisEnable;			//主界面显示使能
	uint8_t sDisRefreshEnable;	//主界面刷新使能
	uint8_t sParamUpdateEnable;	//参数更新使能
	uint8_t sDisFlag;			//显示状态标志
	uint8_t vDisRefreshCnt;		//显示刷新计数
	uint8_t vDisUnit;			//显示单位
	uint8_t vOldDataLen;		//上一次数据长度(显示Kpa使用)
	uint16_t vFontColour;		//当前显示字体颜色
	uint16_t vBackColour;		//当前显示背景颜色
	uint16_t vBacklightPwmVal;	//背光PWM值
	int32_t vAirZeroingVal;		//气压调零值
	int32_t vAirPressureVal;	//当前气压值(100Pa为基准)
	int32_t vAirPressureOriginalVal;	//原始压力值(100Pa为基准)
	uint32_t vADCxVal;			//当前ADC值
	double vRefVoltageVal;		//当前外部参考电压值
}LCD_MainPageDataTypedef;

/****************任务编号****************/
static volatile int32_t vShortTimeTaskNumber = -1;			//1ms任务编号
static volatile int32_t vResultCalculateTaskNumber = -1;	//测量结果计算任务编号
static volatile int32_t vLCD_MainInterfaceTaskNumber = -1;	//主界面显示任务编号

/****************外部参考电压相关数据****************/
static uint16_t VrefAdcVal = 0;
static uint16_t VccVoltage = 0;

/****************ADC滤波****************/
static uint8_t sADCxFilterEnable = 0;		//滤波使能标志
//static uint8_t sADCxFilterFinish = 0;		//滤波完成标志
static uint16_t vADCxFilterCnt = 0;			//滤波次数增量
static uint16_t vADCxFilterOldVal = 0;		//滤波历史值
static uint32_t vADCxFilterVal = 0;			//当前滤波值

/****************主界面参数显示相关标志****************/
//static uint8_t sParamDisFlag = 0;

/****************零点相关数据****************/
//static double vP_ZeroVal = 0.00;
static uint8_t vP_ZeroFlag = 0;
static int32_t vP_ZeroVal = 0;

/****************电流输出计算相关****************/
static int32_t vCurrentCalculateBase = 0;
static double vCurrentCalculateRatio = 0.00;

/****************系统主页面状态****************/
static volatile uint8_t SystemPage = VoltageCalibratePage;

static LCD_MainPageDataTypedef _gLCD_MainPageDat =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	AWHITE,
	BLACK,
	0,
	0,
	0,
	0,
	0,
	0.000000,
};


/****************************************
 *函数名称：MainInterfaceDisplay
 *功能：主界面开启或关闭显示
 *参数说明：
 ****************************************/
static void MainInterfaceDisplay(uint8_t OnOff)
{
	if(OnOff)
	{
//		_gLCD_MainPageDat.sDisEnable = 1;
		_gLCD_MainPageDat.sParamUpdateEnable = 1;
		_gLCD_MainPageDat.sDisRefreshEnable = 1;
	}
	else
	{
		_gLCD_MainPageDat.sDisEnable = 0;
		_gLCD_MainPageDat.sDisRefreshEnable = 0;
	}
}

/****************************************
 *函数名称：MenuPageParamDisplayTask
 *功能：主界面参数显示任务
 *参数说明：
 ****************************************/
static void MainPageParamDisplayTask(uint8_t page_num)
{
	char buf[12];
	uint8_t TempVal_U8 = 0;
	uint16_t Pos = 0;
	uint16_t TempVal_U16 = 0;
	int32_t TempVal_S32 = 0;
	
	//
	if(AppDataRead(APP_SystemLanguage) == Chinese)
	{
		LCD_Fill(SystemDisplay_X,16,LCD_W,LCD_H,_gLCD_MainPageDat.vBackColour);
		switch(page_num)
		{
			case 1:
				LCD_Fill(SystemDisplay_X,0,LCD_W,16,_gLCD_MainPageDat.vFontColour);
			
//				LCD_ShowChinese(60,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,31);
//				LCD_ShowChinese(73,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,10);
//				LCD_ShowChinese(85,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,11);
				LCD_ShowString(67,2,(uint8_t*)"OUT1",_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1);
				LCD_ShowChinese(91,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,23);
				LCD_ShowChinese(103,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,24);
				//模式
				switch(AppDataRead(APP_Out1Mode))
				{
					case SystemChannelMode_Simple: 
						LCD_ShowChinese(130,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,0);
						LCD_ShowChinese(142,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,1);
					break;
					
					case SystemChannelMode_Lag: 
						LCD_ShowChinese(130,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,4);
						LCD_ShowChinese(142,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,5);
					break;
					
					case SystemChannelMode_Window: 
						LCD_ShowChinese(130,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,6);
						LCD_ShowChinese(142,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,7);
					break;
				}
				//通道1参数
				if(AppDataRead(APP_Out1Mode) == SystemChannelMode_Simple)
				{
					Pos = 22;
					LCD_ShowChinese(22,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,12);
					Pos += 12;
					LCD_ShowChinese(34,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
					Pos += 12;
					LCD_ShowChar(46,18,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					//通道1阈值
					TempVal_S32 = AppDataRead(APP_Out1ThresholdVal);
					if(TempVal_S32 < 0)
					{
						TempVal_U16 = ~TempVal_S32 + 1;
						LCD_ShowChar(Pos,18,'-',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
						Pos += 6;
					}
					else
					{
						TempVal_U16 = TempVal_S32;
					}
					TempVal_S32 = TempVal_U16/10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += TempVal_U8*6;
					LCD_ShowChar(Pos,18,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = TempVal_U16%10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);	
				}
				else
				{
					Pos = 22;
					//峰值
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,18);
					Pos += 12;
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
					Pos += 12;
					LCD_ShowChar(Pos,18,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					//通道1峰值
					TempVal_S32 = AppDataRead(APP_Out1PeakVal);
					if(TempVal_S32 < 0)
					{
						TempVal_U16 = ~TempVal_S32 + 1;
						LCD_ShowChar(Pos,18,'-',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
						Pos += 6;
					}
					else
					{
						TempVal_U16 = TempVal_S32;
					}
					TempVal_S32 = TempVal_U16/10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += TempVal_U8*6;
					LCD_ShowChar(Pos,18,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = TempVal_U16%10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += (TempVal_U8+1)*6;			
					//谷值
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,19);
					Pos += 12;
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
					Pos += 12;
					LCD_ShowChar(Pos,18,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = AppDataRead(APP_Out1ValleyVal);
					if(TempVal_S32 < 0)
					{
						TempVal_U16 = ~TempVal_S32 + 1;
						LCD_ShowChar(Pos,18,'-',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
						Pos += 6;
					}
					else
					{
						TempVal_U16 = TempVal_S32;
					}
					TempVal_S32 = TempVal_U16/10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += TempVal_U8*6;
					LCD_ShowChar(Pos,18,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = TempVal_U16%10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				}
				//迟滞值
				LCD_ShowChinese(22,34,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,4);
				LCD_ShowChinese(34,34,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,5);
				LCD_ShowChinese(46,34,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
				LCD_ShowChar(58,34,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				TempVal_S32 = AppDataRead(APP_Out1LagVal);
				TempVal_U16 = TempVal_S32;
				LCD_ShowIntNum(64,34,TempVal_U16/10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowChar(70,34,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				LCD_ShowIntNum(76,34,TempVal_U16%10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowString(82,34,(uint8_t *)"kPa",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				//响应时间
				LCD_ShowChinese(22,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,14);
				LCD_ShowChinese(34,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,15);
				LCD_ShowChinese(46,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,16);
				LCD_ShowChinese(58,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,17);
				TempVal_S32 = AppDataRead(APP_Out1ResponseTimeVal);
				TempVal_U16 = TempVal_S32;
				snprintf(buf, sizeof(buf), ":%dms", TempVal_U16);
				LCD_ShowString(70,50,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				//输出方式
				LCD_ShowChinese(22,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,20);
				LCD_ShowChinese(34,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,21);
				LCD_ShowChinese(46,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,22);
				LCD_ShowChinese(58,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,3);
				LCD_ShowChar(70,66,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				switch(AppDataRead(APP_Out1Way))
				{
					case SystemOutWay_NoReverse: 
						LCD_ShowChinese(76,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,28);
						LCD_ShowChinese(88,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,29);
					break;
					
					case SystemOutWay_Reverse: 
						LCD_ShowChinese(76,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,28);
						LCD_ShowChinese(88,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,30);
					break;
				}
			break;
			
			case 2:
				LCD_Fill(SystemDisplay_X,0,LCD_W,16,_gLCD_MainPageDat.vFontColour);
			
//				LCD_ShowChinese(60,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,32);
//				LCD_ShowChinese(73,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,10);
//				LCD_ShowChinese(85,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,11);
				LCD_ShowString(67,2,(uint8_t*)"OUT2",_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1);
				LCD_ShowChinese(91,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,23);
				LCD_ShowChinese(103,2,_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1,24);
				//模式
				switch(AppDataRead(APP_Out2Mode))
				{
					case SystemChannelMode_Simple: 
						LCD_ShowChinese(130,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,0);
						LCD_ShowChinese(142,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,1);
					break;
					
					case SystemChannelMode_Lag: 
						LCD_ShowChinese(130,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,4);
						LCD_ShowChinese(142,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,5);
					break;
					
					case SystemChannelMode_Window: 
						LCD_ShowChinese(130,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,6);
						LCD_ShowChinese(142,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,7);
					break;
				}
				//通道1参数
				if(AppDataRead(APP_Out2Mode) == SystemChannelMode_Simple)
				{
					Pos = 22;
					LCD_ShowChinese(22,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,12);
					Pos += 12;
					LCD_ShowChinese(34,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
					Pos += 12;
					LCD_ShowChar(46,18,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					//通道1阈值
					TempVal_S32 = AppDataRead(APP_Out2ThresholdVal);
					if(TempVal_S32 < 0)
					{
						TempVal_U16 = ~TempVal_S32 + 1;
						LCD_ShowChar(Pos,18,'-',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
						Pos += 6;
					}
					else
					{
						TempVal_U16 = TempVal_S32;
					}
					TempVal_S32 = TempVal_U16/10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += TempVal_U8*6;
					LCD_ShowChar(Pos,18,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = TempVal_U16%10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);	
				}
				else
				{
					Pos = 22;
					//峰值
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,18);
					Pos += 12;
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
					Pos += 12;
					LCD_ShowChar(Pos,18,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					//通道1峰值
					TempVal_S32 = AppDataRead(APP_Out2PeakVal);
					if(TempVal_S32 < 0)
					{
						TempVal_U16 = ~TempVal_S32 + 1;
						LCD_ShowChar(Pos,18,'-',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
						Pos += 6;
					}
					else
					{
						TempVal_U16 = TempVal_S32;
					}
					TempVal_S32 = TempVal_U16/10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += TempVal_U8*6;
					LCD_ShowChar(Pos,18,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = TempVal_U16%10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += (TempVal_U8+1)*6;			
					//谷值
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,19);
					Pos += 12;
					LCD_ShowChinese(Pos,18,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
					Pos += 12;
					LCD_ShowChar(Pos,18,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = AppDataRead(APP_Out2ValleyVal);
					if(TempVal_S32 < 0)
					{
						TempVal_U16 = ~TempVal_S32 + 1;
						LCD_ShowChar(Pos,18,'-',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
						Pos += 6;
					}
					else
					{
						TempVal_U16 = TempVal_S32;
					}
					TempVal_S32 = TempVal_U16/10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += TempVal_U8*6;
					LCD_ShowChar(Pos,18,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
					Pos += 6;
					TempVal_S32 = TempVal_U16%10;
					TempVal_U8 = snprintf(buf, sizeof(buf), "%d", TempVal_S32);
					LCD_ShowString(Pos,18,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				}
				//迟滞值
				LCD_ShowChinese(22,34,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,4);
				LCD_ShowChinese(34,34,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,5);
				LCD_ShowChinese(46,34,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,13);
				LCD_ShowChar(58,34,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				TempVal_S32 = AppDataRead(APP_Out2LagVal);
				TempVal_U16 = TempVal_S32;
				LCD_ShowIntNum(64,34,TempVal_U16/10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowChar(70,34,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				LCD_ShowIntNum(76,34,TempVal_U16%10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowString(82,34,(uint8_t *)"kPa",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				//响应时间
				LCD_ShowChinese(22,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,14);
				LCD_ShowChinese(34,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,15);
				LCD_ShowChinese(46,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,16);
				LCD_ShowChinese(58,50,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,17);
				TempVal_S32 = AppDataRead(APP_Out2ResponseTimeVal);
				TempVal_U16 = TempVal_S32;
				snprintf(buf, sizeof(buf), ":%dms", TempVal_U16);
				LCD_ShowString(70,50,(uint8_t *)buf,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				//输出方式
				LCD_ShowChinese(22,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,20);
				LCD_ShowChinese(34,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,21);
				LCD_ShowChinese(46,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,22);
				LCD_ShowChinese(58,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,3);
				LCD_ShowChar(70,66,':',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				switch(AppDataRead(APP_Out2Way))
				{
					case SystemOutWay_NoReverse: 
						LCD_ShowChinese(76,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,28);
						LCD_ShowChinese(88,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,29);
					break;
					
					case SystemOutWay_Reverse: 
						LCD_ShowChinese(76,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,28);
						LCD_ShowChinese(88,66,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0,30);
					break;
				}
		}
	}
	else if(AppDataRead(APP_SystemLanguage) == English)
	{
		LCD_Fill(SystemDisplay_X,14,LCD_W,LCD_H,_gLCD_MainPageDat.vBackColour);
		switch(page_num)
		{
			case 1:
				LCD_Fill(SystemDisplay_X,0,LCD_W,14,_gLCD_MainPageDat.vFontColour);
//				LCD_ShowString(46,1,(uint8_t *)"1 Channel Param",_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1);
				LCD_ShowString(61,1,(uint8_t *)"OUT1 Param",_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1);
				//模式
				switch(AppDataRead(APP_Out1Mode))
				{
					case SystemChannelMode_Simple: 
						LCD_ShowString(58,15,(uint8_t *)"Mode:Simple",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
					
					case SystemChannelMode_Lag: 
						LCD_ShowString(46,15,(uint8_t *)"Mode:Hysteresis",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
					
					case SystemChannelMode_Window: 
						LCD_ShowString(58,15,(uint8_t *)"Mode:Window",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
				}
				//迟滞值
				LCD_ShowString(40,28,(uint8_t *)"Hysteresis Value:",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				TempVal_S32 = AppDataRead(APP_Out1LagVal);
				TempVal_U16 = TempVal_S32;
//				LCD_ShowChar(70,40,'+',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowIntNum(73,40,TempVal_U16/10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowChar(79,40,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				LCD_ShowIntNum(85,40,TempVal_U16%10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowString(91,40,(uint8_t *)"kPa",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				//响应时间
				LCD_ShowString(31,53,(uint8_t *)"Response Time:",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				TempVal_S32 = AppDataRead(APP_Out1ResponseTimeVal);
				TempVal_U16 = TempVal_S32;
//				LCD_ShowChar(112,53,'+',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowIntNum(115,53,TempVal_U16,4,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				LCD_ShowString(139,53,(uint8_t *)"ms",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				//输出方式
				switch(AppDataRead(APP_Out1Way))
				{
					case SystemOutWay_NoReverse: 
//						LCD_ShowString(34,66,(uint8_t *)"Output Mode:Forward",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
						LCD_ShowString(43,66,(uint8_t *)"Output Mode:Open",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
					
					case SystemOutWay_Reverse: 
//						LCD_ShowString(31,66,(uint8_t *)"Output Mode:Inverted",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
						LCD_ShowString(40,66,(uint8_t *)"Output Mode:Close",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
				}
			break;
			
			case 2:
				LCD_Fill(SystemDisplay_X,0,LCD_W,14,_gLCD_MainPageDat.vFontColour);
//				LCD_ShowString(46,1,(uint8_t *)"2 Channel Param",_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1);
				LCD_ShowString(61,1,(uint8_t *)"OUT2 Param",_gLCD_MainPageDat.vBackColour,_gLCD_MainPageDat.vFontColour,12,1);
				//模式
				switch(AppDataRead(APP_Out2Mode))
				{
					case SystemChannelMode_Simple: 
						LCD_ShowString(58,15,(uint8_t *)"Mode:Simple",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
					
					case SystemChannelMode_Lag: 
						LCD_ShowString(46,15,(uint8_t *)"Mode:Hysteresis",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
					
					case SystemChannelMode_Window: 
						LCD_ShowString(58,15,(uint8_t *)"Mode:Window",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
				}
				//迟滞值
				LCD_ShowString(40,28,(uint8_t *)"Hysteresis Value:",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				TempVal_S32 = AppDataRead(APP_Out2LagVal);
				TempVal_U16 = TempVal_S32;
//				LCD_ShowChar(70,40,'+',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowIntNum(73,40,TempVal_U16/10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowChar(79,40,'.',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				LCD_ShowIntNum(85,40,TempVal_U16%10,1,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowString(91,40,(uint8_t *)"kPa",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				//响应时间
				LCD_ShowString(31,53,(uint8_t *)"Response Time:",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);	
				TempVal_S32 = AppDataRead(APP_Out2ResponseTimeVal);
				TempVal_U16 = TempVal_S32;
//				LCD_ShowChar(112,53,'+',_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,0);
				LCD_ShowIntNum(115,53,TempVal_U16,4,_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				LCD_ShowString(139,53,(uint8_t *)"ms",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
				//输出方式
				switch(AppDataRead(APP_Out2Way))
				{
					case SystemOutWay_NoReverse: 
//						LCD_ShowString(34,66,(uint8_t *)"Output Mode:Forward",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
						LCD_ShowString(43,66,(uint8_t *)"Output Mode:Open",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
					
					case SystemOutWay_Reverse: 
//						LCD_ShowString(31,66,(uint8_t *)"Output Mode:Inverted",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
						LCD_ShowString(40,66,(uint8_t *)"Output Mode:Close",_gLCD_MainPageDat.vFontColour,_gLCD_MainPageDat.vBackColour,12,1);
					break;
				}
			break;
		}
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void MainPageRefreshScan(LCD_MainPageDataTypedef *LCD_DisTemp, uint8_t Len)
{
	if(LCD_DisTemp->vOldDataLen != Len)
		LCD_DisTemp->vDisRefreshCnt = 0;
	
	switch(Len)
	{
		case 4:
			switch(LCD_DisTemp->vDisRefreshCnt)
			{
				case 0:
					LCD_Fill(147,64,LCD_W,LCD_H,LCD_DisTemp->vBackColour);
					if(LCD_DisTemp->vDisUnit == SystemUnit_Mpa)
						LCD_ShowString(123,64,(uint8_t *)"\x80\x81\x82",LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0);
					else
						LCD_ShowString(123,64,(uint8_t *)"\x83\x81\x82",LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0);
					
					LCD_DisTemp->sDisFlag |= 0x40;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 1:
					LCD_Fill(22,61,59,64,LCD_DisTemp->vBackColour);
					LCD_Fill(66,64,79,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
					
				case 2:
					LCD_Fill(22,64,34,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->sDisFlag |= 0x01;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 3:
					LCD_Fill(111,64,123,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->sDisFlag |= 0x02;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 4:
					LCD_Fill(22,0,59,20,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 5:
					LCD_Fill(22,20,59,40,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 6:
					LCD_Fill(22,40,59,61,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 7:
					LCD_Fill(0,0,22,40,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 8:
					LCD_Fill(0,40,22,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt = 0;
				break;
			}
		break;
		
		case 3:
			switch(LCD_DisTemp->vDisRefreshCnt)
			{
				case 0:
					LCD_Fill(147,64,LCD_W,LCD_H,LCD_DisTemp->vBackColour);
					if(LCD_DisTemp->vDisUnit == SystemUnit_Mpa)
						LCD_ShowString(123,64,(uint8_t *)"\x80\x81\x82",LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0);
					else
						LCD_ShowString(123,64,(uint8_t *)"\x83\x81\x82",LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0);
					
					LCD_DisTemp->sDisFlag |= 0x40;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 1:
					LCD_Fill(22,61,83,64,LCD_DisTemp->vBackColour);
					LCD_Fill(66,64,79,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
					
				case 2:
					LCD_Fill(22,64,34,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->sDisFlag |= 0x01;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 3:
					LCD_Fill(111,64,123,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->sDisFlag |= 0x02;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 4:
					LCD_Fill(22,0,83,12,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 5:
					LCD_Fill(22,12,83,24,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 6:
					LCD_Fill(22,24,83,36,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 7:
					LCD_Fill(22,36,83,48,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 8:
					LCD_Fill(22,48,83,61,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 9:
					LCD_Fill(0,0,22,40,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 10:
					LCD_Fill(0,40,22,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt = 0;
				break;
			}
		break;
		
		case 2:
			switch(LCD_DisTemp->vDisRefreshCnt)
			{
				case 0:
					LCD_Fill(147,64,LCD_W,LCD_H,LCD_DisTemp->vBackColour);
					if(LCD_DisTemp->vDisUnit == SystemUnit_Mpa)
						LCD_ShowString(123,64,(uint8_t *)"\x80\x81\x82",LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0);
					else
						LCD_ShowString(123,64,(uint8_t *)"\x83\x81\x82",LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0);
					
					LCD_DisTemp->sDisFlag |= 0x40;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 1:
					LCD_Fill(22,61,107,64,LCD_DisTemp->vBackColour);
					LCD_Fill(66,64,79,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
					
				case 2:
					LCD_Fill(22,64,34,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->sDisFlag |= 0x01;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 3:
					LCD_Fill(111,64,123,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->sDisFlag |= 0x02;
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 4:
					LCD_Fill(22,0,107,8,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 5:
					LCD_Fill(22,8,107,16,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 6:
					LCD_Fill(22,16,107,25,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 7:
					LCD_Fill(22,25,107,34,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 8:
					LCD_Fill(22,34,107,43,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 9:
					LCD_Fill(22,43,107,52,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 10:
					LCD_Fill(22,52,107,61,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 11:
					LCD_Fill(0,0,22,40,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt++;
				break;
				
				case 12:
					LCD_Fill(0,40,22,LCD_H,LCD_DisTemp->vBackColour);
					LCD_DisTemp->vDisRefreshCnt = 0;
				break;
			}
		break;
	}
}

/****************************************
 *函数名称：ShortTimeTaskHandle
 *功能：周期1ms任务处理函数
 *参数说明：
 ****************************************/
static void ShortTimeTask(void *param)
{
	//按键扫描
	KEY_Scan();
}

/****************************************
 *函数名称：ResultCalculateTask
 *功能：测量结果计算任务
 *参数说明：
 ****************************************/
static void ResultCalculateTask(void *param)
{
//	uint8_t temp = 0;
	int32_t TempVal_AirPressure = 0;
//	double TempVal_LF = 0.00;
	
	LCD_MainPageDataTypedef *LCD_DisTemp = (LCD_MainPageDataTypedef *)param;
	
	//参数刷新
	if(LCD_DisTemp->sParamUpdateEnable)
	{
		if(AppDataRead(APP_AirZeroingVal) >= 0) vP_ZeroFlag = 1;
		else vP_ZeroFlag = 0;
		vP_ZeroVal = AppDataRead(APP_AirZeroingVal);
		
		LCD_DisTemp->vDisUnit = (uint8_t)AppDataRead(APP_SystemUnit);
		LCD_DisTemp->vRefVoltageVal = (double)AppDataRead(APP_SystemInputVoltage)/1000;
		LCD_DisTemp->vBacklightPwmVal = AppDataRead(APP_SystemBacklightGrade) * 10;
		LCD_DisTemp->vBackColour = AppDataRead(APP_SystemBackColour);
		LCD_DisTemp->vFontColour = AppDataRead(APP_SystemFontColour);
		ExternalOutput_Init();	//更新外部输出相关参数
		
		LCD_DisTemp->sParamUpdateEnable = 0;
		LCD_DisTemp->sDisEnable = 1;
	}
	
	TempVal_AirPressure = Get_INT_Value();
	
	//仅为正
	if(Param_Config.AirPressureValueType == 1)
	{
		if(TempVal_AirPressure <= 0)
		{
			TempVal_AirPressure = 0;
		}
	}
		
	//仅为负
if( Param_Config.AirPressureValueType == 2)
{
	if(TempVal_AirPressure >= 0)
	{
		TempVal_AirPressure = 0;
	}
}
		
	
	LCD_DisTemp->vAirPressureOriginalVal = TempVal_AirPressure;
	
	if(vP_ZeroFlag)
	{
		if(TempVal_AirPressure >= 0)
		{
			//消除零点误差
			TempVal_AirPressure -= vP_ZeroVal;
			if(TempVal_AirPressure <= 0) TempVal_AirPressure = 0;
		}
		else
		{
			//消除零点误差
			TempVal_AirPressure += Param_Config.ZeroingExtraScopeVal;
			if(TempVal_AirPressure >= 0) TempVal_AirPressure = 0;
		}
	}
	else
	{
		if(TempVal_AirPressure >= 0)
		{
			//消除零点误差
			TempVal_AirPressure -= Param_Config.ZeroingExtraScopeVal;
			if(TempVal_AirPressure <= 0) TempVal_AirPressure = 0;
		}
		else
		{
			//消除零点误差
			TempVal_AirPressure -= vP_ZeroVal;
			if(TempVal_AirPressure >= 0) TempVal_AirPressure = 0;
		}
	}
	
	if(TempVal_AirPressure >= Param_Config.PressureUpperLimit) TempVal_AirPressure = Param_Config.PressureUpperLimit;
	else if(TempVal_AirPressure <= Param_Config.PressureLowerLimit) TempVal_AirPressure = Param_Config.PressureLowerLimit;
	
//	if(TempVal_AirPressure < 0)
//	{
//		TempVal_AirPressure = 0;
//		LCD_DisTemp->vAirPressureOriginalVal = 0;
//		memset(&TempVal_LF, 0, sizeof(double));
//	}
//	else
//	{
//		LCD_DisTemp->vAirPressureOriginalVal = TempVal_LF * 10;
//		//消除零点误差
//		TempVal_LF -= vP_ZeroVal;
//		TempVal_AirPressure = TempVal_LF;
//		//
//		if(TempVal_AirPressure <= 0)
//		{
//			TempVal_AirPressure = 0;
//			memset(&TempVal_LF, 0, sizeof(double));
//		}
//		else
//		{
//			TempVal_LF *= 10;	//转化成100Pa为单位
//			TempVal_AirPressure = TempVal_LF;
//			TempVal_LF -= TempVal_AirPressure;
//			temp = TempVal_LF*10;
//		}
//	}

	//
	if(TempVal_AirPressure >= 0)
	{
		LCD_DisTemp->vAirPressureVal = TempVal_AirPressure;
		LCD_DisTemp->sDisFlag &= ~0x10;
	}
	else
	{
		LCD_DisTemp->vAirPressureVal = ~TempVal_AirPressure + 1;
		LCD_DisTemp->sDisFlag |= 0x10;
	}
	
//	if(temp >= 5)
//		LCD_DisTemp->vAirPressureVal++;
}

/****************************************
 *函数名称：LCD_MainInterfaceTask
 *功能：主界面显示任务
 *参数说明：
 ****************************************/
static void LCD_MainInterfaceTask(void *param)
{
//	NpnOut_Ctrl(NPN2,0);
	uint8_t i = 0;
	uint8_t Len = 0;
	uint8_t PointPos = 0;
	uint8_t PosBuff[4] = {0,0,0,0};
	uint8_t DataBuff[5] = {0,0,0,0,0};
	uint16_t DataFontCol = 0;
	uint16_t CurrentVal = 0;
	
	LCD_MainPageDataTypedef *LCD_DisTemp = (LCD_MainPageDataTypedef *)param;
	
//	LCD_DisTemp->vAirPressureVal = Get_INT_Value();
	if(LCD_DisTemp->sDisEnable)
	{
		//显示固定内容，重新读取一遍相关数据
		if(LCD_DisTemp->sDisRefreshEnable)
		{
			if((LCD_DisTemp->sDisFlag & 0x80) == 0x00)
			{
				LCD_Fill(0,0,SystemDisplay_X,LCD_H,BLACK);
				LCD_Fill(SystemDisplay_X,0,LCD_W,LCD_H,LCD_DisTemp->vBackColour);
				LCD_DisTemp->sDisFlag |= 0x80;
			}
			
			LCD_DisTemp->sDisFlag |= 0x40;
			LCD_DisTemp->sDisRefreshEnable = 0;
		}            
		
		Len = My_NumberCount(LCD_DisTemp->vAirPressureVal);
		My_IntegerSplit(LCD_DisTemp->vAirPressureVal, Len, DataBuff);
		//
//		if(LCD_DisTemp->vAirPressureVal < 0)
//		{
//			vCurrentCalculateBase
//		}
		CurrentVal = vCurrentCalculateRatio * (LCD_DisTemp->vAirPressureVal + vCurrentCalculateBase);
		CurrentVal += 4000;
		
		if(LCD_DisTemp->vDisUnit == SystemUnit_Mpa) 
		{
			//右对齐显示
			PointPos = 83;
			PosBuff[0] = 59; PosBuff[1] = 88; PosBuff[2] = 112; PosBuff[3] = 136;
			DataFontCol = LCD_DisTemp->vFontColour;
			//
			MainPageRefreshScan(LCD_DisTemp, 4);
			//结果为负
			if(LCD_DisTemp->sDisFlag & 0x10)
			{
				LCD_DisTemp->sDisFlag |= 0x20;
				LCD_ShowChinese(PosBuff[0]-16,24,LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,16,0,64);
			}
			else
			{
				//上一次结果为负，清除符号显示
				if(LCD_DisTemp->sDisFlag & 0x20)
				{
					LCD_DisTemp->sDisFlag &= ~0x20;
//					LCD_DisTemp->sDisFlag |= 0x40;
//					LCD_Fill(PosBuff[0]-16,0,PosBuff[0],60,LCD_DisTemp->vBackColour);
					LCD_ShowChinese(PosBuff[0]-16,24,LCD_DisTemp->vBackColour,LCD_DisTemp->vBackColour,16,0,67);
				}
			}
			Len--;
			Len = 4 - Len;
			//显示压力值
			for(i=0; i<4; i++)
			{
				if(Len > i)
					LCD_ShowChar(PosBuff[i],0,48,LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,64,0);
				else
					LCD_ShowChar(PosBuff[i],0,DataBuff[i-Len]+48,LCD_DisTemp->vFontColour,LCD_DisTemp->vBackColour,64,0);
			}
			
			LCD_DisTemp->vOldDataLen = 4;
		}
		else	//Kpa
		{
			//右对齐显示
			switch(Len)
			{
				//大于等于1Mpa
				case 5:
					PointPos = 131;
					PosBuff[0] = 59; PosBuff[1] = 83; PosBuff[2] = 107; PosBuff[3] = 136;
					DataBuff[0] = 9; DataBuff[1] = 9; DataBuff[2] = 9; DataBuff[3] = 9; DataBuff[4] = 0;
					Len = 4;
					DataFontCol = RED;
				break;
				
				case 4:
					PointPos = 131;
					PosBuff[0] = 59; PosBuff[1] = 83; PosBuff[2] = 107; PosBuff[3] = 136;
					DataFontCol = LCD_DisTemp->vFontColour;
				break;
					
				case 3:
					PointPos = 131;
					PosBuff[0] = 83; PosBuff[1] = 107; PosBuff[2] = 136; PosBuff[3] = 0;
					DataFontCol = LCD_DisTemp->vFontColour;
				break;
				
				case 2:
					PointPos = 131;
					PosBuff[0] = 107; PosBuff[1] = 136; PosBuff[2] = 0; PosBuff[3] = 0;
					DataFontCol = LCD_DisTemp->vFontColour;
				break;
				
				case 1:
					PointPos = 131;
					PosBuff[0] = 107; PosBuff[1] = 136; PosBuff[2] = 0; PosBuff[3] = 0;
					Len = 2;
					DataBuff[1] = DataBuff[0];
					DataBuff[0] = 0;
					DataFontCol = LCD_DisTemp->vFontColour;
				break;
			}
			//
			MainPageRefreshScan(LCD_DisTemp, Len);
			//结果为负
			if(LCD_DisTemp->sDisFlag & 0x10)
			{
				//检查当前显示数据两边是否有历史残留
				if(LCD_DisTemp->vOldDataLen > Len)
				{
					//上一次结果为正
					if((LCD_DisTemp->sDisFlag & 0x20) == 0x00)
					{
						LCD_Fill(PosBuff[0]-(LCD_DisTemp->vOldDataLen - Len)*24,0,PosBuff[0],64,LCD_DisTemp->vBackColour);
					}
					else
					{
						LCD_Fill(PosBuff[0]-16-(LCD_DisTemp->vOldDataLen - Len)*24,0,PosBuff[0],64,LCD_DisTemp->vBackColour);
					}
				}
				LCD_ShowChinese(PosBuff[0]-16,24,DataFontCol,LCD_DisTemp->vBackColour,16,0,64);
				LCD_DisTemp->sDisFlag |= 0x20;
			}
			else
			{
				//检查当前显示数据两边是否有历史残留
				if(LCD_DisTemp->vOldDataLen > Len)
				{
					//上一次结果为负
					if(LCD_DisTemp->sDisFlag & 0x20)
					{
						LCD_Fill(PosBuff[0]-16-(LCD_DisTemp->vOldDataLen - Len)*24,0,PosBuff[0],64,LCD_DisTemp->vBackColour);
						LCD_DisTemp->sDisFlag &= ~0x20;
					}
					else
					{
						LCD_Fill(PosBuff[0]-(LCD_DisTemp->vOldDataLen - Len)*24,0,PosBuff[0],64,LCD_DisTemp->vBackColour);
					}
				}
				else
				{
					//上一次结果为负
					if(LCD_DisTemp->sDisFlag & 0x20)
					{
						LCD_Fill(PosBuff[0]-16,0,PosBuff[0],64,LCD_DisTemp->vBackColour);
						LCD_DisTemp->sDisFlag &= ~0x20;
					}
				}
			}
			//显示压力值
			for(i=0; i<Len; i++)
			{
				LCD_ShowChar(PosBuff[i],0,DataBuff[i]+48,DataFontCol,LCD_DisTemp->vBackColour,64,0);
			}
			//
//			if(LCD_DisTemp->vOldDataLen != Len)
//				LCD_DisTemp->sDisFlag |= 0x40;
			
			LCD_DisTemp->vOldDataLen = Len;
		}
		//检查小数点是否有刷新需求
		if(LCD_DisTemp->sDisFlag & 0x40)
		{
//			if((LCD_DisTemp->sDisFlag & 0x08) == 0x00)
//			{
//				LCD_Fill(PointPos,0,PointPos+5,35,LCD_DisTemp->vBackColour);
//				LCD_Fill(PointPos,35,PointPos+5,40,LCD_DisTemp->vFontColour);
				LCD_Fill(PointPos,0,PointPos+5,64,LCD_DisTemp->vBackColour);
				LCD_Fill(PointPos,49,PointPos+5,54,DataFontCol);
//			}
			LCD_DisTemp->sDisFlag &= ~0x40;
		}
//		//刷新主界面固定内容
//		if(LCD_DisTemp->vDisRefreshCnt >= FixedContentRefreshPer)
//		{
////			LCD_DisTemp->sDisFlag |= 0x80;
//			LCD_DisTemp->vDisRefreshCnt = 0;
//			LCD_DisTemp->sDisRefreshEnable = 1;
//		}
//		LCD_DisTemp->vDisRefreshCnt++;
		
		//刷新显示控件
		if(LCD_DisTemp->sDisFlag & 0x01)
		{
			ExternalOutputImageRef(OutChannel_1);
			LCD_DisTemp->sDisFlag &= ~0x01;
		}
		if(LCD_DisTemp->sDisFlag & 0x02)
		{
			ExternalOutputImageRef(OutChannel_2);
			LCD_DisTemp->sDisFlag &= ~0x02;
		}
		//启动一次外部输出检测
		ExternalOutputChannelEnable(OutChannel_1,1);
		ExternalOutputChannelEnable(OutChannel_2,1);
		//更新电流输出
		CurrentOutput_UA(CurrentVal);
//		//刷新显示占空比
//		PWM_TIM1_Channel2_DutySet(_gLCD_MainPageDat.vBacklightPwmVal);
	}
//	NpnOut_Ctrl(NPN2,1);
}

/****************************************
 *函数名称：System_DeviceInit
 *功能：系统外设初始化
 *参数说明：
 ****************************************/
void System_DeviceInit(void)
{
	GPIO_BSP_Init();
	Timer_Init();
	AppDataInit();
	KEY_Init();
	ADC_Bsp_Init();
	CurrentOutput_Init();
	
	LCD_Init();
	LCD_BackLightCtrl();
	
	//关闭菜单下按键权限
	MeterInterfaceKeyShield(FunctionKey_Disbale);
}

/****************************************
 *函数名称：SystemParameterReset
 *功能：系统参数恢复默认
 *参数说明：
 ****************************************/
void System_ParameterReset(void)
{
	AppDataTypeDef *AppDataTemp;
	AppDataTemp = (AppDataTypeDef *)(uint32_t *)(0x0801F000);
	if((AppDataTemp->Head == 0xAA5555AA) && (AppDataTemp->End == 0x55AAAA55))
	{
		RangeNumber = AppDataTemp->RangeNum;
		RangeSet_Flag = AppDataTemp->RangeSetFlag;
	}
	
	if(RangeNumber == 0 )//
	{
		Param_Config.ZeroingExtraScopeVal = ((int32_t)4);
		Param_Config.ProductClass = LowPressureSeriesProduct;
		Param_Config.AirPressureValueType = 0;
			
		Param_Config.PressureUpperLimit = ((int32_t)1000);
		Param_Config.PressureLowerLimit = ((int32_t)-1000);
		Param_Config.LagValUpperLimit = ((int32_t)10);
		Param_Config.LagValLowerLimit =((int32_t)1);
		Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
		Param_Config.DelayTimeLowerLimit = ((int32_t)2);
		Param_Config.ZeroingValUpperLimit = ((int32_t)50);
		Param_Config.ZeroingValLowerLimit = ((int32_t)-50);
		Param_Config.ZeroingErrorRange = ((int32_t)4);
			
		Param_Config.Default_LagVal = ((int32_t)1);
		Param_Config.Default_Ch1_ThresholdVal = ((int32_t)200);
		Param_Config.Default_Ch1_PeakValVal = ((int32_t)-200);
		Param_Config.Default_Ch1_ValleyVal = ((int32_t)-600);
		Param_Config.Default_Ch2_ThresholdVal = ((int32_t)200);
		Param_Config.Default_Ch2_PeakValVal = ((int32_t)-200);
		Param_Config.Default_Ch2_ValleyVal = ((int32_t)-600);				
	}
		if(RangeNumber == 1 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)5);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 1;
			
			Param_Config.PressureUpperLimit = ((int32_t)2500);
			Param_Config.PressureLowerLimit = ((int32_t)0);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)125);
			Param_Config.ZeroingValLowerLimit = ((int32_t)0);
			Param_Config.ZeroingErrorRange = ((int32_t)5);
			
			Param_Config.Default_LagVal = ((int32_t)1);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)1500);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)1000);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)500);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)1500);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)1000);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)500);
		}
		if(RangeNumber == 2 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)2);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 2;
			
			
			Param_Config.PressureUpperLimit = ((int32_t)0);
			Param_Config.PressureLowerLimit = ((int32_t)-1000);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)0);
			Param_Config.ZeroingValLowerLimit = ((int32_t)-50);
			Param_Config.ZeroingErrorRange = ((int32_t)2);
			
			Param_Config.Default_LagVal = ((int32_t)1);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)-400);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)-600);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)-800);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)-400);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)-600);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)-800);
			
		}
		if(RangeNumber == 3 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)20);
			Param_Config.ProductClass = HighPressureSeriesProduct;
			Param_Config.AirPressureValueType = 1;
			
			Param_Config.PressureUpperLimit = ((int32_t)10000);
			Param_Config.PressureLowerLimit = ((int32_t)0);
			Param_Config.LagValUpperLimit = ((int32_t)100);
			Param_Config.LagValLowerLimit =((int32_t)10);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)500);
			Param_Config.ZeroingValLowerLimit = ((int32_t)0);
			Param_Config.ZeroingErrorRange = ((int32_t)20);
			
			Param_Config.Default_LagVal = ((int32_t)10);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)6000);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)4000);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)2000);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)6000);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)4000);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)2000);
			
		}
		if(RangeNumber == 4 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)2);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 1;
			
			Param_Config.PressureUpperLimit = ((int32_t)1000);
			Param_Config.PressureLowerLimit = ((int32_t)0);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)50);
			Param_Config.ZeroingValLowerLimit = ((int32_t)0);
			Param_Config.ZeroingErrorRange = ((int32_t)2);
			
			Param_Config.Default_LagVal = ((int32_t)1);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)600);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)400);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)200);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)600);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)400);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)200);
			
		}
		if(RangeNumber == 5 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)22);
			Param_Config.ProductClass = HighPressureSeriesProduct;
			Param_Config.AirPressureValueType = 0;
			
			Param_Config.PressureUpperLimit = ((int32_t)10000);
			Param_Config.PressureLowerLimit = ((int32_t)-1000);
			Param_Config.LagValUpperLimit = ((int32_t)100);
			Param_Config.LagValLowerLimit =((int32_t)10);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)500);
			Param_Config.ZeroingValLowerLimit = ((int32_t)-50);
			Param_Config.ZeroingErrorRange = ((int32_t)22);
			
			Param_Config.Default_LagVal = ((int32_t)10);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)5600);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)3400);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)1200);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)5600);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)3400);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)1200);
		}
		if(RangeNumber == 6 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)2);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 2;
			
			Param_Config.PressureUpperLimit = ((int32_t)0);
			Param_Config.PressureLowerLimit = ((int32_t)-1010);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)0);
			Param_Config.ZeroingValLowerLimit = ((int32_t)-50);
			Param_Config.ZeroingErrorRange = ((int32_t)2);
			
			Param_Config.Default_LagVal = ((int32_t)1);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)-400);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)-600);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)-800);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)-400);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)-600);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)-800);
			
		}
		if(RangeNumber == 7 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)11);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 0;
			
			Param_Config.PressureUpperLimit = ((int32_t)5000);
			Param_Config.PressureLowerLimit = ((int32_t)-500);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)250);
			Param_Config.ZeroingValLowerLimit = ((int32_t)-25);
			Param_Config.ZeroingErrorRange = ((int32_t)11);
			
			Param_Config.Default_LagVal = ((int32_t)1);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)2800);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)1700);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)600);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)2000);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)1700);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)600);
			
		}
		if(RangeNumber == 8 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)10);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 1;
			
			Param_Config.PressureUpperLimit = ((int32_t)5000);
			Param_Config.PressureLowerLimit = ((int32_t)0);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)250);
			Param_Config.ZeroingValLowerLimit = ((int32_t)0);
			Param_Config.ZeroingErrorRange = ((int32_t)10);
			
			Param_Config.Default_LagVal = ((int32_t)10);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)3000);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)2000);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)1000);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)3000);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)2000);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)1000);
			
		}
		if(RangeNumber == 9 )//
		{
			Param_Config.ZeroingExtraScopeVal = ((int32_t)4);
			Param_Config.ProductClass = LowPressureSeriesProduct;
			Param_Config.AirPressureValueType = 0;
			
			Param_Config.PressureUpperLimit = ((int32_t)1010);
			Param_Config.PressureLowerLimit = ((int32_t)-1010);
			Param_Config.LagValUpperLimit = ((int32_t)10);
			Param_Config.LagValLowerLimit =((int32_t)1);
			Param_Config.DelayTimeUpperLimit = ((int32_t)5000);
			Param_Config.DelayTimeLowerLimit = ((int32_t)2);
			Param_Config.ZeroingValUpperLimit = ((int32_t)50);
			Param_Config.ZeroingValLowerLimit = ((int32_t)-50);
			Param_Config.ZeroingErrorRange = ((int32_t)4);
			
			Param_Config.Default_LagVal = ((int32_t)1);
			Param_Config.Default_Ch1_ThresholdVal = ((int32_t)200);
			Param_Config.Default_Ch1_PeakValVal = ((int32_t)-200);
			Param_Config.Default_Ch1_ValleyVal = ((int32_t)-600);
			Param_Config.Default_Ch2_ThresholdVal = ((int32_t)200);
			Param_Config.Default_Ch2_PeakValVal = ((int32_t)-200);
			Param_Config.Default_Ch2_ValleyVal = ((int32_t)-600);
		}
	
//	//清空用户数据
//	APPDataEmpty(0);
	//系统语言：中文
	AppDataWrite((int32_t)Chinese, APP_SystemLanguage);
	//用户密码
	AppDataWrite((int32_t)0, APP_SystemPassword);
	//背光等级
	AppDataWrite((int32_t)5, APP_SystemBacklightGrade);
	//系统背景颜色
	AppDataWrite((int32_t)BLACK, APP_SystemBackColour);
	//系统字体颜色
	AppDataWrite((int32_t)AGREEN, APP_SystemFontColour);
	//ADC补偿值
	AppDataWrite((int32_t)0, APP_ADCxCompensationVal);
	//气压调零值
	AppDataWrite((int32_t)0, APP_AirZeroingVal);
	//输入电压：5V
	AppDataWrite((int32_t)5220, APP_SystemInputVoltage);
	//显示单位：Kpa
	AppDataWrite((int32_t)SystemUnit_Kpa, APP_SystemUnit);
	
	
	//通道
	AppDataWrite(Param_Config.PressureUpperLimit, APP_OutPressureUpperLimit);
	//通道
	AppDataWrite(Param_Config.PressureLowerLimit, APP_OutPressureLowerLimit);
	//通道
	AppDataWrite(Param_Config.LagValUpperLimit, APP_OutLagValUpperLimit);
	//通道
	AppDataWrite(Param_Config.LagValLowerLimit, APP_OutLagValLowerLimit);
	//通道
	AppDataWrite(Param_Config.DelayTimeUpperLimit, APP_OutDelayTimeUpperLimit);
	//通道
	AppDataWrite(Param_Config.DelayTimeLowerLimit, APP_OutDelayTimeLowerLimit);
	//通道
	AppDataWrite(Param_Config.ZeroingValUpperLimit, APP_OutZeroingValUpperLimit);
	//通道
	AppDataWrite(Param_Config.ZeroingValLowerLimit, APP_OutZeroingValLowerLimit);
	//通道
	AppDataWrite(Param_Config.ZeroingErrorRange, APP_OutZeroingErrorRange);
	
	
	
	//通道1模式：简易模式
	AppDataWrite((int32_t)SystemChannelMode_Simple, APP_Out1Mode);
	//通道1阈值：
	AppDataWrite(Param_Config.Default_Ch1_ThresholdVal, APP_Out1ThresholdVal);
	
	//通道1峰值：
	AppDataWrite(Param_Config.Default_Ch1_PeakValVal, APP_Out1PeakVal);
	//通道1谷值：
	AppDataWrite(Param_Config.Default_Ch1_ValleyVal, APP_Out1ValleyVal);

	//通道1迟滞值：
	AppDataWrite(Param_Config.Default_LagVal, APP_Out1LagVal);
	//通道1延迟时间：2ms
	AppDataWrite((int32_t)2, APP_Out1ResponseTimeVal);
	//通道1输出方式：正向输出
	AppDataWrite((int32_t)SystemOutWay_NoReverse, APP_Out1Way);
	
	//通道2模式：简易模式
	AppDataWrite((int32_t)SystemChannelMode_Simple, APP_Out2Mode);
	//通道2阈值：
	AppDataWrite(Param_Config.Default_Ch2_ThresholdVal, APP_Out2ThresholdVal);

	//通道2峰值：
	AppDataWrite(Param_Config.Default_Ch2_PeakValVal, APP_Out2PeakVal);
	//通道2谷值：
	AppDataWrite(Param_Config.Default_Ch2_ValleyVal, APP_Out2ValleyVal);
	
	//通道2迟滞值：
	AppDataWrite(Param_Config.Default_LagVal, APP_Out2LagVal);
	//通道2延迟时间：2ms
	AppDataWrite((int32_t)2, APP_Out2ResponseTimeVal);
	//通道2输出方式：正向输出
	AppDataWrite((int32_t)SystemOutWay_NoReverse, APP_Out2Way);
	//擦除并重新写入数据
	
	//标定相关数据
	AppDataWrite((int32_t)11, APP_CalibrationNumber);
	AppDataWrite((int32_t)788, APP_CalibrationVal1);
	AppDataWrite((int32_t)1103, APP_CalibrationVal2);
	AppDataWrite((int32_t)1419, APP_CalibrationVal3);
	AppDataWrite((int32_t)1734, APP_CalibrationVal4);
	AppDataWrite((int32_t)2049, APP_CalibrationVal5);
	AppDataWrite((int32_t)2364, APP_CalibrationVal6);
	AppDataWrite((int32_t)2680, APP_CalibrationVal7);
	AppDataWrite((int32_t)2995, APP_CalibrationVal8);
	AppDataWrite((int32_t)3310, APP_CalibrationVal9);
	AppDataWrite((int32_t)3625, APP_CalibrationVal10);
	AppDataWrite((int32_t)3941, APP_CalibrationVal11);
//	AppDataWrite((int32_t)-100, APP_CalibrationAirVal1);
//	AppDataWrite((int32_t)-80, APP_CalibrationAirVal2);
//	AppDataWrite((int32_t)-60, APP_CalibrationAirVal3);
//	AppDataWrite((int32_t)-40, APP_CalibrationAirVal4);
//	AppDataWrite((int32_t)-20, APP_CalibrationAirVal5);
//	AppDataWrite((int32_t)0, APP_CalibrationAirVal6);
//	AppDataWrite((int32_t)20, APP_CalibrationAirVal7);
//	AppDataWrite((int32_t)40, APP_CalibrationAirVal8);
//	AppDataWrite((int32_t)60, APP_CalibrationAirVal9);
//	AppDataWrite((int32_t)80, APP_CalibrationAirVal10);
//	AppDataWrite((int32_t)100, APP_CalibrationAirVal11);
//	APPDataFlashWrite();
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint32_t My_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint32_t My_NumberCount(uint32_t num)
{
	uint8_t result = 0;	 
	do
	{
		num = num / 10; 
		result++;
	}while(num != 0);		
	return result;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void My_IntegerSplit(uint32_t num, uint8_t len, uint8_t *buf)
{
	uint8_t i;
	
	for(i=0; i<len; i++)
		buf[i] = (num/My_Pow(10,len-i-1))%10;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void GetRefVoltageValue(uint16_t *adc_val, uint16_t *vol_val)
{
	if((adc_val == NULL) || (vol_val == NULL)) return;
	
	*adc_val = VrefAdcVal;
	*vol_val = VccVoltage;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
int32_t GetAirPressureVal(uint8_t flag)
{
	if(flag)
		return _gLCD_MainPageDat.vAirPressureOriginalVal;
	else
	{
		if(_gLCD_MainPageDat.sDisFlag & 0x10)
			return (~_gLCD_MainPageDat.vAirPressureVal + 1);
		else
			return _gLCD_MainPageDat.vAirPressureVal;
	}
}

///****************************************
// *函数名称：
// *功能：
// *参数说明：
// ****************************************/
//int32_t ADC_CompensationValNum(uint16_t true_vol)
//{
//	int32_t TempVal_S32 = 0;
//	double TempVal_LF = 0.00;
//	
//	TempVal_S32 = true_vol * VrefAdcVal - 4914000;
//	TempVal_LF = (double)TempVal_S32/4095;
//	
//	TempVal_LF = TempVal_LF/((double)VccVoltage/4095);
//	TempVal_S32 = round(TempVal_LF);
//	
//	return TempVal_S32;
//}

//static volatile uint8_t vMainPageSta = 0;
//bit7:0,第一次进入

static uint8_t sMainPageDisplaySta = 0;
static uint8_t vMainPageParamDisplayCnt = 0;
static uint32_t vMainPageDisplayTick = 0;
static uint32_t vMainPageIdleTick = 0;
/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void SystemTask(void)
{
	uint16_t TempVal_U16 = 0;
	int32_t TempVal_S32 = 0;
	
	//显示界面控制
	switch(SystemPage)
	{
		//外部参考电压校准
		case VoltageCalibratePage:
			ExternalReferenceVolGetTask();
			if(ExternalRefVolTaskReady(&VrefAdcVal, &VccVoltage))
			{
				//
				vCurrentCalculateBase = ~Param_Config.PressureLowerLimit + 1;
				if(Param_Config.PressureLowerLimit < 0)
				{
					TempVal_S32 = vCurrentCalculateBase;
					TempVal_S32 += Param_Config.PressureUpperLimit;
					vCurrentCalculateRatio = (double)16000 / TempVal_S32;
				}
				else
				{
					TempVal_S32 = Param_Config.PressureUpperLimit - Param_Config.PressureLowerLimit;
					vCurrentCalculateRatio = (double)16000 / TempVal_S32;
				}
				CurrentOutput_UpdateCalcData();
				//参数刷新
				_gLCD_MainPageDat.sParamUpdateEnable = 1;
				//主界面开启刷新
				_gLCD_MainPageDat.sDisRefreshEnable = 1;
				//ADC测量切换为压力传感器
				ADC_DeviceCtrl(0);
//				ADC_Bsp_Init(GasPressureTestPort);
				ADC_Channelx_Init(GasPressureTestPort);
				//ADC滤波开启
				sADCxFilterEnable = 1;
				//参考电压值
				_gLCD_MainPageDat.vRefVoltageVal = (double)VccVoltage/1000;
				//创建相关任务
				vShortTimeTaskNumber = TimerTaskCreate(1, ShortTimeTask, NULL);						//1ms处理任务
				vResultCalculateTaskNumber = TimerTaskCreate(200, ResultCalculateTask, &_gLCD_MainPageDat);			//更新、计算结果任务
				vLCD_MainInterfaceTaskNumber = TimerTaskCreate(350, LCD_MainInterfaceTask, &_gLCD_MainPageDat);		//主界面显示任务
				
				SystemPage = MainPage;
				_gLCD_MainPageDat.sDisFlag = 0x03;
			}
		break;		
		//主页面
		case MainPage:
			if(((_gLCD_MainPageDat.sDisFlag & 0x03) == 0) && ((sMainPageDisplaySta & 0x80) == 0x00))
			{
//				LCD_Fill(SystemDisplay_X,48,112,LCD_H,_gLCD_MainPageDat.vBackColour);
				sMainPageDisplaySta |= 0x80;
			}
			
			if(sMainPageDisplaySta & 0x80)
			{
				if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
				{
					//在参数信息查看页面中
					if(sMainPageDisplaySta & 0x20)
					{
						//开启显示
						MainInterfaceDisplay(1);
						sADCxFilterEnable = 1;
						_gLCD_MainPageDat.sDisFlag = 0x03;
						_gLCD_MainPageDat.vDisRefreshCnt = 0;
						_gLCD_MainPageDat.vOldDataLen = 0;
//						sParamDisFlag = 0;
						sMainPageDisplaySta = 0;
					}
					
					vMainPageDisplayTick = GetSystemTick();
					sMainPageDisplaySta |= 0x40;
				}
				else if(KEY_ReadEvent(DOWN, Short_Press_Once, 1))
				{
					if((sMainPageDisplaySta & 0x40) == 0x00)
					{
						//第一次进入
						if((sMainPageDisplaySta & 0x20) == 0x00)
						{
							vMainPageParamDisplayCnt = 1;
							//关闭显示
							MainInterfaceDisplay(0);
							sADCxFilterEnable = 0;
							//关闭输出
							ExternalOutputChannelEnable(OutChannel_1,0);
							ExternalOutputChannelEnable(OutChannel_2,0);
							sMainPageDisplaySta |= 0x30;
						}
						else
						{
							vMainPageParamDisplayCnt++;
							if(vMainPageParamDisplayCnt > 2)
								vMainPageParamDisplayCnt = 1;
							
							sMainPageDisplaySta |= 0x10;
						}
					}
					vMainPageIdleTick = GetSystemTick();
				}
				else if(KEY_ReadEvent(UP, Short_Press_Once, 1))
				{
					if((sMainPageDisplaySta & 0x40) == 0x00)
					{
						//第一次进入
						if((sMainPageDisplaySta & 0x20) == 0x00)
						{
							vMainPageParamDisplayCnt = 1;
							//关闭显示
							MainInterfaceDisplay(0);
							sADCxFilterEnable = 0;
							//关闭输出
							ExternalOutputChannelEnable(OutChannel_1,0);
							ExternalOutputChannelEnable(OutChannel_2,0);
							sMainPageDisplaySta |= 0x30;
						}
						else
						{
							vMainPageParamDisplayCnt--;
							if(vMainPageParamDisplayCnt == 0)
								vMainPageParamDisplayCnt = 2;
							
							sMainPageDisplaySta |= 0x10;
						}
					}
					vMainPageIdleTick = GetSystemTick();
				}
				else if(KEY_ReadEvent(ENTER, Press_END, 1))
				{
					sMainPageDisplaySta &= ~0x40;
				}
				
				if(sMainPageDisplaySta & 0x10)
				{
					MainPageParamDisplayTask(vMainPageParamDisplayCnt);
					sMainPageDisplaySta &= ~0x10;
				}
			}
			
			if((sMainPageDisplaySta & 0x20) && (GetSystemTick() - vMainPageIdleTick > 8000))
			{
				//开启显示
				MainInterfaceDisplay(1);
				sADCxFilterEnable = 1;
				_gLCD_MainPageDat.sDisFlag = 0x03;
				_gLCD_MainPageDat.vDisRefreshCnt = 0;
				_gLCD_MainPageDat.vOldDataLen = 0;
//				sParamDisFlag = 0;
				sMainPageDisplaySta = 0;
			}
			
			if((sMainPageDisplaySta & 0x40) && (GetSystemTick() - vMainPageDisplayTick > 1000))
			{
				sMainPageDisplaySta = 0;
				SystemPage = MenuDisplayPage;
				MeterInterfaceKeyShield(Enter_Enanble|Up_Enanble|Down_Enanble);
				MeterInterfaceInit();
				//关闭显示
				MainInterfaceDisplay(0);
				sADCxFilterEnable = 0;
				//关闭输出
				ExternalOutputChannelEnable(OutChannel_1,0);
				ExternalOutputChannelEnable(OutChannel_2,0);
				//
				CurrentOutput_PwmValueSet(0);
			}
		break;
		//菜单界面操作处理
		case MenuDisplayPage:
			if(MeterInterfaceTask())
			{
				SystemPage = MainPage;
				MeterInterfaceTaskQuit();
				//开启显示
				MainInterfaceDisplay(1);
				_gLCD_MainPageDat.vDisRefreshCnt = 0;
				_gLCD_MainPageDat.sDisFlag = 0x03;
				_gLCD_MainPageDat.vOldDataLen = 0;
				
//				sADCxFilterEnable = 1;
				sADCxFilterEnable = 1;
				vADCxFilterVal = 0;
				vADCxFilterCnt = 0;
//				sADCxFilterFinish = 0;
			}
		break;
		
		default:
			break;
	}
	//ADC测量端滤波
	if((sADCxFilterEnable) && (ADC_GetData(&TempVal_U16)))
	{
		vADCxFilterVal += TempVal_U16;
		vADCxFilterCnt++;
		if(vADCxFilterCnt >= ADCxFilterCount)
		{
			vADCxFilterVal = vADCxFilterVal >> 9;
			if(vADCxFilterOldVal > vADCxFilterVal)
			{			
				if((vADCxFilterOldVal - vADCxFilterVal) <= 1)
				{
					_gLCD_MainPageDat.vADCxVal = vADCxFilterOldVal;
				}
				else
				{
					_gLCD_MainPageDat.vADCxVal = vADCxFilterVal;
					vADCxFilterOldVal = vADCxFilterVal;
				}
			}
			else
			{
				if((vADCxFilterVal - vADCxFilterOldVal) <= 1)
				{
					_gLCD_MainPageDat.vADCxVal = vADCxFilterOldVal;
				}
				else
				{
					_gLCD_MainPageDat.vADCxVal = vADCxFilterVal;
					vADCxFilterOldVal = vADCxFilterVal;
				}
			}
			
			vADCxFilterVal = 0;
			vADCxFilterCnt = 0;
//			sADCxFilterFinish = 1;
		}
	}
}

