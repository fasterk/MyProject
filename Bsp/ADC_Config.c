#include "ADC_Config.h"
#include <string.h>
#include <stdlib.h>
#include "Uart_Config.h"
#include "Flash_Config.h"

#define ADC_SamplingMax (10)		//ADC滤波采样次数

typedef struct
{
	uint8_t sAdcStart;
//	uint8_t sAdcToggle;
	uint8_t sAdcFinsh;
	uint8_t sAdcEnable;
	uint8_t sDataReady;
	uint8_t vAdcCnt;
	uint16_t vAdcResult;
	uint16_t *vDataBuf;
    uint32_t vAdcNewValue;
}ADC_ScanTaskObjTypeDef;

static int16_t Calibrattion_Val = 0;	//ADC补偿值
static uint16_t ADCxBuffer[ADC_SamplingMax];	//初次滤波采样数据缓存

ADC_HandleTypeDef AdcHandle = {0};
OPA_HandleTypeDef OpaHandle = {0}; 

ADC_ScanTaskObjTypeDef ADCxSacnTaskObj = 
{
	0,
	0,
	0,
	0,
	0,
	0,
	ADCxBuffer,
	0,
};


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ADC_Bsp_Init(void)
{
//	ADC_ChannelConfTypeDef sConfig = {0};
	
	__HAL_RCC_ADC_FORCE_RESET();
	__HAL_RCC_ADC_RELEASE_RESET();
	__HAL_RCC_ADC_CLK_ENABLE();
	
	AdcHandle.Instance = ADC1;
	HAL_ADC_DeInit(&AdcHandle);
	AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;             /* 分辨率12位 */
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;            /* 对齐方式右对齐 */
	AdcHandle.Init.ScanConvMode          = ADC_SCAN_DISABLE;               /* 扫描方式关闭 */
	AdcHandle.Init.ContinuousConvMode    = DISABLE;                        /* 单次模式 */
	AdcHandle.Init.NbrOfConversion       = 1;                              /* 转换通道数1 */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;                        /* 间断模式不使能 */
	AdcHandle.Init.NbrOfDiscConversion   = 1;                              /* 间断模式短序列长度为1 */
	AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;             /* 软件触发 */
	/* ADC初始化 */
	if(HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		while(1);
	}
	
	__HAL_RCC_OPA_FORCE_RESET();
	__HAL_RCC_OPA_RELEASE_RESET();
	__HAL_RCC_OPA_CLK_ENABLE();
	
//	switch(Dev)
//	{
//		case GasPressureTestPort:
////			__HAL_RCC_OPA_FORCE_RESET();
////			__HAL_RCC_OPA_RELEASE_RESET();
//			__HAL_RCC_OPA_CLK_ENABLE();	
//			
//			/* ADC初始化 */
//			if(HAL_ADC_Init(&AdcHandle) != HAL_OK)
//			{
//				while(1);
//			}
//			//外部压力传感器(通道5)
//			sConfig.Channel = ADC_CHANNEL_5;
//			sConfig.Rank = ADC_REGULAR_RANK_1;
//			sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;  
//			HAL_ADC_ConfigChannel(&AdcHandle,&sConfig);
//			//OPA电压跟随器
//			OpaHandle.Instance = OPA;
//			OpaHandle.Init.Part = OPA2;     /* Select OPA2 */
//			HAL_OPA_DeInit(&OpaHandle);
//			/* initialize OPA */
//			HAL_OPA_Init(&OpaHandle);
//			/* Start OPA */
//			HAL_OPA_Start(&OpaHandle);
//		break;
//		
//		case InternalVccTestPort:
//			/* ADC初始化 */
//			if(HAL_ADC_Init(&AdcHandle) != HAL_OK)
//			{
//				while(1);
//			}
//			//内部参考电压(通道17)
//			sConfig.Channel = ADC_CHANNEL_VREFINT;
//			sConfig.Rank = ADC_REGULAR_RANK_1;
//			sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;  
//			HAL_ADC_ConfigChannel(&AdcHandle,&sConfig);
//		break;
//		
//		case AdcDevMax:return;
//		
//		default:return;
//	}
	
	/* ADC calibration */
	if(HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
	{
		while(1);
	}
	
//	//使能ADC
//	ADC_DeviceCtrl(1);
	//获取ADC补偿值
	Calibrattion_Val = AppDataRead(APP_ADCxCompensationVal);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ADC_Channelx_Init(ADC_ChannelDev Dev)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	
	switch(Dev)
	{
		case GasPressureTestPort:
			//外部压力传感器(通道5)
			sConfig.Channel = ADC_CHANNEL_5;
			sConfig.Rank = ADC_REGULAR_RANK_1;
			sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;  
			HAL_ADC_ConfigChannel(&AdcHandle,&sConfig);
			//OPA电压跟随器
			OpaHandle.Instance = OPA;
			OpaHandle.Init.Part = OPA2;     /* Select OPA2 */
			HAL_OPA_DeInit(&OpaHandle);
			/* initialize OPA */
			HAL_OPA_Init(&OpaHandle);
			/* Start OPA */
			HAL_OPA_Start(&OpaHandle);
		break;
		
		case InternalVccTestPort:
			//内部参考电压(通道17)
			sConfig.Channel = ADC_CHANNEL_VREFINT;
			sConfig.Rank = ADC_REGULAR_RANK_1;
			sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;  
			HAL_ADC_ConfigChannel(&AdcHandle,&sConfig);
		break;
		
		case AdcDevMax:return;
		
		default:return;
	}
	//使能ADC
	ADC_DeviceCtrl(1);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ADC_DeviceCtrl(uint8_t OnOff)
{
	uint32_t err_tick = 0;
	
	if(OnOff)
	{
		ADC_Enable(&AdcHandle);
		ADCxSacnTaskObj.sAdcEnable = 1;
		ADCxSacnTaskObj.sAdcStart = 1;
	}
	else
	{
		err_tick = HAL_GetTick();
		while((ADC1->SR & ADC_FLAG_EOC) == 0x00)
		{
			if(HAL_GetTick() - err_tick > 3)
			{
				break;
			}
		}
		ADC_ConversionStop(&AdcHandle);
//		ADC_ConversionStop_Disable(&AdcHandle);
		ADC1->DR;
		ADCxSacnTaskObj.sAdcEnable = 0;
		ADCxSacnTaskObj.sAdcStart = 0;
	}
	
	ADCxSacnTaskObj.sDataReady = 0;
	ADCxSacnTaskObj.sAdcFinsh = 0;
	ADCxSacnTaskObj.vAdcCnt = 0;
	ADCxSacnTaskObj.vAdcNewValue = 0;
	ADCxSacnTaskObj.vAdcResult = 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ADC_CalibrattionValueUpdate(int32_t val)
{
//	Calibrattion_Val = AppDataRead(APP_ADCxCompensationVal);
	Calibrattion_Val = val;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t ADC_GetData(uint16_t *dat)
{
	*dat = ADCxSacnTaskObj.vAdcResult;
	
	if(ADCxSacnTaskObj.sDataReady)
	{
		ADCxSacnTaskObj.sDataReady = 0;
		return 1;
	}
	else
		return 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ADC_ScanTask(void)
{
	uint8_t i;
	int16_t dat = 0;
	uint16_t min = 0,max = 0;
	
	if(ADCxSacnTaskObj.sAdcEnable)
	{
		if(ADCxSacnTaskObj.sAdcStart)
		{
			ADCxSacnTaskObj.sAdcStart = 0;
			SET_BIT(ADC1->CR2, (ADC_CR2_SWSTART | ADC_CR2_EXTTRIG));
		}
		else
		{
			if(ADCxSacnTaskObj.sAdcFinsh)
			{
				ADCxSacnTaskObj.sAdcStart = 1;
				ADCxSacnTaskObj.sAdcFinsh = 0;
				
//				ADCxSacnTaskObj.vDataBuf[ADCxSacnTaskObj.vAdcCnt] = (uint16_t)ADC1->DR;
				dat = (uint16_t)ADC1->DR;
				//ADC值补偿
				if((dat + Calibrattion_Val) < 0 || dat==0)
				{
					dat = 0;
				}
				else if((dat + Calibrattion_Val) > 4095 || dat==4095)
				{
					dat = 4095;
				}
				else
					dat += Calibrattion_Val;
				ADCxSacnTaskObj.vDataBuf[ADCxSacnTaskObj.vAdcCnt] = dat;
				
				ADCxSacnTaskObj.vAdcNewValue += ADCxSacnTaskObj.vDataBuf[ADCxSacnTaskObj.vAdcCnt];
				ADCxSacnTaskObj.vAdcCnt++;
				//采集完成
				if(ADCxSacnTaskObj.vAdcCnt >= ADC_SamplingMax)
				{
					min = ADCxSacnTaskObj.vDataBuf[0];
					max = ADCxSacnTaskObj.vDataBuf[0];
					//剔除一个最高和最低
					for(i=1; i<ADC_SamplingMax; i++)
					{
						if(max < ADCxSacnTaskObj.vDataBuf[i])
							max = ADCxSacnTaskObj.vDataBuf[i];
						else if(min > ADCxSacnTaskObj.vDataBuf[i])
							min = ADCxSacnTaskObj.vDataBuf[i];
					}
					ADCxSacnTaskObj.vAdcNewValue = ADCxSacnTaskObj.vAdcNewValue - max - min;
//					dat = ADCxSacnTaskObj.vAdcNewValue >> 3;
					ADCxSacnTaskObj.vAdcResult = ADCxSacnTaskObj.vAdcNewValue >> 3;
					ADCxSacnTaskObj.vAdcNewValue = 0;
					ADCxSacnTaskObj.vAdcCnt = 0;
//					//ADC值补偿
//					if((dat + Calibrattion_Val) < 0 || dat==0)
//					{
//						dat = 0;
//					}
//					else if((dat + Calibrattion_Val) > 4095 || dat==4095)
//					{
//						dat = 4095;
//					}
//					else
//						dat += Calibrattion_Val;
					
//					ADCxSacnTaskObj.vAdcResult = dat;
					ADCxSacnTaskObj.sDataReady = 1;
				}
			}
		}
		//转换完成
		if((ADC1->SR & ADC_FLAG_EOC) == ADC_FLAG_EOC)
		{
			ADCxSacnTaskObj.sAdcFinsh = 1;
		}
	}
}


