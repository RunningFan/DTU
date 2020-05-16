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
#define APP_TASK_CANRX_PRIO         20
#define APP_TASK_CANRX_STK_SIZE     200

#define APPTASKQUICKPHOTO_PRIO         34
#define APPTASKQUICKPHOTO_STK_SIZE     150

#define APP_TASK_U1RX_PRIO          23 
#define APP_TASK_U1RX_STK_SIZE      300

#define APP_TASK_U2RX_PRIO          19
#define APP_TASK_U2RX_STK_SIZE      128

#define APP_TASK_U3RX_PRIO          27
#define APP_TASK_U3RX_STK_SIZE      300

#define APP_TASK_KEY_PRIO           30
#define APP_TASK_KEY_STK_SIZE       40

#define APP_TASK_INFO_PRIO          36
#define APP_TASK_INFO_STK_SIZE      400

#define APP_TASK_START_PRIO         21                              
#define APP_TASK_START_STK_SIZE     200

#define APP_TASK_GPRS_PRIO          35
#define APP_TASK_GPRS_STK_SIZE      300

#define APP_TASK_CANUPDATE_PRIO         42                              
#define APP_TASK_CANUPDATE_STK_SIZE     40
/*
*********************************************************************************************************
*                                                  LIB
*********************************************************************************************************
*/

#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED

#endif
