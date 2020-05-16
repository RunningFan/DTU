/*****************************************************************************
Copyright: 2014 GREE ELECTRIC APPLIANCES,INC.
File name: bsp.h
Description: 板级支持包，各类硬件底层初始化
Author:  Sean.Y
Version: V1.0
Date:    2015/04/09
History: 修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/

#if !defined( _BSP_H )
#define _BSP_H

/*-----------------------------------头文件------------------------------------*/
#include "common.h"
#include "stm32f10x.h"
#include "Duplicate.h"
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "drv_sms.h"
#include "fsm.h"
#include "statistic.h"
#include "myscanf.h"
/*-----------------------------------函数处------------------------------------*/
void  BSP_Init          (void);
void  BSP_RCC_Init      (void);
void  BSP_CAN_Init      (void);
void  BSP_IWDG_Init     (void);
void  BSP_Flash_Init    (void);
void  softReset         (void);
void  BSP_SYSTICK_Init(void);
void sign_flashread();

extern uint8_t Connect_time;
extern uint32_t f4_time; 
#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
