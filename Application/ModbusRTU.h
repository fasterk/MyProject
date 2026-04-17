#ifndef __MODBUSRTU_H__
#define __MODBUSRTU_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "py32f0xx_hal.h"


void MobudRtuInit(void);
unsigned int FloatToUInt32(float v);
float UInt32ToFloat(unsigned int v);
uint16_t Get_UART_PARA(uint8_t i);
void Set_UART_PARA(uint8_t i, uint16_t v);
void ModbusRTU_Task(void);


#ifdef __cplusplus
}
#endif

#endif

