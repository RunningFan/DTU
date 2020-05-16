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


#ifndef _DRV_USART_H_
#define _DRV_USART_H_

/*-----------------------------------头文件------------------------------------*/
#include "stm32f10x.h"
#include <stdarg.h>

/*-----------------------------------宏定义------------------------------------*/
#define  UART1_QUEUE_ENABLE 1
#define  UART2_QUEUE_ENABLE 1
#define  UART3_QUEUE_ENABLE 1
    
#define  UART1RX_INT_PRIO   7
#define  UART1TX_INT_PRIO   7
    
#define  UART2RX_INT_PRIO   6
#define  UART2TX_INT_PRIO   6

#define  UART3RX_INT_PRIO   5
#define  UART3TX_INT_PRIO   5

/*-----------------------------------变量处------------------------------------*/

#if UART1_QUEUE_ENABLE || UART2_QUEUE_ENABLE || UART3_QUEUE_ENABLE
    #define  MAX_UARTTYPE_LEN   1200
    typedef struct 
    {
        uint16_t DataLen;
        uint8_t  DataBuf[MAX_UARTTYPE_LEN];
    }UARTTYPE;
#endif

#if UART1_QUEUE_ENABLE
    #define  UART1_QUEUE_LEN    1
    extern uint8_t  g_UartCnt1;
    extern OS_EVENT *QSemUART1;
    extern UARTTYPE uartRxBuf1[UART1_QUEUE_LEN];
    extern void     *uartRxPtrArr1[UART1_QUEUE_LEN];
#else
    #define  MAX_RECV1_LEN      500
    extern uint8_t  Uart1Buf[];
    extern uint16_t g_Uart1BufCnt;
#endif

#if UART2_QUEUE_ENABLE
    #define  UART2_QUEUE_LEN    1
    extern uint8_t  g_UartCnt2;
    extern OS_EVENT *QSemUART2;
    extern UARTTYPE uartRxBuf2[UART2_QUEUE_LEN];
    extern void     *uartRxPtrArr2[UART2_QUEUE_LEN];
#else
    #define  MAX_RECV2_LEN      500
    extern uint8_t  Uart2Buf[];
    extern uint16_t g_Uart2BufCnt;
#endif
    
#if UART3_QUEUE_ENABLE
    #define  UART3_QUEUE_LEN    1
    extern uint8_t  g_UartCnt3;
    extern OS_EVENT *QSemUART3;
    extern UARTTYPE uartRxBuf3[UART3_QUEUE_LEN];
    extern void     *uartRxPtrArr3[UART3_QUEUE_LEN];
#else
    #define  MAX_RECV3_LEN      500
    extern uint8_t  Uart3Buf[];
    extern uint16_t g_Uart3BufCnt;
#endif
    
    typedef int (*PTRFUN)(const char* , ...);
    typedef void(*SENDARR)(const uint8_t * , uint16_t Len);
    typedef void(*SENDSTR)(const char*);
    
    extern  const PTRFUN  dbgPrintf   ;
    extern  const SENDARR dgbSendArr  ;
    extern  const SENDSTR dgbSendStr  ;
/*-----------------------------------函数处------------------------------------*/

void    BSP_USART1_Init(void);
void    BSP_USART2_Init(void);
void    BSP_USART3_Init(void);
void    uart1SendStr   (const char *s);
void    uart1SendArry  (const uint8_t *s , uint16_t Len);
void    uart2SendStr   (const char *s);
void    uart2SendArry  (const uint8_t *s , uint16_t Len);
void    uart3SendStr   (const char *s);
void    uart3SendArry  (const uint8_t *s , uint16_t Len);
extern uint32_t tcp_rev_total;


uint8_t u1_printf(const char *format, ...);
uint8_t u2_printf(const char *format, ...);
uint8_t u3_printf(const char *format, ...);
#endif

/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
