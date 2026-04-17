#ifndef ___APP_SENSOR_H__
#define ___APP_SENSOR_H__

#include "py32f0xx_hal.h"

typedef unsigned char 	u8; 
typedef unsigned short 	u16; 
typedef unsigned int 	u32;

extern float k, t;

void Sensor_Task(void);

int Get_CalibAD(u8 i);
int Get_PressureCal(u8 i);
void Set_CalibAD(u8 i, u32 v);
void Set_PressureCal(u8 i, u32 v);

float Get_Extreme_Value(u8 i);
void Set_Extreme_Value(u8 i, float v);
	
u16 Get_Para(u8 i);
void Set_Para(u8 i, u16 v);
	
float Get_Value(void);
int Get_INT_Value(void);

#endif

