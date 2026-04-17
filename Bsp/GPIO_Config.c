#include "GPIO_Config.h"


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void GPIO_BSP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	//  NPN1 PA1  NPN2 PA0
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);
	
	//  LED1 PB8  LED2 PB9 
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);
	
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void LED_Ctrl(LED_Dev Dev, uint8_t OnOff)
{
	switch(Dev)
	{
		case LED1:
			if(OnOff)
				GPIOB->BRR = (uint32_t)GPIO_PIN_8;
			else
				GPIOB->BSRR = (uint32_t)GPIO_PIN_8;
		break;
		
		case LED2:
			if(OnOff)
				GPIOB->BRR = (uint32_t)GPIO_PIN_9;
			else
				GPIOB->BSRR = (uint32_t)GPIO_PIN_9;
		break;
		
		default:
			break;
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void NpnOut_Ctrl(NpnOut_Dev Dev, uint8_t OnOff)
{
	switch(Dev)
	{
		case NPN1:
			if(OnOff)
				GPIOA->BRR = (uint32_t)GPIO_PIN_1;
			else
				GPIOA->BSRR = (uint32_t)GPIO_PIN_1;
		break;
		
		case NPN2:
			if(OnOff)
				GPIOA->BRR = (uint32_t)GPIO_PIN_0;
			else
				GPIOA->BSRR = (uint32_t)GPIO_PIN_0;
		break;
		
		default:
			break;
	}
}

