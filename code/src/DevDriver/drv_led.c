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

#define LED_RCC    (RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD)
/*-----------------------------------变量处------------------------------------*/
LedType  LedValue;
uint8_t LED_all_onoroff = 0;

/*-----------------------------------函数处------------------------------------*/


const SRTLED s_LED[LED_NUMBER] = 
{
    {GPIOD, GPIO_Pin_0 },
    {GPIOD, GPIO_Pin_1 },
    {GPIOD, GPIO_Pin_2 },
    {GPIOC, GPIO_Pin_11},
    {GPIOC, GPIO_Pin_12},
}; 
/*-----------------------------------函数处------------------------------------*/

void  BSP_LED_Init (void)
{
    uint8_t i;
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(LED_RCC , ENABLE);
    for(i = 0; i < LED_NUMBER ; i++)
    {
        GPIO_InitStructure.GPIO_Pin   = s_LED[i].Pin;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init(s_LED[i].Port, &GPIO_InitStructure);
        GPIO_SetBits(s_LED[i].Port, s_LED[i].Pin);
    }
}

void  BSP_LED_CTL (uint8_t led_id , uint8_t led_status)
{
    uint8_t i = 0;
    if(( led_id > 0 ) && ( led_id <= LED_NUMBER)){
        if(led_status == 1){
            GPIO_ResetBits(s_LED[led_id-1].Port, s_LED[led_id-1].Pin);
        }else if(led_status == 0){
            GPIO_SetBits(s_LED[led_id-1].Port, s_LED[led_id-1].Pin);
        }else {
            if(GPIO_ReadInputDataBit(s_LED[led_id-1].Port, s_LED[led_id-1].Pin)){
                GPIO_ResetBits(s_LED[led_id-1].Port, s_LED[led_id-1].Pin);
            }else{
                GPIO_SetBits(s_LED[led_id-1].Port, s_LED[led_id-1].Pin);
            }
        }
    }else{
        for(i=0 ; i< LED_NUMBER ; i++){
            if(led_status == 1){
                GPIO_ResetBits(s_LED[i].Port, s_LED[i].Pin);
            }else if(led_status == 0){
                GPIO_SetBits(s_LED[i].Port, s_LED[i].Pin);
            }else {
                if(GPIO_ReadInputDataBit(s_LED[i].Port, s_LED[i].Pin)){
                    GPIO_ResetBits(s_LED[i].Port, s_LED[i].Pin);
                }else{
                    GPIO_SetBits(s_LED[i].Port, s_LED[i].Pin);
                }
            }
        }
    }
}


void displayLED(LedType *LED)
{
    if(!LED_all_onoroff){
/******************************错误指示灯处理***********************************/
    if(LED->ErrSta == LED_STA_ON){
        BSP_LED_CTL(ERR_LED , LED_ON);
    }else if(LED->ErrSta == LED_STA_OFF){
        BSP_LED_CTL(ERR_LED , LED_OFF);
    }else if(LED->ErrSta == LED_STA_BLINK){
        if(LED->ErrCnt <= LED_BLINK_FST){
            LED->ErrCnt = LED->ErrSpd;
            BSP_LED_CTL(ERR_LED , LED_BLINK);
        }else{
            LED->ErrCnt --;
        }
    }
/******************************连接指示灯处理***********************************/
    if(LED->ConSta == LED_STA_ON){
        BSP_LED_CTL(CON_LED , LED_ON);
    }else if(LED->ConSta == LED_STA_OFF){
        BSP_LED_CTL(CON_LED , LED_OFF);
    }else if(LED->ConSta == LED_STA_BLINK){
        if(LED->ConCnt <= LED_BLINK_FST){
            LED->ConCnt = LED->ConSpd;
            if(LED->BlinkTm == 0){
                BSP_LED_CTL(CON_LED , LED_BLINK);
                LED->ConSta = LED_STA_OFF;
            }else if(LED->BlinkTm > 2){
                LED->BlinkTm --;
                BSP_LED_CTL(CON_LED , LED_BLINK);
            }else{
                LED->ConSta = LED_STA_OFF;
            }
        }else{
            LED->ConCnt --;
        }
    }
/******************************信号指示灯处理***********************************/
    if(LED->Signal <= SIGNAL_LOW_LEVEL){
        BSP_LED_CTL(LOW_LED , LED_ON);
        BSP_LED_CTL(MID_LED , LED_OFF);
        BSP_LED_CTL(HIG_LED , LED_OFF);
    }else if((LED->Signal > SIGNAL_LOW_LEVEL)&&(LED->Signal <= SIGNAL_MID_LEVEL)){
        BSP_LED_CTL(LOW_LED , LED_OFF);
        BSP_LED_CTL(MID_LED , LED_ON);
        BSP_LED_CTL(HIG_LED , LED_OFF);
    }else if((LED->Signal > SIGNAL_MID_LEVEL)&&(LED->Signal <= SIGNAL_HIG_LEVEL)){
        BSP_LED_CTL(LOW_LED , LED_OFF);
        BSP_LED_CTL(MID_LED , LED_OFF);
        BSP_LED_CTL(HIG_LED , LED_ON);
    }else{
        BSP_LED_CTL(LOW_LED , LED_OFF);
        BSP_LED_CTL(MID_LED , LED_OFF);
        BSP_LED_CTL(HIG_LED , LED_OFF);
        }
    }else{
        if(LED->ConSta == LED_STA_ON){
            LED->ConSta = LED_STA_OFF;
            BSP_LED_CTL(CON_LED , LED_OFF);
            BSP_LED_CTL(LOW_LED , LED_OFF);
            BSP_LED_CTL(MID_LED , LED_OFF);
            BSP_LED_CTL(HIG_LED , LED_OFF);
            BSP_LED_CTL(ERR_LED , LED_OFF);
        }else{
            LED->ConSta = LED_STA_ON;
            BSP_LED_CTL(CON_LED , LED_ON);
            BSP_LED_CTL(LOW_LED , LED_ON);
            BSP_LED_CTL(MID_LED , LED_ON);
            BSP_LED_CTL(HIG_LED , LED_ON);
            BSP_LED_CTL(ERR_LED , LED_ON);
        }
    }
}
uint16_t run_time= 0;
uint8_t config_run_flag_fail = 0;//配置飞行模式失败次数
uint8_t close_sim_mode = 0;

uint8_t run_flag = 0;

uint8_t off_gprs(LedType *LED)
{
    static uint8_t gprs_count = 0;
/******************************信号太差就关闭模块的gprs功能***********************************/
    if(LED->Signal <5)
    {
        gprs_count++;
        if(gprs_count >= 5){
			if(dbgPrintf)(*dbgPrintf)("Enter fly mode (off gprs).......\r\n"); 			
            run_flag =1;
            gprs_count = 0;
        } 
    }else {
        gprs_count = 0;
    }
    return 1;
}
/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/

