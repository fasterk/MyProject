#include "APP_ParamInput.h"
#include <string.h>
#include "LCD.h"
#include "Timer_Config.h"
#include "App_Key.h"
#include "App_System.h"

#define ParamSetTaskDataMax (8)

static volatile int32_t vParamSetTaskNumber = -1;		//参数设置任务编号
extern uint32_t RangeNumber;
//压力输入模式
static volatile uint8_t vPressureIntputSta = 0;			//压力输入控制标志位
static uint16_t vPressureIntputUnitPos[2] = {0,0};

//正常参数输入
static volatile uint8_t sParamSetSymbolSta = 0;
static volatile uint8_t vParamSetTaskSta = 0;
static volatile uint8_t vParamSetTaskDisplayEnable = 0;
static volatile uint8_t vParamSetTaskOldNumberCount = 0;
static volatile uint8_t vParamSetChangeVal[2] = {0,0};
static volatile uint32_t vParamSetTaskTick = 0;
static uint8_t vParamSetTaskDataBuf[ParamSetTaskDataMax] = {0,0,0,0,0,0,0,0};
//bit7,1:允许操作标志
//bit6,1:任务第一次进入
//bit5,1:数值变换等级1(快速递增或递减1)
//bit4,1:数值变换等级2(快速递增或递减10)
//bit3,1:空闲事件
//bit2,1:点按事件
//bit1,1:UP按下
//bit0,1:DOWN按下

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void ParamSetInterfaceTask(void *param)
{
	uint8_t i = 0;
	uint8_t Temp_U8 = 0;
	uint8_t DataLength = 0;
	uint8_t Size = 0;
	uint16_t DisplayPos = 0;
	uint32_t Temp_U32 = 0;
	
	ParamSetTaskFormatTypeDef *ParamObj = (ParamSetTaskFormatTypeDef *)param;
	if(vParamSetTaskSta & 0x80)
	{
		if((vParamSetTaskSta & 0x40) == 0x00)
		{
			if(ParamObj->Mode)
			{
//				LCD_DrawRectangle(37,54,56,73,BLUE);
//				LCD_ShowChinese(39,56,ParamObj->FontColour,ParamObj->BackColour,16,0,63);
//				LCD_DrawRectangle(123,54,142,73,BLUE);
//				LCD_ShowChinese(125,56,ParamObj->FontColour,ParamObj->BackColour,16,0,64);
				vParamSetChangeVal[0] = 1;
				vParamSetChangeVal[1] = 10;
				LCD_DrawRectangle(37,54,56,73,ParamObj->FontColour);
				LCD_ShowChinese(39,56,ParamObj->FontColour,ParamObj->BackColour,16,0,63);
				LCD_DrawRectangle(123,54,142,73,ParamObj->FontColour);
				LCD_ShowChinese(125,56,ParamObj->FontColour,ParamObj->BackColour,16,0,64);
			}
			//压力输入模式
			if(vPressureIntputSta & 0x80)
			{
				if((ParamObj->ParamValue > -10000) && (ParamObj->ParamValue < 10000))
				{
					vPressureIntputSta |= 0x40;
				}
			}
			vParamSetTaskDisplayEnable = 1;
			vParamSetTaskSta |= 0x40;
		}
		//有按键按下
		if(vParamSetTaskSta & 0x03)
		{
			vParamSetTaskSta &= ~0x08;
			ParamObj->RefreshFlag = 0;
			Temp_U8 = vParamSetTaskSta & 0x03;
			switch(Temp_U8)
			{
				case 0x02:
					//
					if(vPressureIntputSta & 0x80)
					{
						if((ParamObj->ParamValue >= 10000) || (ParamObj->ParamValue < -10000))
						{
							vParamSetChangeVal[0] = 10;
							vParamSetChangeVal[1] = 100;
						}
						else
						{
							vParamSetChangeVal[0] = 1;
							vParamSetChangeVal[1] = 10;
						}
					}
					//
					if((vParamSetTaskSta & 0x20) || (vParamSetTaskSta & 0x04))
					{
						if((ParamObj->ParamValue + vParamSetChangeVal[0]) <= ParamObj->ParamHighLimit)
						{
							ParamObj->ParamValue += vParamSetChangeVal[0];
							vParamSetTaskSta &= ~0x04;
							vParamSetTaskDisplayEnable = 1;
						}
					}
					else if(vParamSetTaskSta & 0x10)
					{
						if((ParamObj->ParamValue + vParamSetChangeVal[1]) <= ParamObj->ParamHighLimit)
						{
							ParamObj->ParamValue += vParamSetChangeVal[1];
							vParamSetTaskDisplayEnable = 1;
						}
					}
				break;
				
				case 0x01:
					//
					if(vPressureIntputSta & 0x80)
					{
						if((ParamObj->ParamValue > 10000) || (ParamObj->ParamValue <= -10000))
						{
							vParamSetChangeVal[0] = 10;
							vParamSetChangeVal[1] = 100;
						}
						else
						{
							vParamSetChangeVal[0] = 1;
							vParamSetChangeVal[1] = 10;
						}
					}
					//
					if((vParamSetTaskSta & 0x20) || (vParamSetTaskSta & 0x04))
					{
						if((ParamObj->ParamValue - vParamSetChangeVal[0]) >= ParamObj->ParamLowLimit)
						{
							ParamObj->ParamValue -= vParamSetChangeVal[0];
							vParamSetTaskSta &= ~0x04;
							vParamSetTaskDisplayEnable = 1;
						}
					}
					else if(vParamSetTaskSta & 0x10)
					{
						if((ParamObj->ParamValue - vParamSetChangeVal[1]) >= ParamObj->ParamLowLimit)
						{
							ParamObj->ParamValue -= vParamSetChangeVal[1];
							vParamSetTaskDisplayEnable = 1;
						}
					}
				break;
			}
			//检测快速和极速输入设置
			if(GetSystemTick() - vParamSetTaskTick > 3500)
			{
				vParamSetTaskSta &= ~0x20;
				vParamSetTaskSta |= 0x10;
			}
			else if(GetSystemTick() - vParamSetTaskTick > 1000)
			{
				vParamSetTaskSta |= 0x20;
			}
		}
		else
		{
			vParamSetTaskSta |= 0x08;	//标记空闲
			if(ParamObj->RefreshFlag)
			{
				vParamSetTaskDisplayEnable = 1;
				ParamObj->RefreshFlag = 0;
			}
		}
		//检测按键	
		if(((vParamSetTaskSta & 0x01) == 0x00) && (KEY_ReadEvent(UP, Short_Press_Once, 1)))
		{
			if(ParamObj->Mode)
				LCD_ShowChinese(39,56,ParamObj->BackColour,ParamObj->FontColour,16,0,63);
			
			vParamSetTaskSta |= 0x06;
			vParamSetTaskTick = GetSystemTick();
		}
		else if(KEY_ReadEvent(UP, Press_END, 1))
		{
			if(ParamObj->Mode)
				LCD_ShowChinese(39,56,ParamObj->FontColour,ParamObj->BackColour,16,0,63);
			
			vParamSetTaskSta &= ~0x32;
		}
		
		if(((vParamSetTaskSta & 0x02) == 0x00) && (KEY_ReadEvent(DOWN, Short_Press_Once, 1)))
		{
			if(ParamObj->Mode)
				LCD_ShowChinese(125,56,ParamObj->BackColour,ParamObj->FontColour,16,0,64);
			vParamSetTaskSta |= 0x05;
			vParamSetTaskTick = GetSystemTick();
		}
		else if(KEY_ReadEvent(DOWN, Press_END, 1))
		{
			if(ParamObj->Mode)
				LCD_ShowChinese(125,56,ParamObj->FontColour,ParamObj->BackColour,16,0,64);
			
			vParamSetTaskSta &= ~0x31;
		}
	}
	//显示
	if(vParamSetTaskDisplayEnable)
	{
		Size = ParamObj->FontSize>>1;
		DisplayPos = ParamObj->StartPos_X;
		vParamSetTaskDisplayEnable = 0;
		//符号显示
//		if(ParamObj->ParamValue >= 0)
//		{
//			Temp_U32 = ParamObj->ParamValue;
//			LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,'+',ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
//		}
//		else
//		{
//			Temp_U32 = ~ParamObj->ParamValue + 1;
//			LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,'-',ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
//		}
		//
		if(ParamObj->ParamValue < 0)
		{
			sParamSetSymbolSta = 1;
			Temp_U32 = ~ParamObj->ParamValue + 1;
			LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,'-',ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
		}
		else
		{
			if(sParamSetSymbolSta)
			{
				LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,' ',ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
			}
			sParamSetSymbolSta = 0;
			Temp_U32 = ParamObj->ParamValue;
		}
		//检测是否压力输入模式
		if(vPressureIntputSta & 0x80 && RangeNumber != 10 && RangeNumber != 11 && RangeNumber != 12)
		{
			if(Temp_U32 > 9999)	//MPa
			{
				Temp_U32 /= 10;
				if((vPressureIntputSta & 0x40) == 0x00)
				{
					ParamObj->ParamLen = 3;
//					vParamSetChangeVal[0] = 10;
//					vParamSetChangeVal[1] = 100;
//					vParamSetTaskOldNumberCount = 0;
//					LCD_Fill(ParamObj->StartPos_X,ParamObj->StartPos_Y,(ParamObj->FontColour>>1)*6 + ParamObj->StartPos_X,ParamObj->StartPos_Y+ParamObj->FontSize,ParamObj->BackColour);
					LCD_ShowString(vPressureIntputUnitPos[0],vPressureIntputUnitPos[1],(uint8_t *)"MPa",ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
					vPressureIntputSta |= 0x40;
				}
			}
			else	//KPa
			{
				if(vPressureIntputSta & 0x40)
				{
					ParamObj->ParamLen = 1;
//					vParamSetChangeVal[0] = 1;
//					vParamSetChangeVal[1] = 10;
//					vParamSetTaskOldNumberCount = 0;
//					LCD_Fill(ParamObj->StartPos_X,ParamObj->StartPos_Y,(ParamObj->FontColour>>1)*6 + ParamObj->StartPos_X,ParamObj->StartPos_Y+ParamObj->FontSize,ParamObj->BackColour);
					LCD_ShowString(vPressureIntputUnitPos[0],vPressureIntputUnitPos[1],(uint8_t *)"kPa",ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
					vPressureIntputSta &= ~0x40;
				}
			}
		}
		
		//??????????
		if(vPressureIntputSta & 0x80 && (RangeNumber == 10 || RangeNumber == 11 || RangeNumber == 12))
		{
			if(Temp_U32 > 9999)	//MPa
			{
				Temp_U32 /= 10;
				if((vPressureIntputSta & 0x40) == 0x00)
				{
					ParamObj->ParamLen = 2;
//					vParamSetChangeVal[0] = 10;
//					vParamSetChangeVal[1] = 100;
//					vParamSetTaskOldNumberCount = 0;
//					LCD_Fill(ParamObj->StartPos_X,ParamObj->StartPos_Y,(ParamObj->FontColour>>1)*6 + ParamObj->StartPos_X,ParamObj->StartPos_Y+ParamObj->FontSize,ParamObj->BackColour);
					LCD_ShowString(vPressureIntputUnitPos[0],vPressureIntputUnitPos[1],(uint8_t *)"kPa",ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
					vPressureIntputSta |= 0x40;
				}
			}
			else	//KPa
			{
				if(vPressureIntputSta & 0x40)
				{
					ParamObj->ParamLen = 3;
//					vParamSetChangeVal[0] = 1;
//					vParamSetChangeVal[1] = 10;
//					vParamSetTaskOldNumberCount = 0;
//					LCD_Fill(ParamObj->StartPos_X,ParamObj->StartPos_Y,(ParamObj->FontColour>>1)*6 + ParamObj->StartPos_X,ParamObj->StartPos_Y+ParamObj->FontSize,ParamObj->BackColour);
					LCD_ShowString(vPressureIntputUnitPos[0],vPressureIntputUnitPos[1],(uint8_t *)"kPa",ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
					vPressureIntputSta &= ~0x40;
				}
			}
		}
		//浮点显示使能
		if(ParamObj->DecimalDisplayEnable)
		{
			DataLength = My_NumberCount(Temp_U32);
			My_IntegerSplit(Temp_U32, DataLength, vParamSetTaskDataBuf);
			
			if(vParamSetTaskOldNumberCount > DataLength)
			{
				LCD_ShowChar(DisplayPos+Size+vParamSetTaskOldNumberCount*Size,ParamObj->StartPos_Y,' ',ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
			}
			vParamSetTaskOldNumberCount = DataLength;
			
			if(DataLength <= ParamObj->ParamLen)
			{
				Temp_U8 = ParamObj->ParamLen - DataLength;	//小数后补0个数
				DataLength = 0;
				DisplayPos += Size;
				LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,48,ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
			}
			else
			{
				Temp_U8 = 0;
				DataLength = DataLength - ParamObj->ParamLen;
				for(i=0; i<DataLength; i++)
				{
					DisplayPos += Size;
					LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,vParamSetTaskDataBuf[i]+48,ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
				}
			}
			//显示小数点
			DisplayPos += Size;
			LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,'.',ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
			
			//显示小数部分
			for(i=0; i<ParamObj->ParamLen; i++)
			{
				DisplayPos += Size;
				if(Temp_U8 > i)
					LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,48,ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);	//补0
				else
					LCD_ShowChar(DisplayPos,ParamObj->StartPos_Y,vParamSetTaskDataBuf[DataLength+i-Temp_U8]+48,ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,0);
			}
		}
		else
		{
			LCD_ShowIntNum(DisplayPos+Size,ParamObj->StartPos_Y,Temp_U32,ParamObj->ParamLen,ParamObj->FontColour,ParamObj->BackColour,ParamObj->FontSize,1);
		}
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ParamSetTaskCreate(void *param)
{
	vParamSetTaskNumber = TimerTaskCreate(5, ParamSetInterfaceTask, param);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ParamSetTaskStart(uint8_t OnOff)
{
	if(OnOff)
		vParamSetTaskSta |= 0x80;
	else
		vParamSetTaskSta &= ~0x80;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t ParamSetTaskReady(void)
{
	return ((vParamSetTaskSta & 0x40) ? 1 : 0);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t ParamSetTaskIdleQuery(void)
{
	return ((vParamSetTaskSta & 0x08) ? 1 : 0);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ParamSetTaskDelete(void)
{
	if(vParamSetTaskSta & 0x40)
	{
		sParamSetSymbolSta = 0;
		vParamSetTaskSta = 0;
		vParamSetTaskDisplayEnable = 0;
		vParamSetTaskOldNumberCount = 0;
		vParamSetTaskTick = 0;
		vParamSetChangeVal[0] = 0;
		vParamSetChangeVal[1] = 0;
		
		vPressureIntputUnitPos[0] = 0;
		vPressureIntputUnitPos[1] = 0;
		
		TimerTaskDelete(vParamSetTaskNumber);
		vParamSetTaskNumber = -1;
		memset(vParamSetTaskDataBuf,0,ParamSetTaskDataMax);
		memset(vPressureIntputUnitPos,0,sizeof(vPressureIntputUnitPos));
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void PressureParamIntputEnable(uint16_t x, uint16_t y)
{
	vPressureIntputUnitPos[0] = x;
	vPressureIntputUnitPos[1] = y;
	vPressureIntputSta |= 0x80;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t PressureParamIntputUnitRead(void)
{
	if(vPressureIntputSta & 0x40)
	{
		vPressureIntputSta = 0;
		return ParamUnit_Mpa;
	}
	else
	{
		vPressureIntputSta = 0;
		return ParamUnit_Kpa;
	}
}

