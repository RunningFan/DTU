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


#ifndef _DRV_CAN_H_
#define _DRV_CAN_H_


/*-----------------------------------头文件------------------------------------*/
#include "stm32f10x.h"

typedef struct 
{
    #define     CAN_REVNUM_PLUS 1
    #define     CAN_REVNUM_CLR  0 
    #define     CAN_MAXREV_NUM  5
    #define     CAN_ONESEC_PSC  10
    uint32_t    Polar:1;
    uint32_t    Time:4;
    uint32_t    RevCnt:3;  
    uint32_t    SwitchNum:8;
}PolarType;


/*-----------------------------------变量处------------------------------------*/

#define CAN_MAX_BUF_LEN 100

extern PolarType PolarCAN1;

extern void     *canRxPtrArr[];
extern OS_EVENT *QSemCAN;

extern uint8_t  canRxPut;
extern CanRxMsg RxMessage [];
extern uint8_t EE_flag;
/*-----------------------------------函数处------------------------------------*/
void    BSP_CAN_Init(void);
void    CheckPolarRev(PolarType *ptr , uint8_t Mode);
#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
