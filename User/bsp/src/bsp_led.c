/*******************************************************************************
 *   Filename:       bsp_led.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    LED指示灯驱动模块
 *   Notes:
 *     				E-mail:261313062@qq.com
 *
 *******************************************************************************/

#include "bsp_led.h"
#include <stm32f10x.h>

/*
	如果用户的LED指示灯个数小于4个，可以将多余的LED全部定义为和第1个LED一样，
    并不影响程序功能
*/


/* LED 对应的GPIO设置 */
#define LED1_GPIO_RCC    RCC_APB2Periph_GPIOA
#define LED1_GPIO_PORT   GPIOA
#define LED1_GPIO_PIN	 GPIO_Pin_8

#define LED2_GPIO_RCC    RCC_APB2Periph_GPIOD
#define LED2_GPIO_PORT   GPIOD
#define LED2_GPIO_PIN	 GPIO_Pin_2

#define LED3_GPIO_RCC    RCC_APB2Periph_GPIOD
#define LED3_GPIO_PORT   GPIOD
#define LED3_GPIO_PIN	 GPIO_Pin_2

#define LED4_GPIO_RCC    RCC_APB2Periph_GPIOD
#define LED4_GPIO_PORT   GPIOD
#define LED4_GPIO_PIN	 GPIO_Pin_2

//============================================================================//

/*******************************************************************************
 * 名    称： bsp_InitLed
 * 功    能： 配置LED指示灯相关的GPIO,  该函数被 Bsp_Init() 调用。
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2016-08-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_LedInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_APB2PeriphClockCmd(LED1_GPIO_RCC | LED2_GPIO_RCC | LED3_GPIO_RCC | LED4_GPIO_RCC, ENABLE);

	bsp_LedOff(1);
	bsp_LedOff(2);
	bsp_LedOff(3);
	bsp_LedOff(4);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;	/* 推挽输出模式 */

	GPIO_InitStructure.GPIO_Pin   = LED1_GPIO_PIN;
	GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = LED2_GPIO_PIN;
	GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = LED3_GPIO_PIN;
	GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = LED4_GPIO_PIN;
	GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
 * 名    称： Bsp_LedOn
 * 功    能： 点亮指定的LED指示灯
 * 入口参数：  _no : 指示灯序号，范围 1 - 4  0：点亮全部LED
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2016-08-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_LedOn(uint8_t _no)
{
	if(_no == 0) {
        LED1_GPIO_PORT->BRR = LED1_GPIO_PIN;
        LED2_GPIO_PORT->BRR = LED2_GPIO_PIN;
        LED3_GPIO_PORT->BRR = LED3_GPIO_PIN;
        LED4_GPIO_PORT->BRR = LED4_GPIO_PIN;
    } else if (_no == 1) {
		LED1_GPIO_PORT->BRR = LED1_GPIO_PIN;
	} else if (_no == 2) {
		LED2_GPIO_PORT->BRR = LED2_GPIO_PIN;
	} else if (_no == 3) {
		LED3_GPIO_PORT->BRR = LED3_GPIO_PIN;
	} else if (_no == 4) {
		LED4_GPIO_PORT->BRR = LED4_GPIO_PIN;
	}
}

/*******************************************************************************
 * 名    称： Bsp_LedOff
 * 功    能： 熄灭指定的LED指示灯。
 * 入口参数： _no : 指示灯序号，范围 1 - 4  0：熄灭全部LED
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2016-08-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_LedOff(uint8_t _no)
{
	if (_no == 0) {
		LED1_GPIO_PORT->BSRR = LED1_GPIO_PIN;
        LED2_GPIO_PORT->BSRR = LED2_GPIO_PIN;
        LED3_GPIO_PORT->BSRR = LED3_GPIO_PIN;
        LED4_GPIO_PORT->BSRR = LED4_GPIO_PIN;
	} else if(_no == 1) {
        LED1_GPIO_PORT->BSRR = LED1_GPIO_PIN;
    } else if (_no == 2) {
		LED2_GPIO_PORT->BSRR = LED2_GPIO_PIN;
	} else if (_no == 3) {
		LED3_GPIO_PORT->BSRR = LED3_GPIO_PIN;
	} else if (_no == 4) {
		LED4_GPIO_PORT->BSRR = LED4_GPIO_PIN;
	}
}

/*******************************************************************************
 * 名    称： bsp_LedToggle
 * 功    能： 翻转指定的LED指示灯。
 * 入口参数： _no : 指示灯序号，范围 1 - 4   0:翻转全部LED
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2016-08-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void bsp_LedToggle(uint8_t _no)
{
	if(_no == 0){
        LED1_GPIO_PORT->ODR ^= LED1_GPIO_PIN;
		LED2_GPIO_PORT->ODR ^= LED2_GPIO_PIN;
		LED3_GPIO_PORT->ODR ^= LED3_GPIO_PIN;
		LED4_GPIO_PORT->ODR ^= LED4_GPIO_PIN;
    } else if (_no == 1) {
		LED1_GPIO_PORT->ODR ^= LED1_GPIO_PIN;
	} else if (_no == 2) {
		LED2_GPIO_PORT->ODR ^= LED2_GPIO_PIN;
	} else if (_no == 3) {
		LED3_GPIO_PORT->ODR ^= LED3_GPIO_PIN;
	} else if (_no == 4) {
		LED4_GPIO_PORT->ODR ^= LED4_GPIO_PIN;
	}
}

/*******************************************************************************
 * 名    称： bsp_IsLedOn
 * 功    能： 判断LED指示灯是否已经点亮。
 * 入口参数：  _no : 指示灯序号，范围 1 - 4
 * 出口参数： 1表示已经点亮，0表示未点亮  0xFE:形参输入有错误
 * 作　　者： Roger-WY.
 * 创建日期： 2016-08-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
uint8_t bsp_IsLedOn(uint8_t _no)
{
	if (_no == 1) {
		if ((LED1_GPIO_PORT->ODR & LED1_GPIO_PIN) == 0) {
			return 1;
		}
		return 0;
	} else if (_no == 2) {
		if ((LED2_GPIO_PORT->ODR & LED2_GPIO_PIN) == 0) {
			return 1;
		}
		return 0;
	} else if (_no == 3) {
		if ((LED3_GPIO_PORT->ODR & LED3_GPIO_PIN) == 0) {
			return 1;
		}
		return 0;
	} else if (_no == 4) {
		if ((LED4_GPIO_PORT->ODR & LED4_GPIO_PIN) == 0) {
			return 1;
		}
		return 0;
	}
	return 0xFE;
}

/*****************************  (END OF FILE) *********************************/
