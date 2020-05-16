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
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
/****************************按键邮箱事件块指针*********************************/
OS_EVENT * MSemKEY;  
uint8_t g_CanPolarTmr = 0;
/*******************************************************
* Function Name: 	BSP_PB_Init
* Purpose: 		初始化按键底层驱动
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
void  BSP_PB_Init (void)
{
    GPIO_InitTypeDef  gpio_init;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    gpio_init.GPIO_Pin  = GPIO_Pin_10;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &gpio_init);
    
}


/*******************************************************
* Function Name: 	BSP_PB_GetStatus
* Purpose: 		获取按键状态
* Params :              需要读取的按键ID
* Return: 		返回传输按键值当前的状态
* Limitation: 	        返回0代表按键被按下，返回1代表没有被按下
*******************************************************/
uint8_t  BSP_PB_GetStatus (uint8_t  mf_PushButtonId)
{
    uint8_t  m_Status;
    uint32_t m_RegVal;
    m_Status = 0;    
    switch (mf_PushButtonId) {
        case 1:
            m_RegVal = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10);
            if (m_RegVal == 0) {
                m_Status = 1;
            }
        break;
        
        default:
        break;
    }
    return (m_Status);
}

/*******************************************************
* Function Name: 	BSP_PWRKEY_Init
* Purpose: 		初始化GPRS模块开机键
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
void  BSP_PWRKEY_Init (void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 );
}

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
