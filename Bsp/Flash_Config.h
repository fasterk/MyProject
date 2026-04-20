#ifndef __FLASH_CONFIG_H__
#define __FLASH_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"
	 typedef struct
{
	uint32_t Head;
	
	//系统相关参数
	int32_t SystemLanguage;			//系统语言
	int32_t Password;				//用户密码
	int32_t BacklightGrade;			//背光等级
	int32_t SystemBackColour;		//系统背景颜色
	int32_t SystemFontColour;		//系统字体颜色
	int32_t ADCxCompensationVal;	//ADC补偿值
	int32_t AirZeroingVal;			//气压调零值
	int32_t InputVoltage;			//外部输入电压
	int32_t DisUnit;				//当前显示单位
	
	int32_t RangeNum;				//量程编号by zengxing20260310
	int32_t RangeSetFlag;				//量程编号by zengxing20260310
	
	int32_t ZeroingExtraScopeVal;
	int32_t ProductClass;
	int32_t PressureValueType;
	int32_t OutPressureUpperLimit;
	int32_t OutPressureLowerLimit;
	int32_t OutLagValUpperLimit;
	int32_t OutLagValLowerLimit;
	int32_t OutDelayTimeUpperLimit;
	int32_t OutDelayTimeLowerLimit;
	int32_t OutZeroingValUpperLimit;
	int32_t OutZeroingValLowerLimit;
	int32_t OutZeroingErrorRange;
	//通道1参数
	int32_t Out1Mode;				//模式
	int32_t Out1ThresholdVal;		//阈值
	int32_t Out1PeakVal;			//峰值
	int32_t Out1ValleyVal;			//谷值
	int32_t Out1LagVal;				//迟滞值
	int32_t Out1ResponseTimeVal;	//响应时间
	int32_t Out1Way;				//输出方式
	//通道2参数
	int32_t Out2Mode;				//模式
	int32_t Out2ThresholdVal;		//阈值
	int32_t Out2PeakVal;			//峰值
	int32_t Out2ValleyVal;			//谷值
	int32_t Out2LagVal;				//迟滞值
	int32_t Out2ResponseTimeVal;	//响应时间
	int32_t Out2Way;				//输出方式
	//其它预留
	int32_t CompensationVal;		//补偿值
	int32_t CalibrationNumber;		//标定值数量
	int32_t CalibrationVal[15];		//标定值
//	int32_t CalibrationAirVal[15];	//标定值对应的气压值
	//485通讯相关数据
	int32_t DevID;					//设备ID
	int32_t CommBaudRate;			//通讯波特率
	//电流校准相关
	int32_t CurrentOutCalibrationVal_4MA;	//电流输出校准值4ma
	int32_t CurrentOutCalibrationVal_Temp1;	//电流输出校准值备用1
	int32_t CurrentOutCalibrationVal_20MA;	//电流输出校准值20ma
	//久好相关数据
	int32_t CalibADxValue[3];	    //标定ADC值
	int32_t ExtremeValue[3];
	int32_t ParamValue[9];
	int32_t JM1203ParamValue[10];
	
	uint32_t End;
}__attribute__((aligned (4))) AppDataTypeDef;


typedef enum
{
	HEAD = 0,
	APP_SystemLanguage,		//系统语言
	APP_SystemPassword,			//用户密码
	APP_SystemBacklightGrade,	//背光等级
	APP_SystemBackColour,		//系统背景颜色
	APP_SystemFontColour,		//系统字体颜色
	APP_ADCxCompensationVal,	//ADC补偿值
	APP_AirZeroingVal,			//气压调零值
	APP_SystemInputVoltage,		//外部输入电压
	APP_SystemUnit,				//当前显示单位
	
	APP_SystemRange,			//当前量程byzengxing
	APP_SetRangeFlag,			//当前量程byzengxing
	
	APP_ZeroingExtraScopeVal,
	APP_ProductClass,
	App_AirPressureValueType,
	APP_OutPressureUpperLimit,
	APP_OutPressureLowerLimit,
	APP_OutLagValUpperLimit,
	APP_OutLagValLowerLimit,
	APP_OutDelayTimeUpperLimit,
	APP_OutDelayTimeLowerLimit,
	APP_OutZeroingValUpperLimit,
	APP_OutZeroingValLowerLimit,
	APP_OutZeroingErrorRange,
	
	APP_Out1Mode,				//通道1模式
	APP_Out1ThresholdVal,		//通道1阈值
	APP_Out1PeakVal,			//通道1峰值
	APP_Out1ValleyVal,			//通道1谷值
	APP_Out1LagVal,				//通道1迟滞值
	APP_Out1ResponseTimeVal,	//通道1响应时间
	APP_Out1Way,				//通道1输出方式
	APP_Out2Mode,				//通道2模式
	APP_Out2ThresholdVal,		//通道2阈值
	APP_Out2PeakVal,			//通道2峰值
	APP_Out2ValleyVal,			//通道2谷值
	APP_Out2LagVal,				//通道2迟滞值
	APP_Out2ResponseTimeVal,	//通道2响应时间
	APP_Out2Way,				//通道2输出方式
	//其它预留
	APP_CompensationVal,		//补偿值
	APP_CalibrationNumber,		//标定值数量
	APP_CalibrationVal1,		//标定值1
	APP_CalibrationVal2,		//标定值2
	APP_CalibrationVal3,		//标定值3
	APP_CalibrationVal4,		//标定值4
	APP_CalibrationVal5,		//标定值5
	APP_CalibrationVal6,		//标定值6
	APP_CalibrationVal7,		//标定值7
	APP_CalibrationVal8,		//标定值8
	APP_CalibrationVal9,		//标定值9
	APP_CalibrationVal10,		//标定值10
	APP_CalibrationVal11,		//标定值11
	APP_CalibrationVal12,		//标定值12
	APP_CalibrationVal13,		//标定值13
	APP_CalibrationVal14,		//标定值14
	APP_CalibrationVal15,		//标定值15
	//
	APP_DevID,					//设备ID
	APP_CommBaudRate,			//通讯波特率
	//
	APP_CurrentOutCalibrationVal_4MA,	//电流输出校准值4ma
	APP_CurrentOutCalibrationVal_Temp1,	//电流输出校准值备用1
	APP_CurrentOutCalibrationVal_20MA,	//电流输出校准值20ma
	//久好相关数据
	APP_CalibADxVal1,
	APP_CalibADxVal2,
	APP_CalibADxVal3,
	APP_ExtremeVal1,
	APP_ExtremeVal2,
	APP_ExtremeVal3,
	APP_ParamVal1,
	APP_ParamVal2,
	APP_ParamVal3,
	APP_ParamVal4,
	APP_ParamVal5,
	APP_ParamVal6,
	APP_ParamVal7,
	APP_ParamVal8,
	APP_ParamVal9,
	APP_JM1203ParamVal1,
	APP_JM1203ParamVal2,
	APP_JM1203ParamVal3,
	APP_JM1203ParamVal4,
	APP_JM1203ParamVal5,
	APP_JM1203ParamVal6,
	APP_JM1203ParamVal7,
	APP_JM1203ParamVal8,
	APP_JM1203ParamVal9,
	APP_JM1203ParamVal10,

	User_RegNumberError,			//出错
	END
}AppDataRegNumber;


void AppDataInit(void);
void AppDataWrite(int32_t val, AppDataRegNumber reg_num);
int32_t AppDataRead(AppDataRegNumber reg_num);
void APPDataFlashWrite(void);
void APPDataEmpty(uint8_t resetFlag);
void Set_Calib_WriteFlag(uint16_t v);

void CalibDataWritePoll_Task(void);
void AppSetRange(void);

#ifdef __cplusplus
}
#endif

#endif

