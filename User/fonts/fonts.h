/*										
*********************************************************************************************************
*	                                  
*	ģ������ : �ֿ�ģ��
*	�ļ����� : fonts.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __FONTS_H_
#define __FONTS_H_

extern unsigned char const g_Ascii16[];

extern unsigned char const g_Hz16[];

extern unsigned char const g_Ascii12[];

extern unsigned char const g_Hz12[];

#define USE_SMALL_FONT			/* ������б�ʾʹ��С�ֿ⣬ �����ֻ��bsp_tft+lcd.c��ʹ�� */
/* 0x64000000 ��NOR Flash���׵�ַ */
#define HZK12_ADDR (0x64000000 + 0x0)		/* 12�����ֿ��ַ */
#define HZK16_ADDR (0x64000000 + 0x2C9D0)	/* 16�����ֿ��ַ */

#endif
