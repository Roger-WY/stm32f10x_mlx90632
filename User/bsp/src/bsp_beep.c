/*******************************************************************************
*   Filename:       bsp_beep.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:    蜂鸣器驱动模块
*                   驱动蜂鸣器，有源蜂鸣器用IO口控制，无源蜂鸣器使用PWM控制，使用宏定义切换
*   Notes:
*
*
*******************************************************************************/
#include "bsp_beep.h"

//#define BEEP_HAVE_POWER		/* 定义此行表示有源蜂鸣器，直接通过GPIO驱动, 无需PWM */

#ifdef	BEEP_HAVE_POWER		/* 有源蜂鸣器 */
	/* PA8 */
	#define BEEP_GPIO_RCC   RCC_APB2Periph_GPIOA
	#define BEEP_GPIO_PORT	GPIOA
	#define BEEP_GPIO_PIN	GPIO_Pin_8

	#define BEEP_ENABLE()	BEEP_GPIO_PORT->BSRR = BEEP_GPIO_PIN	/* 使能蜂鸣器鸣叫 */
	#define BEEP_DISABLE()	BEEP_GPIO_PORT->BRR  = BEEP_GPIO_PIN	/* 禁止蜂鸣器鸣叫 */
#else		/* 无源蜂鸣器 */
    #include "bsp_tim_pwm.h"
	/* PA8/TIM1_CH1 ---> TIM1_CH1 */
	/* 1500表示频率1.5KHz，5000表示50.00%的占空比 */
	#define BEEP_ENABLE()	bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 1500, 5000);

	/* 禁止蜂鸣器鸣叫 */
	#define BEEP_DISABLE()	bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 1500, 0);
#endif

static BEEP_T s_tBeep;		/* 定义蜂鸣器静态结构体变量 */

//============================================================================//

/*******************************************************************************
 * 名    称： bsp_BeepInit
 * 功    能： 初始化蜂鸣器硬件
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_BeepInit(void)
{
#ifdef	BEEP_HAVE_POWER		/* 有源蜂鸣器 */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 打开GPIO的时钟 */
	RCC_APB2PeriphClockCmd(BEEP_GPIO_RCC, ENABLE);

	BEEP_DISABLE();

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	/* 推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin   = BEEP_GPIO_PIN;
	GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);
#endif
    BEEP_DISABLE();         /* 关闭蜂鸣器 */
}

/*******************************************************************************
 * 名    称： bsp_BeepStart
 * 功    能： 启动蜂鸣音。
 * 入口参数： _usBeepTime : 蜂鸣时间，单位10ms; 0 表示不鸣叫
 *			  _usStopTime : 停止时间，单位10ms; 0 表示持续鸣叫
 *			  _usCycle : 鸣叫次数， 0 表示持续鸣叫
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_BeepStart(uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle)
{
	if (_usBeepTime == 0) {
		return;
	}

	s_tBeep.usBeepTime      = _usBeepTime;
	s_tBeep.usStopTime      = _usStopTime;
	s_tBeep.usCycle         = _usCycle;
	s_tBeep.usCount         = 0;
	s_tBeep.usCycleCount    = 0;
	s_tBeep.ucState         = 0;
	s_tBeep.ucEnalbe        = 1;	/* 设置完全局参数后再使能发声标志 */

	BEEP_ENABLE();			        /* 开始发声 */
}

/*******************************************************************************
 * 名    称： bsp_BeepStop
 * 功    能： 停止蜂鸣音。
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_BeepStop(void)
{
	s_tBeep.ucEnalbe = 0;

	BEEP_DISABLE();	/* 必须在清控制标志后再停止发声，避免停止后在中断中又开启 */
}

/*******************************************************************************
 * 名    称： bsp_BeepKeyTone
 * 功    能： 发送按键音
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_BeepKeyTone(void)
{
	bsp_BeepStart(5, 1, 1);	/* 鸣叫50ms，停10ms， 1次 */
}

/*******************************************************************************
 * 名    称： bsp_BeepPro
 * 功    能： 每隔10ms调用1次该函数，用于控制蜂鸣器发声。
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注： 该函数在 bsp.c 的bsp_RunPer10ms()函数中被调用。
 *******************************************************************************/
void bsp_BeepPro(void)
{
	if ((s_tBeep.ucEnalbe == 0) || (s_tBeep.usStopTime == 0)) {
		return;
	}

	if (s_tBeep.ucState == 0) {
		if (s_tBeep.usStopTime > 0)	{/* 间断发声 */
			if (++s_tBeep.usCount >= s_tBeep.usBeepTime) {
				BEEP_DISABLE();		/* 停止发声 */
				s_tBeep.usCount = 0;
				s_tBeep.ucState = 1;
			}
		} else {
			;	/* 不做任何处理，连续发声 */
		}
	} else if (s_tBeep.ucState == 1) {
		if (++s_tBeep.usCount >= s_tBeep.usStopTime) {
			/* 连续发声时，直到调用stop停止为止 */
			if (s_tBeep.usCycle > 0) {
				if (++s_tBeep.usCycleCount >= s_tBeep.usCycle) {
					/* 循环次数到，停止发声 */
					s_tBeep.ucEnalbe = 0;
				}

				if (s_tBeep.ucEnalbe == 0) {
					s_tBeep.usStopTime = 0;
					return;
				}
			}

			s_tBeep.usCount = 0;
			s_tBeep.ucState = 0;

			BEEP_ENABLE();			/* 开始发声 */
		}
	}
}

/*****************************  (END OF FILE) *********************************/
