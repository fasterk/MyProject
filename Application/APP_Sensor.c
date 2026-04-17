#include "APP_Sensor.h"
#include "Flash_Config.h"
#include "ModbusRTU.h"
#include "APP_JM1203.h"

#define CH_NUM 1 
#define FILTER_COUNT 60

#define Calib_Num 3
static int	CalibAD[Calib_Num];
static int	PressureCal[3];
static float Extreme_Value[Calib_Num];

static float Value;	
static int 	INT_Value;	

#define Para_Num 2
static u8 Para[Para_Num];	

static u8 Init; 

static float Filter(u8 No, float Value, u16 Coff);	//滤波

#define Run_Num 1
//static u8 Run[Run_Num];	
//Run[0]	传感器状态1=故障

float k, t;

void Sensor_Task(void)
{
	float Temp, Filter_Temp;
	u8 i;
    int	CalibAD_TEMP[Calib_Num];	
	int Raw_Data = Get_Bridge_RawData();
	
	/*----------处理负数----------*/	
	if ((Raw_Data & 0x00800000) == 0x00800000)	//负数
	{
		Raw_Data = Raw_Data - 1;					
		Raw_Data = ~Raw_Data;							
		Raw_Data = Raw_Data & 0x80FFFFFF;	
		Raw_Data = ~Raw_Data;							
		Raw_Data = Raw_Data + 1;					
		Raw_Data = Raw_Data | 0x80000000;	
	}
	for(i = 0; i < 3; i++)
	{
		CalibAD_TEMP[i] = CalibAD[i];
		if ((CalibAD_TEMP[i] & 0x00800000) == 0x00800000)	//负数
		{
			CalibAD_TEMP[i] = CalibAD_TEMP[i] - 1;					
			CalibAD_TEMP[i] = ~CalibAD_TEMP[i];							
			CalibAD_TEMP[i] = CalibAD_TEMP[i] & 0x80FFFFFF;	
			CalibAD_TEMP[i] = ~CalibAD_TEMP[i];							
			CalibAD_TEMP[i] = CalibAD_TEMP[i] + 1;					
			CalibAD_TEMP[i] = CalibAD_TEMP[i] | 0x80000000;	
		}		
	}
	
	/*----------线性拟合----------*/
//	if (Raw_Data <= CalibAD_TEMP[1])
//	{
//		k = (float)(Extreme_Value[1] - Extreme_Value[0]) / (CalibAD_TEMP[1] - CalibAD_TEMP[0]);
//		t = Extreme_Value[1] - k * CalibAD_TEMP[1];		
//	}
//	else 
//	{
//		k = (float)(Extreme_Value[2] - Extreme_Value[1]) / (CalibAD_TEMP[2] - CalibAD_TEMP[1]);
//		t = Extreme_Value[2] - k * CalibAD_TEMP[2];			
//	}
	
	if(CalibAD_TEMP[0] < CalibAD_TEMP[2])
	{
		if (Raw_Data <= CalibAD_TEMP[1])
		{
			k = (float)(Extreme_Value[1] - Extreme_Value[0]) / (CalibAD_TEMP[1] - CalibAD_TEMP[0]);
			t = Extreme_Value[1] - k * CalibAD_TEMP[1];		
		}
		else 
		{
			k = (float)(Extreme_Value[2] - Extreme_Value[1]) / (CalibAD_TEMP[2] - CalibAD_TEMP[1]);
			t = Extreme_Value[2] - k * CalibAD_TEMP[2];			
		}
	}
	else
	{
		if (Raw_Data <= CalibAD_TEMP[1])
		{
			k = (float)(Extreme_Value[2] - Extreme_Value[1]) / (CalibAD_TEMP[2] - CalibAD_TEMP[1]);
			t = Extreme_Value[2] - k * CalibAD_TEMP[2];	
		}
		else 
		{
			k = (float)(Extreme_Value[1] - Extreme_Value[0]) / (CalibAD_TEMP[1] - CalibAD_TEMP[0]);
			t = Extreme_Value[1] - k * CalibAD_TEMP[1];	
		}
	}
	
	Temp = k * Raw_Data + t;	

	/*----------滤波----------*/
	if (Init == 0)
	{
		Init = 1;
		Filter_Temp = Filter(0, Temp, 0); 		
	}
	else Filter_Temp = Filter(0, Temp, FILTER_COUNT);	
	
	/*----------单位----------*/
	switch(Para[0])
	{
		//kPa
		case 0:
			Value = Filter_Temp;
		break;
		//MPa
		case 1:
			Value = Filter_Temp / 1000.0;
		break;
		//Pa
		case 2:
			Value = Filter_Temp * 1000;
		break;
		//Bar
		case 3:
			Value = Filter_Temp / 100.0;
		break;
		//mBar
		case 4:
			Value = Filter_Temp * 10;
		break;
		//kg/cm2
		case 5:
			Value = Filter_Temp * 0.010197;
		break;
		//psi
		case 6:
			Value = Filter_Temp * 0.145038;
		break;
		//mH2O
		case 7:
			Value = Filter_Temp * 0.101972;
		break;
		//mmH2O
		case 8:
			Value = Filter_Temp * 101.9716;
		break;
		//kPa
		default: 
			Value = Filter_Temp;
			Set_Para(0, 0);
		break;
	}
	
//	if (Para[0] == 0)				Value = Filter_Temp; 											//kPa
//	else if (Para[0] == 1)	Value = Filter_Temp / 1000.0;							//MPa	
//	else if (Para[0] == 2)	Value = Filter_Temp * 1000;								//Pa		
//	else if (Para[0] == 3)	Value = Filter_Temp / 100.0;							//Bar		
//	else if (Para[0] == 4)	Value = Filter_Temp * 10;									//mBar	
//	else if (Para[0] == 5)	Value = Filter_Temp * 0.010197;						//kg/cm2	
//	else if (Para[0] == 6)	Value = Filter_Temp * 0.145038;						//psi	
//	else if (Para[0] == 7)	Value = Filter_Temp * 0.101972;						//mH2O	
//	else if (Para[0] == 8)	Value = Filter_Temp * 101.9716;						//mmH2O	
//	else
//	{
//		Value = Filter_Temp; 											//kPa
//		Set_Para(0, 0);
//	}
	
	/*----------小数点----------*/
	switch(Para[1])
	{
		//
		case 0:
			INT_Value = (int)(Value);
		break;
		//
		case 1:
			INT_Value = (int)(Value * 10);
		break;
		//
		case 2:
			INT_Value = (int)(Value * 100);
		break;
		//
		case 3:
			INT_Value = (int)(Value * 1000);
		break;
		//
		default: 
			Value = (int)(Value * 10);									//1位小数点
			Set_Para(1, 1);
		break;
	}
	
//	if (Para[1] == 0)		INT_Value = (int)(Value);
//	else if (Para[1] == 1)	INT_Value = (int)(Value * 10);
//	else if (Para[1] == 2)	INT_Value = (int)(Value * 100);	
//	else if (Para[1] == 3)	INT_Value = (int)(Value * 1000);	
//	else
//	{
//		Value = (int)(Value * 10);									//1位小数点
//		Set_Para(1, 1);
//	}	
}

static float Filter(u8 No, float Value, u16 Coff)	//滤波
{
	u16 n;
	float Total = 0;
	static float Filtetr[CH_NUM][FILTER_COUNT];
	if (Coff <= 0) Coff = 1;
	if (Coff > FILTER_COUNT) Coff = FILTER_COUNT;
	if (Coff <= 1)
	{
		for (n = 0; n < FILTER_COUNT; n++) Filtetr[No][n] = Value;
		return Value;
	}
	else
	{
		for (n = 0; n < FILTER_COUNT - 1; n++) Filtetr[No][n] = Filtetr[No][n + 1];
		Filtetr[No][FILTER_COUNT - 1] = Value;
		for (n = 0; n < Coff; n++) Total += Filtetr[No][FILTER_COUNT - 1 - n];
		return Total / Coff;
	}
}

int Get_CalibAD(u8 i)
{
	if (i >= Calib_Num)	return 0;
	return CalibAD[i];
}

int Get_PressureCal(u8 i)
{
	if (i >= 3)	return 0;
	return PressureCal[i];
}

void Set_CalibAD(u8 i, u32 v)
{
	if (i >= Calib_Num)	return;
	CalibAD[i] = v;
	Set_Calib_WriteFlag(1);
}
void Set_PressureCal(u8 i, u32 v)
{
	if (i >= 3)	return;
	PressureCal[i] = v;
	Set_Calib_WriteFlag(1);
}

float Get_Extreme_Value(u8 i)
{
	if (i >= Calib_Num)	return 0;
	return Extreme_Value[i];	
}

void Set_Extreme_Value(u8 i, float v)
{
	if (i >= Calib_Num)	return;
	Extreme_Value[i] = v;
	Set_Calib_WriteFlag(1);
}

u16 Get_Para(u8 i)
{
	if (i >= Para_Num)	return 0;
	return Para[i];
}

void Set_Para(u8 i, u16 v)
{
	if (i >= Para_Num)	return;
	if (i == 0)
	{
		if (v <= 8)
		{
			Para[i] = v;
			Set_Calib_WriteFlag(1);
			Init = 0;	//修改单位重新滤波
		}
	}
	else if (i == 1)
	{
		if (v <= 3)
		{
			Para[i] = v;
			Set_Calib_WriteFlag(1);
		}		
	}
}

float Get_Value(void)
{
	return Value;	
}

int Get_INT_Value(void)
{
	return INT_Value;	
}

