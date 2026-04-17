#include "LCD.h"
#include "LCD_Font.h"
//#include "GPIO_Config.h"
//#include "LCD_Pic.h"

#define PI ((double)3.141593)

#define LCD_DC_SET (GPIOF->BSRR = GPIO_PIN_5)
#define LCD_DC_CLEAR (GPIOF->BRR = GPIO_PIN_5)

#define LCD_CS_SET (GPIOA->BSRR = GPIO_PIN_12)
#define LCD_CS_CLEAR (GPIOA->BRR = GPIO_PIN_12)

#define LCD_RES_SET (GPIOA->BSRR = GPIO_PIN_8)
#define LCD_RES_CLEAR (GPIOA->BRR = GPIO_PIN_8)

#define LCD_SCK_SET (GPIOA->BSRR = GPIO_PIN_11)
#define LCD_SCK_CLEAR (GPIOA->BRR = GPIO_PIN_11)

#define LCD_SDA_SET	(GPIOA->BSRR = GPIO_PIN_10)
#define LCD_SDA_CLEAR (GPIOA->BRR = GPIO_PIN_10)
#define LCD_SDA(n) (n ? (GPIOA->BSRR = GPIO_PIN_10) : (GPIOA->BRR = GPIO_PIN_10))

//DIV_CalculatedTypeDef Calculatervalue;
//DIV_HandleTypeDef Divhandle;


/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void LCD_Delay(uint32_t dat)
{         
	uint8_t i = 0;
	
	while(dat)
	{
		i = 72;
		while(i)
		{
			__NOP();
			i--;
		}
		dat--;
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void LCD_Init_WR_DATA8(uint8_t dat)
{
	uint8_t i;
	LCD_CS_CLEAR;
	LCD_Delay(5);
	for(i=0;i<8;i++)
	{			  
		LCD_SCK_CLEAR;
		LCD_Delay(2);
		if(dat&0x80)
		{
			LCD_SDA_SET;
		}
		else
		{
			LCD_SDA_CLEAR;
		}
		LCD_Delay(2);
		LCD_SCK_SET;
		LCD_Delay(2);
		dat<<=1;
	}
	LCD_CS_SET;
	LCD_Delay(5);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void LCD_Init_WR_Comm(uint8_t comm)
{
	LCD_DC_CLEAR;//写命令
	LCD_Delay(1);
	LCD_Init_WR_DATA8(comm);
	LCD_DC_SET;//写数据
	LCD_Delay(1);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void LCD_WR_DATA8(uint8_t dat) 
{	
//	uint8_t i;
//	LCD_CS_CLEAR;
//	for(i=0;i<8;i++)
//	{			  
//		LCD_SCK_CLEAR;
//		if(dat&0x80)
//		{
//			LCD_SDA_SET;
//		}
//		else
//		{
//			LCD_SDA_CLEAR;
//		}
//		LCD_SCK_SET;
//		dat<<=1;
//	}
//	LCD_CS_SET;
	
	LCD_CS_CLEAR;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x80);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x40);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x20);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x10);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x08);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x04);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x02);
	LCD_SCK_SET;
	__NOP();
	
	LCD_SCK_CLEAR;
	LCD_SDA(dat&0x01);
	LCD_SCK_SET;
	
	__NOP();
	LCD_CS_SET;
	__NOP();
	
//	LCD_SDA_SET;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void LCD_WR_DATA16(uint16_t dat)
{
	LCD_WR_DATA8(dat>>8);
	LCD_WR_DATA8(dat);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void LCD_WR_Comm(uint8_t comm)
{
	LCD_DC_CLEAR;//写命令
//	LCD_Delay(1);
	LCD_WR_DATA8(comm);
	LCD_DC_SET;//写数据
//	LCD_Delay(1);
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
static void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#if USE_HORIZONTAL == 0
		LCD_WR_Comm(0x2a);//列地址设置
		LCD_WR_DATA16(x1+26);
		LCD_WR_DATA16(x2+26);
		LCD_WR_Comm(0x2b);//行地址设置
		LCD_WR_DATA16(y1+1);
		LCD_WR_DATA16(y2+1);
		LCD_WR_Comm(0x2c);//储存器写
#elif USE_HORIZONTAL == 1
		LCD_WR_Comm(0x2a);//列地址设置
		LCD_WR_DATA16(x1+26);
		LCD_WR_DATA16(x2+26);
		LCD_WR_Comm(0x2b);//行地址设置
		LCD_WR_DATA16(y1+1);
		LCD_WR_DATA16(y2+1);
		LCD_WR_Comm(0x2c);//储存器写
#elif USE_HORIZONTAL == 2
		LCD_WR_Comm(0x2a);//列地址设置
		LCD_WR_DATA16(x1+1);
		LCD_WR_DATA16(x2+1);
		LCD_WR_Comm(0x2b);//行地址设置
		LCD_WR_DATA16(y1+26);
		LCD_WR_DATA16(y2+26);
		LCD_WR_Comm(0x2c);//储存器写
#else
		LCD_WR_Comm(0x2a);//列地址设置
		LCD_WR_DATA16(x1+1);
		LCD_WR_DATA16(x2+1);
		LCD_WR_Comm(0x2b);//行地址设置
		LCD_WR_DATA16(y1+26);
		LCD_WR_DATA16(y2+26);
		LCD_WR_Comm(0x2c);//储存器写
#endif
}

/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
static void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA16(color);
}

/******************************************************************************
      函数说明：计算次幂
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
static uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}

/**
 * @brief 以某一点为中心绘制8个点，这八个点和中心点之间的距离为 (dx, dy)
 * @details 偏移距离是想读与一区域上的点而言的
 * @param x0 对称中心的横坐标
 * @param y0 对称中心的纵坐标
 * @param x 8个端点中的一个位于一区域上的点的横坐标
 * @param y 8个端点中的一个位于一区域上的点的纵坐标
 * @param dx 偏移距离的横轴分量
 * @param dy 偏移距离的纵轴分量
 * @param color 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
static void LCD_Draw_8_Pixels_Spread_Out_From_Center(uint8_t x0, uint8_t y0, uint8_t x, uint8_t y, uint8_t dx, uint8_t dy, uint16_t color)
{
	LCD_DrawPoint(x		+dx,	y			+dy,	color);		/* 一 */
	LCD_DrawPoint(y+x0-y0	+dx,	x-x0+y0		+dy,	color);		/* 二 */
	LCD_DrawPoint(y+x0-y0	+dx,	-x+x0+y0	-dy,	color);		/* 三 */
	LCD_DrawPoint(x		+dx,	-y+y0+y0	-dy,	color);		/* 四 */
	LCD_DrawPoint(-x+x0+x0	-dx,	-y+y0+y0	-dy,	color);		/* 五 */
	LCD_DrawPoint(-y+x0+y0	-dx,	-x+x0+y0	-dy,	color);		/* 六 */
	LCD_DrawPoint(-y+x0+y0	-dx,	x-x0+y0		+dy,	color);		/* 七 */
	LCD_DrawPoint(-x+x0+x0	-dx,	y			+dy,	color);		/* 八 */
}

/**
 * @brief OLED放置 4 条镜像线，这 4 条镜像线关于中心点处的x轴对称
 * @details 一区域中的点的特点是：dy>dx>0
 * @param x0 8个点的中心的横坐标
 * @param y0 8个点的中心的纵坐标
 * @param x 8个端点中的一个位于一区域上的点的横坐标
 * @param y 8个端点中的一个位于一区域上的点的纵坐标
 * @param mode 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
static void LCD_Draw_8_Pixels_Lines(uint8_t x0, uint8_t y0, uint8_t x, uint8_t y, uint16_t color)
{
	LCD_DrawLine(2*x0-x,	y,			x,			y,			color);	/* x轴上方的线 */
	LCD_DrawLine(-y+x0+y0,	x-x0+y0,	y+x0-y0,	x-x0+y0,	color);
	LCD_DrawLine(-y+x0+y0,	-x+x0+y0,	y+x0-y0,	-x+x0+y0,	color);	/* x轴下方的线 */
	LCD_DrawLine(2*x0-x,	2*y0-y,		x,			2*y0-y,		color);
}

/**
 * @brief OLED放置 4 个像素，以中心点为坐标原点
 * @param x0 4个点的中心的横坐标
 * @param y0 4个点的中心的纵坐标
 * @param x 4个点中的一个位于第一象限上的点的横坐标
 * @param y 4个点中的一个位于第一象限上的点的纵坐标
 * @param mode 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
void LCD_Draw_4_Pixels(uint8_t x0, uint8_t y0, uint8_t x, uint8_t y, uint16_t color)
{
	LCD_DrawPoint(x,		y,			color);				/* 第一象限 */
	LCD_DrawPoint(x,		2*y0-y,		color);				/* 第二象限 */
	LCD_DrawPoint(2*x0-x,	2*y0-y,		color);				/* 第三象限 */
	LCD_DrawPoint(2*x0-x,	y,			color);				/* 第四象限 */
}

/**
 * @brief OLED放置 2 条镜像线，这两条镜像线关于中心点处的x轴对称
 * @param x0 4个点的中心的横坐标
 * @param y0 4个点的中心的纵坐标
 * @param x 4个端点中的一个位于第一象限上的点的横坐标
 * @param y 4个端点中的一个位于第一象限上的点的纵坐标
 * @param mode 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
void LCD_Draw_4_Pixels_Lines(uint8_t x0, uint8_t y0, uint8_t x, uint8_t y, uint16_t color)
{
	LCD_DrawLine(2*x0-x,	y,			x,			y,			color);	/* x轴上方的线 */
	LCD_DrawLine(2*x0-x,	2*y0-y,		x,			2*y0-y,		color);	/* x轴下方的线 */
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA16(color);
		}
	} 					  	    
} 

/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	int16_t dx = x2 - x1, dy = y2 - y1;
	int16_t xerr=0,yerr=0;
	uint8_t Point_Position_Inverse_Flag = 0X00;  // 0,1,2,3,4bit分别表示第一、二、三、四象限及关于 y=x 对称标识
	
	if(dy==0)			// k==0
	{
		if(dx<0)
		{
			dx = -dx;
			x1 ^= x2; x2 ^= x1; x1 ^= x2;
		}
		for(; xerr<=dx; xerr++)
			LCD_DrawPoint(xerr+x1, yerr+y1, color);
	}
	else if(dx==0)			// k不存在
	{
		if(dy<0)
		{
			dy = -dy;
			y1 ^= y2; y2 ^= y1; y1 ^= y2;
		}
		for(; yerr<=dy; yerr++)
			LCD_DrawPoint(xerr+x1, yerr+y1, color);
	}
	else
	{
		//将 (x1, y1) 和 (x2, y2) 处理成第一象限中的情况
		//第三象限转换之后可以看成本来就是在第一象限中
		if(dx>0 && dy>0) Point_Position_Inverse_Flag |= 0X01;
		else if(dx>0 && dy <0)				//处理第二象限
		{
			Point_Position_Inverse_Flag |= 0X02;
			dy = -dy;
			y2 = 2 * y1 - y2;
		}
		else if(dx<0 && dy < 0)			//处理第三象限
		{
			Point_Position_Inverse_Flag |= 0X04;
			dx = -dx; dy = -dy;
			x1 ^= x2; x2 ^= x1; x1 ^= x2;
			y1 ^= y2; y2 ^= y1; y1 ^= y2;
		}
		else if(dx<0 && dy >0)		//处理第四象限
		{
			Point_Position_Inverse_Flag |= 0X08;
			dx = -dx;
			x2 = 2 * x1 - x2;
		}
		//将 (x1, y1) 和 (x2, y2) 处理成 0<k<=1 的情形（做变换 { x0'=x1, y0'=y1 }）
		if(dx<dy)
		{
			Point_Position_Inverse_Flag |= 0X10;
			uint8_t temp_x,temp_y;
			temp_x = x2;	temp_y = y2;
			x2 = x1 - y1 + temp_y;
			y2 = -x1 + y1 + temp_x;
			dx = x2 - x1; dy = y2 - y1;  // 重新计算 dx, dy 然后再处理成第一象限中的情况
		}
		
		int16_t incrE = 2 * dy, incrNE = 2 * (dy - dx);
		int16_t d = 2 * dy - dx;
		
		LCD_DrawPoint(xerr+x1, yerr+y1, color);
		//开始计算点坐标偏移量
		for(xerr++; xerr<=dx; xerr++)
		{
			if(d<0){			//从东方衍生出新的像素
				d += incrE;
			}else{				//从东北方衍生出新的像素
				yerr++;
				d += incrNE;
			}
			
			//绘制点
			/* -- 相对于点 (x0, y0) 为原点的反函数之坐标 --
			   -- x'= x0-y0+y --
			   -- y'=-x0+y0+x -- */
			switch(Point_Position_Inverse_Flag&0X0F)
			{
				case 0X01:		//第一、三象限
				case 0X04:
					if(Point_Position_Inverse_Flag&0X10)
						LCD_DrawPoint(x1+yerr, y1+xerr, color);
					else
						LCD_DrawPoint(x1+xerr, y1+yerr, color);
					break;
				case 0X02:		//第二象限
					if(Point_Position_Inverse_Flag&0X10)
						LCD_DrawPoint(x1+yerr, y1-xerr, color);
					else
						LCD_DrawPoint(x1+xerr, y1-yerr, color);
					break;
				case 0X08:		//第四象限
					if(Point_Position_Inverse_Flag&0X10)
						LCD_DrawPoint(x1-yerr, y1+xerr, color);
					else
						LCD_DrawPoint(x1-xerr, y1+yerr, color);
					break;
				default: break;
			}
		}
	}
	
//	uint16_t t; 
//	int xerr=0,yerr=0,delta_x,delta_y,distance;
//	int incx,incy,uRow,uCol;
//	delta_x=x2-x1; //计算坐标增量 
//	delta_y=y2-y1;
//	uRow=x1;//画线起点坐标
//	uCol=y1;
//	if(delta_x>0)incx=1; //设置单步方向 
//	else if (delta_x==0)incx=0;//垂直线 
//	else {incx=-1;delta_x=-delta_x;}
//	if(delta_y>0)incy=1;
//	else if (delta_y==0)incy=0;//水平线 
//	else {incy=-1;delta_y=-delta_y;}
//	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
//	else distance=delta_y;
//	for(t=0;t<distance+1;t++)
//	{
//		LCD_DrawPoint(uRow,uCol,color);//画点
//		xerr+=delta_x;
//		yerr+=delta_y;
//		if(xerr>distance)
//		{
//			xerr-=distance;
//			uRow+=incx;
//		}
//		if(yerr>distance)
//		{
//			yerr-=distance;
//			uCol+=incy;
//		}
//	}
}

/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}

/******************************************************************************
      函数说明：画圆
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

/**
 * @brief 画填充圆
 * @method brasenham方法
 * @param x0 圆心的横坐标
 * @param y0 圆心的纵坐标
 * @param r 圆的半径
 * @param color
 * @return None
 */
void LCD_Draw_Filled_Circle(uint8_t x0, uint8_t y0, uint8_t r, uint16_t color)
{
	uint8_t x = 0, y = r;
	
	LCD_Draw_8_Pixels_Lines(x0, y0, x+x0, y+y0, color);
	int8_t d = 1 - r;
	for(x=1; x<=y; x++)
	{
		if(d<0){				/* goEast */
			d += 2 * x + 1;
		}
		else{					/* goSouthEast */
			y--;
			d += 2 * (x - y) + 1;
		}
		LCD_Draw_8_Pixels_Lines(x0, y0, x+x0, y+y0, color);
	}
}

/**
 * @brief 画标准圆角矩形，预留圆角区域，以中心点为坐标原点
 * @method 先按照矩形的画法画出一个缺角矩形，然后按照画圆的方法画出四个圆角
 * @param x0 圆角矩形中心的横坐标
 * @param y0 圆角矩形中心的纵坐标
 * @param a 圆角矩形的上下边的长
 * @param b 圆角矩形的左右边的长
 * @param r 圆角矩形的圆角的半径
 * @param mode 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
void LCD_Draw_Rounded_Rectangle(uint8_t x0, uint8_t y0, uint8_t a, uint8_t b, uint8_t r, uint16_t color)
{
	uint8_t x1,y1,x2,y2;
	
	x1 = x0-a/2; y1 = y0-b/2;
	x2 = x0+a/2; y2 = y0+b/2;
	
	//防止圆角溢出
	if(r>a/2 || r>b/2) r = (a<b)?(a/2):(b/2);
	
	//画缺角矩形
	LCD_DrawLine( x1+r,	y2,	x2-r,	y2,	color );  // 上
	LCD_DrawLine( x1+r,	y1,	x2-r,	y1,	color );  // 下
	LCD_DrawLine( x1,	y2-r,	x1,	y1+r,	color );  // 左
	LCD_DrawLine( x2,	y2-r,	x2,	y1+r,	color );  // 右
	
	//画圆角
	uint8_t x3 = 0, y3 = r;
	
	// 因为取点是一区域
	LCD_Draw_8_Pixels_Spread_Out_From_Center(x0, y0, x3+x0, y3+y0, a/2-r, b/2-r, color);
	int8_t d = 1 - r;
	for(x3=1; x3<=y3; x3++)
	{
		if(d<0){				/* goEast */
			d += 2 * x3 + 1;
		}
		else{					/* goSouthEast */
			y3--;
			d += 2 * (x3 - y3) + 1;
		}
		LCD_Draw_8_Pixels_Spread_Out_From_Center(x0, y0, x3+x0, y3+y0, a/2-r, b/2-r, color);
	}
}

/**
 * @brief 画标准椭圆
 * @method brasenham方法
 * @param x0 椭圆中心的横坐标
 * @param y0 椭圆中心的纵坐标
 * @param a 椭圆的长半轴长
 * @param b 椭圆的短半轴长
 * @param mode 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
void LCD_DrawEllipse(uint8_t x0, uint8_t y0, uint8_t a, uint8_t b, uint16_t color)
{
	/* --------------- Bresenham --------------- */
	int8_t x = 0, y = b;
	float d1 = b * b + a * a * (-b + 0.25f);
	
	LCD_Draw_4_Pixels(x0, y0, x0+x, y0+y, color);
	while (b * b * (x + 1) < a * a * (y - 0.5f))
	{
		if (d1 <= 0)
		{
			d1 += b * b * (2 * x + 3);
			x++;
		}
		else
		{
			d1 += (b * b * (2 * x + 3) + a * a * (-2 * y + 2));
			x++;
			y--;
		}
		LCD_Draw_4_Pixels(x0, y0, x0+x, y0+y, color);
	}
	float d2 = b * b * (x + 0.5f) * (x + 0.5f) + a * a * (y - 1) * (y - 1) - a * a * b * b;
	while (y > 0)
	{
		if (d2 <= 0)
		{
			d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
			x++;
			y--;
		}
		else
		{
			d2 += a * a * (-2 * y + 3);
			y--;
		}
		LCD_Draw_4_Pixels(x0, y0, x0+x, y0+y, color);
	}
	/* --------------- Bresenham --------------- */
}

/**
 * @brief 画标准填充椭圆
 * @param x0 椭圆中心的横坐标
 * @param y0 椭圆中心的纵坐标
 * @param a 椭圆的长半轴长
 * @param b 椭圆的短半轴长
 * @param mode 绘制模式，FILL，填充1；CLEAR，填充0
 * @return None
 */
void LCD_Draw_Filled_Ellipse(uint8_t x0, uint8_t y0, uint8_t a, uint8_t b, uint16_t color)
{
//	/* --------------- Bresenham --------------- */
//	int8_t x = 0, y = b;
//	float d1 = b * b + a * a * (-b + 0.25f);
//	
//	LCD_Draw_4_Pixels_Lines(x0, y0, x0+x, y0+y, color);
//	while (b * b * (x + 1) < a * a * (y - 0.5f))
//	{
//		if (d1 <= 0)
//		{
//			d1 += b * b * (2 * x + 3);
//			x++;
//		}
//		else
//		{
//			d1 += (b * b * (2 * x + 3) + a * a * (-2 * y + 2));
//			x++;
//			y--;
//		}
//		LCD_Draw_4_Pixels_Lines(x0, y0, x0+x, y0+y, color);
//	}
//	float d2 = b * b * (x + 0.5f) * (x + 0.5f) + a * a * (y - 1) * (y - 1) - a * a * b * b;
//	while (y > 0)
//	{
//		if (d2 <= 0)
//		{
//			d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
//			x++;
//			y--;
//		}
//		else
//		{
//			d2 += a * a * (-2 * y + 3);
//			y--;
//		}
//		LCD_Draw_4_Pixels_Lines(x0, y0, x0+x, y0+y, color);
//	}
//	/* --------------- Bresenham --------------- */
	
	/* --------------- Bresenham --------------- */
//	NpnOut_Ctrl(NPN2,0);
	int8_t x = 0, y = b;
	int32_t constant_a = a * a, constant_b = b * b;
	float d1 = constant_b + constant_a * (-b + 0.25f);
	
	LCD_Draw_4_Pixels_Lines(x0, y0, x0+x, y0+y, color);
	while (constant_b * (x + 1) < constant_a * (y - 0.5f))
	{
		if (d1 <= 0)
		{
			d1 += constant_b * (2 * x + 3);
			x++;
		}
		else
		{
			d1 += (constant_b * (2 * x + 3) + constant_a * (-2 * y + 2));
			x++;
			y--;
		}
		LCD_Draw_4_Pixels_Lines(x0, y0, x0+x, y0+y, color);
	}
	float d2 = constant_b * (x + 0.5f) * (x + 0.5f) + constant_a * (y - 1) * (y - 1) - constant_a * constant_b;
	while (y > 0)
	{
		if (d2 <= 0)
		{
			d2 += constant_b * (2 * x + 2) + constant_a * (-2 * y + 3);
			x++;
			y--;
		}
		else
		{
			d2 += constant_a * (-2 * y + 3);
			y--;
		}
		LCD_Draw_4_Pixels_Lines(x0, y0, x0+x, y0+y, color);
	}
	/* --------------- Bresenham --------------- */
//	NpnOut_Ctrl(NPN2,1);
}

/******************************************************************************
      函数说明：显示单个汉字
      入口数据：x,y显示坐标
                fc: 字的颜色
                bc: 字的背景色
				sizey: 字体大小
                mode:  0非叠加模式  1叠加模式
				pos:  汉字储存位置
      返回值：  无
******************************************************************************/
void LCD_ShowChinese(uint16_t x,uint16_t y,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode,uint8_t pos)
{
	uint8_t i,j,sizex,m=0;
	const uint8_t *p;
	uint16_t x0=x;
	uint16_t TypefaceNum;
	
	sizex = (sizey%8) ? 16:sizey;
	TypefaceNum = (sizex * sizey) >> 3;		//计算一个汉字所占字节大小
	
	switch(sizey)
	{
		case 12: p = &tfont12[0][0]; break;		       //调用12x12字体
		case 16: p = &tfont16[0][0]; break;		       //调用16x16字体
		case 24: p = &tfont24[0][0]; break;		       //调用24x24字体
		case 32: p = &tfont32[0][0]; break;		       //调用32x32字体
		default:return;
	}
	p += pos * TypefaceNum;	//不可越界
	
	LCD_Address_Set(x,y,x+sizey-1,y+sizey-1);
	for(i=0;i<TypefaceNum;i++)
	{
		for(j=0;j<8;j++)
		{	
			if(!mode)//非叠加方式
			{
				if(*(p+i) & (0x01<<j))LCD_WR_DATA16(fc);
				else LCD_WR_DATA16(bc);
				
				m++;
				if(m%sizey==0)
				{
					m=0;
					break;
				}
			}
			else//叠加方式
			{
				if(*(p+i) & (0x01<<j))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizey)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}

/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	if(sizey >= 64)
	{
		sizex=24;
		TypefaceNum=192;
		num=num-' ';    //得到偏移后的值
		LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置
	}
	else
	{
		sizex=sizey>>1;
		TypefaceNum=((sizex>>3)+((sizex%8)?1:0))*sizey;
		num=num-' ';    //得到偏移后的值
		LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	}
	//
	for(i=0;i<TypefaceNum;i++)
	{ 
		switch(sizey)
		{
			case 12: temp=ascii_1206[num][i]; break;		       //调用6x12字体
			case 16: temp=ascii_1608[num][i]; break;		       //调用8x16字体
			case 24: temp=ascii_2412[num][i]; break;		       //调用12x24字体
			case 32: temp=ascii_3216[num][i]; break;		       //调用16x32字体
			case 48: temp=ascii_4824[num][i]; break;		       //调用24*48字体
			case 64: temp=ascii_6424[num][i]; break;		       //调用24*64字体
			default:return;
		}
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA16(fc);
				else LCD_WR_DATA16(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}

/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey>>1;
		p++;
	}  
}

/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
				mode：1,显示数字最开始的0不显示;0,显示数字最开始的0显示
      返回值：  无
******************************************************************************/
void LCD_ShowIntNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t sizex=sizey>>1;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1)&&mode)
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[])
{
	uint16_t i,j;
	uint32_t k=0;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			LCD_WR_DATA8(pic[k*2]);
			LCD_WR_DATA8(pic[k*2+1]);
			k++;
		}
	}			
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void LCD_BackLightCtrl(void)
{
//	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
//	HAL_Delay(50);
//	GPIOA->BRR = GPIO_PIN_9;
//	HAL_Delay(20);
//	LCD_Init_WR_Comm(0x29);
//	HAL_Delay(1000);
	
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
	HAL_Delay(10);
//	GPIOA->BRR = GPIO_PIN_9;
	LCD_WR_Comm(0x29);
	HAL_Delay(10);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void LCD_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
//	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
	//PA11(SCLK),PA10(SDA),PA12(CS),PA8(RES),PA9(BLK)
	GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_8 | GPIO_PIN_9;
//	GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//PF5(DC)
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	LCD_DC_SET;
	LCD_CS_SET;
	LCD_RES_SET;
	LCD_SCK_SET;
	LCD_SDA(1);
	GPIOA->BSRR = GPIO_PIN_9;
	
	HAL_Delay(50);
	LCD_RES_CLEAR;//复位
	HAL_Delay(120);
	LCD_RES_SET;
	HAL_Delay(120);
	
	GPIOA->BRR = GPIO_PIN_9;
	HAL_Delay(200);
	
//	LCD_WR_Comm(0x11);     //Sleep out
//	HAL_Delay(200);        //Delay
//	LCD_WR_Comm(0xB1);     //Normal mode
//	LCD_WR_DATA8(0x05);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_Comm(0xB2);     //Idle mode
//	LCD_WR_DATA8(0x05);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_Comm(0xB3);     //Partial mode
//	LCD_WR_DATA8(0x05);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_DATA8(0x05);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_DATA8(0x3C);   
//	LCD_WR_Comm(0xB4);     //Dot inversion
//	LCD_WR_DATA8(0x03);   
//	LCD_WR_Comm(0xC0);     //AVDD GVDD
//	LCD_WR_DATA8(0xAB);   
//	LCD_WR_DATA8(0x0B);   
//	LCD_WR_DATA8(0x04);   
//	LCD_WR_Comm(0xC1);     //VGH VGL
//	LCD_WR_DATA8(0xC5);    //C0
//	LCD_WR_Comm(0xC2);     //Normal Mode
//	LCD_WR_DATA8(0x0D);   
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_Comm(0xC3);     //Idle
//	LCD_WR_DATA8(0x8D);   
//	LCD_WR_DATA8(0x6A);   
//	LCD_WR_Comm(0xC4);     //Partial+Full
//	LCD_WR_DATA8(0x8D);   
//	LCD_WR_DATA8(0xEE);   
//	LCD_WR_Comm(0xC5);     //VCOM
//	LCD_WR_DATA8(0x0F);   
//	LCD_WR_Comm(0xE0);     //positive gamma
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x0E);   
//	LCD_WR_DATA8(0x08);   
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x10);   
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x02);   
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x09);   
//	LCD_WR_DATA8(0x0F);   
//	LCD_WR_DATA8(0x25);   
//	LCD_WR_DATA8(0x36);   
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_DATA8(0x08);   
//	LCD_WR_DATA8(0x04);   
//	LCD_WR_DATA8(0x10);   
//	LCD_WR_Comm(0xE1);     //negative gamma
//	LCD_WR_DATA8(0x0A);   
//	LCD_WR_DATA8(0x0D);   
//	LCD_WR_DATA8(0x08);   
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x0F);   
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x02);   
//	LCD_WR_DATA8(0x07);   
//	LCD_WR_DATA8(0x09);   
//	LCD_WR_DATA8(0x0F);   
//	LCD_WR_DATA8(0x25);   
//	LCD_WR_DATA8(0x35);   
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_DATA8(0x09);   
//	LCD_WR_DATA8(0x04);   
//	LCD_WR_DATA8(0x10);	 
//	LCD_WR_Comm(0xFC);    
//	LCD_WR_DATA8(0x80);  	
//	LCD_WR_Comm(0x3A);     
//	LCD_WR_DATA8(0x05);   
//	LCD_WR_Comm(0x36);
//#if USE_HORIZONTAL == 0
//		LCD_WR_DATA8(0x08);	//竖屏翻转180度
//#elif USE_HORIZONTAL == 1
//		LCD_WR_DATA8(0xC8);	//竖屏
//#elif USE_HORIZONTAL == 2
//		LCD_WR_DATA8(0x78);	//横屏翻转180度
//#else
//		LCD_WR_DATA8(0xA8);	//横屏
//#endif   
//	LCD_WR_Comm(0x21);     //Display inversion
//	LCD_WR_Comm(0x2A);     //Set Column Address
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_DATA8(0X1A);  //26 
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_DATA8(0X69);   //105 
//	LCD_WR_Comm(0x2B);     //Set Page Address
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_DATA8(0x01);    //1
//	LCD_WR_DATA8(0x00);   
//	LCD_WR_DATA8(0xA0);    //160
//	LCD_WR_Comm(0x2C);	
	
//	LCD_Init_WR_Comm(0x11);     //Sleep out
//	HAL_Delay(200);        //Delay
//	LCD_Init_WR_Comm(0xB1);     //Normal mode
//	LCD_Init_WR_DATA8(0x05);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_Comm(0xB2);     //Idle mode
//	LCD_Init_WR_DATA8(0x05);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_Comm(0xB3);     //Partial mode
//	LCD_Init_WR_DATA8(0x05);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_DATA8(0x05);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_DATA8(0x3C);   
//	LCD_Init_WR_Comm(0xB4);     //Dot inversion
//	LCD_Init_WR_DATA8(0x03);   
//	LCD_Init_WR_Comm(0xC0);     //AVDD GVDD
//	LCD_Init_WR_DATA8(0xAB);   
//	LCD_Init_WR_DATA8(0x0B);   
//	LCD_Init_WR_DATA8(0x04);   
//	LCD_Init_WR_Comm(0xC1);     //VGH VGL
//	LCD_Init_WR_DATA8(0xC5);    //C0
//	LCD_Init_WR_Comm(0xC2);     //Normal Mode
//	LCD_Init_WR_DATA8(0x0D);   
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_Comm(0xC3);     //Idle
//	LCD_Init_WR_DATA8(0x8D);   
//	LCD_Init_WR_DATA8(0x6A);   
//	LCD_Init_WR_Comm(0xC4);     //Partial+Full
//	LCD_Init_WR_DATA8(0x8D);   
//	LCD_Init_WR_DATA8(0xEE);   
//	LCD_Init_WR_Comm(0xC5);     //VCOM
//	LCD_Init_WR_DATA8(0x0F);   
//	LCD_Init_WR_Comm(0xE0);     //positive gamma
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x0E);   
//	LCD_Init_WR_DATA8(0x08);   
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x10);   
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x02);   
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x09);   
//	LCD_Init_WR_DATA8(0x0F);   
//	LCD_Init_WR_DATA8(0x25);   
//	LCD_Init_WR_DATA8(0x36);   
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_DATA8(0x08);   
//	LCD_Init_WR_DATA8(0x04);   
//	LCD_Init_WR_DATA8(0x10);   
//	LCD_Init_WR_Comm(0xE1);     //negative gamma
//	LCD_Init_WR_DATA8(0x0A);   
//	LCD_Init_WR_DATA8(0x0D);   
//	LCD_Init_WR_DATA8(0x08);   
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x0F);   
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x02);   
//	LCD_Init_WR_DATA8(0x07);   
//	LCD_Init_WR_DATA8(0x09);   
//	LCD_Init_WR_DATA8(0x0F);   
//	LCD_Init_WR_DATA8(0x25);   
//	LCD_Init_WR_DATA8(0x35);   
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_DATA8(0x09);   
//	LCD_Init_WR_DATA8(0x04);   
//	LCD_Init_WR_DATA8(0x10);	 
//	LCD_Init_WR_Comm(0xFC);    
//	LCD_Init_WR_DATA8(0x80);  	
//	LCD_Init_WR_Comm(0x3A);     
//	LCD_Init_WR_DATA8(0x05);   
//	LCD_Init_WR_Comm(0x36);
//#if USE_HORIZONTAL == 0
//		LCD_Init_WR_DATA8(0x08);	//竖屏翻转180度
//#elif USE_HORIZONTAL == 1
//		LCD_Init_WR_DATA8(0xC8);	//竖屏
//#elif USE_HORIZONTAL == 2
//		LCD_Init_WR_DATA8(0x78);	//横屏翻转180度
//#else
//		LCD_Init_WR_DATA8(0xA8);	//横屏
//#endif   
//	LCD_Init_WR_Comm(0x21);     //Display inversion
//	LCD_Init_WR_Comm(0x2A);     //Set Column Address
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_DATA8(0X1A);  //26 
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_DATA8(0X69);   //105 
//	LCD_Init_WR_Comm(0x2B);     //Set Page Address
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_DATA8(0x01);    //1
//	LCD_Init_WR_DATA8(0x00);   
//	LCD_Init_WR_DATA8(0xA0);    //160
//	LCD_Init_WR_Comm(0x2C);
	
	LCD_Init_WR_Comm(0x11);     //Sleep out
	HAL_Delay(200);             //
	LCD_Init_WR_Comm(0xB1);     //Normal mode
	LCD_Init_WR_DATA8(0x05);   
	LCD_Init_WR_DATA8(0x3C);
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_Comm(0xB2);     //Idle mode
	LCD_Init_WR_DATA8(0x05);   
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_Comm(0xB3);     //Partial mode
	LCD_Init_WR_DATA8(0x05);   
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_DATA8(0x05);   
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_DATA8(0x3C);   
	LCD_Init_WR_Comm(0xB4);     //Dot inversion
	LCD_Init_WR_DATA8(0x03);   
	LCD_Init_WR_Comm(0xC0);     //AVDD GVDD
	LCD_Init_WR_DATA8(0xAB);   
	LCD_Init_WR_DATA8(0x0B);   
	LCD_Init_WR_DATA8(0x04);   
	LCD_Init_WR_Comm(0xC1);     //VGH VGL
	LCD_Init_WR_DATA8(0xC5);   //C0
	LCD_Init_WR_Comm(0xC2);     //Normal Mode
	LCD_Init_WR_DATA8(0x0D);   
	LCD_Init_WR_DATA8(0x00);   
	LCD_Init_WR_Comm(0xC3);     //Idle
	LCD_Init_WR_DATA8(0x8D);   
	LCD_Init_WR_DATA8(0x6A);   
	LCD_Init_WR_Comm(0xC4);     //Partial+Full
	LCD_Init_WR_DATA8(0x8D);   
	LCD_Init_WR_DATA8(0xEE);   
	LCD_Init_WR_Comm(0xC5);     //VCOM
	LCD_Init_WR_DATA8(0x0F);   
	LCD_Init_WR_Comm(0xE0);     //positive gamma
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x0E);   
	LCD_Init_WR_DATA8(0x08);   
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x10);   
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x02);   
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x09);   
	LCD_Init_WR_DATA8(0x0F);   
	LCD_Init_WR_DATA8(0x25);   
	LCD_Init_WR_DATA8(0x36);   
	LCD_Init_WR_DATA8(0x00);   
	LCD_Init_WR_DATA8(0x08);   
	LCD_Init_WR_DATA8(0x04);   
	LCD_Init_WR_DATA8(0x10);   
	LCD_Init_WR_Comm(0xE1);     //negative gamma
	LCD_Init_WR_DATA8(0x0A);   
	LCD_Init_WR_DATA8(0x0D);   
	LCD_Init_WR_DATA8(0x08);   
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x0F);   
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x02);   
	LCD_Init_WR_DATA8(0x07);   
	LCD_Init_WR_DATA8(0x09);   
	LCD_Init_WR_DATA8(0x0F);   
	LCD_Init_WR_DATA8(0x25);   
	LCD_Init_WR_DATA8(0x35);
	LCD_Init_WR_DATA8(0x00);
	LCD_Init_WR_DATA8(0x09);   
	LCD_Init_WR_DATA8(0x04);   
	LCD_Init_WR_DATA8(0x10);
		 
	LCD_Init_WR_Comm(0xFC);  
	LCD_Init_WR_DATA8(0x80); 
		
	LCD_Init_WR_Comm(0x3A);    
	LCD_Init_WR_DATA8(0x05);
	LCD_Init_WR_Comm(0x36);
#if USE_HORIZONTAL == 0
		LCD_Init_WR_DATA8(0x08);	//竖屏翻转180度
#elif USE_HORIZONTAL == 1
		LCD_Init_WR_DATA8(0xC8);	//竖屏
#elif USE_HORIZONTAL == 2
		LCD_Init_WR_DATA8(0x78);	//横屏翻转180度
#else
		LCD_Init_WR_DATA8(0xA8);	//横屏
#endif   
	LCD_Init_WR_Comm(0x21);     //Display inversion
//	LCD_Init_WR_Comm(0x29);     //Display on
	LCD_Init_WR_Comm(0x2A);     //Set Column Address
	LCD_Init_WR_DATA8(0x00);   
	LCD_Init_WR_DATA8(0x1A);  //26  
	LCD_Init_WR_DATA8(0x00);   
	LCD_Init_WR_DATA8(0x69);   //105 
	LCD_Init_WR_Comm(0x2B);     //Set Page Address
	LCD_Init_WR_DATA8(0x00);   
	LCD_Init_WR_DATA8(0x01);    //1
	LCD_Init_WR_DATA8(0x00);   
	LCD_Init_WR_DATA8(0xA0);    //160
	LCD_Init_WR_Comm(0x2C); 
	
	HAL_Delay(10);
//	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
}

