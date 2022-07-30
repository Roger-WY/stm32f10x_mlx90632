/*******************************************************************************
 *   Filename:       Irmptimer.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    IRMP红外多协议解码程序-定时器
 *   Notes:
 *     				E-mail:261313062@qq.com
 *
 *******************************************************************************/
#include "irmptimer.h"

#ifndef F_CPU
#error F_CPU unknown
#endif

/* IRMP使用的定时器定义 */
#define IRMP_TIMER				TIM4
#define IRMP_TIMER_RCC			RCC_APB1Periph_TIM4
#define IRMP_TIMER_IRQ			TIM4_IRQn
#define IRMP_TIMER_IRQHandler	TIM4_IRQHandler

/*******************************************************************************
 * 名    称： SysCtlClockGet
 * 功    能： 获取系统主频
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2018-05-30
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
uint32_t SysCtlClockGet(void)
{
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    return RCC_ClocksStatus.SYSCLK_Frequency;
}

/*******************************************************************************
 * 名    称： irmp_timer_init
 * 功    能： Irmp使用的定时器初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2018-05-30
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void irmp_timer_init (void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(IRMP_TIMER_RCC, ENABLE);

    TIM_TimeBaseStructure.TIM_ClockDivision                 = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode                   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period                        = 7;
    TIM_TimeBaseStructure.TIM_Prescaler                     = ((F_CPU / F_INTERRUPTS)/8) - 1;
    TIM_TimeBaseInit(IRMP_TIMER, &TIM_TimeBaseStructure);

    TIM_ITConfig(IRMP_TIMER, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                      = IRMP_TIMER_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0x0F;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(IRMP_TIMER, ENABLE);
}

/*******************************************************************************
 * 名    称： IRMP_TIMER_IRQHandler
 * 功    能： IRMP使用的定时器中断
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2018-05-30
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void IRMP_TIMER_IRQHandler(void)
{
    if ( TIM_GetITStatus(IRMP_TIMER , TIM_IT_Update) != RESET )  //
    {
        TIM_ClearITPendingBit(IRMP_TIMER , TIM_IT_Update);      //
        (void) irmp_ISR();                                      // call irmp ISR
		// call other timer interrupt routines...
    }
}