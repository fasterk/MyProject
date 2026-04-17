#include "ModbusRTU.h"
#include "APP_ModReg.h"
#include "UART_Config.h"
#include "Flash_Config.h"
#include <string.h>

#define MODBUS_BUFFER_MAX (64)

static uint8_t vModbusID = 0x01;		//从机ID
static uint16_t vModbus_RxLen = 0;	//数据接收长度
static uint16_t UART_PARA[2];		//串口参数

static uint8_t vModbus_Rx_Buff[MODBUS_BUFFER_MAX];	//数据接收缓存
static uint8_t vModbus_Tx_Buff[MODBUS_BUFFER_MAX];	//数据发送缓存

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
static uint16_t Modbus_16CRC(uint8_t *crc_buf, uint8_t crc_len)
{
	uint8_t i,j;
	uint16_t crc_reg = 0xFFFF;
	
	for(i=0; i<crc_len; i++)
	{
		crc_reg = crc_reg ^ *(crc_buf + i);
		
		for(j=0; j<8; j++)
		{
			if (crc_reg & 0x01)
			{
				crc_reg = crc_reg >> 1;
				crc_reg = crc_reg ^ 0xA001;
			}
			else
				crc_reg = crc_reg >> 1;
		}
	}
	return crc_reg;
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
static void Modbus_03H(uint8_t *RxBuf, uint8_t *TxBuf)	//读保持寄存器
{ 
	uint8_t n;
	uint16_t Crc;
	uint16_t TxCount;
	uint16_t reg = (RxBuf[2] << 8) | RxBuf[3];
	uint16_t num = (RxBuf[4] << 8) | RxBuf[5];

	TxCount = 0;
	TxBuf[TxCount++] = RxBuf[0];   
	TxBuf[TxCount++] = 0x03;
	if (2*num > MODBUS_BUFFER_MAX - 5)	num = (MODBUS_BUFFER_MAX - 5)/2;
	TxBuf[TxCount++] = num * 2;
	
	for (n = 0; n < num; n++) 
	{
		uint16_t v;
		App_Register(reg++, &v, R); 
		TxBuf[TxCount] = v >> 8;
		TxBuf[TxCount + 1] = v;	
		TxCount = TxCount+2;
	}

	Crc = Modbus_16CRC(TxBuf, TxCount);
	TxBuf[TxCount++] = (uint8_t)Crc;
	TxBuf[TxCount++] = (uint8_t)(Crc >> 8);
	
	UART_SendData(UART_TTL, TxBuf, TxCount);
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
static void Modbus_06H(uint8_t *RxBuf, uint8_t *TxBuf)	//写单个寄存器
{ 
	uint16_t reg = (RxBuf[2] << 8) | RxBuf[3];
	uint16_t value = (RxBuf[4] << 8) | RxBuf[5];

	App_Register(reg, &value, W);
	
	UART_SendData(UART_TTL, RxBuf, 8);
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
static void Modbus_10H(uint8_t *RxBuf, uint8_t *TxBuf)	//写多个寄存器
{ 
	uint8_t i;
	uint16_t Crc;	
	uint16_t TxCount;
	uint16_t reg = (RxBuf[2] << 8) | RxBuf[3];
	uint16_t num = (RxBuf[4] << 8) | RxBuf[5];

	if (RxBuf[6] == num * 2)  //数据长度校验
	{
		for (i = 0; i < num; i++)
		{
			uint16_t value = (RxBuf[7 + 2 * i] << 8) | RxBuf[8 + 2 * i];
			App_Register(reg + i, &value, W);
		}
	}
		
	TxCount = 0;
	TxBuf[TxCount++] = RxBuf[0];   
	TxBuf[TxCount++] = 0x10;
	
	TxBuf[TxCount++] = (uint8_t)(reg >> 8);
	TxBuf[TxCount++] = (uint8_t)reg;

	TxBuf[TxCount++] = (uint8_t)(num >> 8);
	TxBuf[TxCount++] = (uint8_t)num;
	
	Crc = Modbus_16CRC(TxBuf, TxCount);
	TxBuf[TxCount++] = (uint8_t)Crc;
	TxBuf[TxCount++] = (uint8_t)(Crc >> 8);
	
	UART_SendData(UART_TTL, TxBuf, TxCount);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void TTL_RxCallbackMsp(uint16_t Len)
{
	vModbus_RxLen = Len;
	
//	uint16_t rx_crc;
//	uint16_t cnt_crc;
//	
//	if(vModbus_RxLen > 0)
//	{
//		UART_ReadData(UART_TTL, vModbus_Rx_Buff, vModbus_RxLen);
////		UART_SendData(UART_TTL, vModbus_Rx_Buff, vModbus_RxLen);
//		
//		if(vModbus_Rx_Buff[0] == vModbusID)	//判断ID是否正确
//		{		
//			rx_crc = Modbus_16CRC(vModbus_Rx_Buff, vModbus_RxLen - 2);	//计算接收到的CRC
//			cnt_crc = vModbus_Rx_Buff[vModbus_RxLen - 2] | (((uint16_t)vModbus_Rx_Buff[vModbus_RxLen - 1]) << 8);		//计算接收到主机的CRC
//			
//			if((rx_crc == cnt_crc) && (vModbus_RxLen >= 8))	//校验CRC一级数据长度是否有误
//			{		
//				switch(vModbus_Rx_Buff[1])
//				{
//					case 0x03: Modbus_03H(vModbus_Rx_Buff, vModbus_Tx_Buff); break;		//读保持寄存器
//					case 0x06: Modbus_06H(vModbus_Rx_Buff, vModbus_Tx_Buff); break;		//写单个保持寄存器
//					case 0x10: Modbus_10H(vModbus_Rx_Buff, vModbus_Tx_Buff); break;		//写多个保持寄存器
//				}
//			}
//		}
//		vModbus_RxLen = 0;
//	}
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void MobudRtuInit(void)
{
	UART_DeviceTypeDef UART_DevIniture;
	
	//初始化TTL
	UART_InitAll();
	//
	if (UART_PARA[0] >= 1 && UART_PARA[0] <= 254)	vModbusID = UART_PARA[0];
	else 
	{
		Set_UART_PARA(0, 1);
		vModbusID = 1;
	}
	
	if (UART_PARA[1] == 1)			UART_DevIniture.Bps = 2400;
	else if (UART_PARA[1] == 2)	UART_DevIniture.Bps = 4800;
	else if (UART_PARA[1] == 3)	UART_DevIniture.Bps = 9600;
	else if (UART_PARA[1] == 4)	UART_DevIniture.Bps = 19200;
	else if (UART_PARA[1] == 5)	UART_DevIniture.Bps = 38400;	
	else if (UART_PARA[1] == 6)	UART_DevIniture.Bps = 57600;
	else if (UART_PARA[1] == 7)	UART_DevIniture.Bps = 115200;		
	else 
	{
		Set_UART_PARA(1, 3);
		UART_DevIniture.Bps = 9600;
	}
	UART_DevIniture.Parity = UART_PARITY_NONE;
	UART_DevIniture.StopBits = UART_STOPBITS_1;
	UART_DevIniture.WordLength = UART_WORDLENGTH_8B;
	UART_Bsp_Init(UART_TTL, &UART_DevIniture);
	//设置接收回调
	UART_RxCallBackAdd(UART_TTL, TTL_RxCallbackMsp);
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
unsigned int FloatToUInt32(float v)	//IEEE754格式
{
	return *(unsigned int*)(&v);
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
float UInt32ToFloat(unsigned int v)	//IEEE754格式
{
	return *(float*)(&v);
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
uint16_t Get_UART_PARA(uint8_t i)
{
	if (i >= 2)	return 0;
	return UART_PARA[i];
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void Set_UART_PARA(uint8_t i, uint16_t v)
{
	if (i >= 2)	return;
	if (i == 0 && v <= 254)	
	{
		if (v >= 1 && v <= 254)
		{
			UART_PARA[i] = v;
			Set_Calib_WriteFlag(1);
		}
	}
	else if (i == 1)
	{
		if (v >= 1 && v <= 7)
		{
			UART_PARA[i] = v;
			Set_Calib_WriteFlag(1);
		}
	}		
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void ModbusRTU_Task(void)
{
	uint16_t rx_crc;
	uint16_t cnt_crc;
	
	if(vModbus_RxLen > 0)
	{
		UART_ReadData(UART_TTL, vModbus_Rx_Buff, vModbus_RxLen);
//		UART_SendData(UART_TTL, vModbus_Rx_Buff, vModbus_RxLen);
		
		if(vModbus_Rx_Buff[0] == vModbusID)	//判断ID是否正确
		{		
			rx_crc = Modbus_16CRC(vModbus_Rx_Buff, vModbus_RxLen - 2);	//计算接收到的CRC
			cnt_crc = vModbus_Rx_Buff[vModbus_RxLen - 2] | (((uint16_t)vModbus_Rx_Buff[vModbus_RxLen - 1]) << 8);		//计算接收到主机的CRC
			
			if((rx_crc == cnt_crc) && (vModbus_RxLen >= 8))	//校验CRC一级数据长度是否有误
			{		
				switch(vModbus_Rx_Buff[1])
				{
					case 0x03: Modbus_03H(vModbus_Rx_Buff, vModbus_Tx_Buff); break;		//读保持寄存器
					case 0x06: Modbus_06H(vModbus_Rx_Buff, vModbus_Tx_Buff); break;		//写单个保持寄存器
					case 0x10: Modbus_10H(vModbus_Rx_Buff, vModbus_Tx_Buff); break;		//写多个保持寄存器
				}
			}
		}
		vModbus_RxLen = 0;
	}
}

