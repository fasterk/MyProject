#ifndef __APP_JM1203_H__
#define __APP_JM1203_H__

#include "py32f0xx_hal.h"

#define Start_NOM   0xA8 		//正常模式
#define Start_CM 	0xA9		//命令模式

#define	BM_OTP 		0xA0			//AFE 的设置按地址 0x14 的 OTP 的内容进行设置
#define	BM_ADD 		0xA1			//AFE 的设置按 ssss 的内容设置，ssss 的内容和格式参见地址 0x14 的 OTP
#define	AZBM_OTP 	0xA2			//经过 Auto-Zero 校正过的电桥测量   AFE 的设置按地址 0x14 的 OTP 的内容进行设置
#define	AZBM_ADD 	0xA3			//经过 Auto-Zero 校正过的电桥测量   AFE 的设置按 ssss 的内容设置，ssss 的内容和格式参见地址 0x14 的 OTP
#define	TM_OTP 		0xA4		
#define	TM_ADD 		0xA5	
#define	AZTM_OTP 	0xA6	
#define	AZTM_ADD 	0xA7	

//bit15-bit8
#define Temp_OverSample_Rate_8	(0 << 6)	//0
#define Temp_OverSample_Rate_16	(1 << 6)	//64
#define Temp_OverSample_Rate_32	(2 << 6)	//128
#define Temp_OverSample_Rate_64	(3 << 6)	//192

#define Bridge_OverSample_Rate_128	    (0 << 3)	//0
#define Bridge_OverSample_Rate_64		(1 << 3)	//8
#define Bridge_OverSample_Rate_32		(2 << 3)	//16
#define Bridge_OverSample_Rate_16		(3 << 3)	//24
#define Bridge_OverSample_Rate_8		(4 << 3)	//32
#define Bridge_OverSample_Rate_4		(5 << 3)	//40
#define Bridge_OverSample_Rate_2		(6 << 3)	//48
#define Bridge_OverSample_Rate_1		(7 << 3)	//56

#define Offset1_16	(0 << 0)
#define Offset2_16	(1 << 0)
#define Offset3_16	(2 << 0)
#define Offset4_16	(3 << 0)
#define Offset5_16	(4 << 0)
#define Offset6_16	(5 << 0)
#define Offset7_16	(6 << 0)
#define Offset8_16	(7 << 0)

//bit7-bit0
#define Clock_Sample_Rate5_3	(0 << 6)	//0
#define Clock_Sample_Rate4		(1 << 6)	//64
#define Clock_Sample_Rate2		(2 << 6)	//128
#define Clock_Sample_Rate1		(3 << 6)	//192

#define Polarity_Reverse		(0 << 5)	//0
#define Polarity_Forward 		(1 << 5)	//32

#define TWO_Gain1_1		(0 << 2)	//0
#define TWO_Gain1_2		(1 << 2)	//4
#define TWO_Gain1_3		(2 << 2)	//8	
#define TWO_Gain1_4		(3 << 2)	//12
#define TWO_Gain1_5		(4 << 2)	//16
#define TWO_Gain1_6		(5 << 2)	//20
#define TWO_Gain1_7		(6 << 2)	//24
#define TWO_Gain1_8		(7 << 2)	//28

#define ONE_Gain12		(0 << 0)
#define ONE_Gain20		(1 << 0)
#define ONE_Gain30		(2 << 0)
#define ONE_Gain40		(3 << 0)


uint8_t JM1203_Init(void);

uint32_t Get_Bridge_RawData(void);
uint16_t Get_Temp_RawData(void);
uint16_t Get_JM1203_Set(uint8_t i);
void Set_JM1203_Set(uint8_t i, uint16_t v);

void JM1203_Task(void);

	
#endif


