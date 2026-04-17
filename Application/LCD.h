#ifndef __LCD_H__
#define __LCD_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "py32f0xx_hal.h"

#define USE_HORIZONTAL 3  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W (80)
#define LCD_H (160)

#else
#define LCD_W (160)
#define LCD_H (80)
#endif

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F	//品红
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF	//青色
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
#define Orange		 	 0XFD20 //橙
#define ForestGreen      0X2444	//森林绿
#define DeepSkyBlue      0X05FF //深天蓝
#define SalviaBlue	 	 0X4C1C //鼠尾草蓝
#define Azure   		 0X03FF //湛蓝
#define MediumSlateBlue  0x7B5D	//中岩蓝
#define DarkTurquoise    0x067A	//暗绿松石
#define Violet			 0x780F //紫色

#define ABLUE            0X2496 //自定义蓝色//0X867D
#define BBLUE            0X9619 //自定义浅蓝色
#define AWHITE           0XEF5D //自定义白色
#define AGREEN           0X0640 //自定义绿色
#define ORANGE			 0xFC60 //自定义橙色

void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);//指定区域填充颜色
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);//在指定位置画一条线
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);//在指定位置画一个矩形
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);//在指定位置画一个圆
void LCD_Draw_Filled_Circle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color);//在指定位置画一个填充圆
void LCD_Draw_Rounded_Rectangle(uint8_t x0, uint8_t y0, uint8_t a, uint8_t b, uint8_t r, uint16_t color);//画圆角矩形
void LCD_DrawEllipse(uint8_t x0, uint8_t y0, uint8_t a, uint8_t b, uint16_t color);//画标准椭圆
void LCD_Draw_Filled_Ellipse(uint8_t x0, uint8_t y0, uint8_t a, uint8_t b, uint16_t color);//画标准填充椭圆

void LCD_ShowChinese(uint16_t x,uint16_t y,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode,uint8_t pos);//显示一个汉字
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode);//显示一个字符
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode);//显示字符串
void LCD_ShowIntNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode);//显示整数变量

void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[]);//显示图片

//void LCD_BackLightCtrl(uint8_t OnOff);
void LCD_BackLightCtrl(void);
void LCD_Init(void);


#ifdef __cplusplus
}
#endif

#endif

