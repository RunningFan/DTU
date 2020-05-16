/*****************************************************************************
Copyright:   2014 GREE ELECTRIC APPLIANCES,INC.
File name:   drv_usart.c
Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
Author:      Sean.Y
Version:     V1.0
Date:        2015/04/09
History:     修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。
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
#include "duplicate.h"

/*-----------------------------------变量处------------------------------------*/ 
#define UARTPORT USART2
void    noEnterSendArry(const uint8_t *s , uint16_t Len);
const   PTRFUN  dbgPrintf   = printf;
#ifdef TEST_PRINT_TEST
const   SENDARR dgbSendArr  = NULL;
#else
const   SENDARR dgbSendArr  = noEnterSendArry;
#endif
const   SENDSTR dgbSendStr  = NULL;

#if UART1_QUEUE_ENABLE
    uint8_t  g_UartCnt1 = 0;
    OS_EVENT *QSemUART1 = NULL;
    UARTTYPE uartRxBuf1[UART1_QUEUE_LEN];
    void     *uartRxPtrArr1[UART1_QUEUE_LEN];
#else
    uint16_t g_Uart1BufCnt;
    uint8_t  Uart1Buf[MAX_UARTTYPE_LEN];
#endif
    
#if UART2_QUEUE_ENABLE
    uint8_t  g_UartCnt2 = 0;
    OS_EVENT *QSemUART2 = NULL;
    UARTTYPE uartRxBuf2[UART2_QUEUE_LEN];
    void     *uartRxPtrArr2[UART2_QUEUE_LEN];
#else
    uint16_t g_Uart2BufCnt;
    uint8_t  Uart2Buf[MAX_RECV2_LEN]; 
#endif
    
#if UART3_QUEUE_ENABLE
    uint8_t  g_UartCnt3 = 0;
    OS_EVENT *QSemUART3 = NULL;
    UARTTYPE uartRxBuf3[UART3_QUEUE_LEN];
    void     *uartRxPtrArr3[UART3_QUEUE_LEN];
#else
    uint16_t g_Uart3BufCnt;
    uint8_t  Uart3Buf[MAX_RECV3_LEN]; 
#endif

/*******************************************************
* Function Name: 	BSP_USART1_Init
* Purpose: 		    串口1 初始化程序，包含引脚配置、中断配置及串口各参数的配置
* Params :          无
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void  BSP_USART1_Init (void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	                //使能GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);                  //使能UART时钟

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ; 	                //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	                        //子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                             //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                             //根据指定的参数初始化NVIC寄存器
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Init(USART1, &USART_InitStruct);                                      /* Init USART1  */
    USART_Cmd(USART1, ENABLE);                                                  /* Enable USART */
    TIM3_Init (499,5999);		                                                /* Initialize the Timer 10ms for the Usart1 module        */
    TIM3_Set        (0);                                                        /* Disable    the Usart1 Timer                            */
}


/*******************************************************
* Function Name: 	BSP_USART2_Init
* Purpose: 		    调试串口 初始化程序，包含引脚配置、中断配置及串口各参数的配置
* Params :          无
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void  BSP_USART2_Init    (void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART2 Pins Software Remapping */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	                //使能GPIO时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);                  //使能UART时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
    /* Configure USART2 Tx (PD.05) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    /* Configure USART2 Rx (PD.06) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USART2 Interrupt*/
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;                    //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	                        //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                             //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                             //根据指定的参数初始化NVIC寄存器
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    USART_Init(USART2, &USART_InitStruct);                                      /* Init USART2  */
    USART_Cmd(USART2, ENABLE);                                                  /* Enable USART */
    TIM4_Init (199,5999);		                                                /* Initialize the Timer 10ms for the Usart2 module         */
    TIM4_Set        (0);                                                        /* Disable    the Usart2 Timer                             */
}


/*******************************************************
* Function Name: 	BSP_USART3_Init
* Purpose: 		    AT指令处理串口 初始化程序，包含引脚配置、中断配置及串口各参数的配置
* Params :          无
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void  BSP_USART3_Init  (void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART3 Pins Software Remapping */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	                //使能GPIO时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);                  //使能UART时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
    /* Configure USART3 Tx (PD.08) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    /* Configure USART3 Rx (PD.09) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USART3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;                    //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	                        //子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                             //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                             //根据指定的参数初始化NVIC寄存器

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Init(USART3, &USART_InitStruct);                                      /* Init USART3  */
    USART_Cmd(USART3, ENABLE);                                                  /* Enable USART */
    TIM5_Init (199,5999);		                                                /* Initialize the Timer 10ms for the Usart3 module         */
    TIM5_Set        (0);                                                        /* Disable    the Usart3 Timer                             */
}

/*******************************************************
* Function Name: 	USART1_IRQHandler
* Purpose: 		    串口1 中断服务程序 ，串口1用于PPP协议数据传输
* Params :          无
* Return: 		    无
* Limitation: 	    串口接收的数据放在gcha_UartPPPRxBuf内，当接收数据时间超过10ms、
*                   或在数据量已经满了的时候将进行一次完成接收的标记
*******************************************************/
void  USART1_IRQHandler        (void)
{
    uint8_t err , res;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif

    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
        res = USART_ReceiveData(USART1);
        if(uartRxBuf1[g_UartCnt1].DataLen < MAX_UARTTYPE_LEN)                   //还可以接收数据
        {
            TIM3_Set(1);
            /*********************记录接收到的值**********************/
            uartRxBuf1[g_UartCnt1].DataBuf[uartRxBuf1[g_UartCnt1].DataLen++] = res;              
        }else if((uartRxBuf1[g_UartCnt1].DataLen & 0x8000) != 0x8000){
            uartRxBuf1[g_UartCnt1].DataBuf[MAX_UARTTYPE_LEN - 1] = '\0';
            err = OSQPost(QSemUART1,(void *)&uartRxBuf1[g_UartCnt1]);
            if(OS_ERR_NONE == err){
                uartRxBuf1[g_UartCnt1].DataLen |= 1<<15;                        //强制标记接收完成
                g_UartCnt1++;
            }
            if(g_UartCnt1 >= UART1_QUEUE_LEN){
                g_UartCnt1 = 0;
            }
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);                 //清除接收中断标志位
    }
    
    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}


/*******************************************************
* Function Name: 	USART2_IRQHandler
* Purpose: 		    串口2 中断服务程序 ，串口2用调试信息显示
* Params :          无
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void  USART2_IRQHandler        (void)
{
    uint8_t err , res;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif

    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
        res = USART_ReceiveData(USART2);
        if(uartRxBuf2[g_UartCnt2].DataLen < MAX_UARTTYPE_LEN)                   //还可以接收数据
        {
            TIM4_Set(1);
            /*********************记录接收到的值**********************/
            uartRxBuf2[g_UartCnt2].DataBuf[uartRxBuf2[g_UartCnt2].DataLen++] = res;               
        }else if((uartRxBuf2[g_UartCnt2].DataLen & 0x8000) != 0x8000){
            uartRxBuf2[g_UartCnt2].DataBuf[MAX_UARTTYPE_LEN - 1] = '\0';
            err = OSQPost(QSemUART2,(void *)&uartRxBuf2[g_UartCnt2]);
            if(OS_ERR_NONE == err){
                uartRxBuf2[g_UartCnt2].DataLen |= 1<<15;                        //强制标记接收完成
                g_UartCnt2++;
            }
            if(g_UartCnt2 >= UART2_QUEUE_LEN){
                g_UartCnt2 = 0;
            }
        }
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);                         //清除接收中断标志位
    }

    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}

/*******************************************************
* Function Name: 	USART3_IRQHandler
* Purpose: 		    串口3 中断服务程序 ，串口3用于AT指令处理，包含短信、语音等内容
* Params :          无
* Return: 		    无
* Limitation: 	    串口接收的数据放在gcha_UartATRxBuf内，当接收数据时间超过10ms、
*                   或在数据量已经满了的时候将进行一次完成接收的标记
*******************************************************/
uint32_t tcp_rev_total;

void  USART3_IRQHandler        (void)
{
    uint8_t err , res;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif

    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
        res = USART_ReceiveData(USART3);
//		tcp_rev_total++;
        if(uartRxBuf3[g_UartCnt3].DataLen < MAX_UARTTYPE_LEN)                   //还可以接收数据
        {
            TIM5_Set(1);
            /*********************记录接收到的值**********************/
            uartRxBuf3[g_UartCnt3].DataBuf[uartRxBuf3[g_UartCnt3].DataLen++] = res;               
        }else if((uartRxBuf3[g_UartCnt3].DataLen & 0x8000) != 0x8000){
            uartRxBuf3[g_UartCnt3].DataBuf[MAX_UARTTYPE_LEN - 1] = '\0';
            err = OSQPost(QSemUART3,(void *)&uartRxBuf3[g_UartCnt3]);
            if(OS_ERR_NONE == err){
                uartRxBuf3[g_UartCnt3].DataLen |= 1<<15;                        //强制标记接收完成
                g_UartCnt3++;
            }
            if(g_UartCnt3 >= UART3_QUEUE_LEN){
                g_UartCnt3 = 0;
            }
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);                         //清除接收中断标志位
    }

    OSIntExit();            /* Tell uC/OS-II that we are leaving the ISR            */
}

/*******************************************************
* Function Name: 	sendArrData
* Purpose: 		    用串口3向模块发送一个数据
* Params :          *s:要发送的数据起始地址
*                   Arry:要发送的数据大小
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void uart1SendArry(const uint8_t *s , uint16_t Len)
{
    uint16_t i = 0;
    uint8_t err = 0;
//    OSMutexPend(MutexSend1 , 0 , &err); 
    for(i = 0 ; i < Len; i++ ){
        USART_ClearFlag(USART1,USART_FLAG_TC);
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART */
        USART_SendData(USART1, (uint8_t)*s++);
        /* Loop until the end of transmission */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {}
    }
//    OSMutexPost(MutexSend1);
}


/*******************************************************
* Function Name: 	uart1SendStr
* Purpose: 		    用串口3向模块发送一个字符串
* Params :          *s:要发送的字符串地址
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void uart1SendStr(const char *s)
{
    uint8_t err = 0;
//    OSMutexPend(MutexSend1 , 0 , &err); 
    while(*s != '\0'){
        USART_ClearFlag(USART1,USART_FLAG_TC);
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART */
        USART_SendData(USART1, (uint8_t)*s++);
        /* Loop until the end of transmission */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {}
    }
//    OSMutexPost(MutexSend1);
}


/*******************************************************
* Function Name: 	uart2SendArry
* Purpose: 		    用串口3向模块发送一个数据
* Params :          *s:要发送的数据起始地址
*                   Arry:要发送的数据大小
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void uart2SendArry(const uint8_t *s , uint16_t Len)
{
    uint16_t i = 0; 
    uint8_t err = 0;
   	OSMutexPend(MutexSend2 , 0 , &err); 
    for(i = 0 ; i < Len; i++ ){   
        USART_ClearFlag(USART2,USART_FLAG_TC);
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART */
        USART_SendData(USART2, (uint8_t)*s++);
        /* Loop until the end of transmission */
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
        {}
    }
        OSMutexPost(MutexSend2);
}

/*******************************************************
* Function Name: 	uart2SendStr
* Purpose: 		    用串口3向模块发送一个字符串
* Params :          *s:要发送的字符串地址
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void uart2SendStr(const char *s)
{
    uint8_t err = 0;
    OSMutexPend(MutexSend2 , 0 , &err); 
    while(*s != '\0'){
        USART_ClearFlag(USART2,USART_FLAG_TC);
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART */
        USART_SendData(USART2, (uint8_t)*s++);
        /* Loop until the end of transmission */
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
        {}
    }
    OSMutexPost(MutexSend2);
}

/*******************************************************
* Function Name: 	uart3SendArry
* Purpose: 		    用串口1向模块发送一个数据
* Params :          *s:要发送的数据起始地址
*                   Arry:要发送的数据大小
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void uart3SendArry(const uint8_t *s , uint16_t Len)
{
    uint16_t i = 0;
    uint8_t err = 0;
//    OSMutexPend(MutexSend3 , 0 , &err);//互斥锁 开启
    for(i = 0 ; i < Len; i++ ){
        USART_ClearFlag(USART3,USART_FLAG_TC);
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART */
        USART_SendData(USART3, (uint8_t)*s++);
        /* Loop until the end of transmission */
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
        {}
    }
	OSSemPost(SemHandleTxFrame);	
//    OSMutexPost(MutexSend3); //互斥锁 关闭
}


/*******************************************************
* Function Name: 	uart3SendStr
* Purpose: 		    用串口3向模块发送一个字符串
* Params :          *s:要发送的字符串地址
* Return: 		    无
* Limitation: 	    无
*******************************************************/
void uart3SendStr(const char *s)
{
    uint8_t err=0;
//    OSMutexPend(MutexSend3 , 0 , &err); 
    while(*s != '\0'){
        USART_ClearFlag(USART3,USART_FLAG_TC);
        /* Place your implementation of fputc here */
        /* e.g. write a character to the USART */
        USART_SendData(USART3, (uint8_t)*s++);
        /* Loop until the end of transmission */
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
        {}
    }
//    OSMutexPost(MutexSend3); //互斥锁 关闭
}

/*******************************************************
* Function Name: 	int fputc(int ch, FILE *f)
* Purpose: 		    标准打印函数重定义
* Params :          int ch 要输出的字符, FILE *f 没有用到
* Return: 		    无
* Limitation: 	    根据编译器类型选择相应的重定义函数
*******************************************************/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
    USART_ClearFlag(UARTPORT,USART_FLAG_TC);

    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(UARTPORT, (uint8_t) ch);

    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(UARTPORT, USART_FLAG_TC) == RESET)
    {}

    return ch;
}

uint8_t u1_printf(const char *format, ...)
{
    char s[200];
    va_list args;
    if(s){
        va_start(args, format);
        vsprintf(s,format,args);
        va_end(args);
        uart1SendStr(s);
        return 1;
    }else{
        return 0;
    }
}

uint8_t u2_printf(const char *format, ...)
{
    char s[200];
    va_list args;
    if(s){
        va_start(args, format);
        vsprintf(s,format,args);
        va_end(args);
        uart2SendStr(s);
        return 1;
    }else{
        return 0;
    }
}

uint8_t u3_printf(const char *format, ...)
{
    char s[200];
    va_list args;
    if(s){
        va_start(args, format);
        vsprintf(s,format,args);
        va_end(args);
        uart3SendStr(s);
        return 1;
    }else{
        return 0;
    }
}

char sendOneChar(char ch)
{
    USART_ClearFlag(USART2,USART_FLAG_TC);
    USART_SendData(USART2, (uint8_t) ch);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    return ch;
}

void noEnterSendArry(const uint8_t *s , uint16_t Len)
{
    uint16_t i = 0;
    for(i = 0 ; i < Len; i++ ){
        if((*s != '\r') && (*s != '\n')){
            sendOneChar(*s++);
        }else{
            sendOneChar(' ');
            s++;
        }
    }
    uart2SendStr("\r\n");
}
/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
