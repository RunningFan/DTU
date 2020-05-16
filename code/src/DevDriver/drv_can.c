/*****************************************************************************
Copyright: 2014 GREE ELECTRIC APPLIANCES,INC.
File name: drv_can.c
Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出
值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
Author:  Sean.Y
Version: V1.0
Date:    2015/04/09
History: 修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/

/*---------------------------------头文件处------------------------------------*/
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "duplicate.h"
#include "fsm.h"

PolarType PolarCAN1;

uint8_t  canRxPut = 0;
CanRxMsg RxMessage [CAN_MAX_BUF_LEN];
void    *canRxPtrArr[CAN_MAX_BUF_LEN];
OS_EVENT *QSemCAN;

uint8_t  canTxPut = 0;
CanTxMsg TxMessage [CANTX_MAX_BUF_LEN];
void    *canTxPtrArr[CANTX_MAX_BUF_LEN];
OS_EVENT *QSem_CAN_Tx;
/*-----------------------------------函数处------------------------------------*/

uint32_t CanErrCount;
uint32_t total_count;

void CAN1_SCE_IRQHandler(void)
{
   
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();
     
    CanErrCount++;
    if(CAN_GetITStatus(CAN1 , CAN_IT_ERR))
    {
        CAN_ClearITPendingBit(CAN1 , CAN_IT_ERR);
    }
    if(CAN_GetITStatus(CAN1 , CAN_IT_LEC))
    {
        CAN_ClearITPendingBit(CAN1 , CAN_IT_LEC);
    }
    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}


/*******************************************************
* Function Name: 	USB_LP_CAN1_RX0_IRQHandler
* Purpose: 		CAN中断服务程序，进行数据初级缓存，数据处理邮箱发送等任务
* Params :              无
* Return: 		无
* Limitation: 	        注意调整g_CanRxPut-g_CanRxGet差值来修改处理频率
*******************************************************/
void  USB_LP_CAN1_RX0_IRQHandler(void)
{
    uint8_t err;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();
    
    if(CAN_GetITStatus(CAN1 , CAN_IT_FMP0))
    {
		total_count++;
        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage[canRxPut]);         //接收CAN机组数据
        err = OSQPost(QSemCAN,(void *)&RxMessage[canRxPut]);
        if(err == 0){
            canRxPut++;
        }
        if(canRxPut >= CAN_MAX_BUF_LEN){
            canRxPut = 0;
        }
        CAN_ClearITPendingBit(CAN1 , CAN_IT_FMP0);
    }
    if(CAN_GetITStatus(CAN1 , CAN_IT_FOV0))
    {
        CAN_ClearITPendingBit(CAN1 , CAN_IT_FOV0);
    }
    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
    
}


/*******************************************************
* Function Name: 	BSP_CAN_Init
* Purpose: 		CAN初始化程序，进行BSP初始化
* Params :              无
* Return: 		无
* Limitation: 	        注意调整参数【引脚配置】【波特率】【模式】【过滤器】【中断优先级】等
*******************************************************/
void  BSP_CAN_Init(void)
{
    GPIO_InitTypeDef       GPIO_InitStructure;
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    NVIC_InitTypeDef       NVIC_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    /* Configure CAN Tx (PA.12) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure CAN Rx (PA.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure CAN SW (PA.08) as output PP */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Init CAN1 */
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;                           //CAN_Mode_LoopBack;
    
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
    CAN_InitStructure.CAN_Prescaler =100;
//    CAN_InitStructure.CAN_Prescaler = 40;
    CAN_Init(CAN1, &CAN_InitStructure);
    /* Configure CAN1 Filter */
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;//0xE700;//
    CAN_FilterInitStructure.CAN_FilterIdLow  = 0x0000;//0x0000;//
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//0xFF00;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow  = 0x0000;//0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
    
    /* Configure CAN1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;                    //抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	                        //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                             //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                             //根据指定的参数初始化NVIC寄存器
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                             //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);  
    CAN_ITConfig( CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0 |                \
                        CAN_IT_FMP1 | CAN_IT_FF1 | CAN_IT_FOV1 |				\
                        CAN_IT_ERR  | CAN_IT_LEC | CAN_IT_EWG | CAN_IT_EPV |	\
                        CAN_IT_BOF , ENABLE);
}

/*******************************************************
* Function Name: 	CheckPolarRev
* Purpose: 		无极性CAN引脚电平切换
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
uint8_t EE_flag;//发送EE帧标志
uint8_t send_start = 0;//开始发送EE帧

void CheckPolarRev(PolarType *ptr , uint8_t Mode)
{

    if(!ptr){
        return;
    }else{
        if(Mode == CAN_REVNUM_PLUS){
            if((ptr->Time ++) >= CAN_ONESEC_PSC){                               //计时到1S
                ptr->Time = 0;
                ptr->RevCnt ++;
                if(ptr->RevCnt >= CAN_MAXREV_NUM){
                    ptr->RevCnt = 0;
                    if(ptr->SwitchNum < 5){
                        ptr->SwitchNum ++;
                    }else if(LedValue.ErrSta == LED_STA_OFF){
                        LedValue.ErrSta = LED_STA_ON;
                        send_start = 1;
                    }                            
                    
//                    if((g_Upload.DebugOver)&&(EE_flag ==1)&&(send_start ==1)){
//                            if(!ConSta.Status)
//                            {
//                                if(!connectToServer()){
//	                                while(!disConnectToServer());
//	                                return;
//                                }
//                            }
//                            sendEEFrame(&g_TcpType ,0x07);
//				              if(ConSta.Status){
//				                  while(!disConnectToServer());
//				              }							
//                            send_start = 0;
//                            EE_flag = 0;
//                    }
                    
                    if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8)){              //CAN极性翻转
                        GPIO_ResetBits(GPIOA, GPIO_Pin_8 );
                    }else{
                        GPIO_SetBits(GPIOA, GPIO_Pin_8 );                       //CAN极性翻转
                    }
					g_Upload.can_cnnct_ok = 0;
                    if(dbgPrintf)(*dbgPrintf)("PolarSwitch!\r\n");
                }
            }
        }else{
            if(LedValue.ErrSta == LED_STA_ON){
                LedValue.ErrSta = LED_STA_OFF;
                send_start = 0;
            }
            ptr->SwitchNum = 0;
            ptr->RevCnt = 0;
        }
    }
}

void check_EE_frame(uint8_t trans_mode,uint8_t curSta){
	    if((g_Upload.DebugOver)&&(EE_flag ==1)&&(send_start ==1)){
	        if(!ConSta.Status)
	        {
				SendDataToServer(trans_mode,curSta);
//	            if(!connectToServer()){
//	                while(!disConnectToServer());
//	                return;
//	            }
	        }
	        sendEEFrame(&g_TcpType ,0x07);
#ifdef KEEP_TCP_LIVE
#else
          if(ConSta.Status){
              while(!disConnectToServer());
          }
#endif		  
	        send_start = 0;
	        EE_flag = 0;
	}
		return;
}


/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
