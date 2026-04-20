#include "APP_ModReg.h"
#include "ModbusRTU.h"
#include "UART_Config.h"
#include "APP_Sensor.h"
#include "APP_JM1203.h"
#include "App_System.h"

extern TypeParam_Config Param_Config;
void App_Register(uint16_t Reg, uint16_t *v, uint8_t Flag) 
{
	static uint16_t TempH;
	int32_t TempVal_S32 = Get_INT_Value();
	
	if (Reg <= 1)
	{
		if (Reg % 2 == 0)
		{
			if (Flag == R) 
			{
				//仅为正
				if (Param_Config.AirPressureValueType == 1)
				{
					if(TempVal_S32 <= 0) *v = 0;
				}		
					else *v = FloatToUInt32(Get_Value()) >> 16;
				//仅为负
				if (Param_Config.AirPressureValueType == 2)
				{
					if(TempVal_S32 >= 0) *v = 0;
					else *v = FloatToUInt32(Get_Value()) >> 16;
				}				
				else
					*v = FloatToUInt32(Get_Value()) >> 16;
				
			}
		}
		else
		{
			if (Flag == R) 
			{
				//仅为正
				if(Param_Config.AirPressureValueType == 1)
				{
					if(TempVal_S32 <= 0) 
						*v = 0;
					else 
						*v = FloatToUInt32(Get_Value());
				}
					
				//仅为负
				if (Param_Config.AirPressureValueType == 2)
				{
					if(TempVal_S32 >= 0) *v = 0;
					else *v = FloatToUInt32(Get_Value());
				}				
				else
					*v = FloatToUInt32(Get_Value());

			}
		}
	}
	else if (Reg >= 2 && Reg <= 3)	
	{
		if (Reg % 2 == 0)
		{
			if (Flag == R) 
			{
				//仅为正
				if( Param_Config.AirPressureValueType == 1)
				{
					if(TempVal_S32 <= 0) *v = 0;
					else *v = Get_INT_Value() >> 16;
				}
					
				//仅为负
				if (Param_Config.AirPressureValueType == 2)
				{
					if(TempVal_S32 >= 0) *v = 0;
					else *v = Get_INT_Value() >> 16;
				}					
				else
					*v = Get_INT_Value() >> 16;
			}
		}
		else
		{
			if (Flag == R) 
			{
				//仅为正
				if(Param_Config.AirPressureValueType == 1)
				{
					if(TempVal_S32 <= 0) *v = 0;
					else *v = Get_INT_Value();
				}
					
				//仅为负
				if (Param_Config.AirPressureValueType == 2)
				{
					if(TempVal_S32 >= 0) *v = 0;
					else *v = Get_INT_Value();
				}				
				else
					*v = Get_INT_Value();
			}
		}
	}
	else if (Reg >= 4 && Reg <= 5)
	{
		if (Flag == R) *v = Get_Para(Reg - 4);
//		else if(Flag == W)	Set_Para(Reg - 4, *v);
	}		
	else if (Reg >= 6 && Reg <= 7)
	{
		if (Flag == R) *v = Get_UART_PARA(Reg - 6);
//		else if(Flag == W)	Set_UART_PARA(Reg - 6, *v);
	}	
	else if (Reg == 8)
	{
		if (Flag == R) *v = 102;
	}
	else if (Reg >= 30 && Reg <= 31)	
	{
		if (Reg % 2 == 0)
		{
			if (Flag == R) *v = Get_Bridge_RawData() >> 16;
		}
		else
		{
			if (Flag == R) *v = Get_Bridge_RawData();
		}
	}
	else if (Reg >= 32 && Reg <= 37)
	{
		if (Reg % 2 == 0)
		{
			if (Flag == R) *v = Get_CalibAD((Reg - 32) / 2) >> 16;
			else if(Flag == W)	TempH = *v;
		}
		else
		{
			if (Flag == R) *v = Get_CalibAD((Reg - 32) / 2);
			else if(Flag == W)	Set_CalibAD((Reg - 32) / 2, (TempH << 16) | *v);
		}
	}
	else if (Reg >= 38 && Reg <= 43)
	{
		if (Reg % 2 == 0)
		{
			if (Flag == R) *v = FloatToUInt32(Get_Extreme_Value((Reg - 38) / 2)) >> 16;
			else if(Flag == W)	TempH = *v;
		}
		else
		{
			if (Flag == R) *v = FloatToUInt32(Get_Extreme_Value((Reg - 38) / 2));
			else if(Flag == W)	Set_Extreme_Value((Reg - 38) / 2, UInt32ToFloat((TempH << 16) | *v));
		}	
	}			

	else if (Reg >= 100 && Reg <= 109)	
	{
		if (Flag == R) 			*v = Get_JM1203_Set(Reg - 100);
		else if (Flag == W)	Set_JM1203_Set(Reg - 100, *v);
	}	
	
	else if (Reg >= 110 && Reg <= 115)
		{
			if (Reg % 2 == 0)
			{
				if (Flag == R) *v = Get_PressureCal((Reg - 110) / 2) >> 16;
				else if(Flag == W)	TempH = *v;
			}
			else
			{
				if (Flag == R) *v = Get_PressureCal((Reg - 110) / 2);
				else if(Flag == W)	Set_PressureCal((Reg - 110) / 2, (TempH << 16) | *v);
			}
		}	
	
	/*     ?       0*/
	else 
	{
		if (Flag == R)	*v = 0;
		else if (Flag == W) *v = 0;    		
	}
}

