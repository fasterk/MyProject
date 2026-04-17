#ifndef __ADC_CONFIG_H__
#define __ADC_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

typedef enum
{
	GasPressureTestPort = 0,	//气压检测端
	InternalVccTestPort,		//内部基准电压检测端
	AdcDevMax
}ADC_ChannelDev;


void ADC_Bsp_Init(void);
void ADC_Channelx_Init(ADC_ChannelDev Dev);
void ADC_DeviceCtrl(uint8_t OnOff);
void ADC_CalibrattionValueUpdate(int32_t val);
uint8_t ADC_GetData(uint16_t *dat);
void ADC_ScanTask(void);


#ifdef __cplusplus
}
#endif

#endif

