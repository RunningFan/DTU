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


/*-----------------------------------头文件------------------------------------*/
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "statistic.h"
/*-----------------------------------变量处------------------------------------*/
struct tm localTime = {	0,0,0,1,0,115,0,0,0};
time_t unixTimeStamp = 0;

/*-----------------------------------函数处------------------------------------*/

/*******************************************************
* Function Name: 	RTC_IRQHandler
* Purpose: 		    RTC中断服务程序
* Params :          无
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void RTC_IRQHandler(void)
{
//#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
//        CPU_SR         cpu_sr;
//#endif
//        CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
//        OSIntNesting++;
//        CPU_CRITICAL_EXIT();

        RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_ALR | RTC_IT_OW);
        RTC_WaitForLastTask();

//        OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}


/*******************************************************
* Function Name: 	BSP_RTC_Init
* Purpose: 		    RTC初始化程序
* Params :          无
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void  BSP_RTC_Init    (void)
{
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

        NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
        PWR_BackupAccessCmd(ENABLE);
        /* Reset Backup Domain */
        BKP_DeInit();
#if 0
        //！！！使用内部低速晶振
        RCC_LSICmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#else
        //使用外部高速晶振8M/128 = 62.5K
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
#endif
        //允许RTC
        RCC_RTCCLKCmd(ENABLE);
        //等待RTC寄存器同步
        RTC_WaitForSynchro();

        RTC_WaitForLastTask();
        //允许RTC的秒中断(还有闹钟中断和溢出中断可设置)
        RTC_ITConfig(RTC_IT_SEC, ENABLE);

        RTC_WaitForLastTask();
        //62500晶振预分频值是62500,不过一般来说晶振都不那么准
#if 0
        //！！！使用内部低速晶振
        RTC_SetPrescaler(39980); //如果需要校准晶振,可修改此分频值
#else
        RTC_SetPrescaler(78125); //如果需要校准晶振,可修改此分频值
#endif
        RTC_WaitForLastTask();
        //RTC 的时钟校准，暂不使用
        //BKP_SetRTCCalibrationValue(120);
        setLocalTime(&localTime);

        //PWR_BackupAccessCmd(DISABLE);

        //清除标志
        RCC_ClearFlag();
}


/*******************************************************
* Function Name:    setLocaTtime
* Purpose:          将时间写入到本地，传入一个已赋值的tm结构体
* Params :          m_Time：需要写入的时间结构体
* Return:           time_t：返回写入时间后的时间戳
* Limitation: 	    进行了时区处理,需要指定时区
*******************************************************/
time_t setLocalTime(struct tm *m_Time)
{
    unixTimeStamp = mktime(m_Time);
    unixTimeStamp -=UTC_TIME_ZONE*ONEHOUR;
#if 0
    RTC_EnterConfigMode();
    RTC_SetCounter(unixTimeStamp);
    RTC_WaitForLastTask();
    RTC_ExitConfigMode();
#endif
    return unixTimeStamp;
}

/*******************************************************
* Function Name:    getLocalTime
* Purpose:          获取内地时间，将unix时间戳转换为标志时间tm
* Params :          m_DstTime：需要存放到的时间结构体
* Return:           无
* Limitation: 	    需要定义时区 UTC_TIME_ZONE，北京时间从1970/1/1 08:00:00开始
*******************************************************/
time_t getLocalTime(struct tm *m_DstTime)
{
    struct tm *gm_date;
    time_t seconds;
    //unixTimeStamp = RTC_GetCounter();
    seconds =  unixTimeStamp + UTC_TIME_ZONE*ONEHOUR;
    gm_date = gmtime(&seconds);
    if(gm_date){
        memcpy(m_DstTime , gm_date , sizeof(struct tm));
        mktime(m_DstTime);
    }
    return seconds;
}

time_t getTime(struct tm *m_DstTime , time_t TimeStamp)
{
    struct tm *gm_date;
    time_t seconds;
    seconds =  TimeStamp + UTC_TIME_ZONE*ONEHOUR;
    gm_date = gmtime(&seconds);
    if(gm_date){
        memcpy(m_DstTime , gm_date , sizeof(struct tm));
        mktime(m_DstTime);
    }
    return seconds;
}

time_t getUnixTimeStamp(void)
{
    //unixTimeStamp = RTC_GetCounter();
    return unixTimeStamp;
}
/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
