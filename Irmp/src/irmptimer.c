/*******************************************************************************
 *   Filename:       Irmptimer.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    IRMP�����Э��������-��ʱ��
 *   Notes:
 *     				E-mail:261313062@qq.com
 *
 *******************************************************************************/
#include "irmptimer.h"

#ifndef F_CPU
#error F_CPU unknown
#endif

/* IRMPʹ�õĶ�ʱ������ */
#define IRMP_TIMER				TIM4
#define IRMP_TIMER_RCC			RCC_APB1Periph_TIM4
#define IRMP_TIMER_IRQ			TIM4_IRQn
#define IRMP_TIMER_IRQHandler	TIM4_IRQHandler

/*******************************************************************************
 * ��    �ƣ� SysCtlClockGet
 * ��    �ܣ� ��ȡϵͳ��Ƶ
 * ��ڲ����� ��
 * ���ڲ����� ��
 * �������ߣ� Roger-WY.
 * �������ڣ� 2018-05-30
 * ��    �ģ�
 * �޸����ڣ�
 * ��    ע��
 *******************************************************************************/
uint32_t SysCtlClockGet(void)
{
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    return RCC_ClocksStatus.SYSCLK_Frequency;
}

/*******************************************************************************
 * ��    �ƣ� irmp_timer_init
 * ��    �ܣ� Irmpʹ�õĶ�ʱ����ʼ��
 * ��ڲ����� ��
 * ���ڲ����� ��
 * �������ߣ� Roger-WY.
 * �������ڣ� 2018-05-30
 * ��    �ģ�
 * �޸����ڣ�
 * ��    ע��
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
 * ��    �ƣ� IRMP_TIMER_IRQHandler
 * ��    �ܣ� IRMPʹ�õĶ�ʱ���ж�
 * ��ڲ����� ��
 * ���ڲ����� ��
 * �������ߣ� Roger-WY.
 * �������ڣ� 2018-05-30
 * ��    �ģ�
 * �޸����ڣ�
 * ��    ע��
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