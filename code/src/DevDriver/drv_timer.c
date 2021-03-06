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
#include "fsm.h"
#include "duplicate.h"
#include "bsp.h"
/*-----------------------------------变量处------------------------------------*/


/*-----------------------------------函数处------------------------------------*/


/*******************************************************
* Function Name: 	TIM3_Init
* Purpose: 		    通用定时器中断初始化
* Params :          arr：自动重装值。;psc：时钟预分频数;
* Return: 		    无
* Limitation: 	    这里始终选择为APB1的2倍，而APB1为36M
*******************************************************/
void TIM3_Init(uint16_t arr,uint16_t psc)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);                    //时钟使能//TIM3时钟使能
    
    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = arr;                                 //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler =psc;                               //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;                 //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;             //TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                         //根据指定的参数初始化TIMx的时间基数单位
    
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );                               //使能指定的TIM3中断,允许更新中断
    
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;                //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;                      //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	                                        //根据指定的参数初始化VIC寄存器
}


/*******************************************************
* Function Name: 	TIM4_Set
* Purpose: 		    通用定时器中断初始化
* Params :          arr：自动重装值。;psc：时钟预分频数;
* Return: 		    无
* Limitation: 	    这里始终选择为APB1的2倍，而APB1为36M
*******************************************************/
void TIM4_Init(uint16_t arr,uint16_t psc)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);                    //时钟使能//TIM4时钟使能
    
    //定时器TIM4初始化
    TIM_TimeBaseStructure.TIM_Period = arr;                                 //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler =psc;                               //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;                 //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;             //TIM向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);                         //根据指定的参数初始化TIMx的时间基数单位
    
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE );                               //使能指定的TIM4中断,允许更新中断
    
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;                //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;                      //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	                                        //根据指定的参数初始化VIC寄存器
}



/*******************************************************
* Function Name: 	TIM5_Init
* Purpose: 		    通用定时器中断初始化
* Params :          arr：自动重装值。;psc：时钟预分频数;
* Return: 		    无
* Limitation: 	    这里始终选择为APB1的2倍，而APB1为36M
*******************************************************/
void TIM5_Init(uint16_t arr,uint16_t psc)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);                    //时钟使能//TIM5时钟使能
    
    //定时器TIM5初始化
    TIM_TimeBaseStructure.TIM_Period = arr;                                 //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler =psc;                               //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;                 //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;             //TIM向上计数模式
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);                         //根据指定的参数初始化TIMx的时间基数单位
    
    TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE );                               //使能指定的TIM5中断,允许更新中断
    
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;                //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;                      //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                         //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	                                        //根据指定的参数初始化VIC寄存器
}


/*******************************************************
* Function Name: 	TIM3_IRQHandler
* Purpose: 		    定时器4中断服务程序
* Params :          无
* Return: 		    无
* Limitation: 	    串口接收数据超时完成标记
*******************************************************/
void TIM3_IRQHandler(void)
{
    uint8_t err;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();
    
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){                         //是更新中断
        TIM3_Set(0);			                                                //关闭TIM3
        if((uartRxBuf1[g_UartCnt1].DataLen & 0x7FFF) > 0){
            uartRxBuf1[g_UartCnt1].DataBuf[uartRxBuf1[g_UartCnt1].DataLen + 1] = '\0';
            err = OSQPost(QSemUART1,(void *)&uartRxBuf1[g_UartCnt1]);
            if(OS_ERR_NONE == err){
                uartRxBuf1[g_UartCnt1].DataLen |= 1<<15;	//标记接收完成
                g_UartCnt1++;
            }
            if(g_UartCnt1 >= UART1_QUEUE_LEN){
                g_UartCnt1 = 0;
            }
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );                           //清除TIMx更新中断标志
    }
    
    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}

/*******************************************************
* Function Name: 	TIM3_Set
* Purpose: 		    设置TIM3的开关
* Params :          sta:0，关闭;1,开启;
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void TIM3_Set(uint8_t sta)
{
    if(sta){
        TIM_SetCounter(TIM3,0);                                         //计数器清空
        TIM_Cmd(TIM3, ENABLE);                                          //使能TIMx
    }else{
        TIM_Cmd(TIM3, DISABLE);                                         //关闭定时器3
    }
}


/*******************************************************
* Function Name: 	TIM4_IRQHandler
* Purpose: 		    定时器4中断服务程序
* Params :          无
* Return: 		    无
* Limitation: 	    串口接收数据超时完成标记
*******************************************************/
void TIM4_IRQHandler(void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();
    
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){                     //是更新中断
        TIM4_Set(0);
#if 0
        if((uartRxBuf2[g_UartCnt2].DataLen & 0x7FFF) > 0){
            uartRxBuf2[g_UartCnt2].DataBuf[uartRxBuf2[g_UartCnt2].DataLen + 1] = '\0';
            err = OSQPost(QSemUART2,(void *)&uartRxBuf2[g_UartCnt2]);
            if(OS_ERR_NONE == err){
                uartRxBuf2[g_UartCnt2].DataLen |= 1<<15;	//标记接收完成
                g_UartCnt2++;
            }
            if(g_UartCnt2 >= UART2_QUEUE_LEN){
                g_UartCnt2 = 0;
            }
        }
#endif
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );                   //清除TIMx更新中断标志			                                        //关闭TIM4
    }
    
    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}


/*******************************************************
* Function Name: 	TIM4_Set
* Purpose: 		    设置TIM4的开关
* Params :          sta:0，关闭;1,开启;
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void TIM4_Set(uint8_t sta)
{
    if(sta){
        TIM_SetCounter(TIM4,0);//计数器清空
        TIM_Cmd(TIM4, ENABLE);  //使能TIMx
    }else{
        TIM_Cmd(TIM4, DISABLE);//关闭定时器4
    }
}


/*******************************************************
* Function Name: 	TIM5_IRQHandler
* Purpose: 		    定时器5中断服务程序
* Params :          无
* Return: 		    无
* Limitation: 	    CAN接收数据超时完成标记
*******************************************************/
uint32_t lost_pack;
void TIM5_IRQHandler(void)
{
    uint8_t err;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();
    
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET){                     //是更新中断
        TIM5_Set(0);
        if((uartRxBuf3[g_UartCnt3].DataLen & 0x7FFF) > 0){
            uartRxBuf3[g_UartCnt3].DataBuf[uartRxBuf3[g_UartCnt3].DataLen + 1] = '\0';
            if(var.Hyaline == 1)
            {
                if(strstr((const char *)&uartRxBuf3[g_UartCnt3].DataBuf[0] ,"DATA ACCEPT")){
                    fail_count = 0;
					tcp_rev_total++;
				}
                else if(strstr((const char *)&uartRxBuf3[g_UartCnt3].DataBuf[0] ,"SEND FAIL")){
					lost_pack++;
                    fail_count ++;
				}
            }else{
            	if(strstr((const char *)&uartRxBuf3[g_UartCnt3].DataBuf[0] ,"SEND FAIL"))
					lost_pack++;
            }
            err = OSQPost(QSemUART3,(void *)&uartRxBuf3[g_UartCnt3]);
            if(OS_ERR_NONE == err){
                uartRxBuf3[g_UartCnt3].DataLen |= 1<<15;	//标记接收完成
                g_UartCnt3++;
            }
            if(g_UartCnt3 >= UART3_QUEUE_LEN){
                g_UartCnt3 = 0;
            }
        }
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );                   //清除TIMx更新中断标志			                                        //关闭TIM5
    }
    
    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}


/*******************************************************
* Function Name: 	TIM5_Set
* Purpose: 		    设置TIM5的开关
* Params :          sta:0，关闭;1,开启;
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void TIM5_Set(uint8_t sta)
{
    if(sta){
        TIM_SetCounter(TIM5,0);                                         //计数器清空
        TIM_Cmd(TIM5, ENABLE);                                          //使能TIMx
    }else{
        TIM_Cmd(TIM5, DISABLE);                                         //关闭定时器5
    }
}


/*******************************************************
* Function Name: 	Time_count
* Purpose: 		    计时函数
* Params :          void
* Return: 		    void
* Limitation: 	    无
*******************************************************/
void Time_count(void)
{
    static uint32_t Hours,one_day,five_min;
    uint32_t temp_hour = 0;
	uint32_t temp_min = 0;
    uint8_t i;
//    temp_hour = unixTimeStamp/3600;
//#ifdef TEST_TEST_OVER
	temp_hour = f4_time/3600;
//#else
//	temp_hour = f4_time/60;
//#endif
	temp_min = f4_time/300;
	if(five_min != temp_min){
		five_min = temp_min;
		five_min_flag = !five_min_flag;
	}


    if(Hours != temp_hour){                                         //full one hour
        Hours = temp_hour;
        one_day++;
        Getf3Time = 0;                                             //reget the f3;
        try_time=0;
        connect_flag = 1;
		if(g_Upload.BigDataLen > 6){
			switch_one_two = 1;
        	bigData_sample_into_flash(NONSENCE,NONSENCE,NONSENCE,NONSENCE);
			switch_one_two = 0;
		}
        one_two_flag = !one_two_flag;
		
		Getf2Heart = 0;
        if(wait_update != 0){
            var.s_update = 1;
        }
    }
	
    if(one_day == 24){
        one_day = 0;
        for(i=0; i < INDOOR_MCH_NUM; i++){//每天对故障累计的次数进行清零操作
            ACErrParam.water_full_protect_err_num[i] = 0;
            ACErrParam.indoor_wind_protect_err_num[i] = 0;
            ACErrParam.freeze_protect_err_num[i] = 0;
			ACErrParam.divide_water_set_problem_num[i] = 0;
			ACErrParam.line_control_problem_num[i] = 0;
//			ACErrParam.voltage_supply_short_num[i] = 0;
			ACErrParam.water_switch_problem_num[i] = 0;
			ACErrParam.one_control_multi_machine_num[i] = 0;
			ACErrParam.swift_wind_error_num[i] = 0;
        }
		if(dbgPrintf)(*dbgPrintf)("----------->Day Past,Err count Clear...<------------\r\n");

    }
//    temp_hour = unixTimeStamp%360;
}


/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
