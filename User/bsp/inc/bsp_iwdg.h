/*******************************************************************************
*   Filename:       bsp_iwdg.h
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2016-06-29
*   Revision:       v1.0
*   Writer:	        Roger-WY
*
*   Description:    STM32内部独立看门狗驱动模块 头文件
*******************************************************************************/

#ifndef	__BSP_IWDG_H__
#define	__BSP_IWDG_H__
/*******************************************************************************
 * INCLUDES
 */
#include  <stm32f10x.h>

/*******************************************************************************
 * CONSTANTS
 */
#define BSP_WDT_MODE_NONE   0        // 0:禁止
#define BSP_WDT_MODE_EXT    1        // 1：外部看门狗
#define BSP_WDT_MODE_INT    2        // 2：内部看门狗
#define BSP_WDT_MODE_ALL    3        // 3：同时使用内部和外部看门狗


/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

uint8_t  bsp_IwdgInit(uint8_t mode);
uint8_t  bsp_IwdgGetMode(void);
void     bsp_IwdgFeed(void);


/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#endif