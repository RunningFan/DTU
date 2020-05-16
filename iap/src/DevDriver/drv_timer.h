/*****************************************************************************
Copyright: 2014 GREE ELECTRIC APPLIANCES,INC.
File name: app.c
Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出
值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
Author:  Sean.Y
Version: V1.0
Date:    2015/04/09
History: 修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/


#ifndef _DRV_TIMER_H_
#define _DRV_TIMER_H_

/*-----------------------------------头文件------------------------------------*/
#include "stm32f10x.h"

/*-----------------------------------变量处------------------------------------*/



/*-----------------------------------函数处------------------------------------*/
void TIM4_Init(uint16_t arr,uint16_t psc);
void TIM4_IRQHandler(void);
void TIM4_Set(uint8_t sta);
void TIM5_Init(uint16_t arr,uint16_t psc);
void TIM5_IRQHandler(void);
void TIM5_Set(uint8_t sta);
void TIM3_Init(uint16_t arr,uint16_t psc);
void TIM3_IRQHandler(void);
void TIM3_Set(uint8_t sta);

#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
