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


#ifndef _DRV_H_LED_
#define _DRV_H_LED_

/*-----------------------------------头文件------------------------------------*/
#include "stm32f10x.h"

/*-----------------------------------变量处------------------------------------*/
#define LED_NUMBER      5
#define ALL_LED         0
#define CON_LED         1
#define HIG_LED         2
#define MID_LED         3
#define LOW_LED         4
#define ERR_LED         5

#define LED_OFF         0
#define LED_ON          1
#define LED_BLINK       2

#define LED_STA_NONE    0
#define LED_STA_OFF     1
#define LED_STA_ON      2
#define LED_STA_BLINK   3
    
#define LED_BLINK_SLW   6
#define LED_BLINK_FST   1

#define SIGNAL_NON_LEVEL 0
#define SIGNAL_LOW_LEVEL 15
#define SIGNAL_MID_LEVEL 23
#define SIGNAL_HIG_LEVEL 31

typedef struct
{
    GPIO_TypeDef * Port;
    uint16_t       Pin;
}SRTLED; 
    
typedef struct 
{
    uint32_t ErrSta:2;  //故障灯状态
    uint32_t ErrSpd:3;  //闪烁速度
    uint32_t ErrCnt:3;  //计时器
    uint32_t ConSta:2;  //连接灯状态
    uint32_t ConSpd:3;  //闪烁速度
    uint32_t ConCnt:3;  //计时器
    uint32_t Signal:5;  //信号灯状态
    uint32_t SimSig:3;  //注册状态
    uint32_t BlinkTm:8; //闪烁时间
}LedType;
 
extern LedType  LedValue;
extern uint16_t run_time;
extern uint8_t run_flag;
extern uint8_t config_run_flag_fail;//配置飞行模式失败次数
extern uint8_t close_sim_mode;

extern uint8_t check_time;
extern uint8_t LED_all_onoroff;
//extern uint8_t ATmutex;

/*-----------------------------------函数处------------------------------------*/
void    BSP_LED_Init(void);
void    BSP_LED_CTL (uint8_t led_id , uint8_t led_status);
void    displayLED(LedType *LED);
uint8_t off_gprs(LedType *LED);
#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
