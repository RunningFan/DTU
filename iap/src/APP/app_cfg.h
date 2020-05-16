/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-LK1 Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.10
* Programmer(s) : BH3NVN
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#define APP_TASK_CANRX_PRIO         4
#define APP_TASK_CANRX_STK_SIZE     128

#define APP_TASK_U1RX_PRIO          5 
#define APP_TASK_U1RX_STK_SIZE      128

#define APP_TASK_U2RX_PRIO          6
#define APP_TASK_U2RX_STK_SIZE      128

#define APP_TASK_U3RX_PRIO          7
#define APP_TASK_U3RX_STK_SIZE      128

#define APP_TASK_KEY_PRIO           8
#define APP_TASK_KEY_STK_SIZE       128

#define APP_TASK_INFO_PRIO          9
#define APP_TASK_INFO_STK_SIZE      512

#define APP_TASK_START_PRIO         10                              
#define APP_TASK_START_STK_SIZE     128

#define APP_TASK_GPRS_PRIO          11
#define APP_TASK_GPRS_STK_SIZE      512//256

/*
*********************************************************************************************************
*                                                  LIB
*********************************************************************************************************
*/

#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED

#endif
