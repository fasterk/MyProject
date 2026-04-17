#include "Uart_Config.h"
#include <string.h>
#include <stdio.h>

#define RXFIFO 0
#define TXFIFO 1

//TTL接收发送缓存大小
#define TTL_BUFFER_MAX (64)

typedef struct
{
	uint8_t *Buff;
	uint16_t RxIndex;
	uint16_t TxIndex;
	uint16_t BuffLen;
}UartFifoTypeDef;

typedef struct
{
	uint8_t sTxing;
	uint8_t sRxing;
	uint8_t sError;
	UartFifoTypeDef RxFifo;
	UartFifoTypeDef TxFifo;
	void (*RxCallback)(uint16_t Len);	
}UartDevObjTypeDef;

static uint8_t TTL_RxBuff[TTL_BUFFER_MAX];
static uint8_t TTL_TxBuff[TTL_BUFFER_MAX];

//
static UART_HandleTypeDef UartHandle = {0};
//
static UartDevObjTypeDef UartDevObj[UART_Max] = {0,0,0,{TTL_RxBuff,0,0,TTL_BUFFER_MAX},{TTL_TxBuff,0,0,TTL_BUFFER_MAX},0};


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static uint16_t UartGetRxFifoLength(UART_Dev dev)
{
	if(UartDevObj[dev].RxFifo.TxIndex > UartDevObj[dev].RxFifo.RxIndex)
	{
		return (UartDevObj[dev].RxFifo.TxIndex - UartDevObj[dev].RxFifo.RxIndex);
	}
	else if(UartDevObj[dev].RxFifo.TxIndex < UartDevObj[dev].RxFifo.RxIndex)
	{
		return (UartDevObj[dev].RxFifo.TxIndex + UartDevObj[dev].RxFifo.BuffLen - UartDevObj[dev].RxFifo.RxIndex);
	}
	else
		return 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static uint16_t UartGetTxFifoLength(UART_Dev dev)
{
	if(UartDevObj[dev].TxFifo.TxIndex > UartDevObj[dev].TxFifo.RxIndex)
	{
		return (UartDevObj[dev].TxFifo.TxIndex - UartDevObj[dev].TxFifo.RxIndex);
	}
	else if(UartDevObj[dev].TxFifo.TxIndex < UartDevObj[dev].TxFifo.RxIndex)
	{
		return (UartDevObj[dev].TxFifo.TxIndex + UartDevObj[dev].TxFifo.BuffLen - UartDevObj[dev].TxFifo.RxIndex);
	}
	else
		return 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void UartWriteFromFifo(UART_Dev dev, uint8_t rx_or_tx, uint8_t dat)
{
	uint8_t *buf;
	
	if(rx_or_tx == RXFIFO)
	{
		buf = UartDevObj[dev].RxFifo.Buff;
		buf[UartDevObj[dev].RxFifo.TxIndex] = dat;
		UartDevObj[dev].RxFifo.TxIndex++;
		
		if(UartDevObj[dev].RxFifo.TxIndex >= UartDevObj[dev].RxFifo.BuffLen)
			UartDevObj[dev].RxFifo.TxIndex = 0;
	}
	else if(rx_or_tx == TXFIFO)
	{
		buf = UartDevObj[dev].TxFifo.Buff;
		buf[UartDevObj[dev].TxFifo.TxIndex] = dat;
		UartDevObj[dev].TxFifo.TxIndex++;
		
		if(UartDevObj[dev].TxFifo.TxIndex >= UartDevObj[dev].TxFifo.BuffLen)
			UartDevObj[dev].TxFifo.TxIndex = 0;
	}
	else
		return;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static int16_t UartReadFromFifo(UART_Dev dev, uint8_t rx_or_tx)
{
	uint8_t *buf, dat;
	
	if(rx_or_tx == RXFIFO)
	{
		buf = UartDevObj[dev].RxFifo.Buff;
		
		if(UartDevObj[dev].RxFifo.RxIndex == UartDevObj[dev].RxFifo.TxIndex)
		{
			return -1;
		}
		else
		{
			dat = buf[UartDevObj[dev].RxFifo.RxIndex];
			UartDevObj[dev].RxFifo.RxIndex++;
			
			if(UartDevObj[dev].RxFifo.RxIndex >= UartDevObj[dev].RxFifo.BuffLen)
				UartDevObj[dev].RxFifo.RxIndex = 0;
		}
	}
	else if(rx_or_tx == TXFIFO)
	{
		buf = UartDevObj[dev].TxFifo.Buff;
		
		if(UartDevObj[dev].TxFifo.RxIndex == UartDevObj[dev].TxFifo.TxIndex)
		{
			return -1;
		}
		else
		{
			dat = buf[UartDevObj[dev].TxFifo.RxIndex];
			UartDevObj[dev].TxFifo.RxIndex++;
			
			if(UartDevObj[dev].TxFifo.RxIndex >= UartDevObj[dev].TxFifo.BuffLen)
				UartDevObj[dev].TxFifo.RxIndex = 0;
		}
	}
	else
		return -1;
	
	return dat;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void UartStartTx(UART_Dev dev)
{
	int16_t dat;
	
	switch(dev)
	{
		case UART_TTL:
			dat = UartReadFromFifo(UART_TTL, TXFIFO);
			UartDevObj[dev].sTxing = 1;
			USART2->DR = (uint8_t)dat;
			//开启发送中断
			__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_TXE);
			break;
		
		case UART_Max:
			break;
		
		default:
			break;
	}
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void UART_InitAll(void)
{
//	UART_DeviceTypeDef UART_DevIniture;
	
	memset(TTL_RxBuff, 0, sizeof(TTL_RxBuff));
	memset(TTL_TxBuff, 0, sizeof(TTL_TxBuff));
	
//	UART_DevIniture.Bps = 9600;
//	UART_DevIniture.Parity = UART_PARITY_NONE;
//	UART_DevIniture.StopBits = UART_STOPBITS_1;
//	UART_DevIniture.WordLength = UART_WORDLENGTH_8B;
//	UART_Bsp_Init(UART_TTL, &UART_DevIniture);
}

/****************************************
 *函数名称：
 *参数：
 *功能：
 ****************************************/
void UART_Bsp_Init(UART_Dev dev, UART_DeviceTypeDef *uart_init)
{
	switch(dev)
	{
		case UART_TTL:
			//
			UartHandle.Instance          = USART2;
			UartHandle.Init.BaudRate     = uart_init->Bps;
			UartHandle.Init.WordLength   = uart_init->WordLength;
			UartHandle.Init.StopBits     = uart_init->StopBits;
			UartHandle.Init.Parity       = uart_init->Parity;
			UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
			UartHandle.Init.Mode         = UART_MODE_TX_RX;
			UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
			UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
			if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
			{
				while(1);
			}
			if (HAL_UART_Init(&UartHandle) != HAL_OK)
			{
				while(1);
			}
			//开启接收中断
			__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
			//开启空闲中断
			__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_IDLE);
			//开启校验错误中断
			__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_PE);
			//清除空闲中断标志
			__HAL_UART_CLEAR_IDLEFLAG(&UartHandle);
		break;
		
		case UART_Max: break;
		
		default: break;
	}
}	

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint16_t UART_ReadData(UART_Dev dev, uint8_t *data, uint16_t len)
{
	int16_t ch;
	uint16_t index=0, x=0;
	
	if((dev >= UART_Max)||(data == NULL)||(len > TTL_BUFFER_MAX)) return 0;
	
	__disable_irq();
	for(x=0; x<len; x++)
	{
		ch = UartReadFromFifo(UART_TTL, RXFIFO);
		if(ch != -1)
		{
			*(data + x) = ch;
			index++;
		}
		else
		{
			break;
		}
	}
	__enable_irq();
	
	return index;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void UART_SendData(UART_Dev dev, uint8_t *data, uint16_t len)
{
	uint16_t x=0;
	
	if((dev >= UART_Max)||(data == NULL)||(len > TTL_BUFFER_MAX)) return;
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
//	HAL_Delay(1);
	__disable_irq();
	for(x=0; x<len; x++)
	{
		UartWriteFromFifo(UART_TTL, TXFIFO, *(data+x));
	}
	__enable_irq();
	
	UartStartTx(dev);
	
//	while(UART_TTLBusyCheck() == 1)
//	{
//		HAL_Delay(2);
//	}
//	
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void UART_ClearTxFifo(UART_Dev dev)
{
	if(dev >= UART_Max) return;
	UartDevObj[dev].TxFifo.RxIndex = 0;
	UartDevObj[dev].TxFifo.TxIndex = 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void UART_ClearRxFifo(UART_Dev dev)
{
	if(dev >= UART_Max) return;
	UartDevObj[dev].RxFifo.RxIndex = 0;
	UartDevObj[dev].RxFifo.TxIndex = 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t UART_TTLBusyCheck(void)
{
	if(UartDevObj[UART_TTL].sTxing || UartDevObj[UART_TTL].sRxing)
		return 1;
	else
		return 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void UART_RxCallBackAdd(UART_Dev dev, void (*callback)(uint16_t len))
{
	if((dev >= UART_Max)||(callback == NULL)) return;
	UartDevObj[dev].RxCallback = callback;
}

//串口2中断服务函数
void USART2_IRQHandler(void)
{
	int16_t ch;
	
	//发送FIFO空
	if(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_TXE))
    {
//		__USART_FLAG_CLEAR(USART2, USART_IFCLR_TXFEFC);
		ch = UartReadFromFifo(UART_TTL, TXFIFO);
		if(ch != -1)
		{
			USART2->DR = (uint8_t)ch;
		}
		else
		{
			__HAL_UART_DISABLE_IT(&UartHandle, UART_IT_TXE);
			__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_TC);
		}
	}
	//发送完成
	if(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_TC))
    {
		__HAL_UART_CLEAR_FLAG(&UartHandle, UART_FLAG_TC);
		__HAL_UART_DISABLE_IT(&UartHandle, UART_IT_TC);
		
		UartDevObj[UART_TTL].sTxing = 0;
		if(UartGetTxFifoLength(UART_TTL) > 0)
		{
			UartStartTx(UART_TTL);
		}
	}
	//校验错误
	if(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_PE))
    {
		__HAL_UART_CLEAR_PEFLAG(&UartHandle);
		UartDevObj[UART_TTL].sError = 1;
	}
	//接收到一个字节数据
	if(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE))
	{
		UartDevObj[UART_TTL].sRxing = 1;
		ch = (uint8_t)USART2->DR;
		UartWriteFromFifo(UART_TTL, RXFIFO, ch);
	}
	//接收完成
	if(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_IDLE))
    {
		__HAL_UART_CLEAR_IDLEFLAG(&UartHandle);
		
		if(UartDevObj[UART_TTL].sError)
		{
			UartDevObj[UART_TTL].RxFifo.RxIndex = UartDevObj[UART_TTL].RxFifo.TxIndex;
			UartDevObj[UART_TTL].sError = 0;
		}
		else
		{
			if(UartDevObj[UART_TTL].RxCallback)
			{
				UartDevObj[UART_TTL].RxCallback(UartGetRxFifoLength(UART_TTL));
			}
		}
		UartDevObj[UART_TTL].sRxing = 0;
	}
}

