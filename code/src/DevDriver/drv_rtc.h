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


#ifndef _DRV_H_RTC_
#define _DRV_H_RTC_

/*-----------------------------------头文件------------------------------------*/
#include <time.h>
#include "stm32f10x.h"

#define ONESEC          1
#define ONEMIN          60*ONESEC
#define ONEHOUR         60*ONEMIN
#define ONEDAY          24*ONEHOUR
#define UTC_TIME_ZONE   8  

extern struct tm localTime;
extern struct tm error_time_tag;
extern time_t unixTimeStamp;
time_t getErrorTime(struct tm *m_DstTime,time_t unix_time);

    
/*-----------------------------------函数处------------------------------------*/
void    BSP_RTC_Init(void);
time_t  getUnixTimeStamp(void);
time_t  setLocalTime(struct tm *m_Time);
time_t  getLocalTime(struct tm *m_DstTime);
time_t getTime(struct tm *m_DstTime , time_t TimeStamp);

#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
