#include "APP_JM1203.h"
#include "Timer_Config.h"
#include "Flash_Config.h"
#include "Uart_Config.h"
#include <string.h>

#define Set_Num 10

#define JM1203_W (0xF0)
#define JM1203_R (0xF1)

//#define SDA_IN  do{GPIOB->MODER &= 0xFFFF3FFF;}while(0)
//#define SDA_OUT do{GPIOB->MODER &= 0xFFFF3FFF; GPIOB->MODER |= (uint32_t)(1<<14);}while(0)

#define IIC_SCL(n) (n ? (GPIOB->BSRR = GPIO_PIN_6) : (GPIOB->BRR = GPIO_PIN_6))
#define IIC_SDA(n) (n ? (GPIOB->BSRR = GPIO_PIN_7) : (GPIOB->BRR = GPIO_PIN_7))	 
#define READ_SDA   (GPIOB->IDR & GPIO_PIN_7)

static uint8_t sJM1203State = 0;
static uint32_t vJM1203Tick = 0;

static uint8_t Bridge_RawBuf[4];	
static uint8_t Temp_RawBuf[4];	

static uint16_t Temp_RawData;
static uint32_t Bridge_RawData;   

static uint8_t Order_Write[1];
static uint8_t Bridge_Write[3];
static uint8_t Temp_Write[3];

static uint8_t JM1203_Set[Set_Num];


static void Delay(uint16_t dat)
{         
   	uint8_t i = 0;
   	
   	while(dat)
   	{
   	   	i = 24;
   	   	while(i)
   	   	{
   	   	   	__NOP();
   	   	   	i--;
   	   	}
   	   	dat--;
   	}
}
 
/*************************************************
 * 函数名   ：IIC_Start
 * 函数功能 ：产生IIC起始信号
 * 函数说明 ：无
 *************************************************/
static void IIC_Start(void)
{
   	IIC_SDA(1); 	   	  
   	IIC_SCL(1);
   	Delay(2);
   	IIC_SDA(0);
   	Delay(2);
   	IIC_SCL(0);
}

/*************************************************
 * 函数名   ：IIC_Stop
 * 函数功能 ：产生IIC停止信号
 * 函数说明 ：无
 *************************************************/
static void IIC_Stop(void)
{
   	IIC_SCL(0);
   	IIC_SDA(0);
   	Delay(2);
   	IIC_SCL(1); 
   	IIC_SDA(1);
   	Delay(2);      	   	   	   	   	   	   	   	
}

/*************************************************
 * 函数名   ：IIC_ACK
 * 函数功能 ：产生ACK应答信号
 * 函数说明 ：无
 *************************************************/
static void IIC_Ack(void)
{
   	IIC_SCL(0);
   	IIC_SDA(0);
   	Delay(1);
   	IIC_SCL(1);
   	Delay(1);
   	IIC_SCL(0);
}

/*************************************************
 * 函数名   ：IIC_NACK
 * 函数功能 ：不产生ACK应答信号
 * 函数说明 ：无
 *************************************************/
static void IIC_NAck(void)
{
   	IIC_SCL(0);
   	IIC_SDA(1);
   	Delay(1);
   	IIC_SCL(1);
   	Delay(1);
   	IIC_SCL(0);
}

/*************************************************
 * 函数名   ：IIC_Wait_Ack
 * 函数功能 ：等待ACK应答信号
 * 函数说明 ：返回0：接收到，1：未接收到
 *************************************************/
static uint8_t IIC_Wait_Ack(void)
{
   	uint8_t ucErrTime=0;
	
	IIC_SDA(1);
	Delay(1);
   	IIC_SCL(1);
   	Delay(1);  	 
   	while(READ_SDA)
   	{
   	   	ucErrTime++;
   	   	if(ucErrTime > 250)
   	   	{
//   	   	   	IIC_Stop();
   	   	   	return 1;
   	   	}
   	}
   	IIC_SCL(0); 	   
   	return 0;  
}

/*************************************************
 * 函数名   ：IIC_Send_Byte
 * 函数功能 ：发送一个字节数据
 * 函数说明 ：无
 *************************************************/
static void IIC_Send_Byte(uint8_t txd)
{            
    uint8_t t;  
	  	    
    IIC_SCL(0); 
    for(t=0;t<8;t++)
    {              
        IIC_SDA((txd&0x80) >> 7);
        txd <<= 1;   	  
   	   	Delay(1);
   	   	IIC_SCL(1);
   	   	Delay(1); 
   	   	IIC_SCL(0); 	
   	   	Delay(1);
    }  	 
} 

/*************************************************
 * 函数名   ：IIC_Read_Byte
 * 函数功能 ：接收一个字节数据
 * 函数说明 ：ack为1：发送ACK，为0：不发送ACK
 * 	   	   	   	返回接收到的数据
 *************************************************/
static uint8_t IIC_Read_Byte(uint8_t ack)
{
   	uint8_t i,receive=0;
	
   	IIC_SDA(1);
    for(i=0; i<8; i++)
   	{
        IIC_SCL(0); 
        Delay(1);
   	   	IIC_SCL(1);
        receive<<=1;
        if(READ_SDA)receive++;   
   	   	Delay(1); 
    }  	   	   	   	   	 
    if(ack)
        IIC_Ack();	//发送ACK
    else
        IIC_NAck(); //发送nACK
   
    return receive;
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
static uint8_t JM1203_Write_Reg(uint8_t *p, uint8_t num)
{
	uint8_t i;
	
	IIC_Start();
	IIC_Send_Byte(JM1203_W);	
	if(IIC_Wait_Ack()) 
	{
		IIC_Stop();	
		return 1;
	}
	for(i = 0; i < num; i++)
	{
		IIC_Send_Byte(p[i]);
		if(IIC_Wait_Ack()) 
		{
			IIC_Stop();	
			return 1;
		}
	}
	IIC_Stop();		
	return 0;
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
static uint8_t JM1203_Read_Reg(uint8_t *p, uint8_t num)
{
	uint8_t i;
	
	IIC_Start();
	IIC_Send_Byte(JM1203_R);	
	if(IIC_Wait_Ack()) 
	{
		IIC_Stop();		
		return 1;
	}
	for(i = 0; i < num ; i++)	
	{
		if (i < num - 1) p[i] = IIC_Read_Byte(1);
		else p[i] = IIC_Read_Byte(0);
	}
	IIC_Stop();		
	return 0;
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
uint8_t JM1203_Init(void)
{
	uint8_t err_flag = 0;
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	IIC_SDA(1); 	   	  
   	IIC_SCL(1);
	HAL_Delay(200);
	
	Order_Write[0] = JM1203_Set[0];
	err_flag = JM1203_Write_Reg(Order_Write, 1);
	
	return err_flag;
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
uint32_t Get_Bridge_RawData(void)	
{
	return Bridge_RawData;
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
uint16_t Get_Temp_RawData(void)	
{
	return Temp_RawData;
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
uint16_t Get_JM1203_Set(uint8_t i)	
{
	if (i >= Set_Num)	return 0;
	return JM1203_Set[i];	
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
void Set_JM1203_Set(uint8_t i, uint16_t v)
{
	if (i >= Set_Num)	return ;	
	JM1203_Set[i] = v;
	Set_Calib_WriteFlag(1);
}

/*************************************************
 * 函数名   ：
 * 函数功能 ：
 * 函数说明 ：
 *************************************************/
void JM1203_Task(void)
{
	switch(sJM1203State)
	{
		case 0:
			Bridge_Write[0] = JM1203_Set[1];
			Bridge_Write[1] = JM1203_Set[3] | JM1203_Set[4] | JM1203_Set[5];
			Bridge_Write[2] = JM1203_Set[6] | JM1203_Set[7] | JM1203_Set[8] | JM1203_Set[9];		
			JM1203_Write_Reg(Bridge_Write, 3); 
			sJM1203State = 1;
			vJM1203Tick = GetSystemTick();
		break;
		//
		case 1:
			if(GetSystemTick() - vJM1203Tick > 10)
			{
				sJM1203State = 2;
				vJM1203Tick = GetSystemTick();
			}
		break;
		//
		case 2:
			JM1203_Read_Reg(Bridge_RawBuf, 1);
			if ((Bridge_RawBuf[0] & 0x20) == 0x00)	//空闲
			{
				sJM1203State = 3;
			}
		break;
		//
		case 3:
			JM1203_Read_Reg(Bridge_RawBuf, 4);
			Bridge_RawData = (Bridge_RawBuf[1] << 16) 
					| (Bridge_RawBuf[2] << 8) 
					| (Bridge_RawBuf[3]);
			
			sJM1203State = 4;
		break;
		//
		case 4:
			Temp_Write[0] = JM1203_Set[2];
			Temp_Write[1] = 0x00;
			Temp_Write[2] = 0x00;		
			JM1203_Write_Reg(Temp_Write, 3);    
			sJM1203State = 5;
			vJM1203Tick = GetSystemTick();
		break;
		//
		case 5:
			if(GetSystemTick() - vJM1203Tick > 10)
			{
				sJM1203State = 6;
				vJM1203Tick = GetSystemTick();
			}
		break;
		//
		case 6:
			JM1203_Read_Reg(Temp_RawBuf, 1);
			if ((Temp_RawBuf[0] & 0x20) == 0x00)	//空闲
			{
				sJM1203State = 7;
			}
		break;
		//
		case 7:
			JM1203_Read_Reg(Temp_RawBuf, 4);
			Temp_RawData = (Temp_RawBuf[1] << 8) | (Temp_RawBuf[2]);
			sJM1203State = 8;
			vJM1203Tick = GetSystemTick();
		break;
		//
		case 8:
			if(GetSystemTick() - vJM1203Tick > 200)
			{
				sJM1203State = 0;
				vJM1203Tick = GetSystemTick();
			}
		break;
	}
}


