#include "SPI_Config.h"


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void SPI_Init(void)
{
	SPI_HandleTypeDef Spi1Handle = {0};

	/*De-Initialize the SPI peripheral*/
	Spi1Handle.Instance               = SPI1;                       /* SPI1 */
	Spi1Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	Spi1Handle.Init.Direction         = SPI_DIRECTION_1LINE;
	Spi1Handle.Init.CLKPolarity       = SPI_POLARITY_LOW;
	Spi1Handle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	Spi1Handle.Init.DataSize          = SPI_DATASIZE_8BIT;
	Spi1Handle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	Spi1Handle.Init.NSS               = SPI_NSS_HARD_OUTPUT;
	Spi1Handle.Init.Mode = SPI_MODE_MASTER;                       
	Spi1Handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	SPI_1LINE_TX(&Spi1Handle);
	
	/* Initialize SPI peripheral */
	if (HAL_SPI_Init(&Spi1Handle) != HAL_OK)
	{
		while(1);
	}
}
