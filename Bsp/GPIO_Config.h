#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

typedef enum
{
	LED1 = 0,
	LED2
}LED_Dev;

typedef enum
{
	NPN1 = 0,
	NPN2
}NpnOut_Dev;


void GPIO_BSP_Init(void);
void LED_Ctrl(LED_Dev Dev, uint8_t OnOff);
void NpnOut_Ctrl(NpnOut_Dev Dev, uint8_t OnOff);


#ifdef __cplusplus
}
#endif

#endif

