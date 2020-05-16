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


#ifndef _DRV_H_BUTTON_
#define _DRV_H_BUTTON_

#include "stm32f10x.h"

#define PWR_KEY_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_12)

#define PWR_KEY_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_12)

/***************************************************************************/
extern OS_EVENT * MSemKEY; 
/***************************************************************************/
void     BSP_PB_Init  (void);
void     CAN_Polar_SW(void);
void     BSP_PWRKEY_Init (void);
uint8_t  BSP_PB_GetStatus (uint8_t  mf_PushButtonId);

#endif


/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
