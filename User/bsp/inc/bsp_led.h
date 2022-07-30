/*******************************************************************************
 *   Filename:       bsp_led.h
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    LED指示灯驱动模块 头文件
 *******************************************************************************/

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include <stdint.h>

/* 供外部调用的函数声明 */
void bsp_LedInit(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif

/*****************************  (END OF FILE) *********************************/
