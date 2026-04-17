#include "PWM_Config.h"
#include "Flash_Config.h"
#include <math.h>

static uint16_t CurrentCalibrationVal[2] = {0,0};
static double CurrentCalcRatio = 0.00;

TIM_HandleTypeDef	TimerIniture = {0};
TIM_OC_InitTypeDef  TimerOcIniture = {0};


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void CurrentOutput_Init(void)
{
	/* 选择TIM3 */
	TimerIniture.Instance = TIM3;                                                  
	/* 自动重装载值 */
	TimerIniture.Init.Period            = 9000 - 1;	//1Khz                                
	/* 预分频系数 */
	TimerIniture.Init.Prescaler         = 8 - 1;                                 
	/* 时钟不分频 */
	TimerIniture.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;                  
	/* 向上计数*/
	TimerIniture.Init.CounterMode       = TIM_COUNTERMODE_UP;                      
	/* 不重复计数 */
	TimerIniture.Init.RepetitionCounter = 0;                                   
	/* 自动重装载寄存器没有缓冲 */
	TimerIniture.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;          
	/* 基础时钟初始化 */
	if (HAL_TIM_Base_Init(&TimerIniture) != HAL_OK)                                
	{
		while(1);
	}
	
	/* PWM模式2 */
	TimerOcIniture.OCMode       = TIM_OCMODE_PWM2;                                     
	/*OC通道输出高电平有效 */
	TimerOcIniture.OCPolarity   = TIM_OCPOLARITY_LOW;                                 
	/*输出快速使能关闭 */
	TimerOcIniture.OCFastMode   = TIM_OCFAST_DISABLE;                                  
	/*OCN通道输出高电平有效 */
	TimerOcIniture.OCNPolarity  = TIM_OCNPOLARITY_HIGH;                                
	/*空闲状态OC1N输出低电平 */
	TimerOcIniture.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	/*空闲状态OC1输出低电平*/
	TimerOcIniture.OCIdleState  = TIM_OCIDLESTATE_RESET;
	//写入比较通道的值
	TimerOcIniture.Pulse = 0;
	/* 通道1配置 */
	if (HAL_TIM_PWM_ConfigChannel(&TimerIniture, &TimerOcIniture, TIM_CHANNEL_1) != HAL_OK)
	{
		while(1);
	}
	
	HAL_TIM_PWM_Init(&TimerIniture);
	
	/* 通道1开始输出PWM */
	if (HAL_TIM_PWM_Start(&TimerIniture, TIM_CHANNEL_1) != HAL_OK)                  
	{
		while(1);
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void CurrentOutput_PwmValueSet(uint16_t val)
{
	TimerIniture.Instance->CCR1 = val;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void CurrentOutput_UpdateCalcData(void)
{
	uint16_t pwm_range = 0;
	
	CurrentCalibrationVal[0] = AppDataRead(APP_CurrentOutCalibrationVal_4MA);
	CurrentCalibrationVal[1] = AppDataRead(APP_CurrentOutCalibrationVal_20MA);
	pwm_range = CurrentCalibrationVal[1] - CurrentCalibrationVal[0];
	CurrentCalcRatio = (double)pwm_range/16000;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void CurrentOutput_UA(uint16_t ua_val)
{
	uint16_t freq_val = 0;
	double duty_cycle = 0.00;
	
	ua_val -= 4000;
	duty_cycle = CurrentCalcRatio * ua_val + CurrentCalibrationVal[0];
	freq_val = round(duty_cycle);
	
	TimerIniture.Instance->CCR1 = freq_val;
}

