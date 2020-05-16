/*****************************************************************************
Copyright: 2014 GREE ELECTRIC APPLIANCES,INC.
File name: bsp.c
Description: 板级支持包，各类硬件底层初始化
Author:  Sean.Y
Version: V1.0
Date:    2015/04/09
History: 修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include  "bsp.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

//#define  FLASH_MEM_ON       //打开擦除flash的宏定义，掉电记忆参数就屏蔽此宏

#define  BSP_RCC_TO_VAL     0x00000FFF                                          /* Max Timeout for RCC register                             */
#ifdef   FLASH_MEM_ON
#define  FLASH_FLAG    0xAA
#else
#define  FLASH_FLAG    0xBB
#endif



void Dev_Param_Init(void);
/*******************************************************
* Function Name: 	BSP_Init
* Purpose: 		各底层驱动初始化
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
void BSP_Init(void)
{     
        BSP_IWDG_Init    ();                                                    /* Initialize the WatchDog module                   */
        BSP_RCC_Init     ();                                                    /* Initialize the Reset and Control (RCC) module    */
        BSP_SYSTICK_Init ();                                                    /* Initialize the SysTick Timer For uCOS-II         */
        BSP_LED_Init     ();                                                    /* Initialize the I/Os for the LEDs                 */
//        BSP_PWRKEY_Init  ();
        BSP_PB_Init      ();                                                    /* Initialize the I/Os for the PBs                  */
        BSP_Flash_Init   ();                                                    /* Initialize the I/Os for the SPIFlash module      */
//        BSP_USART1_Init  ();                                                    /* Initialize the I/Os for the Usart1               */
        BSP_USART2_Init  ();                                                    /* Initialize the I/Os for the Usart2               */
 //       BSP_USART3_Init  ();                                                    /* Initialize the I/Os for the Usart3               */
//        BSP_CAN_Init     ();                                                    /* Initialize the I/Os for the CAN module           */
//        BSP_RTC_Init     ();                                                    /* Initialize the I/Os for the RTC module           */           

 //       if(dbgPrintf)(*dbgPrintf)("bsp init ok!\r\nuCOS-ii Starting...\r\n");
}
/*******************************************************
* Function Name: 	sign_flashread
* Purpose: 		上电读取flash值
* Params :              无
* Return: 		        无
* Limitation: 	        无
*******************************************************/


/*******************************************************
* Function Name: 	BSP_IWDG_Init
* Purpose: 		看门狗初始化程序
* Params :              无
* Return: 		        无
* Limitation: 	        无
*******************************************************/
void  BSP_IWDG_Init  (void)
{
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);                           //进行写操作使能
        IWDG_SetPrescaler(IWDG_Prescaler_256);                                   //设置分频系数
        IWDG_SetReload(0xC35);                                                  //设置重载参数
        IWDG_ReloadCounter();                                                   //喂狗
        IWDG_Enable();                                                          //开启看门狗
}


/*******************************************************
* Function Name: 	BSP_RCC_Init
* Purpose: 		RCC初始化程序
* Params :              无
* Return: 		        无
* Limitation: 	        注意选择内、外部时钟源，时钟数配置。
*******************************************************/
void  BSP_RCC_Init (void)
{
        uint32_t  rcc_to;                                                       /*  RCC registers timeout                                   */
        RCC_DeInit();                                                           /*  Reset the RCC clock config to the default reset state   */
        RCC_HSEConfig(RCC_HSE_ON);                                              /*  HSE Oscillator ON                                       */

        rcc_to = BSP_RCC_TO_VAL;
        while ((rcc_to > 0) &&
               (RCC_WaitForHSEStartUp() != SUCCESS)) {                          /* Wait until the oscilator is stable                       */
                       rcc_to--;
        }

        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);                    /* Fcpu = (PLL_src * PLL_MUL) = (10 Mhz / 1) * (6) = 60Mhz   */
        RCC_PLLCmd(ENABLE);

        rcc_to = BSP_RCC_TO_VAL;
        while ((rcc_to > 0) &&
               (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)) {
                       rcc_to--;
        }

        RCC_HCLKConfig(RCC_SYSCLK_Div1);                                        //设置系统时钟
        RCC_PCLK2Config(RCC_HCLK_Div1);                                         //设置APB2时钟
        RCC_PCLK1Config(RCC_HCLK_Div2);                                         //设置APB1时钟
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);                                       //设置ADC时钟

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);                              //选择系统时钟源
}

/*******************************************************
* Function Name: 	BSP_SYSTICK_Init
* Purpose: 		系统滴答时钟初始化程序，ucOS-ii 时钟源
* Params :              无
* Return: 		无
* Limitation: 	        OS_TICKS_PER_SEC 该宏定义决定滴答频率
*******************************************************/
void  BSP_SYSTICK_Init(void)
{
        RCC_ClocksTypeDef  RCC_Clocks;
        uint32_t cnts;
        RCC_GetClocksFreq(&RCC_Clocks);                                         //获取系统各时钟参数
        cnts = (uint32_t)RCC_Clocks.HCLK_Frequency/OS_TICKS_PER_SEC;

        SysTick_Config(cnts);                                                   //配置滴答时钟中断频率
}

/*******************************************************
* Function Name: 	BSP_Flash_Init
* Purpose: 		Flash初始化程序
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
void  BSP_Flash_Init(void)
{
        sFLASH_Init();                                                          //初始化SPI Flash 底层驱动
}

/*******************************************************
* Function Name: 	Dev_Param_Init
* Purpose: 		设备参数初始化
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
uint8_t Connect_time = 0;



/*******************************************************
* Function Name: 	softReset
* Purpose: 		软复位
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
void softReset( void )
{
        __set_FAULTMASK(1);
        NVIC_SystemReset();
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
        /* User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

        /* Infinite loop */
        while (1)
        {
        }
}
#endif


/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/

