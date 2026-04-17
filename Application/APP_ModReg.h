#ifndef __APP_MODREG_H__
#define __APP_MODREG_H__

#include "py32f0xx_hal.h"

#define R 0 
#define W 1


void App_Register(uint16_t Reg, uint16_t *v, uint8_t Flag);


#endif

