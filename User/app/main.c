/*******************************************************************************
 *   Filename:       main.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    主函数模块
 *
 *   Notes:
 *     				E-mail:261313062@qq.com
 *
 *******************************************************************************/


/***************************头文件声明区***************************************/

#include "includes.h"
#include "app_var.h"

#include "bsp_mlx90632.h"

/********************************宏定义区**************************************/
/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"STM32F103XXX_MLX90632_Demo"
#define EXAMPLE_DATE	"2022-05-02"
#define DEMO_VER		"1.0.0"

/***************************变量定义及声明区***********************************/

double pre_ambient, pre_object, ambient, object;
stMlx90632CalibraParTypeDef stmlxCalibraPar = {0};

/***************************函数定义及声明区***********************************/
void app_SeggerRttInit(void);
static void app_ShowPowerOnInfo(void);


/*============================================================================*/
/*============================================================================*/
/*============================================================================*/


/*******************************************************************************
 * 名    称： main
 * 功    能： 主函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int main(void)
{
	int16_t ambient_new_raw;
	int16_t ambient_old_raw;
	int16_t object_new_raw;
	int16_t object_old_raw;
    /***********************************************
    * 描述： 初始化硬件设备
    */
    bsp_Init(); /* 为了是main函数看起来更简洁些，将硬件初始化的代码封装到这个函数 */

    bsp_Mlx90632Init();
    
    /***********************************************
    * 描述：打印上电信息
    */
    app_SeggerRttInit();
	app_ShowPowerOnInfo();

    /***********************************************
    * 描述： 创建软定时器
    */
    bsp_StartAutoTimer( 0,1000 );   /* 启动自动重载软定时器0 用于1S定时任务 */
    /***********************************************
    * 描述： 进入主程序循环体
    */
    while(1)
    {
        /***********************************************
        * 描述：用户可以在此函数内执行CPU休眠和喂狗等
        * 这个函数在bsp.c 中定义。
        */
        bsp_Idle();                                     /* 调用MODS_Poll() */

        /*   用户执行的任务    */
        /*   1S定时到    */
        if(bsp_CheckTimer(0)) {
            bsp_LedToggle(1);  /* 翻转LED1 */
            
            /* Get raw data from MLX90632 */
            bsp_Mlx90632ReadTempRaw(&ambient_new_raw, &ambient_old_raw, &object_new_raw, &object_old_raw);
            /* Pre-calculations for ambient and object temperature calculation */
            pre_ambient = bsp_Mlx90632PreprocessTempAmbient(ambient_new_raw, ambient_old_raw, stMlxCalibraPar.Gb);
            pre_object = bsp_Mlx90632PreprocessTempObject(object_new_raw, object_old_raw, ambient_new_raw, ambient_old_raw, stMlxCalibraPar.Ka);
            /* Set emissivity = 1 */
            bsp_Mlx90632SetEmissivity(1.0);
            /* Calculate ambient and object temperature */
            ambient = bsp_Mlx90632CalcTempAmbient(ambient_new_raw, ambient_old_raw, stMlxCalibraPar.PT, stMlxCalibraPar.PR, stMlxCalibraPar.PG, stMlxCalibraPar.PO, stMlxCalibraPar.Gb);
            object = bsp_Mlx90632CalcTempObject(pre_object, pre_ambient, stMlxCalibraPar.Ea, stMlxCalibraPar.Eb, stMlxCalibraPar.Ga, stMlxCalibraPar.Fa, stMlxCalibraPar.Fb, stMlxCalibraPar.Ha, stMlxCalibraPar.Hb);
            

            printf("MLX90632 Measure Object Temp:%f ℃,Ambient Temp:%f\n",object,ambient);
            
         }
    }
}


void app_SeggerRttInit(void)
{
    /***********************************************
    * 描述： RTT使用配置
    */
    /* 配置通道0，上行配置*/
	SEGGER_RTT_ConfigUpBuffer(0, "RTTUP", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	/* 配置通道0，下行配置*/
	SEGGER_RTT_ConfigDownBuffer(0, "RTTDOWN", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
}
/*******************************************************************************
 * 名    称： app_ShowPowerOnInfo
 * 功    能： 上电打印相关信息
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void app_ShowPowerOnInfo(void)
{
	/***********************************************
    * 描述： 通过RTT打印信息
    */
    SEGGER_RTT_SetTerminal(0);
    SEGGER_RTT_printf(0,"\n\r");
	SEGGER_RTT_printf(0,"*************************************************************\n\r");
	SEGGER_RTT_printf(0,"* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	SEGGER_RTT_printf(0,"* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	SEGGER_RTT_printf(0,"* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
	SEGGER_RTT_printf(0,"* 固件库版本 : V%d.%d.%d (STM32F10x_StdPeriph_Driver)\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
           __STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	SEGGER_RTT_printf(0,"* \r\n");	/* 打印一行空格 */
	SEGGER_RTT_printf(0,"* Email : wangyao@zhiyigroup.cn \r\n");
	SEGGER_RTT_printf(0,"* 淘宝店: zhisheng.taobao.com\r\n");
	SEGGER_RTT_printf(0,"* 技术支持QQ群  : 539041646 \r\n");
	SEGGER_RTT_printf(0,"* Copyright www.zhiyigroup.cn 至一电子科技\r\n");
	SEGGER_RTT_printf(0,"*************************************************************\n\r");

	/***********************************************
    * 描述： 通过串口打印信息
    */
    printf("\n\r");
	printf("*************************************************************\n");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */
	printf("* --------------------------\r\n");	/*  */
	printf("* 接线方式       \r\n");	/* 接线方式 */
    printf("* VCC ---- 3.3VDC~5VDC      \r\n");	/* 接线方式 */
    printf("* GND ---- GND              \r\n");	/* 接线方式 */
    printf("* SCL ---- PC4              \r\n");	/* 接线方式 */
    printf("* SDA ---- PC5              \r\n");	/* 接线方式 */
	printf("* 串口使用 UASRT1(PA9/PA10) \r\n");	/*  */
	printf("* --------------------------\r\n");	/* 接线方式 */
    
	/* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
	printf("* 固件库版本 : V%d.%d.%d (STM32F10x_StdPeriph_Driver)\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
           __STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("* \r\n");	/* 打印一行空格 */
	printf("* Email : wangyao@zhiyigroup.cn \r\n");
	printf("* 淘宝店: zhisheng.taobao.com\r\n");
	printf("* 技术支持QQ群  : 539041646 \r\n");
	printf("* Copyright www.zhiyigroup.cn 至一电子科技\r\n");
	printf("*************************************************************\n\r");
}

