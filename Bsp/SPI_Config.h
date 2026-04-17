#ifndef __SPI_CONFIG_H__
#define __SPI_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"


void SPI_Init(void);
void SPI_SendData(uint8_t *data, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif

