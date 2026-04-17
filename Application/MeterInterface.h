#ifndef __METERINTERFACE_H__
#define __METERINTERFACE_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"
#include "LCD.h"

#define FunctionKey_Disbale ((uint8_t)0x00)
#define Enter_Enanble ((uint8_t)0x02)
#define Up_Enanble ((uint8_t)0x01)
#define Down_Enanble ((uint8_t)0x04)


void MeterInterfaceKeyShield(uint8_t key_shield);
void MeterInterfaceInit(void);
uint8_t MeterInterfaceTask(void);
void MeterInterfaceTaskQuit(void);

void ExternalReferenceVolGetTask(void);
uint8_t ExternalRefVolTaskReady(uint16_t *adc_val, uint16_t *vol_val);


#ifdef __cplusplus
}
#endif

#endif

