#ifndef __APP_EXTERNALOUTPUT_H__
#define __APP_EXTERNALOUTPUT_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

typedef enum
{
	OutChannel_1 = 0,
	OutChannel_2,
	OutputChannelMax
}OutputChannel_Dev;


void ExternalOutput_Init(void);
void ExternalOutputChannelEnable(OutputChannel_Dev Dev, uint8_t sta);
void ExternalOutputImageRef(OutputChannel_Dev Dev);
void ExternalOutputScanTask(void);


#ifdef __cplusplus
}
#endif

#endif

