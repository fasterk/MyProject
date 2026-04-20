#include "Flash_Config.h"
#include "App_System.h"
#include "APP_ModReg.h"
#include "APP_JM1203.h"
#include "ModbusRTU.h"
#include "APP_Sensor.h"
#include "Timer_Config.h"
#include "PWM_Config.h"
#include <string.h>

//应用数据储存地址(Page 496)
#define APP_DATA_Memory_ADDR (0x0801F000)
extern uint32_t RangeNumber;
extern uint32_t RangeSet_Flag;

static volatile uint8_t sFlashEraseState = 0;
static int32_t FlashDataBuffer[256];//1024 byte（写数据需1K字节对齐，否则部分数据写入不保存）
static volatile uint8_t Calib_WriteFlag = 0;
static uint32_t vCalibDataWritePollTick = 0;
TypeParam_Config Param_Config = {0};

/****************************************
 *函数名称：FlashPageErase
 *功能：擦除应用数据段并写入
 *参数说明：擦写一次约
 ****************************************/
static void FlashPageErase(void)
{
	uint32_t PAGEError = 0;
	uint32_t flash_program_start = APP_DATA_Memory_ADDR;				
	uint32_t flash_program_end = (APP_DATA_Memory_ADDR + sizeof(FlashDataBuffer));
	uint32_t *src = (uint32_t *)FlashDataBuffer; 
	FLASH_EraseInitTypeDef EraseInitStruct;
	
	HAL_FLASH_Unlock();
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGEERASE;
	EraseInitStruct.PageAddress = APP_DATA_Memory_ADDR;
	EraseInitStruct.NbPages  = sizeof(FlashDataBuffer) >> 8;
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		while(1);
	}

	while(flash_program_start < flash_program_end)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, flash_program_start, src) == HAL_OK)
		{
			flash_program_start += FLASH_PAGE_SIZE;
			src += FLASH_PAGE_SIZE >> 2;
		}
	}
	HAL_FLASH_Lock();
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void Flash_Read_DATA(int32_t *p, uint32_t Start_Add, uint32_t Len)
{
	uint32_t i;
	for (i = 0; i < Len; i++)
		p[i] = *(uint32_t *)(Start_Add + i * 4);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void Set_Calib_WriteFlag(uint16_t v)
{
	Calib_WriteFlag = v;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void AppDataInit(void)
{
	AppDataTypeDef *AppDataTemp = (AppDataTypeDef *)(uint32_t *)APP_DATA_Memory_ADDR;
	
	//非第一次初始化
	if((AppDataTemp->Head == 0xAA5555AA) && (AppDataTemp->End == 0x55AAAA55))
	{
		Flash_Read_DATA(FlashDataBuffer, APP_DATA_Memory_ADDR, 256);
		RangeSet_Flag = AppDataTemp->RangeSetFlag;
		
		Param_Config.ZeroingExtraScopeVal = AppDataTemp->ZeroingExtraScopeVal;
		Param_Config.ProductClass = AppDataTemp->ProductClass;
		Param_Config.AirPressureValueType = AppDataTemp->PressureValueType;
			
		Param_Config.PressureUpperLimit = AppDataTemp->OutPressureUpperLimit;
		Param_Config.PressureLowerLimit = AppDataTemp->OutPressureLowerLimit;
		Param_Config.LagValUpperLimit = AppDataTemp->OutLagValUpperLimit;
		Param_Config.LagValLowerLimit =AppDataTemp->OutPressureLowerLimit;
		Param_Config.DelayTimeUpperLimit = AppDataTemp->OutDelayTimeUpperLimit;
		Param_Config.DelayTimeLowerLimit = AppDataTemp->OutDelayTimeLowerLimit;
		Param_Config.ZeroingValUpperLimit = AppDataTemp->OutZeroingValUpperLimit;
		Param_Config.ZeroingValLowerLimit = AppDataTemp->OutZeroingValLowerLimit;
		Param_Config.ZeroingErrorRange = AppDataTemp->OutZeroingErrorRange;
		
		Param_Config.Default_LagVal = AppDataTemp->Out1LagVal;		
		Param_Config.Default_Ch1_ThresholdVal = AppDataTemp->Out1ThresholdVal;
		Param_Config.Default_Ch1_PeakValVal = AppDataTemp->Out1PeakVal;
		Param_Config.Default_Ch1_ValleyVal = AppDataTemp->Out1ValleyVal;
		Param_Config.Default_Ch2_ThresholdVal = AppDataTemp->Out2ThresholdVal;
		Param_Config.Default_Ch2_PeakValVal = AppDataTemp->Out2PeakVal;
		Param_Config.Default_Ch2_ValleyVal = AppDataTemp->Out2ValleyVal;	
	}
		
	else
	{
		APPDataEmpty(1);
		System_ParameterReset();
		APPDataFlashWrite();
		Flash_Read_DATA(FlashDataBuffer, APP_DATA_Memory_ADDR, 256);
	}
	//
	Set_UART_PARA(0,AppDataRead(APP_DevID));
	Set_UART_PARA(1,AppDataRead(APP_CommBaudRate));
	//
	Set_PressureCal(0, AppDataRead(APP_CurrentOutCalibrationVal_4MA));
	Set_PressureCal(1, AppDataRead(APP_CurrentOutCalibrationVal_Temp1));
	Set_PressureCal(2, AppDataRead(APP_CurrentOutCalibrationVal_20MA));
	//
	Set_CalibAD(0, AppDataRead(APP_CalibADxVal1));
	Set_CalibAD(1, AppDataRead(APP_CalibADxVal2));
	Set_CalibAD(2, AppDataRead(APP_CalibADxVal3));
	//
	Set_Extreme_Value(0, UInt32ToFloat(AppDataRead(APP_ExtremeVal1)));
	Set_Extreme_Value(1, UInt32ToFloat(AppDataRead(APP_ExtremeVal2)));
	Set_Extreme_Value(2, UInt32ToFloat(AppDataRead(APP_ExtremeVal3)));
	//
	Set_Para(0, AppDataRead(APP_ParamVal1));
	Set_Para(1, AppDataRead(APP_ParamVal2));
	//
	Set_JM1203_Set(0, AppDataRead(APP_JM1203ParamVal1));
	Set_JM1203_Set(1, AppDataRead(APP_JM1203ParamVal2));
	Set_JM1203_Set(2, AppDataRead(APP_JM1203ParamVal3));
	Set_JM1203_Set(3, AppDataRead(APP_JM1203ParamVal4));
	Set_JM1203_Set(4, AppDataRead(APP_JM1203ParamVal5));
	Set_JM1203_Set(5, AppDataRead(APP_JM1203ParamVal6));
	Set_JM1203_Set(6, AppDataRead(APP_JM1203ParamVal7));
	Set_JM1203_Set(7, AppDataRead(APP_JM1203ParamVal8));
	Set_JM1203_Set(8, AppDataRead(APP_JM1203ParamVal9));
	Set_JM1203_Set(9, AppDataRead(APP_JM1203ParamVal10));
}
/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void AppSetRange(void)
{
//	AppDataTypeDef *AppDataTemp = (AppDataTypeDef *)(uint32_t *)APP_DATA_Memory_ADDR;	
	System_ParameterReset();
	APPDataFlashWrite();
	Flash_Read_DATA(FlashDataBuffer, APP_DATA_Memory_ADDR, 256);
}
/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void AppDataWrite(int32_t val, AppDataRegNumber reg_num)
{
	if(reg_num >= User_RegNumberError || reg_num == 0) return;
	
	if(val != FlashDataBuffer[reg_num])
	{
		FlashDataBuffer[reg_num] = val;
		sFlashEraseState = 1;	//有擦写需求
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
int32_t AppDataRead(AppDataRegNumber reg_num)
{
	if(reg_num >= User_RegNumberError || reg_num == 0) return 0;
	
	return FlashDataBuffer[reg_num];
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void APPDataFlashWrite(void)
{
	if(sFlashEraseState)
	{
		sFlashEraseState = 0;
		FlashPageErase();
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void APPDataEmpty(uint8_t resetFlag)
{
	AppDataTypeDef *AppDataTemp = (AppDataTypeDef *)(uint32_t *)FlashDataBuffer;
	
	memset(FlashDataBuffer, 0, sizeof(FlashDataBuffer));
	
	AppDataTemp->Head = 0xAA5555AA;
	AppDataTemp->End = 0x55AAAA55;
	AppDataTemp->DevID = 1;
	AppDataTemp->CommBaudRate = 3;
	
	if(resetFlag)
	{
		AppDataTemp->CurrentOutCalibrationVal_4MA = 3200;
		AppDataTemp->CurrentOutCalibrationVal_Temp1 = 0;
		AppDataTemp->CurrentOutCalibrationVal_20MA = 5600;
		
		AppDataTemp->ExtremeValue[0] = 0;
		AppDataTemp->ExtremeValue[1] = 50;
		AppDataTemp->ExtremeValue[2] = 100;
		AppDataTemp->ParamValue[0] = 0;
		AppDataTemp->ParamValue[1] = 1;
		AppDataTemp->JM1203ParamValue[0] = 0xA9;
		AppDataTemp->JM1203ParamValue[1] = AZBM_ADD;
		AppDataTemp->JM1203ParamValue[2] = AZTM_ADD;
		AppDataTemp->JM1203ParamValue[3] = Temp_OverSample_Rate_8;
		AppDataTemp->JM1203ParamValue[4] = Bridge_OverSample_Rate_128;
		AppDataTemp->JM1203ParamValue[5] = Offset1_16;
		AppDataTemp->JM1203ParamValue[6] = Clock_Sample_Rate5_3;
		AppDataTemp->JM1203ParamValue[7] = Polarity_Forward;
		AppDataTemp->JM1203ParamValue[8] = TWO_Gain1_3;
		AppDataTemp->JM1203ParamValue[9] = ONE_Gain40;
	}
	else
	{
		AppDataTemp = (AppDataTypeDef *)(uint32_t *)APP_DATA_Memory_ADDR;
		
		FlashDataBuffer[APP_CurrentOutCalibrationVal_4MA] = AppDataTemp->CurrentOutCalibrationVal_4MA;
		FlashDataBuffer[APP_CurrentOutCalibrationVal_Temp1] = AppDataTemp->CurrentOutCalibrationVal_Temp1;
		FlashDataBuffer[APP_CurrentOutCalibrationVal_20MA] = AppDataTemp->CurrentOutCalibrationVal_20MA;
		
		FlashDataBuffer[APP_CalibADxVal1] = AppDataTemp->CalibADxValue[0];
		FlashDataBuffer[APP_CalibADxVal2] = AppDataTemp->CalibADxValue[1];
		FlashDataBuffer[APP_CalibADxVal3] = AppDataTemp->CalibADxValue[2];
		FlashDataBuffer[APP_ExtremeVal1] = AppDataTemp->ExtremeValue[0];
		FlashDataBuffer[APP_ExtremeVal2] = AppDataTemp->ExtremeValue[1];
		FlashDataBuffer[APP_ExtremeVal3] = AppDataTemp->ExtremeValue[2];
		FlashDataBuffer[APP_ParamVal1] = AppDataTemp->ParamValue[0];
		FlashDataBuffer[APP_ParamVal2] = AppDataTemp->ParamValue[1];
		FlashDataBuffer[APP_ParamVal3] = AppDataTemp->ParamValue[2];
		FlashDataBuffer[APP_ParamVal4] = AppDataTemp->ParamValue[3];
		FlashDataBuffer[APP_ParamVal5] = AppDataTemp->ParamValue[4];
		FlashDataBuffer[APP_ParamVal6] = AppDataTemp->ParamValue[5];
		FlashDataBuffer[APP_ParamVal7] = AppDataTemp->ParamValue[6];
		FlashDataBuffer[APP_ParamVal8] = AppDataTemp->ParamValue[7];
		FlashDataBuffer[APP_ParamVal9] = AppDataTemp->ParamValue[8];
		FlashDataBuffer[APP_JM1203ParamVal1] = AppDataTemp->JM1203ParamValue[0];
		FlashDataBuffer[APP_JM1203ParamVal2] = AppDataTemp->JM1203ParamValue[1];
		FlashDataBuffer[APP_JM1203ParamVal3] = AppDataTemp->JM1203ParamValue[2];
		FlashDataBuffer[APP_JM1203ParamVal4] = AppDataTemp->JM1203ParamValue[3];
		FlashDataBuffer[APP_JM1203ParamVal5] = AppDataTemp->JM1203ParamValue[4];
		FlashDataBuffer[APP_JM1203ParamVal6] = AppDataTemp->JM1203ParamValue[5];
		FlashDataBuffer[APP_JM1203ParamVal7] = AppDataTemp->JM1203ParamValue[6];
		FlashDataBuffer[APP_JM1203ParamVal8] = AppDataTemp->JM1203ParamValue[7];
		FlashDataBuffer[APP_JM1203ParamVal9] = AppDataTemp->JM1203ParamValue[8];
		FlashDataBuffer[APP_JM1203ParamVal10] = AppDataTemp->JM1203ParamValue[9];
	}
	
	sFlashEraseState = 1;
}



/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void CalibDataWritePoll_Task(void)
{
	if(GetSystemTick() - vCalibDataWritePollTick > 3000)
	{
		if(Calib_WriteFlag == 1)
		{
			Calib_WriteFlag = 0;
			//
			AppDataWrite(Get_UART_PARA(0), APP_DevID);
			AppDataWrite(Get_UART_PARA(1), APP_CommBaudRate);
			//
			AppDataWrite(Get_PressureCal(0), APP_CurrentOutCalibrationVal_4MA);
			AppDataWrite(Get_PressureCal(1), APP_CurrentOutCalibrationVal_Temp1);
			AppDataWrite(Get_PressureCal(2), APP_CurrentOutCalibrationVal_20MA);
			//
			AppDataWrite(Get_CalibAD(0), APP_CalibADxVal1);
			AppDataWrite(Get_CalibAD(1), APP_CalibADxVal2);
			AppDataWrite(Get_CalibAD(2), APP_CalibADxVal3);
			//
			AppDataWrite(FloatToUInt32(Get_Extreme_Value(0)), APP_ExtremeVal1);
			AppDataWrite(FloatToUInt32(Get_Extreme_Value(1)), APP_ExtremeVal2);
			AppDataWrite(FloatToUInt32(Get_Extreme_Value(2)), APP_ExtremeVal3);
			//
			AppDataWrite(Get_Para(0), APP_ParamVal1);
			AppDataWrite(Get_Para(1), APP_ParamVal2);
			//
			AppDataWrite(Get_JM1203_Set(0), APP_JM1203ParamVal1);
			AppDataWrite(Get_JM1203_Set(1), APP_JM1203ParamVal2);
			AppDataWrite(Get_JM1203_Set(2), APP_JM1203ParamVal3);
			AppDataWrite(Get_JM1203_Set(3), APP_JM1203ParamVal4);
			AppDataWrite(Get_JM1203_Set(4), APP_JM1203ParamVal5);
			AppDataWrite(Get_JM1203_Set(5), APP_JM1203ParamVal6);
			AppDataWrite(Get_JM1203_Set(6), APP_JM1203ParamVal7);
			AppDataWrite(Get_JM1203_Set(7), APP_JM1203ParamVal8);
			AppDataWrite(Get_JM1203_Set(8), APP_JM1203ParamVal9);
			AppDataWrite(Get_JM1203_Set(9), APP_JM1203ParamVal10);
			
			APPDataFlashWrite();
		}
		vCalibDataWritePollTick = GetSystemTick();
	}
}
