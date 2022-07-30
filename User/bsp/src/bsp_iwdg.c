/*******************************************************************************
*   Filename:       bsp_iwdg.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:    STM32内部独立看门狗驱动模块
*
*   Notes:
*   独立看门狗工作原理：在键值寄存器（IWDG_KR）中写入0XCCCC，开始启用独立看门狗，
*   此时计数器开始从其复位值OXFFF递减计数，当计数器计数到末尾0X000的时候，会产生
*   一个复位信号（IWDG_RESET），无论何时，只要寄存器IWDG_KR中被写入0XAAAA，IWDG_RLR
*   中的值就会被重新加载到计数器中从而避免产生看门狗复位。
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include "bsp_iwdg.h"


static uint8_t s_ucIwdgMode  = 0;    // 0:禁止；1：外部看门狗；2：内部看门狗；3：同时使用内部和外部看门狗;

//============================================================================//

/*******************************************************************************
 * 名    称： Bsp_IwdgFeed
 * 功    能： 独立看门狗喂狗（看门狗复位）
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void  bsp_IwdgFeed(void)
{
    if ( s_ucIwdgMode == BSP_WDT_MODE_NONE )
        return;
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif
    /***********************************************
    * 描述：STM内部看门狗喂狗 Reload IWDG counter
    */
    if ( ( s_ucIwdgMode == BSP_WDT_MODE_INT ) || ( s_ucIwdgMode == BSP_WDT_MODE_ALL ) ) {
        IWDG_ReloadCounter();
    }
    /***********************************************
    * 描述：外部看门狗喂狗
    */
    if ( ( s_ucIwdgMode == BSP_WDT_MODE_EXT ) || ( s_ucIwdgMode == BSP_WDT_MODE_ALL ) ) {
        //OS_ENTER_CRITICAL();
        //GPIO_SetBits(GPIO_WDT, BSP_WDT_WDI);
        //Delay_Nus(10);
        //GPIO_ResetBits(GPIO_WDT, BSP_WDT_WDI);
        //OS_EXIT_CRITICAL();
    }
}

/*******************************************************************************
 * 名    称： Bsp_IwdgGetMode
 * 功    能： 获取用了上面狗（内部的？外部的？还是两个都用？）
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
uint8_t  bsp_IwdgGetMode(void)
{
    return s_ucIwdgMode;
}

/*******************************************************************************
 * 名    称： Bsp_IwdtInit
 * 功    能： 独立看门狗初始化
 * 入口参数： 0:禁止；1：外部看门狗；2：内部看门狗；3：同时使用内部和外部看门狗;
 * 出口参数： 0：初始化成功 1：初始化失败
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
uint8_t bsp_IwdgInit(uint8_t mode)
{
    s_ucIwdgMode = mode;

    if ( mode == BSP_WDT_MODE_NONE )  //禁止狗
        return 0;

    if ( ( mode == BSP_WDT_MODE_INT ) || ( mode == BSP_WDT_MODE_ALL ) ) {//使用内部狗或者内部外部狗一起用
        /* Check if the system has resumed from IWDG reset */
        if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  {
            /* Clear reset flags */
            RCC_ClearFlag();
        } else {
            /* IWDGRST flag is not set */
        }

        //2.独立看门狗(IWDG)由专用的40kHz 的低速时钟为驱动；因此，即使主时钟发生
        //故障它也仍然有效。窗口看门狗由从APB1 时钟分频后得到的时钟驱动，通过可
        //配置的时间窗口来检测应用程序非正常的过迟或过早的行为。可通过
        //IWDG_SetPrescaler(IWDG_Prescaler_32); 对其时钟进行分频，4-256，
        //通过以下方式喂狗 ：
        ///* Reload IWDG counter */
        //IWDG_ReloadCounter();
        //3. 0.625KHz 即每周期 为1.6ms
        //共计时 1000 个周期，即1000*1.6ms=1.6s
        //看门狗定时时限= IWDG_SetReload（）的值 / 看门狗时钟频率
        //看门狗时钟频率=LSI（内部低速时钟）的频率（40KHz）/ 分频数

        RCC_LSICmd(ENABLE);                              //打开LSI
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);

        /* Enable write access to IWDG_PR and IWDG_RLR registers */
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
        /* IWDG counter clock: 40KHz(LSI) / 64 * 2  = 0.625 / 2  KHz */
        IWDG_SetPrescaler(IWDG_Prescaler_128);
        /* 3.2s ,max 0xFFF  0~4095  */
        IWDG_SetReload(3000);
        IWDG_ReloadCounter();
        IWDG_Enable();
    }
    if ( ( mode == BSP_WDT_MODE_EXT ) || ( mode == BSP_WDT_MODE_ALL ) ) {//使用外狗或者内狗外狗一起用
//        GPIO_InitTypeDef  gpio_init;
//
//        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_WDT, ENABLE);
//
//        gpio_init.GPIO_Pin   = BSP_WDT_WDI;
//        gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
//        gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
//
//        GPIO_Init(GPIO_WDT, &gpio_init);
    }

    return 1;
}

/*******************************************************************************
*              end of file                                                    *
*******************************************************************************/
