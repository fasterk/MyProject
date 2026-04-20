#include "MeterInterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Timer_Config.h"
#include "GPIO_Config.h"
#include "Flash_Config.h"
#include "ADC_Config.h"
#include "PWM_Config.h"
#include "App_Key.h"
#include "App_System.h"
#include "APP_ParamInput.h"
#include "APP_Sensor.h"

#define SoftwareVer	"V1.2.4"	//软件版本
#define HardwareVer	"V1.1.0"	//硬件版本

#define MenuItemHanziFontSize (24)	//菜单条目汉字字体大小
//#define Menu_ItemDisplay_Max (2)	//每个菜单当前最大显示条目

#define MenuTaskIdleExitTick (12000)	//菜单任务空闲退出Tick
#define FunctionSetParamDisTick (1000)	//回调函数设置参数结果显示Tick

extern TypeParam_Config Param_Config;
extern uint32_t RangeSet_Flag;

typedef struct
{
	uint8_t dis_start_pos;		//显示起始的横坐标
	uint8_t chr_size;			//显示字符的大小
	uint8_t font_size;			//显示其它字体的大小
	uint8_t *str;				//显示的字符串
	uint8_t font_pos[8];		//其它字库储存位置
}LCD_ShowContentTypeDef;

struct MenuItem
{
	uint8_t MenuNum;						//菜单条目数
	uint8_t MenuID;							//当前目录ID
	void (*Function)(void *param);			//条目对应的功能函数
	struct MenuItem *ChildrenMenu_t;		//条目的子菜单
	struct MenuItem *ParentMenu_t;			//条目的父菜单
	LCD_ShowContentTypeDef PresentContent[LanguageMax];	//显示的内容
};

typedef struct
{
	uint16_t CatalogMiddleCol;	//中间目录颜色
	uint16_t BackCol;	//背景色
	uint16_t ItemCol;	//条目色
	uint16_t FontCol;	//字体颜色
	uint16_t ParamSetCatalogCol;		//参数设置目录颜色
	uint16_t ParamSetCatalogFontCol;	//参数设置目录字体颜色
	uint16_t ParamSetFontCol;			//参数设置字体颜色
	uint16_t ParamSetBackCol;			//参数设置背景颜色
}MenuInterfaceColourTypeDef;

typedef struct
{
	uint8_t Catalog_W;		//目录宽度
	uint8_t ItemAreaRefEnd;	//条目显示区域刷屏结束坐标
	uint8_t ItemFrameStart;	//条目框显示起始坐标
	uint8_t ItemFrame_W;	//条目框宽度
	uint8_t ItemFontStart_Y;	//条目字体起始纵坐标
}MenuDisplayCoordTypeDef;

//回调函数共享变量定义
typedef struct
{
	uint8_t State;
	uint8_t Count;
	uint8_t DataBuf[2];
	uint16_t TempVal_U16[18];
	int16_t TempVal_S16[18];
	uint32_t TaskTick;
	int32_t TempVal_S32[2];
	uint32_t TempVal_U32[2];
	double TempVal_LF[2];
	ParamSetTaskFormatTypeDef ParamSetObj;
}TaskShareDataTypeDef;

/*******不同菜单下目录定义*******/
//主菜单子目录
struct MenuItem ParamSetMenu[7];		//参数设置(量程设置，1通道设置，2通道设置，单位设置，零点校准，零点复位，退出)
struct MenuItem ParamSetMenu2[6];
#if DEBUG == 1
struct MenuItem SystemSetMenu[10];		//系统设置(语言设置，背光设置，输出测试，背景颜色，字体颜色，恢复出厂，电压校准，电压补偿，版本信息，退出)
#else
struct MenuItem SystemSetMenu[7];		//系统设置(语言设置，背光设置，输出测试，电流校准，字体颜色，恢复出厂，版本信息，退出)
#endif
//参数设置菜单下子目录
struct MenuItem ParamSetOut1[8];		//通道1参数设置(模式，阈值，峰值，谷值，迟滞值，响应时间，输出方式，退出)
struct MenuItem ParamSetOut2[8];		//通道2参数设置
struct MenuItem ParamSetUnit[3];		//显示单位设置(Mpa,Kpa,退出)
struct MenuItem ParamSetRange[11];		//量程设置(36个量程,退出)---by zengxing20260306
//参数设置菜单下通道1参数设置
struct MenuItem ParamSetOut1Mode[4];	//模式设置(简易模式，迟滞模式，窗口模式，退出)
struct MenuItem ParamSetOut1Way[3];		//输出方式设置(正向输出，反向输出，退出)
//参数设置菜单下通道2参数设置
struct MenuItem ParamSetOut2Mode[4];	//模式设置(简易模式，迟滞模式，窗口模式，退出)
struct MenuItem ParamSetOut2Way[3];		//输出方式设置(正向输出，反向输出，退出)
//系统设置菜单下子目录
struct MenuItem SystemSetLanguage[3];		//语言设置(中文，English，退出)
struct MenuItem SystemSetOutputTest[3];		//输出测试(1通道输出，2通道输出，退出)
struct MenuItem SystemSetOutput1Test[3];	//1通道输出(开启，关闭，退出)
struct MenuItem SystemSetOutput2Test[3];	//2通道输出(开启，关闭，退出)
//struct MenuItem SystemSetBackColour[7];		//背景颜色(黑色，白色，蓝色，橙色，黄色，绿色，退出)
struct MenuItem SystemSetFontColour[7];		//字体颜色(白色，蓝色，橙色，黄色，绿色，紫色，退出)
struct MenuItem SystemSetCurrents[3];		//电流校准设置(4mA校准，20mA校准，退出)

/*******主菜单目录执行功能函数*******/
void ParamSetMenuFuntion(void *param);		//参数设置功能函数
void SystemSetMenuFuntion(void *param);	//系统设置功能函数
void QuitMenuFuntion(void *param);			//退出菜单功能函数
/*******参数设置菜单目录执行功能函数*******/
void ParamSetOut1Funtion(void *param);		//通道1设置
void ParamSetOut2Funtion(void *param);		//通道2设置
void ParamSetUnitFuntion(void *param);		//单位设置
void ParamSetCompensationValFuntion(void *param);	//调零校准
void ParamSetZeroResetFuntion(void *param);			//零点复位
void ParamSetRangeFuntion(void *param);			//量程设置 
void ParamSetRange1Funtion(void *param);		//量程1
void ParamSetRange2Funtion(void *param);		//量程2
void ParamSetRange3Funtion(void *param);		//量程3
void ParamSetRange4Funtion(void *param);		//量程4
void ParamSetRange5Funtion(void *param);		//量程5
void ParamSetRange6Funtion(void *param);		//量程6
void ParamSetRange7Funtion(void *param);		//量程7
void ParamSetRange8Funtion(void *param);		//量程8
void ParamSetRange9Funtion(void *param);		//量程9
void ParamSetRange10Funtion(void *param);		//量程10

/*******通道1参数设置菜单目录执行功能函数*******/
void ParamSetOut1ModeFuntion(void *param);				//模式
void ParamSetOut1ThresholdValueFuntion(void *param);	//阈值
void ParamSetOut1PeakValueFuntion(void *param);		//峰值
void ParamSetOut1ValleyValueFuntion(void *param);		//谷值
void ParamSetOut1LagValFuntion(void *param);			//迟滞值
void ParamSetOut1DelayTimeFuntion(void *param);		//响应时间
void ParamSetOut1WayFuntion(void *param);				//输出方式
/*******通道2参数设置菜单目录执行功能函数*******/
void ParamSetOut2ModeFuntion(void *param);				//模式
void ParamSetOut2ThresholdValueFuntion(void *param);	//阈值
void ParamSetOut2PeakValueFuntion(void *param);		//峰值
void ParamSetOut2ValleyValueFuntion(void *param);		//谷值
void ParamSetOut2LagValFuntion(void *param);			//迟滞值
void ParamSetOut2DelayTimeFuntion(void *param);		//响应时间
void ParamSetOut2WayFuntion(void *param);				//输出方式
/*******显示单位设置菜单目录执行功能函数*******/
void ParamSetUnitMpaFuntion(void *param);		//Mpa
void ParamSetUnitKpaFuntion(void *param);		//Kpa
/*******通道1模式设置菜单目录执行功能函数*******/
void ParamSetOut1ModeOrdinaryFuntion(void *param);	//1通道设置简易模式
void ParamSetOut1ModeLagFuntion(void *param);		//1通道设置迟滞模式
void ParamSetOut1ModeWindowFuntion(void *param);	//1通道设置窗口模式
/*******通道1输出方式设置菜单目录执行功能函数*******/
void ParamSetOut1WayNoReverseFuntion(void *param);	//正向输出
void ParamSetOut1WayReverseFuntion(void *param);	//反向输出
/*******通道2模式设置菜单目录执行功能函数*******/
void ParamSetOut2ModeOrdinaryFuntion(void *param);	//2通道设置简易模式
void ParamSetOut2ModeLagFuntion(void *param);		//2通道设置迟滞模式
void ParamSetOut2ModeWindowFuntion(void *param);	//2通道设置窗口模式
/*******通道2输出方式设置菜单目录执行功能函数*******/
void ParamSetOut2WayNoReverseFuntion(void *param);	//正向输出
void ParamSetOut2WayReverseFuntion(void *param);	//反向输出
/*******系统设置菜单目录执行功能函数*******/
void SystemSetLanguageFuntion(void *param);			//语言设置
//void SystemSetBackLightSetFuntion(void *param);		//背光设置
void SystemSetOutputTestFuntion(void *param);			//输出测试
void SystemSetCurrentsFuntion(void *param);				//电流校准
//void SystemSetBackColourFuntion(void *param);			//背景颜色
void SystemSetFontColourFuntion(void *param);			//字体颜色
void SystemSetFactoryResetFuntion(void *param);		//恢复出厂设置
void SystemSetVersionFuntion(void *param);				//版本信息
#if DEBUG == 1
void SystemSetVolCalibrationFuntion(void *param);		//电压校准
void SystemSetVolCompensationFuntion(void *param);		//电压补偿
#endif
/*******语言设置菜单目录执行功能函数*******/
void SystemSetLanguageChineseFuntion(void *param);		//中文
void SystemSetLanguageEnglishFuntion(void *param);		//英文
/*******输出测试菜单目录执行功能函数*******/
void SystemSetOutput1TestOpenFuntion(void *param);		//1通道输出开启
void SystemSetOutput1TestCloseFuntion(void *param);	//1通道输出关闭
void SystemSetOutput2TestOpenFuntion(void *param);		//2通道输出开启
void SystemSetOutput2TestCloseFuntion(void *param);	//2通道输出关闭
/*******背景颜色菜单目录执行功能函数*******/
//void SystemSetBackColourBlackFuntion(void *param);		//黑色
//void SystemSetBackColourWhiteFuntion(void *param);		//白色
//void SystemSetBackColourBlueFuntion(void *param);		//蓝色
//void SystemSetBackColourOrangeFuntion(void *param);	//橙色
//void SystemSetBackColourYellowFuntion(void *param);	//黄色
//void SystemSetBackColourGreenFuntion(void *param);	//绿色
/*******字体颜色菜单目录执行功能函数*******/
//void SystemSetFontColourBlackFuntion(void *param);		//黑色
void SystemSetFontColourWhiteFuntion(void *param);		//白色
void SystemSetFontColourBlueFuntion(void *param);		//蓝色
void SystemSetFontColourOrangeFuntion(void *param);	//橙色
void SystemSetFontColourYellowFuntion(void *param);	//黄色
void SystemSetFontColourGreenFuntion(void *param);	//绿色
void SystemSetFontColourVioletFuntion(void *param);	//紫色
/*******输出测试菜单目录执行功能函数*******/
void SystemSetOutput1Funtion(void *param);	//通道1输出
void SystemSetOutput2Funtion(void *param);	//通道2输出
/*******电流校准菜单目录执行功能函数*******/
void CurrentsCalibration_4mAFuntion(void *param);	//4mA校准
void CurrentsCalibration_20mAFuntion(void *param);	//20mA校准

static void ParamSetEndDisplayPage(uint8_t sta, uint8_t Scene);
//static void MenuSetDisplayCoord(MenuDisplayCoordTypeDef *coord_obj, uint8_t language);
static void MenuExtraInformationDisplayTask(struct MenuItem *item_obj, uint8_t menu_page);
static uint8_t MenuItemFrameEndCalcuate(struct MenuItem *item_obj);
static void MenuCatalogMessagePrint(struct MenuItem *item_obj);

/**********************主目录菜单**********************/
struct MenuItem MainMenu[3] =
{
	//参数设置(Param Setting 13)
	{
		3,
		0,
		ParamSetMenuFuntion,
		ParamSetMenu,
		NULL,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{15,16,6,7,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Param Setting",{0,0,0,0,0,0,0,0}}}
	},
	//系统设置(System Setting 14)
	{
		3,
		0,
		SystemSetMenuFuntion,
		SystemSetMenu,
		NULL,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{17,18,6,7,0,0,0,0}},
		 {35,16,0,(uint8_t *)"System Setting",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit 4)
	{
		3,
		0,
		QuitMenuFuntion,
		NULL,
		NULL,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		{75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/**********************参数设置菜单**********************/
struct MenuItem ParamSetMenu[] =
{
	//量程设置(Range Set 10)
	{
		7,
		1,
		ParamSetRangeFuntion,
		ParamSetRange,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{118,119,6,7,0,0,0,0}},
		 {51,16,0,(uint8_t *)"Range Set",{0,0,0,0,0,0,0,0}}}
	},
	//通道1设置(OUT1 Set 8)
	{
		7,
		1,
		ParamSetOut1Funtion,
		ParamSetOut1,
		MainMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"OUT1\x80\x81",{6,7,0,0,0,0,0,0}},
		 {59,16,0,(uint8_t *)"OUT1 Set",{0,0,0,0,0,0,0,0}}}
	},
	//通道2设置(OUT2 Set 8)
	{
		7,
		1,
		ParamSetOut2Funtion,
		ParamSetOut2,
		MainMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"OUT2\x80\x81",{6,7,0,0,0,0,0,0}},
		 {59,16,0,(uint8_t *)"OUT2 Set",{0,0,0,0,0,0,0,0}}}
	},
	//单位设置(Unit Setting 12)
	{
		7,
		1,
		ParamSetUnitFuntion,
		ParamSetUnit,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{9,22,6,7,0,0,0,0}},
		 {43,16,0,(uint8_t *)"Unit Setting",{0,0,0,0,0,0,0,0}}}
	},
	//零点校准(Zero Calibration 16)
	{
		7,
		1,
		ParamSetCompensationValFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{104,105,65,66,0,0,0,0}},
		 {27,16,0,(uint8_t *)"Zero Calibration",{0,0,0,0,0,0,0,0}}}
	},
	//零点复位(Zero Reset 10)
	{
		7,
		1,
		ParamSetZeroResetFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{104,105,53,106,0,0,0,0}},
		 {51,16,0,(uint8_t *)"Zero Reset",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		7,
		1,
		QuitMenuFuntion,
		NULL,
		MainMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

struct MenuItem ParamSetMenu2[] =
{
	//OUT1设置(OUT1 Set 8)
	{
		6,
		1,
		ParamSetOut1Funtion,
		ParamSetOut1,
		MainMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"OUT1\x80\x81",{6,7,0,0,0,0,0,0}},
		 {59,16,0,(uint8_t *)"OUT1 Set",{0,0,0,0,0,0,0,0}}}
	},
	//OUT2设置(OUT2 Set 8)
	{
		6,
		1,
		ParamSetOut2Funtion,
		ParamSetOut2,
		MainMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"OUT2\x80\x81",{6,7,0,0,0,0,0,0}},
		 {59,16,0,(uint8_t *)"OUT2 Set",{0,0,0,0,0,0,0,0}}}
	},
	//单位设置(Unit Setting 12)
	{
		6,
		1,
		ParamSetUnitFuntion,
		ParamSetUnit,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{9,22,6,7,0,0,0,0}},
		 {43,16,0,(uint8_t *)"Unit Setting",{0,0,0,0,0,0,0,0}}}
	},
	//零点校准(Zero Calibration 16)
	{
		6,
		1,
		ParamSetCompensationValFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{104,105,65,66,0,0,0,0}},
		 {27,16,0,(uint8_t *)"Zero Calibration",{0,0,0,0,0,0,0,0}}}
	},
	//零点复位(Zero Reset 10)
	{
		6,
		1,
		ParamSetZeroResetFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{104,105,53,106,0,0,0,0}},
		 {51,16,0,(uint8_t *)"Zero Reset",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		6,
		1,
		QuitMenuFuntion,
		NULL,
		MainMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
//量程设置
struct MenuItem ParamSetRange[] =
{
	//100~-100Kpa
	{
		11,
		1,
		ParamSetRange1Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"100~-100K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"100~-100Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//0~250Kpa
	{
		11,
		1,
		ParamSetRange2Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"0~250K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"0~250Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//0~-100Kpa
	{
		11,
		1,
		ParamSetRange3Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"0~-100K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"0~-100Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//0~1Mpa
	{
		11,
		1,
		ParamSetRange4Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"0~1M",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"0~1Mpa",{0,0,0,0,0,0,0,0}}}
	},
	//0~100Kpa
	{
		11,
		1,
		ParamSetRange5Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"0~100K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"0~100Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//-100Kpa~1Mpa
	{
		11,
		1,
		ParamSetRange6Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"-100K~1M",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"-100Kpa~1Mpa",{0,0,0,0,0,0,0,0}}}
	},
	//0~-101Kpa
	{
		11,
		1,
		ParamSetRange7Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"0~-101K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"0~-101Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//-50~500Kpa
	{
		11,
		1,
		ParamSetRange8Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"-50~500K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"-50~500Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//0~500Kpa
	{
		11,
		1,
		ParamSetRange9Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"0~500K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"0~500Kpa",{0,0,0,0,0,0,0,0}}}
	},
	//101~-101Kpa
	{
		11,
		1,
		ParamSetRange10Funtion,
		NULL,
		ParamSetMenu,
		{{40,24,MenuItemHanziFontSize,(uint8_t *)"101~-101K",{0,0,0,0,0,0,0,0}},
		 {28,16,0,(uint8_t *)"101~-101Kpa",{0,0,0,0,0,0,0,0}}}
	},
//退出(Exit)
	{
		11,
		1,
		QuitMenuFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},	
};
//通道1设置
struct MenuItem ParamSetOut1[] =
{
	//模式(Mode 4)
	{
		8,
		2,
		ParamSetOut1ModeFuntion,
		ParamSetOut1Mode,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{10,11,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Mode",{0,0,0,0,0,0,0,0}}}
	},
	//阈值(Threshold Value 15)
	{
		8,
		2,
		ParamSetOut1ThresholdValueFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{3,2,0,0,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Threshold Value",{0,0,0,0,0,0,0,0}}}
	},
	//峰值(Peak Value 10)
	{
		8,
		2,
		ParamSetOut1PeakValueFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{0,2,0,0,0,0,0,0}},
		 {51,16,0,(uint8_t *)"Peak Value",{0,0,0,0,0,0,0,0}}}
	},
	//谷值(Valley Value 12)
	{
		8,
		2,
		ParamSetOut1ValleyValueFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{1,2,0,0,0,0,0,0}},
		 {43,16,0,(uint8_t *)"Valley Value",{0,0,0,0,0,0,0,0}}}
	},
	//迟滞值(Hysteresis Value 16)
	{
		8,
		2,
		ParamSetOut1LagValFuntion,
		NULL,
		ParamSetMenu,
		{{55,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82",{28,29,2,0,0,0,0,0}},
		 {27,16,0,(uint8_t *)"Hysteresis Value",{0,0,0,0,0,0,0,0}}}
	},
	//响应时间(Response Time 13)
	{
		8,
		2,
		ParamSetOut1DelayTimeFuntion,
		NULL,
		ParamSetMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{102,103,32,33,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Response Time",{0,0,0,0,0,0,0,0}}}
	},
	//输出方式(Output Mode 11)
	{
		8,
		2,
		ParamSetOut1WayFuntion,
		ParamSetOut1Way,
		ParamSetMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{34,35,36,37,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Output Mode",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit 4)
	{
		8,
		2,
		QuitMenuFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
//通道2设置
struct MenuItem ParamSetOut2[] =
{
	//模式(Mode)
	{
		8,
		3,
		ParamSetOut2ModeFuntion,
		ParamSetOut2Mode,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{10,11,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Mode",{0,0,0,0,0,0,0,0}}}
	},
	//阈值(Threshold Value)
	{
		8,
		3,
		ParamSetOut2ThresholdValueFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{3,2,0,0,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Threshold Value",{0,0,0,0,0,0,0,0}}}
	},
	//峰值(Peak Value)
	{
		8,
		3,
		ParamSetOut2PeakValueFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{0,2,0,0,0,0,0,0}},
		 {51,16,0,(uint8_t *)"Peak Value",{0,0,0,0,0,0,0,0}}}
	},
	//谷值(Valley Value)
	{
		8,
		3,
		ParamSetOut2ValleyValueFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{1,2,0,0,0,0,0,0}},
		 {43,16,0,(uint8_t *)"Valley Value",{0,0,0,0,0,0,0,0}}}
	},
	//迟滞值(Hysteresis Value)
	{
		8,
		3,
		ParamSetOut2LagValFuntion,
		NULL,
		ParamSetMenu,
		{{55,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82",{28,29,2,0,0,0,0,0}},
		 {27,16,0,(uint8_t *)"Hysteresis Value",{0,0,0,0,0,0,0,0}}}
	},
	//响应时间(Response Time 13)
	{
		8,
		3,
		ParamSetOut2DelayTimeFuntion,
		NULL,
		ParamSetMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{102,103,32,33,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Response Time",{0,0,0,0,0,0,0,0}}}
	},
	//输出方式(Output Mode)
	{
		8,
		3,
		ParamSetOut2WayFuntion,
		ParamSetOut2Way,
		ParamSetMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{34,35,36,37,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Output Mode",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		8,
		3,
		QuitMenuFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
//单位设置
struct MenuItem ParamSetUnit[] =
{
	//Mpa
	{
		3,
		4,
		ParamSetUnitMpaFuntion,
		NULL,
		ParamSetMenu,
		{{73,24,MenuItemHanziFontSize,(uint8_t *)"MPa",{0,0,0,0,0,0,0,0}},
		 {79,16,0,(uint8_t *)"MPa",{0,0,0,0,0,0,0,0}}}
	},
	//Kpa
	{
		3,
		4,
		ParamSetUnitKpaFuntion,
		NULL,
		ParamSetMenu,
		{{73,24,MenuItemHanziFontSize,(uint8_t *)"kPa",{0,0,0,0,0,0,0,0}},
		 {79,16,0,(uint8_t *)"kPa",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		4,
		QuitMenuFuntion,
		NULL,
		ParamSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/**********************通道1设置菜单**********************/
//模式
struct MenuItem ParamSetOut1Mode[] =
{
	//简易模式(Simple Mode 11)
	{
		4,
		5,
		ParamSetOut1ModeOrdinaryFuntion,
		NULL,
		ParamSetOut1,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{8,21,10,11,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Simple Mode",{0,0,0,0,0,0,0,0}}}
	},
	//迟滞模式(Hysteresis Mode 15)
	{
		4,
		5,
		ParamSetOut1ModeLagFuntion,
		NULL,
		ParamSetOut1,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{28,29,10,11,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Hysteresis Mode",{0,0,0,0,0,0,0,0}}}
	},
	//窗口模式(Window Mode 11)
	{
		4,
		5,
		ParamSetOut1ModeWindowFuntion,
		NULL,
		ParamSetOut1,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{38,39,10,11,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Window Mode",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit 4)
	{
		4,
		5,
		QuitMenuFuntion,
		NULL,
		ParamSetOut1,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
//输出方式
struct MenuItem ParamSetOut1Way[] =
{
	//正向输出(Forward Output 14) 常开(Normally Open 13)
	{
		3,
		6,
		ParamSetOut1WayNoReverseFuntion,
		NULL,
		ParamSetOut1,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{107,59,34,35,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Normally Open",{0,0,0,0,0,0,0,0}}}
	},
	//反向输出(Inverted Output 15) 常闭(Normally Close 14)
	{
		3,
		6,
		ParamSetOut1WayReverseFuntion,
		NULL,
		ParamSetOut1,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{107,101,34,35,0,0,0,0}},
		 {35,16,0,(uint8_t *)"Normally Close",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit 4)
	{
		3,
		6,
		QuitMenuFuntion,
		NULL,
		ParamSetOut1,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/**********************通道2设置菜单**********************/
struct MenuItem ParamSetOut2Mode[] =
{
	//简易模式(Simple Mode)
	{
		4,
		7,
		ParamSetOut2ModeOrdinaryFuntion,
		NULL,
		ParamSetOut2,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{8,21,10,11,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Simple Mode",{0,0,0,0,0,0,0,0}}}
	},
	//迟滞模式(Hysteresis Mode)
	{
		4,
		7,
		ParamSetOut2ModeLagFuntion,
		NULL,
		ParamSetOut2,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{28,29,10,11,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Hysteresis Mode",{0,0,0,0,0,0,0,0}}}
	},
	//窗口模式(Window Mode)
	{
		4,
		7,
		ParamSetOut2ModeWindowFuntion,
		NULL,
		ParamSetOut2,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{38,39,10,11,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Window Mode",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		4,
		7,
		QuitMenuFuntion,
		NULL,
		ParamSetOut2,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
//输出方式
struct MenuItem ParamSetOut2Way[] =
{
	//正向输出(Forward Output)
	{
		3,
		8,
		ParamSetOut2WayNoReverseFuntion,
		NULL,
		ParamSetOut2,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{107,59,34,35,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Normally Open",{0,0,0,0,0,0,0,0}}}
	},
	//反向输出(Inverted Output)
	{
		3,
		8,
		ParamSetOut2WayReverseFuntion,
		NULL,
		ParamSetOut2,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{107,101,34,35,0,0,0,0}},
		 {35,16,0,(uint8_t *)"Normally Close",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		8,
		QuitMenuFuntion,
		NULL,
		ParamSetOut2,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/**********************系统设置菜单**********************/
#if DEBUG == 1
struct MenuItem SystemSetMenu[] = 
{
	//语言设置(Language 8)
	{
		8,
		9,
		SystemSetLanguageFuntion,
		SystemSetLanguage,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{85,86,6,7,0,0,0,0}},
		 {59,16,0,(uint8_t *)"Language",{0,0,0,0,0,0,0,0}}}
	},
//	//背光控制(Backlight 9)
//	{
//		9,
//		9,
//		SystemSetBackLightSetFuntion,
//		NULL,
//		MainMenu,
//		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{43,44,45,46,0,0,0,0}},
//		 {55,16,0,(uint8_t *)"Backlight",{0,0,0,0,0,0,0,0}}}
//	},
	//输出测试(Output Test 11)
	{
		8,
		9,
		SystemSetOutputTestFuntion,
		SystemSetOutputTest,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{34,35,47,48,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Output Test",{0,0,0,0,0,0,0,0}}}
	},
//	//背景颜色(Back Colour 11)
//	{
//		10,
//		9,
//		SystemSetBackColourFuntion,
//		SystemSetBackColour,
//		MainMenu,
//		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{43,80,81,82,0,0,0,0}},
//		 {47,16,0,(uint8_t *)"Back Colour",{0,0,0,0,0,0,0,0}}}
//	},
	//字体颜色(Font Colour 11)
	{
		8,
		9,
		SystemSetFontColourFuntion,
		SystemSetFontColour,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{83,84,81,82,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Font Colour",{0,0,0,0,0,0,0,0}}}
	},
	//恢复出厂(Factory Reset 13)
	{
		8,
		9,
		SystemSetFactoryResetFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{52,53,35,54,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Factory Reset",{0,0,0,0,0,0,0,0}}}
	},
	//电压校准(Vol Calibration 15)
	{
		8,
		9,
		SystemSetVolCalibrationFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{50,51,65,66,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Vol Calibration",{0,0,0,0,0,0,0,0}}}
	},
	//电压补偿(Vol Compensation 16)
	{
		8,
		9,
		SystemSetVolCompensationFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{50,51,23,24,0,0,0,0}},
		 {27,16,0,(uint8_t *)"Vol Compensation",{0,0,0,0,0,0,0,0}}}
	},
	//版本信息(Version 7)
	{
		8,
		9,
		SystemSetVersionFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{73,74,75,76,0,0,0,0}},
		 {63,16,0,(uint8_t *)"Version",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit 4)
	{
		8,
		9,
		QuitMenuFuntion,
		NULL,
		MainMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
#else

struct MenuItem SystemSetMenu[] = 
{
	//语言设置(Language 8)
	{
		7,
		9,
		SystemSetLanguageFuntion,
		SystemSetLanguage,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{85,86,6,7,0,0,0,0}},
		 {59,16,0,(uint8_t *)"Language",{0,0,0,0,0,0,0,0}}}
	},
//	//背光控制(Backlight 9)
//	{
//		7,
//		9,
//		SystemSetBackLightSetFuntion,
//		NULL,
//		MainMenu,
//		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{43,44,45,46,0,0,0,0}},
//		 {55,16,0,(uint8_t *)"Backlight",{0,0,0,0,0,0,0,0}}}
//	},
	//输出测试(Output Test 11)
	{
		7,
		9,
		SystemSetOutputTestFuntion,
		SystemSetOutputTest,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{34,35,47,48,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Output Test",{0,0,0,0,0,0,0,0}}}
	},
	//电流校准(Currents Set 12)
	{
		7,
		9,
		SystemSetCurrentsFuntion,
		SystemSetCurrents,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{34,35,65,66,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Output Set",{0,0,0,0,0,0,0,0}}}
	},
	//字体颜色(Font Colour 11)
	{
		7,
		9,
		SystemSetFontColourFuntion,
		SystemSetFontColour,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{83,84,81,82,0,0,0,0}},
		 {47,16,0,(uint8_t *)"Font Colour",{0,0,0,0,0,0,0,0}}}
	},
	//恢复出厂(Factory Reset 13)
	{
		7,
		9,
		SystemSetFactoryResetFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{52,53,35,54,0,0,0,0}},
		 {39,16,0,(uint8_t *)"Factory Reset",{0,0,0,0,0,0,0,0}}}
	},
	//版本信息(Version 7)
	{
		7,
		9,
		SystemSetVersionFuntion,
		NULL,
		MainMenu,
		{{43,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{73,74,75,76,0,0,0,0}},
		 {63,16,0,(uint8_t *)"Version",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit 4)
	{
		7,
		9,
		QuitMenuFuntion,
		NULL,
		MainMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};
#endif

/*******语言设置菜单目录执行功能函数*******/
struct MenuItem SystemSetLanguage[] =
{
	//中文
	{
		3,
		10,
		SystemSetLanguageChineseFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{87,88,0,0,0,0,0,0}},
		 {75,0,16,(uint8_t *)"\x80\x81",{87,88,0,0,0,0,0,0}}}
	},
	//英文(English 7)
	{
		3,
		10,
		SystemSetLanguageEnglishFuntion,
		NULL,
		SystemSetMenu,
		{{49,24,MenuItemHanziFontSize,(uint8_t *)"English",{0,0,0,0,0,0,0,0}},
		 {63,16,0,(uint8_t *)"English",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		10,
		QuitMenuFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/*******输出测试菜单目录执行功能函数*******/
struct MenuItem SystemSetOutputTest[] =
{
	//通道1测试(OUT1 Test 9)
	{
		3,
		11,
		SystemSetOutput1Funtion,
		SystemSetOutput1Test,
		SystemSetMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"OUT1\x80\x81",{47,48,0,0,0,0,0,0}},
		 {55,16,0,(uint8_t *)"OUT1 Test",{0,0,0,0,0,0,0,0}}}
	},
	//通道2测试(OUT1 Test 9)
	{
		3,
		11,
		SystemSetOutput2Funtion,
		SystemSetOutput2Test,
		SystemSetMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"OUT2\x80\x81",{47,48,0,0,0,0,0,0}},
		 {55,16,0,(uint8_t *)"OUT2 Test",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		11,
		QuitMenuFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/*******通道1测试菜单目录执行功能函数*******/
struct MenuItem SystemSetOutput1Test[] =	
{
	//开启(Open 4)
	{
		3,
		12,
		SystemSetOutput1TestOpenFuntion,
		NULL,
		SystemSetOutputTest,
		{{67,24,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{59,100,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Open",{0,0,0,0,0,0,0,0}}}
	},
	//关闭(Close 5)
	{
		3,
		12,
		SystemSetOutput1TestCloseFuntion,
		NULL,
		SystemSetOutputTest,
		{{67,24,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{60,101,0,0,0,0,0,0}},
		 {71,16,0,(uint8_t *)"Close",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		12,
		QuitMenuFuntion,
		NULL,
		SystemSetOutputTest,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/*******通道2测试菜单目录执行功能函数*******/
struct MenuItem SystemSetOutput2Test[] =
{
	//开启(Open 4)
	{
		3,
		13,
		SystemSetOutput2TestOpenFuntion,
		NULL,
		SystemSetOutputTest,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{59,100,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Open",{0,0,0,0,0,0,0,0}}}
	},
	//关闭(Close 5)
	{
		3,
		13,
		SystemSetOutput2TestCloseFuntion,
		NULL,
		SystemSetOutputTest,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{60,101,0,0,0,0,0,0}},
		 {71,16,0,(uint8_t *)"Close",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		13,
		QuitMenuFuntion,
		NULL,
		SystemSetOutputTest,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

///*******背景颜色菜单目录执行功能函数*******/
//struct MenuItem SystemSetBackColour[] =
//{
//	//黑色(Black 5)
//	{
//		7,
//		13,
//		SystemSetBackColourBlackFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{89,82,0,0,0,0,0,0}},
//		 {71,16,0,(uint8_t *)"Black",{0,0,0,0,0,0,0,0}}}
//	},
//	//白色(White 5)
//	{
//		7,
//		13,
//		SystemSetBackColourWhiteFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{90,82,0,0,0,0,0,0}},
//		 {71,16,0,(uint8_t *)"White",{0,0,0,0,0,0,0,0}}}
//	},
//	//蓝色(Blue 4)
//	{
//		7,
//		13,
//		SystemSetBackColourBlueFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{91,82,0,0,0,0,0,0}},
//		 {75,16,0,(uint8_t *)"Blue",{0,0,0,0,0,0,0,0}}}
//	},
//	//橙色(Orange 6)
//	{
//		7,
//		13,
//		SystemSetBackColourOrangeFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{92,82,0,0,0,0,0,0}},
//		 {67,16,0,(uint8_t *)"Orange",{0,0,0,0,0,0,0,0}}}
//	},
//	//黄色(Yellow 6)
//	{
//		7,
//		13,
//		SystemSetBackColourYellowFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{95,82,0,0,0,0,0,0}},
//		 {67,16,0,(uint8_t *)"Yellow",{0,0,0,0,0,0,0,0}}}
//	},
//	//绿色(Green 5)
//	{
//		7,
//		13,
//		SystemSetBackColourGreenFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{93,82,0,0,0,0,0,0}},
//		 {71,16,0,(uint8_t *)"Green",{0,0,0,0,0,0,0,0}}}
//	},
//	//退出(Exit 4)
//	{
//		7,
//		13,
//		QuitMenuFuntion,
//		NULL,
//		SystemSetMenu,
//		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
//		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
//	},
//};

/*******字体颜色菜单目录执行功能函数*******/
struct MenuItem SystemSetFontColour[] =
{
	//白色(White)
	{
		7,
		14,
		SystemSetFontColourWhiteFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{90,82,0,0,0,0,0,0}},
		 {71,16,0,(uint8_t *)"White",{0,0,0,0,0,0,0,0}}}
	},
	//蓝色(Blue)
	{
		7,
		14,
		SystemSetFontColourBlueFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{91,82,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Blue",{0,0,0,0,0,0,0,0}}}
	},
	//橙色(Orange)
	{
		7,
		14,
		SystemSetFontColourOrangeFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{92,82,0,0,0,0,0,0}},
		 {67,16,0,(uint8_t *)"Orange",{0,0,0,0,0,0,0,0}}}
	},
	//黄色(Orange)
	{
		7,
		14,
		SystemSetFontColourYellowFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{95,82,0,0,0,0,0,0}},
		 {67,16,0,(uint8_t *)"Yellow",{0,0,0,0,0,0,0,0}}}
	},
	//绿色(Green 5)
	{
		7,
		14,
		SystemSetFontColourGreenFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{93,82,0,0,0,0,0,0}},
		 {71,16,0,(uint8_t *)"Green",{0,0,0,0,0,0,0,0}}}
	},
	//紫色(Violet 6)
	{
		7,
		14,
		SystemSetFontColourVioletFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{108,82,0,0,0,0,0,0}},
		 {67,16,0,(uint8_t *)"Violet",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		7,
		14,
		QuitMenuFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};

/*******电流校准菜单目录执行功能函数*******/
struct MenuItem SystemSetCurrents[] =
{
	//4mA校准(4mA Calibration 15)
	{
		3,
		15,
		CurrentsCalibration_4mAFuntion,
		NULL,
		SystemSetMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{104,105,65,66,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Zero Calibration",{0,0,0,0,0,0,0,0}}}
	},
	//20mA校准(20mA Calibration 16)
	{
		3,
		15,
		CurrentsCalibration_20mAFuntion,
		NULL,
		SystemSetMenu,
		{{43,24,MenuItemHanziFontSize,(uint8_t *)"\x80\x81\x82\x83",{117,105,65,66,0,0,0,0}},
		 {31,16,0,(uint8_t *)"Full Calibration",{0,0,0,0,0,0,0,0}}}
	},
	//退出(Exit)
	{
		3,
		15,
		QuitMenuFuntion,
		NULL,
		SystemSetMenu,
		{{67,0,MenuItemHanziFontSize,(uint8_t *)"\x80\x81",{19,20,0,0,0,0,0,0}},
		 {75,16,0,(uint8_t *)"Exit",{0,0,0,0,0,0,0,0}}}
	},
};


static volatile uint8_t sFunctionQuit = 0;				
static volatile uint8_t sFunctionExecute = 0;

static volatile uint8_t sMenuFlag = 0;	//
//bit7:菜单功能允许使用
//bit6:菜单任务空闲标志
//bit5:菜单退出Tick记时标志
//...
//bit3:外部参考电压获取完成
static volatile uint8_t sMenuExtraInfoFlag = 0;		//
static volatile uint8_t MenuLanguage = LanguageMax;	//菜单显示语言
static volatile uint8_t MenuItemDisMax = 0;			//每个菜单当前最大显示条目
static volatile uint8_t sAutoReturnLastMenu = 0;	//自动返回上一菜单标志
static volatile uint8_t sFunctionKeyShield = 0;		//菜单功能按键权限
static volatile uint8_t sMenuRefEnable = 0;			//当前菜单刷新使能
static volatile uint8_t sMenuTaskRefEnable = 1;		//菜单显示条目刷新使能
static volatile uint8_t vOldItemPos = 0;			//上一次的条目显示位置
static volatile uint8_t vOldPage = 0;				//上一次显示的页
static volatile uint8_t vManualItemNum = 0;			//当前菜单对应的条目为手动值
static volatile uint8_t vSelectItemNum = 0;			//当前菜单对应的条目
static volatile uint8_t vItemDepthCnt = 0;			//条目深度
static uint8_t vItemNumBuf[10] = {0,0,0,0,0,0,0,0,0,0};		//不同下条目缓存(最大纵深为10)
	
static uint32_t vMenuTaskIdleTick = 0;

//static volatile int32_t vMenuAssistTaskNumber = -1;		//菜单协助任务编号

//菜单对象
static struct MenuItem *gSelectMenuObj = &MainMenu[0];		
//菜单颜色相关设置
static MenuInterfaceColourTypeDef MenuInterfaceColObj = 	
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};
//static MenuInterfaceColourTypeDef MenuInterfaceColObj = 	
//{
//	LBBLUE,
//	DARKBLUE,
//	LGRAY,
//	LGRAY,
//	LGRAY,
//};
//菜单显示坐标相关设置
static MenuDisplayCoordTypeDef MenuCoordObj = 		
{
	0,
	0,
	0,
	0,
	0,
};
//条目回调函数共享变量定义
static TaskShareDataTypeDef _gTaskShareDatObj =
{
	0,
	0,
	{0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	0,
	{0,0},
	{0,0},
	{0.00,0.00},
	{0,0,0,0,0,0,0,0,0,0,0,0},
};

/*******主菜单目录执行功能函数*******/
//参数设置显示任务
void ParamSetMenuFuntion(void *param)
{
	
	if(RangeSet_Flag == 1)
	{
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//系统设置功能函数
void SystemSetMenuFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}

//退出菜单功能函数
void QuitMenuFuntion(void *param)
{
	_gTaskShareDatObj.State = 0;
	
	sFunctionExecute = 0;
	sAutoReturnLastMenu = 1;
}

/*******参数设置菜单目录执行功能函数*******/
//量程设置
void ParamSetRangeFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	switch(AppDataRead(APP_SystemRange))
	{
		case 0: vManualItemNum = 0; break;
		case 1: vManualItemNum = 1; break;
		case 2: vManualItemNum = 2; break;
		case 3: vManualItemNum = 3; break;
		case 4: vManualItemNum = 4; break;
		case 5: vManualItemNum = 5; break;
		case 6: vManualItemNum = 6; break;
		case 7: vManualItemNum = 7; break;
		case 8: vManualItemNum = 8; break;
		case 9: vManualItemNum = 9; break;
		case 10: vManualItemNum = 10; break;
		default: vManualItemNum = 0; break;
	}
}	

//量程设置range1
void ParamSetRange1Funtion(void *param)
{

	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(0, APP_SystemRange);
		APPDataFlashWrite();
			
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();
		
		AppSetRange();			
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range2
void ParamSetRange2Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(1, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();
		
		AppSetRange();		
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range3
void ParamSetRange3Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{	
		AppDataWrite(2, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range4
void ParamSetRange4Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{		
		AppDataWrite(3, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range5
void ParamSetRange5Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(4, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();	
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range6
void ParamSetRange6Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(5, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();	
		
		AppSetRange();	
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range7
void ParamSetRange7Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{	
		AppDataWrite(6, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();		
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range8
void ParamSetRange8Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(7, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();	
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range9
void ParamSetRange9Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{	
		AppDataWrite(8, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//量程设置range10
void ParamSetRange10Funtion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(9, APP_SystemRange);
		APPDataFlashWrite();
		
		AppDataWrite(1, APP_SetRangeFlag);
		APPDataFlashWrite();	
		
		AppSetRange();
		sAutoReturnLastMenu = 1;
		
		MainMenu[0].ChildrenMenu_t = ParamSetMenu2;
		ParamSetOut1[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut1[7].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[0].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[1].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[2].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[3].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[4].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[5].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[6].ParentMenu_t = ParamSetMenu2;
		ParamSetOut2[7].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[0].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[1].ParentMenu_t = ParamSetMenu2;
		ParamSetUnit[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[0].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[1].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[2].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[3].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[4].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[5].ParentMenu_t = ParamSetMenu2;		
		ParamSetRange[6].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[7].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[8].ParentMenu_t = ParamSetMenu2;
		ParamSetRange[9].ParentMenu_t = ParamSetMenu2;
	}
}
//通道1设置
void ParamSetOut1Funtion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}
//通道2设置
void ParamSetOut2Funtion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}	
//单位设置
void ParamSetUnitFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	switch(AppDataRead(APP_SystemUnit))
	{
		case SystemUnit_Mpa: vManualItemNum = 0; break;
		case SystemUnit_Kpa: vManualItemNum = 1; break;
		default: vManualItemNum = 0; break;
	}
}	
//零点校准
void ParamSetCompensationValFuntion(void *param)
{
	uint8_t i;
	int32_t AirPressure = 0;
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(55,23,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,65);
			LCD_ShowChinese(71,23,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,66);
			LCD_ShowChinese(87,23,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,87);
			
			LCD_ShowChinese(118,60,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,57);
			LCD_ShowChinese(134,60,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,58);
			_gTaskShareDatObj.DataBuf[0] = 103;
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(35,23,(uint8_t *)"In Progress",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			LCD_ShowString(102,60,(uint8_t *)"CANCEL",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			_gTaskShareDatObj.DataBuf[0] = 123;
		}
//		_gTaskShareDatObj.TempVal_S32[0] = AppDataRead(APP_AirZeroingVal);
		_gTaskShareDatObj.TempVal_U32[1] = 0;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		//350ms周期获取当前气压值
		if(GetSystemTick() - _gTaskShareDatObj.TempVal_U32[0] > 350)
		{
			sMenuFlag &= ~0x60;
			AirPressure = GetAirPressureVal(1);
			//压力超限
			if((AirPressure > Param_Config.ZeroingValUpperLimit) || (AirPressure < Param_Config.ZeroingValLowerLimit))
			{
				if((_gTaskShareDatObj.State & 0x20) == 0x00)
				{
					if(MenuLanguage == Chinese)
					{
						LCD_ShowChinese(58,41,RED,MenuInterfaceColObj.ParamSetBackCol,16,0,107);
						LCD_ShowChinese(74,41,RED,MenuInterfaceColObj.ParamSetBackCol,16,0,110);
						LCD_ShowChinese(90,41,RED,MenuInterfaceColObj.ParamSetBackCol,16,0,108);
						LCD_ShowChinese(106,41,RED,MenuInterfaceColObj.ParamSetBackCol,16,0,109);
					}
					else if(MenuLanguage == English)
					{
						LCD_ShowString(31,41,(uint8_t *)"Pressure Exceed",RED,MenuInterfaceColObj.ParamSetBackCol,16,0);
					}
					_gTaskShareDatObj.State |= 0x20;
				}
				
				_gTaskShareDatObj.Count = 0;
				_gTaskShareDatObj.TempVal_S32[0] = 0;
			}
			else
			{
				if(_gTaskShareDatObj.State & 0x20)
				{
					LCD_Fill(31,41,151,57,MenuInterfaceColObj.ParamSetBackCol);
					_gTaskShareDatObj.State &= ~0x20;
				}
				_gTaskShareDatObj.TempVal_S32[0] += AirPressure;
				_gTaskShareDatObj.TempVal_S16[_gTaskShareDatObj.Count] = AirPressure;
				_gTaskShareDatObj.Count++;
			}
			//显示进度点.
			if(_gTaskShareDatObj.TempVal_U16[16] < 3)
			{
				LCD_ShowChar(_gTaskShareDatObj.DataBuf[0]+_gTaskShareDatObj.TempVal_U16[16]*8,23,'.',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				_gTaskShareDatObj.TempVal_U16[16]++;
			}
			else
			{
				_gTaskShareDatObj.TempVal_U16[16] = 0;
				LCD_Fill(_gTaskShareDatObj.DataBuf[0],23,_gTaskShareDatObj.DataBuf[0]+8*3,39,MenuInterfaceColObj.ParamSetBackCol);
			}
			
			_gTaskShareDatObj.TempVal_U32[0] = GetSystemTick();
		}
		//采样完毕
		if(_gTaskShareDatObj.Count >= 16)
		{
			_gTaskShareDatObj.TempVal_S32[0] >>= 4;
			//校验数据
			for(i=0; i<16; i++)
			{
				if(abs(_gTaskShareDatObj.TempVal_S32[0] - (int32_t)_gTaskShareDatObj.TempVal_S16[i]) > Param_Config.ZeroingErrorRange)
				{
					_gTaskShareDatObj.TempVal_U32[1] = 1;
					break;
				}
			}
			//检查采集的电压数据是否在合理范围
			if(!_gTaskShareDatObj.TempVal_U32[1])
			{
				_gTaskShareDatObj.State |= 0x40;
				ParamSetEndDisplayPage(1,0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
			{
				_gTaskShareDatObj.Count = 0;
				_gTaskShareDatObj.TempVal_S32[0] = 0;
				_gTaskShareDatObj.TempVal_U32[1] = 0;
			}
		}
		
		if(KEY_ReadEvent(DOWN, Short_Press_Once, 1))
		{
			_gTaskShareDatObj.DataBuf[1] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(118,60,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,57);
				LCD_ShowChinese(134,60,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,58);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(102,60,(uint8_t *)"CANCEL",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(DOWN, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
			{
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(118,60,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,57);
					LCD_ShowChinese(134,60,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,58);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(102,60,(uint8_t *)"CANCEL",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				sAutoReturnLastMenu = 1;
			}
			else
				_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.TempVal_S32[0] >= 0)
			AppDataWrite(_gTaskShareDatObj.TempVal_S32[0] + Param_Config.ZeroingExtraScopeVal, APP_AirZeroingVal);
		else
			AppDataWrite(_gTaskShareDatObj.TempVal_S32[0] - Param_Config.ZeroingExtraScopeVal, APP_AirZeroingVal);
		APPDataFlashWrite();
//		CalculateZeroingData();
		sAutoReturnLastMenu = 1;
	}
}
//零点复位
void ParamSetZeroResetFuntion(void *param)			
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(0, APP_AirZeroingVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
/*******显示单位设置菜单目录执行功能函数*******/
//Mpa
void ParamSetUnitMpaFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemUnit_Mpa, APP_SystemUnit);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//Kpa
void ParamSetUnitKpaFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemUnit_Kpa, APP_SystemUnit);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
/*******通道1参数设置菜单目录执行功能函数*******/
//模式
void ParamSetOut1ModeFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	switch(AppDataRead(APP_Out1Mode))
	{
		case SystemChannelMode_Simple: vManualItemNum = 0; break;
		case SystemChannelMode_Lag: vManualItemNum = 1; break;
		case SystemChannelMode_Window: vManualItemNum = 2; break;
		default: vManualItemNum = 0; break;
	}
}
//阈值
void ParamSetOut1ThresholdValueFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1ThresholdVal);	
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.PressureUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.PressureLowerLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1); 
		PressureParamIntputEnable(109,25);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
			PressureParamIntputUnitRead();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		_gTaskShareDatObj.DataBuf[0] = PressureParamIntputUnitRead();
		if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Mpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue / 10 * 10, APP_Out1ThresholdVal);
		else if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Kpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1ThresholdVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//峰值
void ParamSetOut1PeakValueFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
//		_gTaskShareDatObj.ParamSetObj.Mode = 1;
//		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
//		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
//		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
//		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
//		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
//		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
//		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1PeakVal);			
//		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = PressureUpperLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = AppDataRead(APP_Out1ValleyVal);
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
//		ParamSetTaskStart(1);
//		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1PeakVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.PressureUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = AppDataRead(APP_Out1ValleyVal);
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1); 
		PressureParamIntputEnable(109,25);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[0] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
			PressureParamIntputUnitRead();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		_gTaskShareDatObj.DataBuf[0] = PressureParamIntputUnitRead();
		if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Mpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue / 10 * 10, APP_Out1PeakVal);
		else if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Kpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1PeakVal);
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1PeakVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//谷值
void ParamSetOut1ValleyValueFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
//		_gTaskShareDatObj.ParamSetObj.Mode = 1;
//		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
//		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
//		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
//		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
//		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
//		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
//		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1ValleyVal);			
//		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = AppDataRead(APP_Out1PeakVal);
//		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = PressureLowerLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
//		ParamSetTaskStart(1);
//		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1ValleyVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = AppDataRead(APP_Out1PeakVal);
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.PressureLowerLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1); 
		PressureParamIntputEnable(109,25);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[0] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
			PressureParamIntputUnitRead();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		_gTaskShareDatObj.DataBuf[0] = PressureParamIntputUnitRead();
		if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Mpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue / 10 * 10, APP_Out1ValleyVal);
		else if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Kpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1ValleyVal);
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1ValleyVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//迟滞值
void ParamSetOut1LagValFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(66,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(82,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(98,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
if(Param_Config.ProductClass == LowPressureSeriesProduct)
{
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 49;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1LagVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.LagValUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.LagValLowerLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
		LCD_ShowString(97,25,(uint8_t *)"kPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
}
		
if (Param_Config.ProductClass == HighPressureSeriesProduct)
{
	_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 0;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 55;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1LagVal)/10;
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.LagValUpperLimit/10;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.LagValLowerLimit/10;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 2;
		LCD_ShowString(91,25,(uint8_t *)"kPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
}
			
		_gTaskShareDatObj.DataBuf[0] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
if(Param_Config.ProductClass == LowPressureSeriesProduct)
{
	AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1LagVal);
}
		
if (Param_Config.ProductClass == HighPressureSeriesProduct)
{
	AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue * 10, APP_Out1LagVal);
}
		
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1LagVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//响应时间
void ParamSetOut1DelayTimeFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 0;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out1ResponseTimeVal);
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 49;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.DelayTimeUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.DelayTimeLowerLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 4;
		LCD_ShowString(109,25,(uint8_t *)"ms",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[1] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[1] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out1ResponseTimeVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//输出方式
void ParamSetOut1WayFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	switch(AppDataRead(APP_Out1Way))
	{
		case SystemOutWay_NoReverse: vManualItemNum = 0; break;
		case SystemOutWay_Reverse: vManualItemNum = 1; break;
		default: vManualItemNum = 0; break;
	}
}	
/*******通道1模式设置菜单目录执行功能函数*******/
//1通道设置简易模式
void ParamSetOut1ModeOrdinaryFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemChannelMode_Simple, APP_Out1Mode);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//1通道设置迟滞模式
void ParamSetOut1ModeLagFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemChannelMode_Lag, APP_Out1Mode);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//1通道设置窗口模式
void ParamSetOut1ModeWindowFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemChannelMode_Window, APP_Out1Mode);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
/*******通道1输出方式设置菜单目录执行功能函数*******/
//正向输出
void ParamSetOut1WayNoReverseFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemOutWay_NoReverse, APP_Out1Way);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//反向输出
void ParamSetOut1WayReverseFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemOutWay_Reverse, APP_Out1Way);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
/*******通道2参数设置菜单目录执行功能函数*******/
//模式
void ParamSetOut2ModeFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	switch(AppDataRead(APP_Out2Mode))
	{
		case SystemChannelMode_Simple: vManualItemNum = 0; break;
		case SystemChannelMode_Lag: vManualItemNum = 1; break;
		case SystemChannelMode_Window: vManualItemNum = 2; break;
		default: vManualItemNum = 0; break;
	}
}
//阈值
void ParamSetOut2ThresholdValueFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2ThresholdVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.PressureUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.PressureLowerLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1); 
		PressureParamIntputEnable(109,25);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[1] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[1] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
			PressureParamIntputUnitRead();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		_gTaskShareDatObj.DataBuf[0] = PressureParamIntputUnitRead();
		if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Mpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue / 10 * 10, APP_Out2ThresholdVal);
		else if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Kpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2ThresholdVal);
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2ThresholdVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//峰值
void ParamSetOut2PeakValueFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
//		_gTaskShareDatObj.ParamSetObj.Mode = 1;
//		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
//		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
//		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
//		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
//		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
//		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
//		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2PeakVal);			
//		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = PressureUpperLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = AppDataRead(APP_Out2ValleyVal);
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
//		ParamSetTaskStart(1);
//		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2PeakVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.PressureUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = AppDataRead(APP_Out2ValleyVal);
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1); 
		PressureParamIntputEnable(109,25);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[0] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
			PressureParamIntputUnitRead();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		_gTaskShareDatObj.DataBuf[0] = PressureParamIntputUnitRead();
		if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Mpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue / 10 * 10, APP_Out2PeakVal);
		else if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Kpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2PeakVal);
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2PeakVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//谷值
void ParamSetOut2ValleyValueFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
//		_gTaskShareDatObj.ParamSetObj.Mode = 1;
//		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
//		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
//		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
//		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
//		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
//		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
//		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2ValleyVal);			
//		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = AppDataRead(APP_Out2PeakVal);
//		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = PressureLowerLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
//		ParamSetTaskStart(1);
//		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 37;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2ValleyVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = AppDataRead(APP_Out2PeakVal);
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.PressureLowerLimit;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
//		LCD_ShowString(109,25,(uint8_t *)"KPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1); 
		PressureParamIntputEnable(109,25);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[0] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
			PressureParamIntputUnitRead();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		_gTaskShareDatObj.DataBuf[0] = PressureParamIntputUnitRead();
		if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Mpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue / 10 * 10, APP_Out2ValleyVal);
		else if(_gTaskShareDatObj.DataBuf[0] == ParamUnit_Kpa)
			AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2ValleyVal);
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2ValleyVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//迟滞值
void ParamSetOut2LagValFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(66,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(82,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(98,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
if(Param_Config.ProductClass == LowPressureSeriesProduct)
{
	_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 49;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2LagVal);			
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.LagValUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.LagValLowerLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 1;
		LCD_ShowString(97,25,(uint8_t *)"kPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
}
		
if (Param_Config.ProductClass == HighPressureSeriesProduct)
{
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 0;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 55;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2LagVal)/10;
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.LagValUpperLimit/10;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.LagValLowerLimit/10;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 2;
		LCD_ShowString(91,25,(uint8_t *)"kPa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
}
			
		_gTaskShareDatObj.DataBuf[0] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] = 1;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
if(Param_Config.ProductClass == LowPressureSeriesProduct)
{
	AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2LagVal);
}
		
if (Param_Config.ProductClass == HighPressureSeriesProduct)
{
	AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue * 10, APP_Out2LagVal);
}
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//响应时间
void ParamSetOut2DelayTimeFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 0;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_Out2ResponseTimeVal);
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 49;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = Param_Config.DelayTimeUpperLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = Param_Config.DelayTimeLowerLimit;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 4;
		LCD_ShowString(109,25,(uint8_t *)"ms",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetFontCol,24,1);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[1] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[1] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				
				ParamSetTaskStart(0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		ParamSetTaskDelete();
		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_Out2ResponseTimeVal);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//输出方式
void ParamSetOut2WayFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	switch(AppDataRead(APP_Out2Way))
	{
		case SystemOutWay_NoReverse: vManualItemNum = 0; break;
		case SystemOutWay_Reverse: vManualItemNum = 1; break;
		default: vManualItemNum = 0; break;
	}
}	
/*******通道2模式设置菜单目录执行功能函数*******/
//2通道设置简易模式
void ParamSetOut2ModeOrdinaryFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemChannelMode_Simple, APP_Out2Mode);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//2通道设置迟滞模式
void ParamSetOut2ModeLagFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemChannelMode_Lag, APP_Out2Mode);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}	
//2通道设置窗口模式
void ParamSetOut2ModeWindowFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemChannelMode_Window, APP_Out2Mode);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
/*******通道2输出方式设置菜单目录执行功能函数*******/
//2通道正向输出
void ParamSetOut2WayNoReverseFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemOutWay_NoReverse, APP_Out2Way);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//2通道反向输出
void ParamSetOut2WayReverseFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)SystemOutWay_Reverse, APP_Out2Way);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}

/*******系统设置菜单目录执行功能函数*******/
//语言设置
void SystemSetLanguageFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}
////背光设置
//void SystemSetBackLightSetFuntion(void *param)
//{
//	struct MenuItem * gItemTemp = (struct MenuItem *)param;
//	
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
//		if(MenuLanguage == Chinese)
//		{
//			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
//			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
//			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
//			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
//		}
//		else if(MenuLanguage == English)
//		{
//			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
//						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
//		}
//		
//		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
//		if(MenuLanguage == Chinese)
//		{
//			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
//			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
//			LCD_ShowChinese(58,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,43);
//			LCD_ShowChinese(74,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,44);
//			LCD_ShowChinese(90,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,71);
//			LCD_ShowChinese(106,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,72);
//		}
//		else if(MenuLanguage == English)
//		{
//			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
//			LCD_ShowString(31,20,(uint8_t *)"Backlight Class",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		KEY_ReadEvent(ENTER, Press_END, 1);
//		KEY_ReadEvent(UP, Press_END, 1);
//		KEY_ReadEvent(DOWN, Press_END, 1);
//		
//		_gTaskShareDatObj.ParamSetObj.Mode = 1;
//		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
//		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
//		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 0;
//		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_SystemBacklightGrade);
//		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = 10;
//		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = 1;
//		_gTaskShareDatObj.ParamSetObj.StartPos_X = 71;
//		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 38;
//		_gTaskShareDatObj.ParamSetObj.FontSize = 16;
//		_gTaskShareDatObj.ParamSetObj.ParamLen = 2;
//		ParamSetTaskStart(1);
//		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
//		
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		_gTaskShareDatObj.DataBuf[1] = 0;
//		sFunctionQuit = 1;
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//	}
//	
//	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
//	{
//		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
//		{
//			sMenuFlag &= ~0x60;
//			_gTaskShareDatObj.DataBuf[1] |= 0x80;
//			
//			if(MenuLanguage == Chinese)
//			{
//				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
//				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
//			}
//			else if(MenuLanguage == English)
//			{
//				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
//			}
//		}
//		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
//		{
//			sAutoReturnLastMenu = 1;
//			ParamSetTaskDelete();
//			_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_SystemBacklightGrade);
//		}
//		else if(KEY_ReadEvent(ENTER, Press_END, 1))
//		{
//			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
//			{
//				_gTaskShareDatObj.State |= 0x40;
//				
//				if(MenuLanguage == Chinese)
//				{
//					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
//					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
//				}
//				else if(MenuLanguage == English)
//				{
//					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
//				}
//				
//				ParamSetEndDisplayPage(1,0);
//				
//				ParamSetTaskStart(0);
//				ParamSetTaskDelete();
//				_gTaskShareDatObj.TaskTick = GetSystemTick();
//			}
//			else
//				_gTaskShareDatObj.DataBuf[1] = 0;
//		}
//		
//		PWM_TIM1_Channel2_DutySet(_gTaskShareDatObj.ParamSetObj.ParamValue * 10);
//	}
//	
//	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		_gTaskShareDatObj.State &= ~0x40;
//		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue, APP_SystemBacklightGrade);
//		APPDataFlashWrite();
//		sAutoReturnLastMenu = 1;
//	}
//}	
//输出测试
void SystemSetOutputTestFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
	
	LED_Ctrl(LED1,0);
	NpnOut_Ctrl(NPN1,0);
	LED_Ctrl(LED2,0);
	NpnOut_Ctrl(NPN2,0);
}
//电流校准
void SystemSetCurrentsFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}
////背景颜色
//void SystemSetBackColourFuntion(void *param)
//{	
//	MenuCatalogMessagePrint((struct MenuItem *)param);
//	
//	switch(AppDataRead(APP_SystemBackColour))
//	{
//		case BLACK: vManualItemNum = 0; break;
//		case AWHITE: vManualItemNum = 1; break;
//		case ABLUE: vManualItemNum = 2; break;
//		case ORANGE: vManualItemNum = 3; break;
//		case YELLOW: vManualItemNum = 4; break;
//		case AGREEN: vManualItemNum = 5; break;
//		default: vManualItemNum = 0; break;
//	}
//}
//字体颜色
void SystemSetFontColourFuntion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);

	switch(AppDataRead(APP_SystemFontColour))
	{
		case AWHITE: vManualItemNum = 0; break;
		case Azure: vManualItemNum = 1; break;
		case ORANGE: vManualItemNum = 2; break;
		case YELLOW: vManualItemNum = 3; break;
		case AGREEN: vManualItemNum = 4; break;
		case Violet: vManualItemNum = 5; break;
		default: vManualItemNum = 0; break;
	}
}	
//恢复出厂设置
void SystemSetFactoryResetFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,LCD_H,MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(38,13,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,68);
			LCD_ShowChinese(54,13,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,69);
			LCD_ShowChinese(70,13,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,52);
			LCD_ShowChinese(86,13,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,53);
			LCD_ShowChinese(102,13,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,35);
			LCD_ShowChinese(118,13,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,54);
			LCD_ShowChar(134,13,'?',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			
			LCD_ShowChinese(32,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(48,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
			LCD_ShowChinese(118,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,57);
			LCD_ShowChinese(134,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,58);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(51,8,(uint8_t *)"Whether to",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			LCD_ShowString(31,24,(uint8_t *)"restore factory",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			LCD_ShowChar(87,40,'?',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			
			LCD_ShowString(32,60,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			LCD_ShowString(102,60,(uint8_t *)"CANCEL",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		_gTaskShareDatObj.DataBuf[0] = 0;
		_gTaskShareDatObj.DataBuf[1] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(((_gTaskShareDatObj.State & 0x02) == 0x00) && (KEY_ReadEvent(UP, Short_Press_Once, 1)))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.State |= 0x01;
			_gTaskShareDatObj.DataBuf[0] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(32,49,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(48,49,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(32,60,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(UP, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0] & 0x80)
			{
				sMenuFlag &= ~0x60;
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(32,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(48,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(32,60,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
			{
				_gTaskShareDatObj.DataBuf[0] = 0;
				_gTaskShareDatObj.State &= ~0x01;
			}
		}
		
		if(((_gTaskShareDatObj.State & 0x01) == 0x00) && (KEY_ReadEvent(DOWN, Short_Press_Once, 1)))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.State |= 0x02;
			_gTaskShareDatObj.DataBuf[1] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(118,49,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,57);
				LCD_ShowChinese(134,49,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,58);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(102,60,(uint8_t *)"CANCEL",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(DOWN, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
			{
				sMenuFlag &= ~0x60;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(118,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,57);
					LCD_ShowChinese(134,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,58);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(102,60,(uint8_t *)"CANCEL",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				sAutoReturnLastMenu = 1;
			}
			else
			{
				_gTaskShareDatObj.DataBuf[1] = 0;
				_gTaskShareDatObj.State &= ~0x02;
			}
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		_gTaskShareDatObj.State &= ~0x40;
		
		#if DEBUG == 0
		_gTaskShareDatObj.TempVal_U16[0] = AppDataRead(APP_SystemInputVoltage);
		_gTaskShareDatObj.TempVal_S16[0] = AppDataRead(APP_ADCxCompensationVal);
		_gTaskShareDatObj.TempVal_U16[1] = AppDataRead(APP_SystemLanguage);
		APPDataEmpty(0);
		System_ParameterReset();
		AppDataWrite(_gTaskShareDatObj.TempVal_U16[0], APP_SystemInputVoltage);
		AppDataWrite(_gTaskShareDatObj.TempVal_S16[0], APP_ADCxCompensationVal);
		AppDataWrite(_gTaskShareDatObj.TempVal_U16[1], APP_SystemLanguage);
		#else
		System_ParameterReset();
		#endif
//		APPDataFlashWrite();
		//重新读取相关颜色数据
		MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
		MenuInterfaceColObj.ItemCol = ABLUE;
		MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
		MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
		MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
		MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
		MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
		MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		//
		MenuLanguage = AppDataRead(APP_SystemLanguage);
		
//		MenuLanguage = AppDataRead(APP_SystemLanguage);
//		MenuItemDisMax = 2;
//		MenuCoordObj.Catalog_W = 26;
//		MenuCoordObj.ItemAreaRefEnd = 149;
//		MenuCoordObj.ItemFrameStart = 28;
//		MenuCoordObj.ItemFrame_W = 26;
//		MenuCoordObj.ItemFontStart_Y = 29;
		//复位电流更新计算数据
		CurrentOutput_UpdateCalcData();
		sAutoReturnLastMenu = 1;
	}
}
#if DEBUG == 1
//电压校准
void SystemSetVolCalibrationFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		GetRefVoltageValue(&_gTaskShareDatObj.TempVal_U16[0], &_gTaskShareDatObj.TempVal_U16[1]);
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		_gTaskShareDatObj.ParamSetObj.Mode = 1;
		_gTaskShareDatObj.ParamSetObj.FontColour = MenuInterfaceColObj.ParamSetFontCol;
		_gTaskShareDatObj.ParamSetObj.BackColour = MenuInterfaceColObj.ParamSetBackCol;
		_gTaskShareDatObj.ParamSetObj.DecimalDisplayEnable = 1;
//		_gTaskShareDatObj.ParamSetObj.ParamValue = 5000;
		_gTaskShareDatObj.ParamSetObj.ParamValue = AppDataRead(APP_SystemInputVoltage);
		_gTaskShareDatObj.ParamSetObj.ParamHighLimit = 5400;
		_gTaskShareDatObj.ParamSetObj.ParamLowLimit = 3000;
		_gTaskShareDatObj.ParamSetObj.StartPos_X = 49;
		_gTaskShareDatObj.ParamSetObj.StartPos_Y = 25;
		_gTaskShareDatObj.ParamSetObj.FontSize = 24;
		_gTaskShareDatObj.ParamSetObj.ParamLen = 3;
		LCD_ShowChar(121,25,'V',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,24,0);
		ParamSetTaskStart(1);
		ParamSetTaskCreate(&_gTaskShareDatObj.ParamSetObj);
		
		_gTaskShareDatObj.DataBuf[1] = 0;
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[1] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			sAutoReturnLastMenu = 1;
			ParamSetTaskDelete();
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[1] & 0x80)
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				ParamSetTaskStart(0);
				ParamSetTaskDelete();
				_gTaskShareDatObj.TaskTick = GetSystemTick();
				
//				_gTaskShareDatObj.TempVal_S32[0] = ADC_CompensationValNum(_gTaskShareDatObj.ParamSetObj.ParamValue);
			}
			else
				_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		_gTaskShareDatObj.State &= ~0x40;
//		AppDataWrite(_gTaskShareDatObj.TempVal_S32[0], APP_ADCxCompensationVal);
		AppDataWrite(_gTaskShareDatObj.ParamSetObj.ParamValue,APP_SystemInputVoltage);
		APPDataFlashWrite();
		sAutoReturnLastMenu = 1;
	}
}
//电压补偿
void SystemSetVolCompensationFuntion(void *param)
{
	uint16_t TempVal_U16 = 0;
	double TempVal_LF = 0.00;
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		
		//电压值
		LCD_ShowChinese(39,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,50);
		LCD_ShowChinese(55,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,51);
		LCD_ShowChinese(71,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,2);
		LCD_ShowChar(87,20,':',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		LCD_ShowChar(135,20,'V',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		//补偿值
		LCD_ShowChinese(51,38,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,23);
		LCD_ShowChinese(67,38,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,24);
		LCD_ShowChinese(83,38,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,2);
		LCD_ShowChar(99,38,':',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		
		LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
		LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		
		_gTaskShareDatObj.TempVal_S32[0] = AppDataRead(APP_ADCxCompensationVal);
		_gTaskShareDatObj.TempVal_LF[0] = (double)AppDataRead(APP_SystemInputVoltage)/1000;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		sMenuFlag &= ~0x60;
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			_gTaskShareDatObj.DataBuf[0] = 1;
			
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
		}
		else if(KEY_ReadEvent(UP, Short_Press_Once, 1))
		{
			if(_gTaskShareDatObj.TempVal_S32[0] < 15)
			{
				_gTaskShareDatObj.TempVal_S32[0]++;
				ADC_CalibrattionValueUpdate(_gTaskShareDatObj.TempVal_S32[0]);
			}
		}
		else if(KEY_ReadEvent(DOWN, Short_Press_Once, 1))
		{
			if(_gTaskShareDatObj.TempVal_S32[0] > -15)
			{
				_gTaskShareDatObj.TempVal_S32[0]--;
				ADC_CalibrattionValueUpdate(_gTaskShareDatObj.TempVal_S32[0]);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			ADC_CalibrattionValueUpdate(AppDataRead(APP_ADCxCompensationVal));
			sAutoReturnLastMenu = 1;
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0])
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				
				_gTaskShareDatObj.State |= 0x40;
				ParamSetEndDisplayPage(1,0);
				
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			_gTaskShareDatObj.DataBuf[0] = 0;
		}
		//获取ADC采样值
		if(ADC_GetData(&TempVal_U16))
		{
			_gTaskShareDatObj.TempVal_U32[0] += TempVal_U16;
			_gTaskShareDatObj.TempVal_U32[1]++;
			if(_gTaskShareDatObj.TempVal_U32[1] >= 1024)
			{
				_gTaskShareDatObj.TempVal_U16[1] = _gTaskShareDatObj.TempVal_U32[0] >> 10;
				
//				if(_gTaskShareDatObj.TempVal_U16[0] > _gTaskShareDatObj.TempVal_U32[0])
//				{			
//					if((_gTaskShareDatObj.TempVal_U16[0] - _gTaskShareDatObj.TempVal_U32[0]) <= 1)
//					{
//						_gTaskShareDatObj.TempVal_U16[1] = _gTaskShareDatObj.TempVal_U16[0];
//					}
//					else
//					{
//						_gTaskShareDatObj.TempVal_U16[1] = _gTaskShareDatObj.TempVal_U32[0];
//						_gTaskShareDatObj.TempVal_U16[0] = _gTaskShareDatObj.TempVal_U32[0];
//					}
//				}
//				else
//				{
//					if((_gTaskShareDatObj.TempVal_U32[0] - _gTaskShareDatObj.TempVal_U16[0]) <= 1)
//					{
//						_gTaskShareDatObj.TempVal_U16[1] = _gTaskShareDatObj.TempVal_U16[0];
//					}
//					else
//					{
//						_gTaskShareDatObj.TempVal_U16[1] = _gTaskShareDatObj.TempVal_U32[0];
//						_gTaskShareDatObj.TempVal_U16[0] = _gTaskShareDatObj.TempVal_U32[0];
//					}
//				}
				_gTaskShareDatObj.TempVal_U32[0] = 0;
				_gTaskShareDatObj.TempVal_U32[1] = 0;
				_gTaskShareDatObj.DataBuf[1] = 1;
			}
		}
		//显示
		if(_gTaskShareDatObj.DataBuf[1])
		{
			TempVal_LF = _gTaskShareDatObj.TempVal_LF[0]/4095*_gTaskShareDatObj.TempVal_U16[1];
			TempVal_LF *= 2;
			TempVal_U16 = TempVal_LF;
			LCD_ShowIntNum(95,20,TempVal_U16,1,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			TempVal_LF -= TempVal_U16;
			TempVal_U16 = TempVal_LF*1000;
			LCD_ShowChar(103,20,'.',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			LCD_ShowIntNum(111,20,TempVal_U16,3,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			
			if(_gTaskShareDatObj.TempVal_S32[0] < 0)
			{
				TempVal_U16 = ~_gTaskShareDatObj.TempVal_S32[0] + 1;
				LCD_ShowChar(107,38,'-',MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
			else
			{
				TempVal_U16 = _gTaskShareDatObj.TempVal_S32[0];
				LCD_ShowChar(107,38,'+',MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
			LCD_ShowIntNum(115,38,TempVal_U16,2,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,1);
			
			_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite(_gTaskShareDatObj.TempVal_S32[0], APP_ADCxCompensationVal);
		APPDataFlashWrite();
		ADC_CalibrattionValueUpdate(AppDataRead(APP_ADCxCompensationVal));
		sAutoReturnLastMenu = 1;
	}
}	
#endif

//版本信息
void SystemSetVersionFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(58,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[2]);
			LCD_ShowChinese(106,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[3]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(54,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,77);
			LCD_ShowChinese(70,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,79);
			LCD_ShowChinese(86,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,73);
			LCD_ShowChinese(102,20,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,74);
			LCD_ShowChar(118,20,':',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			
			LCD_ShowChinese(54,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,78);
			LCD_ShowChinese(70,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,79);
			LCD_ShowChinese(86,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,73);
			LCD_ShowChinese(102,49,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,74);
			LCD_ShowChar(118,49,':',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(27,20,(uint8_t *)"SoftwareVersion:",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
			LCD_ShowString(27,49,(uint8_t *)"HardwareVersion:",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		LCD_ShowString(71,36,(uint8_t *)SoftwareVer,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,0);
		LCD_ShowString(71,65,(uint8_t *)HardwareVer,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,0);
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		GetRefVoltageValue(&_gTaskShareDatObj.TempVal_U16[0], &_gTaskShareDatObj.TempVal_U16[1]);
		_gTaskShareDatObj.TempVal_S32[0] = AppDataRead(APP_ADCxCompensationVal);
		_gTaskShareDatObj.TempVal_S32[1] = AppDataRead(APP_AirZeroingVal);
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	#if DEBUG == 1
	//显示计算的VCC等相关数据
	if(KEY_ReadEvent(UP, Short_Press_Once, 1))
	{
		if((_gTaskShareDatObj.DataBuf[0] & 0x80) == 0x00)
		{
			LCD_ShowIntNum(130,36,_gTaskShareDatObj.TempVal_U16[0],4,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
			LCD_ShowIntNum(130,65,_gTaskShareDatObj.TempVal_U16[1],4,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
			
			if(_gTaskShareDatObj.TempVal_S32[0] < 0)
			{
				_gTaskShareDatObj.TempVal_U16[2] = ~_gTaskShareDatObj.TempVal_S32[0] + 1;
				LCD_ShowChar(30,65,'-',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,0);
			}
			else
			{
				_gTaskShareDatObj.TempVal_U16[2] = _gTaskShareDatObj.TempVal_S32[0];
				LCD_ShowChar(30,65,'+',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,0);
			}
			LCD_ShowIntNum(36,65,_gTaskShareDatObj.TempVal_U16[2],2,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
			//
			LCD_ShowIntNum(30,36,_gTaskShareDatObj.TempVal_S32[1],4,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
			
//			if(_gTaskShareDatObj.TempVal_S32[1] < 0)
//			{
//				_gTaskShareDatObj.TempVal_U16[3] = ~_gTaskShareDatObj.TempVal_S32[1] + 1;
//				LCD_ShowChar(24,36,'-',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,0);
//			}
//			else
//			{
//				_gTaskShareDatObj.TempVal_U16[3] = _gTaskShareDatObj.TempVal_S32[1];
//				LCD_ShowChar(24,36,'+',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,0);
//			}
//			LCD_ShowIntNum(30,36,_gTaskShareDatObj.TempVal_U16[3]/10,1,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
//			LCD_ShowChar(36,36,'.',MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
//			LCD_ShowIntNum(42,36,_gTaskShareDatObj.TempVal_U16[3]%10,1,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
//			LCD_ShowString(48,36,(uint8_t *)"Kpa",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,12,1);
			
			_gTaskShareDatObj.DataBuf[0] |= 0x80;
			_gTaskShareDatObj.TaskTick = GetSystemTick();
		}
	}
	if((_gTaskShareDatObj.DataBuf[0] & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > 3000))
	{
		LCD_Fill(130, 36, 154, 48, MenuInterfaceColObj.ParamSetBackCol);
		LCD_Fill(30, 65, 48, 77, MenuInterfaceColObj.ParamSetBackCol);
		LCD_Fill(130, 65, 154, 77, MenuInterfaceColObj.ParamSetBackCol);
		LCD_Fill(30, 36, 54, 48, MenuInterfaceColObj.ParamSetBackCol);
		_gTaskShareDatObj.DataBuf[0] &= ~0x80;
	}
	#endif
	
	if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
	{
		sAutoReturnLastMenu = 1;
		KEY_ReadEvent(ENTER, Short_Press_Once, 1);
	}
}

/*******输出测试菜单目录执行功能函数*******/
//中文
void SystemSetLanguageChineseFuntion(void *param)
{	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		MenuLanguage = Chinese;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)Chinese, APP_SystemLanguage);
		APPDataFlashWrite();
		
		MenuItemDisMax = 2;
		MenuCoordObj.Catalog_W = 26;
		MenuCoordObj.ItemAreaRefEnd = 149;
		MenuCoordObj.ItemFrameStart = 28;
		MenuCoordObj.ItemFrame_W = 26;
		MenuCoordObj.ItemFontStart_Y = 29;
		
		sAutoReturnLastMenu = 1;
	}
}
//英文
void SystemSetLanguageEnglishFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		MenuLanguage = English;
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		AppDataWrite((int32_t)English, APP_SystemLanguage);
		APPDataFlashWrite();
		
		MenuItemDisMax = 3;
		MenuCoordObj.Catalog_W = 18;
		MenuCoordObj.ItemAreaRefEnd = LCD_W;
		MenuCoordObj.ItemFrameStart = 20;
		MenuCoordObj.ItemFrame_W = 20;
		MenuCoordObj.ItemFontStart_Y = 22;
		
		sAutoReturnLastMenu = 1;
	}
}	

/*******输出测试菜单目录执行功能函数*******/
//通道1测试
void SystemSetOutput1Funtion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}
//通道2测试
void SystemSetOutput2Funtion(void *param)
{
	MenuCatalogMessagePrint((struct MenuItem *)param);
}

///*******背景颜色菜单目录执行功能函数*******/
////黑色
//void SystemSetBackColourBlackFuntion(void *param)		
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemFontColour) == BLACK)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)BLACK, APP_SystemBackColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}
////白色
//void SystemSetBackColourWhiteFuntion(void *param)		
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemFontColour) == AWHITE)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)AWHITE, APP_SystemBackColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}
////蓝色
//void SystemSetBackColourBlueFuntion(void *param)		
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemFontColour) == Azure)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)Azure, APP_SystemBackColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}
////橙色
//void SystemSetBackColourOrangeFuntion(void *param)	
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemFontColour) == ORANGE)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)ORANGE, APP_SystemBackColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}
////黄色
//void SystemSetBackColourYellowFuntion(void *param)		
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemFontColour) == YELLOW)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)YELLOW, APP_SystemBackColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}
////绿色
//void SystemSetBackColourGreenFuntion(void *param)
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemFontColour) == AGREEN)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)AGREEN, APP_SystemBackColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}

/*******字体颜色菜单目录执行功能函数*******/
////黑色
//void SystemSetFontColourBlackFuntion(void *param)
//{
//	if((_gTaskShareDatObj.State & 0x80) == 0x00)
//	{
//		sMenuFlag &= ~0x60;
//		_gTaskShareDatObj.DataBuf[0] = 0;
//		
//		if(AppDataRead(APP_SystemBackColour) == BLACK)
//			ParamSetEndDisplayPage(0,1);
//		else
//		{
//			ParamSetEndDisplayPage(1,1);
//			_gTaskShareDatObj.DataBuf[0] = 1;
//		}
//		
//		MeterInterfaceKeyShield(FunctionKey_Disbale);
//		
//		sFunctionExecute = 1;
//		_gTaskShareDatObj.State |= 0x80;
//		_gTaskShareDatObj.TaskTick = GetSystemTick();
//	}
//	
//	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
//	{
//		if(_gTaskShareDatObj.DataBuf[0])
//		{
//			AppDataWrite((int32_t)BLACK, APP_SystemFontColour);
//			APPDataFlashWrite();
//			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ItemCol = ABLUE;
//			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
//			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
//			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
//		}
//		sAutoReturnLastMenu = 1;
//	}
//}
//白色
void SystemSetFontColourWhiteFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		ParamSetEndDisplayPage(1,1);
		_gTaskShareDatObj.DataBuf[0] = 1;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.DataBuf[0])
		{
			AppDataWrite((int32_t)AWHITE, APP_SystemFontColour);
			APPDataFlashWrite();
			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ItemCol = ABLUE;
			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		}
		sAutoReturnLastMenu = 1;
	}
}	
//蓝色
void SystemSetFontColourBlueFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		ParamSetEndDisplayPage(1,1);
		_gTaskShareDatObj.DataBuf[0] = 1;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.DataBuf[0])
		{
			AppDataWrite((int32_t)Azure, APP_SystemFontColour);
			APPDataFlashWrite();
			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ItemCol = ABLUE;
			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		}
		sAutoReturnLastMenu = 1;
	}
}	
//橙色
void SystemSetFontColourOrangeFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		ParamSetEndDisplayPage(1,1);
		_gTaskShareDatObj.DataBuf[0] = 1;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.DataBuf[0])
		{
			AppDataWrite((int32_t)ORANGE, APP_SystemFontColour);
			APPDataFlashWrite();
			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ItemCol = ABLUE;
			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		}
		sAutoReturnLastMenu = 1;
	}
}	
//黄色
void SystemSetFontColourYellowFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		ParamSetEndDisplayPage(1,1);
		_gTaskShareDatObj.DataBuf[0] = 1;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.DataBuf[0])
		{
			AppDataWrite((int32_t)YELLOW, APP_SystemFontColour);
			APPDataFlashWrite();
			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ItemCol = ABLUE;
			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		}
		sAutoReturnLastMenu = 1;
	}
}
//绿色
void SystemSetFontColourGreenFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		ParamSetEndDisplayPage(1,1);
		_gTaskShareDatObj.DataBuf[0] = 1;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.DataBuf[0])
		{
			AppDataWrite((int32_t)AGREEN, APP_SystemFontColour);
			APPDataFlashWrite();
			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ItemCol = ABLUE;
			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		}
		sAutoReturnLastMenu = 1;
	}
}

//紫色
void SystemSetFontColourVioletFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		_gTaskShareDatObj.DataBuf[0] = 0;
		
		ParamSetEndDisplayPage(1,1);
		_gTaskShareDatObj.DataBuf[0] = 1;
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		if(_gTaskShareDatObj.DataBuf[0])
		{
			AppDataWrite((int32_t)Violet, APP_SystemFontColour);
			APPDataFlashWrite();
			MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ItemCol = ABLUE;
			MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
			MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
			MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
		}
		sAutoReturnLastMenu = 1;
	}
}

/*******通道1测试菜单目录执行功能函数*******/
//1通道输出开启
void SystemSetOutput1TestOpenFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LED_Ctrl(LED1,1);
		NpnOut_Ctrl(NPN1,1);
		
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		_gTaskShareDatObj.State = 0;
		sAutoReturnLastMenu = 1;
	}
}
//1通道输出关闭
void SystemSetOutput1TestCloseFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LED_Ctrl(LED1,0);
		NpnOut_Ctrl(NPN1,0);
		
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		_gTaskShareDatObj.State = 0;
		sAutoReturnLastMenu = 1;
	}
}

/*******通道2测试菜单目录执行功能函数*******/
//2通道输出开启
void SystemSetOutput2TestOpenFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LED_Ctrl(LED2,1);
		NpnOut_Ctrl(NPN2,1);
		
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		_gTaskShareDatObj.State = 0;
		sAutoReturnLastMenu = 1;
	}
}
//2通道输出关闭
void SystemSetOutput2TestCloseFuntion(void *param)
{
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LED_Ctrl(LED2,0);
		NpnOut_Ctrl(NPN2,0);
		
		ParamSetEndDisplayPage(1,1);
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
		_gTaskShareDatObj.TaskTick = GetSystemTick();
	}
	
	if((_gTaskShareDatObj.State & 0x80) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		_gTaskShareDatObj.State = 0;
		sAutoReturnLastMenu = 1;
	}
}

/*******电流校准菜单目录执行功能函数*******/
//4mA校准
void CurrentsCalibration_4mAFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
//			LCD_ShowString(59,1,(uint8_t*)"1V ", MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		LCD_DrawRectangle(37,54,56,73, MenuInterfaceColObj.ParamSetFontCol);
		LCD_ShowChinese(39,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,63);
		LCD_DrawRectangle(123,54,142,73, MenuInterfaceColObj.ParamSetFontCol);
		LCD_ShowChinese(125,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,64);
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		//
		_gTaskShareDatObj.DataBuf[0] = 0;
		_gTaskShareDatObj.TempVal_U16[0] = AppDataRead(APP_CurrentOutCalibrationVal_4MA);
		//刷新一次显示
		_gTaskShareDatObj.DataBuf[1] = 1;
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(UP, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			if((_gTaskShareDatObj.TempVal_U16[0] + 2) <= 9000)
			{
				_gTaskShareDatObj.TempVal_U16[0] += 2;
				_gTaskShareDatObj.DataBuf[1] = 1;
				
				_gTaskShareDatObj.State &= ~0x02;
				_gTaskShareDatObj.State |= 0x01;
				_gTaskShareDatObj.TempVal_U32[0] = GetSystemTick();
			}
			LCD_ShowChinese(39,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,63);
		}
		else if(KEY_ReadEvent(UP, Press_END, 1))
		{
			_gTaskShareDatObj.State &= ~0x01;
			LCD_ShowChinese(39,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,63);
		}
		
		if(KEY_ReadEvent(DOWN, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			if((_gTaskShareDatObj.TempVal_U16[0] - 2) >= 0)//////////
			{
				_gTaskShareDatObj.TempVal_U16[0] -= 2;
				_gTaskShareDatObj.DataBuf[1] = 1;
				
				_gTaskShareDatObj.State &= ~0x01;
				_gTaskShareDatObj.State |= 0x02;
				_gTaskShareDatObj.TempVal_U32[0] = GetSystemTick();
			}
			LCD_ShowChinese(125,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,64);
		}
		else if(KEY_ReadEvent(DOWN, Press_END, 1))
		{
			_gTaskShareDatObj.State &= ~0x02;
			LCD_ShowChinese(125,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,64);
		}
		
		if((_gTaskShareDatObj.State & 0x03) && (GetSystemTick() - _gTaskShareDatObj.TempVal_U32[0] > 1000))
		{
			if(_gTaskShareDatObj.State & 0x01)
			{
				if((_gTaskShareDatObj.TempVal_U16[0] + 50) <= 9000)
				{
					_gTaskShareDatObj.TempVal_U16[0] += 50;
				}
				_gTaskShareDatObj.State &= ~0x01;
			}
			else if(_gTaskShareDatObj.State & 0x02)
			{
				if((_gTaskShareDatObj.TempVal_U16[0] - 50) >= 200)
				{
					_gTaskShareDatObj.TempVal_U16[0] -= 50;
				}
				_gTaskShareDatObj.State &= ~0x02;
			}
			_gTaskShareDatObj.DataBuf[1] = 1;
		}
		
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			CurrentOutput_PwmValueSet(0);
			sAutoReturnLastMenu = 1;
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0] & 0x80)
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
		//
		if(_gTaskShareDatObj.DataBuf[1])
		{
			LCD_ShowIntNum(67,25,_gTaskShareDatObj.TempVal_U16[0],4,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,24,1);
			CurrentOutput_PwmValueSet(_gTaskShareDatObj.TempVal_U16[0]);
			_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		CurrentOutput_PwmValueSet(0);
		AppDataWrite(_gTaskShareDatObj.TempVal_U16[0], APP_CurrentOutCalibrationVal_4MA);
//		AppDataWrite(_gTaskShareDatObj.TempVal_U16[11], APP_CurrentOutCalibrationVal_20MA);
		APPDataFlashWrite();
		CurrentOutput_UpdateCalcData();
		sAutoReturnLastMenu = 1;
	}
}
//20mA校准
void CurrentsCalibration_20mAFuntion(void *param)
{
	struct MenuItem * gItemTemp = (struct MenuItem *)param;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		sMenuFlag &= ~0x60;
		LCD_Fill(22,0,LCD_W,18,MenuInterfaceColObj.ParamSetCatalogCol);
		if(MenuLanguage == Chinese)
		{
//			LCD_ShowString(55,1,(uint8_t*)"5V ", MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1);
			LCD_ShowChinese(74,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[0]);
			LCD_ShowChinese(90,1,MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,16,1,gItemTemp->PresentContent[MenuLanguage].font_pos[1]);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(gItemTemp->PresentContent[MenuLanguage].dis_start_pos,1,gItemTemp->PresentContent[MenuLanguage].str,			\
						   MenuInterfaceColObj.ParamSetCatalogFontCol,MenuInterfaceColObj.ParamSetCatalogFontCol,gItemTemp->PresentContent[MenuLanguage].chr_size,1);
		}
		
		LCD_Fill(22, 18, LCD_W, LCD_H, MenuInterfaceColObj.ParamSetBackCol);
		if(MenuLanguage == Chinese)
		{
			LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
			LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
		}
		else if(MenuLanguage == English)
		{
			LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
		}
		
		LCD_DrawRectangle(37,54,56,73, MenuInterfaceColObj.ParamSetFontCol);
		LCD_ShowChinese(39,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,63);
		LCD_DrawRectangle(123,54,142,73, MenuInterfaceColObj.ParamSetFontCol);
		LCD_ShowChinese(125,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,64);
		
		MeterInterfaceKeyShield(FunctionKey_Disbale);
		KEY_ReadEvent(ENTER, Press_END, 1);
		KEY_ReadEvent(UP, Press_END, 1);
		KEY_ReadEvent(DOWN, Press_END, 1);
		
		//
		_gTaskShareDatObj.DataBuf[0] = 0;
		_gTaskShareDatObj.TempVal_U16[0] = AppDataRead(APP_CurrentOutCalibrationVal_20MA);
		//刷新一次显示
		_gTaskShareDatObj.DataBuf[1] = 1;
		
		sFunctionQuit = 1;
		sFunctionExecute = 1;
		_gTaskShareDatObj.State |= 0x80;
	}
	
	if((_gTaskShareDatObj.State & 0xC0) == 0x80)
	{
		if(KEY_ReadEvent(UP, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			if((_gTaskShareDatObj.TempVal_U16[0] + 2) <= 9000)
			{
				_gTaskShareDatObj.TempVal_U16[0] += 2;
				_gTaskShareDatObj.DataBuf[1] = 1;
				
				_gTaskShareDatObj.State &= ~0x02;
				_gTaskShareDatObj.State |= 0x01;
				_gTaskShareDatObj.TempVal_U32[0] = GetSystemTick();
			}
			LCD_ShowChinese(39,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,63);
		}
		else if(KEY_ReadEvent(UP, Press_END, 1))
		{
			_gTaskShareDatObj.State &= ~0x01;
			LCD_ShowChinese(39,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,63);
		}
		
		if(KEY_ReadEvent(DOWN, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			if((_gTaskShareDatObj.TempVal_U16[0] - 2) >= 200)
			{
				_gTaskShareDatObj.TempVal_U16[0] -= 2;
				_gTaskShareDatObj.DataBuf[1] = 1;
				
				_gTaskShareDatObj.State &= ~0x01;
				_gTaskShareDatObj.State |= 0x02;
				_gTaskShareDatObj.TempVal_U32[0] = GetSystemTick();
			}
			LCD_ShowChinese(125,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,64);
		}
		else if(KEY_ReadEvent(DOWN, Press_END, 1))
		{
			_gTaskShareDatObj.State &= ~0x02;
			LCD_ShowChinese(125,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,64);
		}
		
		if((_gTaskShareDatObj.State & 0x03) && (GetSystemTick() - _gTaskShareDatObj.TempVal_U32[0] > 1000))
		{
			if(_gTaskShareDatObj.State & 0x01)
			{
				if((_gTaskShareDatObj.TempVal_U16[0] + 50) <= 9000)
				{
					_gTaskShareDatObj.TempVal_U16[0] += 50;
				}
				_gTaskShareDatObj.State &= ~0x01;
			}
			else if(_gTaskShareDatObj.State & 0x02)
			{
				if((_gTaskShareDatObj.TempVal_U16[0] - 50) >= 200)
				{
					_gTaskShareDatObj.TempVal_U16[0] -= 50;
				}
				_gTaskShareDatObj.State &= ~0x02;
			}
			_gTaskShareDatObj.DataBuf[1] = 1;
		}
		
		if(KEY_ReadEvent(ENTER, Short_Press_Once, 1))
		{
			sMenuFlag &= ~0x60;
			_gTaskShareDatObj.DataBuf[0] |= 0x80;
			
			if(MenuLanguage == Chinese)
			{
				LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,55);
				LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0,56);
			}
			else if(MenuLanguage == English)
			{
				LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetBackCol,MenuInterfaceColObj.ParamSetFontCol,16,0);
			}
		}
		else if(KEY_ReadEvent(ENTER, Long_Press_Once, 1))
		{
			CurrentOutput_PwmValueSet(0);
			sAutoReturnLastMenu = 1;
		}
		else if(KEY_ReadEvent(ENTER, Press_END, 1))
		{
			if(_gTaskShareDatObj.DataBuf[0] & 0x80)
			{
				_gTaskShareDatObj.State |= 0x40;
				
				if(MenuLanguage == Chinese)
				{
					LCD_ShowChinese(74,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,55);
					LCD_ShowChinese(90,56,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0,56);
				}
				else if(MenuLanguage == English)
				{
					LCD_ShowString(70,56,(uint8_t *)"ENTER",MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,16,0);
				}
				
				ParamSetEndDisplayPage(1,0);
				_gTaskShareDatObj.TaskTick = GetSystemTick();
			}
			else
				_gTaskShareDatObj.DataBuf[0] = 0;
		}
		//
		if(_gTaskShareDatObj.DataBuf[1])
		{
			LCD_ShowIntNum(67,25,_gTaskShareDatObj.TempVal_U16[0],4,MenuInterfaceColObj.ParamSetFontCol,MenuInterfaceColObj.ParamSetBackCol,24,1);
			CurrentOutput_PwmValueSet(_gTaskShareDatObj.TempVal_U16[0]);
			_gTaskShareDatObj.DataBuf[1] = 0;
		}
	}
	
	if((_gTaskShareDatObj.State & 0x40) && (GetSystemTick() - _gTaskShareDatObj.TaskTick > FunctionSetParamDisTick))
	{
		CurrentOutput_PwmValueSet(0);
//		AppDataWrite(_gTaskShareDatObj.TempVal_U16[0], APP_CurrentOutCalibrationVal_4MA);
		AppDataWrite(_gTaskShareDatObj.TempVal_U16[0], APP_CurrentOutCalibrationVal_20MA);
		APPDataFlashWrite();
		CurrentOutput_UpdateCalcData();
		sAutoReturnLastMenu = 1;
	}
}		

/*************************************************************************Menu Item Msp Function End*************************************************************************/



/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void ParamSetEndDisplayPage(uint8_t sta, uint8_t scene)
{	
	uint8_t Y_Pos = 0;
	
	if(MenuLanguage == Chinese)
	{
		if(scene)	//在外显示
		{
			Y_Pos = 41;
		}
		else		//在内显示
		{
			Y_Pos = 28;
		}
		//
		if(sta)
		{
			LCD_ShowChinese(43,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,6);
			LCD_ShowChinese(67,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,7);
			LCD_ShowChinese(91,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,96);
			LCD_ShowChinese(115,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,97);
		}
		else
		{
			LCD_ShowChinese(43,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,15);
			LCD_ShowChinese(67,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,16);
			LCD_ShowChinese(91,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,98);
			LCD_ShowChinese(115,Y_Pos,MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0,99);
		}
	}
	else if(MenuLanguage == English)
	{
		if(sta)
			LCD_ShowString(49,31,(uint8_t *)"Succeed",MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0);
		else
			LCD_ShowString(67,31,(uint8_t *)"Fail",MenuInterfaceColObj.ParamSetBackCol,BLUE,24,0);
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void MenuExtraInformationDisplayTask(struct MenuItem *item_obj, uint8_t menu_page)
{
	uint8_t page = menu_page/MenuItemDisMax;
	
	//当前菜单为背景颜色或字体颜色
//	if((item_obj->MenuID == 13) || (item_obj->MenuID == 14))
	if(item_obj->MenuID == 14)
	{
		if((sMenuExtraInfoFlag & 0x80) == 0x00)
		{
			if(MenuLanguage == Chinese)
			{
				LCD_Fill(134, MenuCoordObj.ItemFrameStart, 149, LCD_H, MenuInterfaceColObj.BackCol);
				MenuCoordObj.ItemAreaRefEnd = 134;
			}
			else if(MenuLanguage == English)
			{				
				LCD_Fill(142, MenuCoordObj.ItemFrameStart, LCD_W, LCD_H, MenuInterfaceColObj.BackCol);
				MenuCoordObj.ItemAreaRefEnd = 142;
			}
			
			sMenuExtraInfoFlag |= 0x80;
		}
		
		if(MenuLanguage == Chinese)
		{
			switch(page)
			{
				case 0:	
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 24, AWHITE);
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 24, Azure);
				break;
				
				case 1:
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 24, ORANGE);
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 24, YELLOW);
				break;
				
				case 2:
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 24, AGREEN);
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 24, Violet);
				break;
				
				case 3:
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 24, MenuInterfaceColObj.BackCol);
					LCD_Fill(134, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 24, MenuInterfaceColObj.BackCol);
				break;
				
				default: break;
			}
		}
		else if(MenuLanguage == English)
		{
			switch(page)
			{
				case 0:	
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 16, AWHITE);
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 16, Azure);
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y + 2*MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + 2*MenuCoordObj.ItemFrame_W + 16, ORANGE);
				break;
				
				case 1:
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 16, YELLOW);
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 16, AGREEN);
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y + 2*MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + 2*MenuCoordObj.ItemFrame_W + 16, Violet);
				break;
				
				case 2:
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y, 158, MenuCoordObj.ItemFontStart_Y + 16, MenuInterfaceColObj.BackCol);
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + MenuCoordObj.ItemFrame_W + 16, MenuInterfaceColObj.BackCol);
					LCD_Fill(142, MenuCoordObj.ItemFontStart_Y + 2*MenuCoordObj.ItemFrame_W, 158, MenuCoordObj.ItemFontStart_Y + 2*MenuCoordObj.ItemFrame_W + 16, MenuInterfaceColObj.BackCol);
				break;
				
				default: break;
			}
		}
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static uint8_t MenuItemFrameEndCalcuate(struct MenuItem *item_obj)
{
	uint8_t i = 0, str_offset, end_pos;
	str_offset = item_obj->PresentContent[MenuLanguage].chr_size >> 1;
	end_pos = item_obj->PresentContent[MenuLanguage].dis_start_pos;
	
	while (item_obj->PresentContent[MenuLanguage].str[i] != '\0')
	{
		if(item_obj->PresentContent[MenuLanguage].str[i] >= 0x80)
		{
			end_pos += item_obj->PresentContent[MenuLanguage].font_size;
		}
		else
		{
			end_pos += str_offset;
		}
		i++;
	}
	return end_pos;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void MenuCatalogMessagePrint(struct MenuItem *item_obj)
{
	uint8_t j= 0;
	uint8_t Pos = 0;
	uint8_t FontBuff = 0;
	
	Pos = item_obj->PresentContent[MenuLanguage].dis_start_pos;
	LCD_Fill(SystemDisplay_X,0,LCD_W,MenuCoordObj.Catalog_W,MenuInterfaceColObj.CatalogMiddleCol);
	if(MenuLanguage == Chinese)
	{
		LCD_DrawLine(SystemDisplay_X,25,LCD_W,25,MenuInterfaceColObj.FontCol);
		while(item_obj->PresentContent[MenuLanguage].str[j] != '\0')
		{
			FontBuff = item_obj->PresentContent[MenuLanguage].str[j];
			if(FontBuff >= 0x80)
			{
				LCD_ShowChinese(Pos, 1, MenuInterfaceColObj.FontCol,MenuInterfaceColObj.FontCol, item_obj->PresentContent[MenuLanguage].font_size, 1,	\
								item_obj->PresentContent[MenuLanguage].font_pos[FontBuff&0x7F]);
				Pos += item_obj->PresentContent[MenuLanguage].font_size;
			}
			else
			{
				LCD_ShowChar(Pos, 1, FontBuff, MenuInterfaceColObj.FontCol,MenuInterfaceColObj.FontCol, item_obj->PresentContent[MenuLanguage].chr_size, 1);
				Pos += item_obj->PresentContent[MenuLanguage].chr_size >> 1;
			}
			j++;
		}
	}
	else if(MenuLanguage == English)
	{
		LCD_DrawLine(SystemDisplay_X,17,LCD_W,17,MenuInterfaceColObj.FontCol);
		LCD_ShowString(item_obj->PresentContent[MenuLanguage].dis_start_pos, 1, item_obj->PresentContent[MenuLanguage].str, MenuInterfaceColObj.FontCol,	\
					   MenuInterfaceColObj.FontCol, item_obj->PresentContent[MenuLanguage].chr_size, 1);
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
static void Menu_Display(void)
{
	uint8_t i,j = 0;
	uint8_t page_offset,ref_flag;
	uint8_t item_end_pos = 0, dis_offset = 0, font_buf = 0, chr_offset = 0;
	struct MenuItem *ItemObj = NULL;
	
	page_offset = vSelectItemNum/MenuItemDisMax;
	if(page_offset != vOldPage)	sMenuTaskRefEnable = 1;		//页切换，刷新一遍子目录
	vOldPage = page_offset;
	page_offset = page_offset*MenuItemDisMax;
	ref_flag = vSelectItemNum%MenuItemDisMax;
	ItemObj = (gSelectMenuObj + vSelectItemNum);
	
	if(sMenuTaskRefEnable)	//刷新子目录
	{
		LCD_Fill(SystemDisplay_X, MenuCoordObj.Catalog_W, MenuCoordObj.ItemAreaRefEnd, LCD_H, MenuInterfaceColObj.BackCol);
		MenuExtraInformationDisplayTask(ItemObj, page_offset);
		sMenuTaskRefEnable = 0;
	}
	
	item_end_pos = MenuItemFrameEndCalcuate(ItemObj);
	
	//生成条目框
	if(ref_flag == vOldItemPos)
	{
		LCD_Fill(ItemObj->PresentContent[MenuLanguage].dis_start_pos-3, MenuCoordObj.ItemFrameStart+ref_flag*MenuCoordObj.ItemFrame_W,	\
				 item_end_pos+4, MenuCoordObj.ItemFrameStart+MenuCoordObj.ItemFrame_W+ref_flag*MenuCoordObj.ItemFrame_W, MenuInterfaceColObj.ItemCol);
	}
	else
	{
		LCD_Fill(SystemDisplay_X, MenuCoordObj.ItemFrameStart+vOldItemPos*MenuCoordObj.ItemFrame_W, MenuCoordObj.ItemAreaRefEnd,	\
				 MenuCoordObj.ItemFrameStart+MenuCoordObj.ItemFrame_W+vOldItemPos*MenuCoordObj.ItemFrame_W, MenuInterfaceColObj.BackCol);
		
		LCD_Fill(ItemObj->PresentContent[MenuLanguage].dis_start_pos-3, MenuCoordObj.ItemFrameStart+ref_flag*MenuCoordObj.ItemFrame_W,	\
				 item_end_pos+4, MenuCoordObj.ItemFrameStart+MenuCoordObj.ItemFrame_W+ref_flag*MenuCoordObj.ItemFrame_W, MenuInterfaceColObj.ItemCol);
		
		vOldItemPos = ref_flag;
	}
	//条目内容显示
	ItemObj = (gSelectMenuObj + page_offset);
	for(i=0; i<MenuItemDisMax; i++)
	{
		dis_offset = (ItemObj + i)->PresentContent[MenuLanguage].dis_start_pos;
		//计算字符显示纵偏移
		if(((ItemObj + i)->PresentContent[MenuLanguage].chr_size) && ((ItemObj + i)->PresentContent[MenuLanguage].chr_size < (ItemObj + i)->PresentContent[MenuLanguage].font_size))
		{
			chr_offset = (ItemObj + i)->PresentContent[MenuLanguage].font_size - (ItemObj + i)->PresentContent[MenuLanguage].chr_size;
			chr_offset >>= 1;
		}
		while((ItemObj + i)->PresentContent[MenuLanguage].str[j] != '\0')
		{
			font_buf = (ItemObj +i)->PresentContent[MenuLanguage].str[j];
			if(font_buf >= 0x80)
			{
				if(i != ref_flag)
					LCD_ShowChinese(dis_offset, MenuCoordObj.ItemFontStart_Y+i*MenuCoordObj.ItemFrame_W, MenuInterfaceColObj.FontCol, MenuInterfaceColObj.BackCol,		\
									(ItemObj + i)->PresentContent[MenuLanguage].font_size, 0, (ItemObj + i)->PresentContent[MenuLanguage].font_pos[font_buf&0x7F]);
				else
					LCD_ShowChinese(dis_offset, MenuCoordObj.ItemFontStart_Y+i*MenuCoordObj.ItemFrame_W, MenuInterfaceColObj.BackCol, MenuInterfaceColObj.FontCol,		\
									(ItemObj + i)->PresentContent[MenuLanguage].font_size, 1, (ItemObj + i)->PresentContent[MenuLanguage].font_pos[font_buf&0x7F]);
				dis_offset += (ItemObj + i)->PresentContent[MenuLanguage].font_size;
			}
			else
			{
				if(i != ref_flag)
					LCD_ShowChar(dis_offset, chr_offset+MenuCoordObj.ItemFontStart_Y+i*MenuCoordObj.ItemFrame_W, font_buf, MenuInterfaceColObj.FontCol, MenuInterfaceColObj.BackCol,	\
								(ItemObj + i)->PresentContent[MenuLanguage].chr_size, 0);
				else
					LCD_ShowChar(dis_offset, chr_offset+MenuCoordObj.ItemFontStart_Y+i*MenuCoordObj.ItemFrame_W, font_buf, MenuInterfaceColObj.BackCol, MenuInterfaceColObj.FontCol,	\
								(ItemObj + i)->PresentContent[MenuLanguage].chr_size, 1);
				dis_offset += (ItemObj + i)->PresentContent[MenuLanguage].chr_size >> 1;
			}
			j++;
		}
		j = 0;
			
		if((page_offset + i + 1) >= (ItemObj + i)->MenuNum)
			break;
	}
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void MeterInterfaceKeyShield(uint8_t key_shield)
{
	sFunctionKeyShield = key_shield;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void MeterInterfaceInit(void)
{
	if(sMenuFlag & 0x08)
		sMenuFlag |= 0x80;
	else
		return;
	
	MenuLanguage = AppDataRead(APP_SystemLanguage);
	
	MenuInterfaceColObj.CatalogMiddleCol = AppDataRead(APP_SystemBackColour);
	MenuInterfaceColObj.ItemCol = ABLUE;
	MenuInterfaceColObj.BackCol = AppDataRead(APP_SystemBackColour);
	MenuInterfaceColObj.FontCol =  AppDataRead(APP_SystemFontColour);
	MenuInterfaceColObj.ParamSetCatalogCol = AppDataRead(APP_SystemFontColour);
	MenuInterfaceColObj.ParamSetCatalogFontCol = AppDataRead(APP_SystemBackColour);
	MenuInterfaceColObj.ParamSetFontCol = AppDataRead(APP_SystemFontColour);
	MenuInterfaceColObj.ParamSetBackCol = AppDataRead(APP_SystemBackColour);
	if(MenuLanguage == Chinese)
	{
		MenuItemDisMax = 2;
		MenuCoordObj.Catalog_W = 26;
		MenuCoordObj.ItemAreaRefEnd = 160;
		MenuCoordObj.ItemFrameStart = 28;
		MenuCoordObj.ItemFrame_W = 26;
		MenuCoordObj.ItemFontStart_Y = 29;
		
		LCD_Fill(SystemDisplay_X,0,LCD_W,26,MenuInterfaceColObj.CatalogMiddleCol);
		LCD_DrawLine(SystemDisplay_X,25,LCD_W,25,MenuInterfaceColObj.FontCol);
		LCD_ShowChinese(55,1,MenuInterfaceColObj.FontCol,MenuInterfaceColObj.FontCol,24,1,12);
		LCD_ShowChinese(79,1,MenuInterfaceColObj.FontCol,MenuInterfaceColObj.FontCol,24,1,13);
		LCD_ShowChinese(103,1,MenuInterfaceColObj.FontCol,MenuInterfaceColObj.FontCol,24,1,14);
		LCD_Fill(MenuCoordObj.ItemAreaRefEnd, MenuCoordObj.Catalog_W, LCD_W, LCD_H, MenuInterfaceColObj.BackCol);
	}
	else if(MenuLanguage == English)
	{
		MenuItemDisMax = 3;
		MenuCoordObj.Catalog_W = 18;
		MenuCoordObj.ItemAreaRefEnd = LCD_W;
		MenuCoordObj.ItemFrameStart = 20;
		MenuCoordObj.ItemFrame_W = 20;
		MenuCoordObj.ItemFontStart_Y = 22;
		
		LCD_Fill(SystemDisplay_X,0,LCD_W,18,MenuInterfaceColObj.CatalogMiddleCol);
		LCD_DrawLine(SystemDisplay_X,17,LCD_W,17,MenuInterfaceColObj.FontCol);
		LCD_ShowString(55,1,(uint8_t *)"Main Menu",MenuInterfaceColObj.FontCol,MenuInterfaceColObj.FontCol,16,1);
		LCD_Fill(MenuCoordObj.ItemAreaRefEnd, MenuCoordObj.Catalog_W, LCD_W, LCD_H, MenuInterfaceColObj.BackCol);
	}
	
	sMenuRefEnable = 1;
	sMenuTaskRefEnable = 1;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t MeterInterfaceTask(void)
{
	uint8_t gItemNumTemp = 0;
	struct MenuItem *gSelectMenuTemp = NULL;
	
	if((sMenuFlag & 0x80) == 0x00) return 1;
	
	if((sFunctionKeyShield & Up_Enanble) && (KEY_ReadEvent(UP, Short_Press_Once, 1)))		//方向键（上）
	{
		if(vSelectItemNum == 0)
		{
			if(gSelectMenuObj->MenuNum > MenuItemDisMax)		//换页
			{
				vSelectItemNum = (gSelectMenuObj->MenuNum - 1) / MenuItemDisMax;
				vSelectItemNum *= MenuItemDisMax;
				sMenuTaskRefEnable = 1;
			}
			else
			{
//				if(gSelectMenuObj->MenuNum % MenuItemDisMax)
//					vSelectItemNum = gSelectMenuObj->MenuNum % MenuItemDisMax - 1;
//				else
//					vSelectItemNum = gSelectMenuObj->MenuNum - 1;
				
				vSelectItemNum = gSelectMenuObj->MenuNum - 1;
			}
		}
		else
			vSelectItemNum --;
		
		sMenuRefEnable = 1;
	}
	else if((sFunctionKeyShield & Down_Enanble) && (KEY_ReadEvent(DOWN, Short_Press_Once, 1)))		//方向键（下）
	{
		if(++vSelectItemNum >= gSelectMenuObj->MenuNum)
		{
			vSelectItemNum = 0;
		}
		sMenuRefEnable = 1;
	}
	else if((sFunctionKeyShield & Enter_Enanble) && (KEY_ReadEvent(ENTER, Short_Press_Once, 1)))		//确认键
	{
//		if((gSelectMenuObj + vSelectItemNum)->Function)
//		{
//			(gSelectMenuObj + vSelectItemNum)->Function(gSelectMenuObj + vSelectItemNum);
//		}
//		if((gSelectMenuObj + vSelectItemNum)->ChildrenMenu_t)
//		{
//			vItemNumBuf[(gSelectMenuObj + vSelectItemNum)->MenuID] = vSelectItemNum;
//			gSelectMenuObj = (gSelectMenuObj + vSelectItemNum)->ChildrenMenu_t;
//			
//			if(vManualItemNum)
//			{
//				vSelectItemNum = vManualItemNum;
//				vManualItemNum = 0;
//			}
//			else
//				vSelectItemNum = 0;
//			
//			vOldItemPos = 0;
//			vOldPage = 0;
//			sMenuRefEnable = 1;
//			sMenuTaskRefEnable = 1;
//		}

		if((gSelectMenuObj + vSelectItemNum)->Function)
		{
			(gSelectMenuObj + vSelectItemNum)->Function(gSelectMenuObj + vSelectItemNum);
		}
		if((gSelectMenuObj + vSelectItemNum)->ChildrenMenu_t)
		{
			vItemNumBuf[vItemDepthCnt] = vSelectItemNum;
			vItemDepthCnt++;
			
			gSelectMenuObj = (gSelectMenuObj + vSelectItemNum)->ChildrenMenu_t;
			
			if(vManualItemNum)
			{
				vSelectItemNum = vManualItemNum;
				vManualItemNum = 0;
			}
			else
				vSelectItemNum = 0;
			
			vOldItemPos = 0;
			vOldPage = 0;
			sMenuRefEnable = 1;
			sMenuTaskRefEnable = 1;
		}
	}
	
	//返回上一菜单
	if(sAutoReturnLastMenu)
	{
//		if(sFunctionQuit)	//从回调函数退出，回到进入页面状态
//		{
//			gSelectMenuTemp = gSelectMenuObj->ParentMenu_t;
//			gItemNumTemp = vItemNumBuf[gSelectMenuTemp->MenuID];
//			if((gSelectMenuTemp + gItemNumTemp)->Function)
//			{
//				(gSelectMenuTemp + gItemNumTemp)->Function(gSelectMenuTemp + gItemNumTemp);
//			}
//			
//			vOldPage = 0;
//			sMenuRefEnable = 1;
//			sMenuTaskRefEnable = 1;
//			sFunctionQuit = 0;
//		}
//		else
//		{
//			if((gSelectMenuObj + vSelectItemNum)->ParentMenu_t)		//是否有父菜单
//			{	
//				gSelectMenuObj = (gSelectMenuObj + vSelectItemNum)->ParentMenu_t;		
//				vSelectItemNum = vItemNumBuf[gSelectMenuObj->MenuID];
//				
//				if((gSelectMenuObj + vSelectItemNum)->MenuID == 0)	//主界面
//				{
//					MeterInterfaceInit();
//				}
//				else
//				{
//					gSelectMenuTemp = gSelectMenuObj->ParentMenu_t;
//					gItemNumTemp = vItemNumBuf[gSelectMenuTemp->MenuID];
//					if((gSelectMenuTemp + gItemNumTemp)->Function)
//					{
//						(gSelectMenuTemp + gItemNumTemp)->Function(gSelectMenuTemp + gItemNumTemp);
//					}
//				}
//				vOldPage = 0;
//				sMenuRefEnable = 1;
//				sMenuTaskRefEnable = 1;
//			}
//			else	//退回初始界面
//			{
//				return 1;
//			}
//		}
		
		if(sFunctionQuit)	//从回调函数退出，回到进入页面状态
		{
			gSelectMenuTemp = gSelectMenuObj->ParentMenu_t;
			gItemNumTemp = vItemNumBuf[vItemDepthCnt - 1];
			
			if((gSelectMenuTemp + gItemNumTemp)->Function)
			{
				(gSelectMenuTemp + gItemNumTemp)->Function(gSelectMenuTemp + gItemNumTemp);
			}
			
			vOldPage = 0;
			sMenuRefEnable = 1;
			sMenuTaskRefEnable = 1;
			sFunctionQuit = 0;
		}
		else
		{
			if((gSelectMenuObj + vSelectItemNum)->ParentMenu_t)		//是否有父菜单
			{	
				gSelectMenuObj = (gSelectMenuObj + vSelectItemNum)->ParentMenu_t;
				vItemDepthCnt--;
				vSelectItemNum = vItemNumBuf[vItemDepthCnt];
				
				if((gSelectMenuObj + vSelectItemNum)->MenuID == 0)	//主界面
				{
					MeterInterfaceInit();
				}
				else
				{
					gSelectMenuTemp = gSelectMenuObj->ParentMenu_t;
					gItemNumTemp = vItemNumBuf[vItemDepthCnt - 1];
					if((gSelectMenuTemp + gItemNumTemp)->Function)
					{
						(gSelectMenuTemp + gItemNumTemp)->Function(gSelectMenuTemp + gItemNumTemp);
					}
				}
				vOldPage = 0;
				sMenuRefEnable = 1;
				sMenuTaskRefEnable = 1;
			}
			else	//退回初始界面
			{
				return 1;
			}
		}
		
		if(vManualItemNum)
			vManualItemNum = 0;
		
		if(sFunctionExecute)
		{
			sFunctionExecute = 0;
			KEY_ReadEvent(UP, Short_Press_Once, 1);
			KEY_ReadEvent(DOWN, Short_Press_Once, 1);
			KEY_ReadEvent(ENTER, Short_Press_Once, 1);
			MeterInterfaceKeyShield(Up_Enanble|Enter_Enanble|Down_Enanble);
//			ParamInputTaskDelete();
			memset(&_gTaskShareDatObj, 0, sizeof(_gTaskShareDatObj));
		}
		vOldPage = 0;
		sMenuRefEnable = 1;
		sAutoReturnLastMenu = 0;
		
		LCD_Fill(MenuCoordObj.ItemAreaRefEnd, MenuCoordObj.Catalog_W, LCD_W, LCD_H, MenuInterfaceColObj.BackCol);
		if(sMenuExtraInfoFlag & 0x80)
		{
			if(MenuLanguage == Chinese)
				MenuCoordObj.ItemAreaRefEnd = 149;
			else if(MenuLanguage == English)	
				MenuCoordObj.ItemAreaRefEnd = LCD_W;
			sMenuExtraInfoFlag = 0;
		}
	}
	//刷新显示
	if(sMenuRefEnable)
	{
		Menu_Display();
		sMenuRefEnable = 0;
		sMenuFlag &= ~0x60;
	}
	else
	{
		//参数设置任务已开启
		if(ParamSetTaskReady())
		{
			if(ParamSetTaskIdleQuery())	//空闲无操作中
				sMenuFlag |= 0x40;
			else
				sMenuFlag &= ~0x60;
		}
		else
		{
			//菜单无操作中
			sMenuFlag |= 0x40;
		}
	}
	//菜单条目回调函数执行
	if(sFunctionExecute)
	{
		if((gSelectMenuObj + vSelectItemNum)->Function)
			(gSelectMenuObj + vSelectItemNum)->Function(gSelectMenuObj + vSelectItemNum);
	}
	//菜单退出Tick记时标志
	if(((sMenuFlag & 0x20) == 0x00) && (sMenuFlag & 0x40))
	{
		sMenuFlag |= 0x20;
		vMenuTaskIdleTick = GetSystemTick();
	}
	//空闲退出时间到
	if((sMenuFlag & 0x20) && (GetSystemTick() - vMenuTaskIdleTick > MenuTaskIdleExitTick))
		return 1;
	else
		return 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void MeterInterfaceTaskQuit(void)
{
	sMenuFlag &= 0x0F;
	vItemDepthCnt = 0;
	sMenuExtraInfoFlag = 0;
	vSelectItemNum = 0;
	gSelectMenuObj = &MainMenu[0];
	sMenuRefEnable = 0;
	sFunctionExecute = 0;
	vOldItemPos = 0;
	vOldPage = 0;
	vManualItemNum = 0;
	sFunctionQuit = 0;
	sAutoReturnLastMenu = 0;
	sFunctionKeyShield = FunctionKey_Disbale;
	vMenuTaskIdleTick = 0;
	ParamSetTaskDelete();
	memset(vItemNumBuf, 0, sizeof(vItemNumBuf));
	memset(&_gTaskShareDatObj, 0, sizeof(_gTaskShareDatObj));
	KEY_ReadEvent(ENTER,Press_END,1);
	KEY_ReadEvent(UP,Press_END,1);
	KEY_ReadEvent(DOWN,Press_END,1);
	LED_Ctrl(LED1,0);	LED_Ctrl(LED2,0);
	NpnOut_Ctrl(NPN1,0);	NpnOut_Ctrl(NPN2,0);
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
uint8_t ExternalRefVolTaskReady(uint16_t *adc_val, uint16_t *vol_val)
{
	if(sMenuFlag & 0x08)
	{
		*adc_val = _gTaskShareDatObj.TempVal_U32[0];
		*vol_val = _gTaskShareDatObj.TempVal_U32[1];
		memset(&_gTaskShareDatObj, 0, sizeof(_gTaskShareDatObj));
		return 1;
	}
	else
		return 0;
}

/****************************************
 *函数名称：
 *功能：
 *参数说明：
 ****************************************/
void ExternalReferenceVolGetTask(void)
{
	uint8_t i;
	int16_t calibrate_val = 0;
	uint16_t min = 0,max = 0;
	
	if((_gTaskShareDatObj.State & 0x80) == 0x00)
	{
		//读取ADC当前补偿值
		calibrate_val = AppDataRead(APP_ADCxCompensationVal);
		//设置ADC检测端口为内部基准电压
//		ADC_Bsp_Init(InternalVccTestPort);
		ADC_Channelx_Init(InternalVccTestPort);
		_gTaskShareDatObj.State |= 0xC0;
	}
	//
	if(_gTaskShareDatObj.State & 0x40)
	{
		//采样计数
		if(ADC_GetData(&_gTaskShareDatObj.TempVal_U16[_gTaskShareDatObj.Count]))
		{
			if(calibrate_val < 0)
			{
//				calibrate_val = ~calibrate_val + 1;
				_gTaskShareDatObj.TempVal_U16[_gTaskShareDatObj.Count] += ~calibrate_val + 1;
			}
			else
				_gTaskShareDatObj.TempVal_U16[_gTaskShareDatObj.Count] -= calibrate_val;
			
			_gTaskShareDatObj.TempVal_S32[0] += _gTaskShareDatObj.TempVal_U16[_gTaskShareDatObj.Count];
			_gTaskShareDatObj.Count++;
			HAL_Delay(1);
		}
		//采样完成
		if(_gTaskShareDatObj.Count >= 18)
		{
			min = _gTaskShareDatObj.TempVal_U16[0];
			max = _gTaskShareDatObj.TempVal_U16[0];
			//剔除一个最高和最低
			for(i=1; i<18; i++)
			{
				if(max < _gTaskShareDatObj.TempVal_U16[i])
					max = _gTaskShareDatObj.TempVal_U16[i];
				else if(min > _gTaskShareDatObj.TempVal_U16[i])
					min = _gTaskShareDatObj.TempVal_U16[i];
			}
			_gTaskShareDatObj.TempVal_S32[0] = _gTaskShareDatObj.TempVal_S32[0] - max - min;
			_gTaskShareDatObj.TempVal_S32[0] >>= 4;
			//校验数据
			for(i=0; i<18; i++)
			{
				if(abs(_gTaskShareDatObj.TempVal_S32[0] - (int32_t)_gTaskShareDatObj.TempVal_U16[i]) > 2)
				{
					_gTaskShareDatObj.DataBuf[0] = 1;
					break;
				}
			}
			//检查采集的电压数据是否在合理范围
			if(!_gTaskShareDatObj.DataBuf[0])
			{
				_gTaskShareDatObj.State &= ~0x40;
				_gTaskShareDatObj.TempVal_U32[0] = _gTaskShareDatObj.TempVal_S32[0];
				_gTaskShareDatObj.TempVal_U32[1] = 1200*4095/_gTaskShareDatObj.TempVal_S32[0];
				sMenuFlag |= 0x08;
			}
			
			_gTaskShareDatObj.TempVal_S32[0] = 0;
			_gTaskShareDatObj.Count = 0;
			_gTaskShareDatObj.DataBuf[0] = 0;
		}
	}
}

