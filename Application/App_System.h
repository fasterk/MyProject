#ifndef __APP_SYSTEM_H__
#define __APP_SYSTEM_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"
#include <math.h>

#define DEBUG (0)	//调试状态(可修改采样电压参数)

//#define RangeAbility (3)	//量程范围定义

#define LowPressureSeriesProduct (0)	//低压系列产品
#define HighPressureSeriesProduct (1)	//高压系列产品

//#define ZeroingExtraScopeVal 		((int32_t)6)	//调零额外范围值(0.6Kpa)
	 
typedef struct
{
	int32_t ZeroingExtraScopeVal;//参考电压
	int32_t ProductClass;//低压产品
	int32_t AirPressureValueType;//气压值由负到正
	
	int32_t PressureUpperLimit;		//压力值上限
	int32_t PressureLowerLimit;		//压力值下限
	int32_t LagValUpperLimit;		//迟滞值上限
	int32_t LagValLowerLimit;		//迟滞值下限
	int32_t DelayTimeUpperLimit;	//延迟时间上限
	int32_t DelayTimeLowerLimit;	//延迟时间下限
	int32_t ZeroingValUpperLimit;	//调零值上限
	int32_t ZeroingValLowerLimit;	//调零值下限
	int32_t ZeroingErrorRange;		//调零值计算误差范围
	
	int32_t Default_LagVal;
	int32_t Default_Ch1_ThresholdVal;	//通道1默认阈值
	int32_t Default_Ch1_PeakValVal;		//通道1默认峰值
	int32_t Default_Ch1_ValleyVal;		//通道1默认谷值
	int32_t Default_Ch2_ThresholdVal;	//通道2默认阈值
	int32_t Default_Ch2_PeakValVal;		//通道2默认峰值
	int32_t Default_Ch2_ValleyVal;		//通道2默认谷值
}TypeParam_Config;

//相关数据基于±2%精度
//100Kpa~-100Kpa
//#if RangeAbility == 0
//	
//	#define ZeroingExtraScopeVal 		((int32_t)4)	//调零额外范围值(0.4Kpa 默认0.2%)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (0)	//气压值由负到正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)1000)		//压力值上限100Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)-1000)	//压力值下限-100Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)50)	//调零值上限5Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)-50)	//调零值下限-5Kpa
//	#define ZeroingErrorRange       ((int32_t)4)	//调零值计算误差范围（±0.4Kpa默认0.2%）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)200)		//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)-200)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)-600)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)200)		//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)-200)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)-600)	//通道2默认谷值
//	
////0~250Kpa
//#elif RangeAbility == 1		//

//	#define ZeroingExtraScopeVal 		((int32_t)5)	//调零额外范围值(0.5Kpa)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (1)	//气压值仅为正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)2500)		//压力值上限250Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)0)		//压力值下限0Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)125)	//调零值上限12.5Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)0)	//调零值下限0Kpa
//	#define ZeroingErrorRange       ((int32_t)5)	//调零值计算误差范围（±0.5Kpa）
//	/********************通道参数默认值定义********************/ 
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)1500)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)1000)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)500)	    //通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)1500)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)1000)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)500)		//通道2默认谷值

////0~-100Kpa
//#elif RangeAbility == 2		//

//	#define ZeroingExtraScopeVal 		((int32_t)2)	//调零额外范围值(0.2Kpa)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (2)	//气压值仅为负
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)0)		//压力值上限0Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)-1000)	//压力值下限-100Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)0)	//调零值上限0Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)-50)	//调零值下限-5Kpa
//	#define ZeroingErrorRange       ((int32_t)2)	//调零值计算误差范围（±0.2Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)-400)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)-600)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)-800)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)-400)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)-600)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)-800)	//通道2默认谷值

////0~1Mpa
//#elif RangeAbility == 3		//

//	#define ZeroingExtraScopeVal 		((int32_t)20)	//调零额外范围值(2Kpa)
//	#define ProductClass HighPressureSeriesProduct	//高压产品
//	#define AirPressureValueType (1)	//气压值仅为正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)10000)	//压力值上限1Mpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)0)		//压力值下限
//	#define LagValUpperLimit   	((int32_t)100)		//迟滞值上限10Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)10)		//迟滞值下限
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)500)	//调零值上限50Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)0)	//调零值下限0Kpa
//	#define ZeroingErrorRange       ((int32_t)20)	//调零值计算误差范围（±2Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)10)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)6000)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)4000)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)2000)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)6000)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)4000)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)2000)	//通道2默认谷值
//	
////0~100Kpa
//#elif RangeAbility == 4		//

//	#define ZeroingExtraScopeVal 		((int32_t)2)	//调零额外范围值(0.2Kpa)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (1)	//气压值仅为正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)1000)		//压力值上限100Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)0)		//压力值下限0Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)50)	//调零值上限5Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)0)	//调零值下限0Kpa
//	#define ZeroingErrorRange       ((int32_t)2)	//调零值计算误差范围（±0.2Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)600)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)400)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)200)	    //通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)600)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)400)//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)200)		//通道2默认谷值
//	
////-100Kpa~1Mpa
//#elif RangeAbility == 5		//
//	
//	#define ZeroingExtraScopeVal 		((int32_t)22)	//调零额外范围值(22Kpa)
//	#define ProductClass HighPressureSeriesProduct	//高压产品
//	#define AirPressureValueType (0)	//气压值由负到正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)10000)	//压力值上限1Mpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)-1000)	//压力值下限-100Kpa
//	#define LagValUpperLimit   	((int32_t)100)		//迟滞值上限10Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)10)		//迟滞值下限
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)500)	//调零值上限50Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)-50)	//调零值下限-5Kpa
//	#define ZeroingErrorRange       ((int32_t)22)	//调零值计算误差范围（±2.2Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)10)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)5600)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)3400)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)1200)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)5600)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)3400)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)1200)	//通道2默认谷值
//	
////0~-101Kpa
//#elif RangeAbility == 6		//

//	#define ZeroingExtraScopeVal 		((int32_t)2)	//调零额外范围值(0.2Kpa)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (2)	//气压值仅为负
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)0)		//压力值上限0Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)-1010)	//压力值下限-100Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)0)	//调零值上限0Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)-50)	//调零值下限-5Kpa
//	#define ZeroingErrorRange       ((int32_t)2)	//调零值计算误差范围（±0.2Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)-400)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)-600)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)-800)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)-400)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)-600)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)-800)	//通道2默认谷值

////-50~500Kpa
//#elif RangeAbility == 7

//	#define ZeroingExtraScopeVal 		((int32_t)11)	//调零额外范围值(1.1Kpa)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (0)	//气压值由负到正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)5000)		//压力值上限500Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)-500)		//压力值下限-50Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)250)	//调零值上限25Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)-25)	//调零值下限-2.5Kpa
//	#define ZeroingErrorRange       ((int32_t)11)	//调零值计算误差范围（±1.1Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)2800)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)1700)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)600)		//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)2800)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)1700)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)600)		//通道2默认谷值	

////0~500Kpa
//#elif RangeAbility == 8		//

//	#define ZeroingExtraScopeVal 		((int32_t)10)	//调零额外范围值(1Kpa)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (1)	//气压值仅为正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)5000)		//压力值上限500Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)0)		//压力值下限
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)250)	//调零值上限25Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)0)	//调零值下限0Kpa
//	#define ZeroingErrorRange       ((int32_t)10)	//调零值计算误差范围（±1Kpa）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)10)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)3000)	//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)2000)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)1000)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)3000)	//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)2000)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)1000)	//通道2默认谷值

////101~-101Kpa
//#elif RangeAbility == 9		//

//	#define ZeroingExtraScopeVal 		((int32_t)4)	//调零额外范围值(0.4Kpa 默认0.2%)
//	#define ProductClass LowPressureSeriesProduct	//低压产品
//	#define AirPressureValueType (0)	//气压值由负到正
//	/**********************数据范围定义**********************/
//	#define PressureUpperLimit 	((int32_t)1010)		//压力值上限101Kpa(100Pa为基准)
//	#define PressureLowerLimit 	((int32_t)-1010)	//压力值下限-101Kpa
//	#define LagValUpperLimit   	((int32_t)10)		//迟滞值上限1Kpa(100Pa为基准)
//	#define LagValLowerLimit   	((int32_t)1)		//迟滞值下限0.1KPa
//	#define DelayTimeUpperLimit	((int32_t)5000)		//延迟时间上限5000ms
//	#define DelayTimeLowerLimit ((int32_t)2)		//延迟时间下限2ms
//	#define ZeroingValUpperLimit	((int32_t)50)	//调零值上限5Kpa（5%）
//	#define ZeroingValLowerLimit	((int32_t)-50)	//调零值下限-5Kpa
//	#define ZeroingErrorRange       ((int32_t)4)	//调零值计算误差范围（±0.4Kpa默认0.2%）
//	/********************通道参数默认值定义********************/
//	#define Default_LagVal           ((int32_t)1)		//默认迟滞值
//	#define Default_Ch1_ThresholdVal ((int32_t)200)		//通道1默认阈值（默认60%）
//	#define Default_Ch1_PeakValVal   ((int32_t)-200)	//通道1默认峰值（默认40%）
//	#define Default_Ch1_ValleyVal    ((int32_t)-600)	//通道1默认谷值（默认20%）
//	#define Default_Ch2_ThresholdVal ((int32_t)200)		//通道2默认阈值
//	#define Default_Ch2_PeakValVal   ((int32_t)-200)	//通道2默认峰值
//	#define Default_Ch2_ValleyVal    ((int32_t)-600)	//通道2默认谷值
//	
//#endif

/**********************显示相关定义**********************/
#define SystemDisplay_X (22)	//显示横坐标起始

/**********************参数类型定义**********************/
//模式选择
#define SystemChannelMode_Simple (0)	//通道模式为简易模式
#define SystemChannelMode_Lag (1)		//通道模式为迟滞模式
#define SystemChannelMode_Window (2)	//通道模式为窗口模式
//气压单位
#define SystemUnit_Mpa (0)		//当前显示单位为Mpa
#define SystemUnit_Kpa (1)		//当前显示单位为Kpa
//输出方式
#define SystemOutWay_NoReverse (0)	//正向输出(常开)
#define SystemOutWay_Reverse (1)	//反向输出(常闭)

//系统语言定义
typedef enum
{
	Chinese = 0,
	English,
	LanguageMax
}SystemLanguageTypeDef;


void System_DeviceInit(void);
void System_ParameterReset(void);

uint32_t My_Pow(uint8_t m,uint8_t n);
uint32_t My_NumberCount(uint32_t num);
void My_IntegerSplit(uint32_t num, uint8_t len, uint8_t *buf);
void GetRefVoltageValue(uint16_t *adc_val, uint16_t *vol_val);
int32_t GetAirPressureVal(uint8_t flag);

void SystemTask(void);


#ifdef __cplusplus
}
#endif

#endif

