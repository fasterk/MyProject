#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

typedef enum
{
	UART_TTL = 0,
	UART_Max
}UART_Dev;

typedef struct
{
	uint32_t Bps;
	uint32_t WordLength;
	uint32_t Parity;
	uint32_t StopBits;
}UART_DeviceTypeDef;


void UART_InitAll(void);
void UART_Bsp_Init(UART_Dev dev, UART_DeviceTypeDef *uart_init);
uint16_t UART_ReadData(UART_Dev dev, uint8_t *data, uint16_t len);
void UART_SendData(UART_Dev dev, uint8_t *data, uint16_t len);
void UART_ClearTxFifo(UART_Dev dev);
void UART_ClearRxFifo(UART_Dev dev);
uint8_t UART_TTLBusyCheck(void);
void UART_RxCallBackAdd(UART_Dev dev, void (*callback)(uint16_t len));


#ifdef __cplusplus
}
#endif

#endif


