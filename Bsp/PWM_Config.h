#ifndef __PWM_CONFIG_H__
#define __PWM_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

void CurrentOutput_Init(void);
void CurrentOutput_PwmValueSet(uint16_t val);
void CurrentOutput_UpdateCalcData(void);
void CurrentOutput_UA(uint16_t ua_val);


#ifdef __cplusplus
}
#endif

#endif

