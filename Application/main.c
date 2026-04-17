#include "py32f0xx_hal.h"
#include "Timer_Config.h"
#include "ADC_Config.h"
#include "App_System.h"
#include "App_ExternalOutput.h"
#include "APP_JM1203.h"
#include "APP_Sensor.h"
#include "Flash_Config.h"
#include "ModbusRTU.h"
#include "MeterInterface.h"
#include "PWM_Config.h"

#if DEBUG == 0
#define IWDG_Start (1)
#else
#define IWDG_Start (0)
#endif


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void APP_SystemClockConfig(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInit = {0};

	/* Configure clock source: HSE/HSI/LSE/LSI */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    /* Enable HSI */
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    /* HSI not divided */
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                           /* Configure HSI output clock as 16MHz */
//	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                   /* Disable HSE */
	RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                               /* HSE crystal frequency range 16M~32M */
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                                   /* Disable LSI */
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                                   /* Disable LSE */
	RCC_OscInitStruct.LSEDriver = RCC_ECSCR_LSE_DRIVER_1;                                       /* Default LSE  drive capability */
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                                                /* Enable PLL */
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;                                        /* Select PLL source as HSI */
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;                                                /* PLL multiplication factor set to 2 (output frequency is doubled) */
	
	/* Initialize RCC oscillator */
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		while(1);
	}
	
	//72Mhz
	/*Initialize CPU, AHB, and APB bus clocks*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* RCC system clock types */
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;                                      /* SYSCLK source selection as PLL */
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                             /* AHB clock not divided */
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                              /* APB clock not divided */
	/* Initialize RCC system clock */
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		while(1);
	}
	
	//ADC时钟配置位12M
	RCC_PeriphCLKInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	RCC_PeriphCLKInit.ADCClockSelection    = RCC_ADCCLKSOURCE_PCLK_DIV6;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInit);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
#if IWDG_Start == 1
void IwdgInit(void)
{
	IWDG_HandleTypeDef IwdgHandle;
	
	/* Enable LSI clock */
	__HAL_RCC_LSI_ENABLE();
	/* Wait until LSI READY bit is set */
	while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U);

	IwdgHandle.Instance = IWDG;                     /* Select IWDG */
	IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;  /* Configure prescaler as 32 */
	IwdgHandle.Init.Reload = (2000);                /* IWDG counter reload value is 1000, 1s */
	/* Initialize IWDG */
	if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)       
	{
		while(1);
	}
}
#endif

int main(void)
{
//	u8 i;
//	__disable_irq();
	HAL_Init();
	APP_SystemClockConfig();
	SystemCoreClockUpdate();
	System_DeviceInit();
	
	MobudRtuInit();
	JM1203_Init();
	
	
	#if IWDG_Start == 1
	IwdgInit();
	#endif
//	__enable_irq();
	
	while (1)
	{
		//主界面相关任务
		SystemTask();
		//ADC扫描采样任务
		ADC_ScanTask();
		//外部输出触发扫描
		ExternalOutputScanTask();
		//定时器任务处理
		TimerTaskHandle();
		
		JM1203_Task();
		CalibDataWritePoll_Task();
		Sensor_Task();
		ModbusRTU_Task();
		
		#if IWDG_Start == 1
		IWDG->KR = IWDG_KEY_RELOAD;
		#endif
	}
}

