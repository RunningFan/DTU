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

int record_getelec_init(void)
{
	stat_buffer.dtu_param_buff[DTU_GET_ELEC_COUNT]++;//DTU上电次数：机组得电和DTU复位认为是上电
	stat_buffer.dtu_param_buff[DTU_RESET_COUNT]++;
	return 0;
}


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
        BSP_RCC_Init     ();                                                    /* Initialize the Reset and Control (RCC) module   */
        BSP_SYSTICK_Init ();                                                    /* Initialize the SysTick Timer For uCOS-II         */
        BSP_LED_Init     ();                                                    /* Initialize the I/Os for the LEDs                 */
        BSP_PWRKEY_Init  ();
        BSP_PB_Init      ();                                                    /* Initialize the I/Os for the PBs                  */
        BSP_Flash_Init   ();                                                    /* Initialize the I/Os for the SPIFlash module      */
        BSP_USART1_Init  ();                                                    /* Initialize the I/Os for the Usart1               */
        BSP_USART2_Init  ();                                                    /* Initialize the I/Os for the Usart2               */
        BSP_USART3_Init  ();                                                    /* Initialize the I/Os for the Usart3               */
        BSP_CAN_Init     ();                                                    /* Initialize the I/Os for the CAN module           */
        BSP_RTC_Init     ();                                                    /* Initialize the I/Os for the RTC module           */
        sign_flashread   ();
        Dev_Param_Init   ();
		record_getelec_init();
//		check_power_addr();
        if(dbgPrintf)(*dbgPrintf)("bsp init ok!\r\nuCOS-ii Starting...\r\n");
}
/*******************************************************
* Function Name: 	sign_flashread
* Purpose: 		上电读取flash值
* Params :              无
* Return: 		        无
* Limitation: 	        无
*******************************************************/

void sign_flashread()
{
        uint8_t flash_flag, ee_flag,back_flash_flag;
        uint8_t err = 0 ;
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_ReadBuffer(&flash_flag, STATISTIC_START_ADDR, 1);
		sFLASH_ReadBuffer(&back_flash_flag, BACKUP_STATISTIC_START_ADDR, 1);
        sFLASH_ReadBuffer((uint8_t *)&F4_COUNT, STATISTIC_START_ADDR+1, 2);
        sFLASH_ReadBuffer((uint8_t *)&is_lost_data_flag, STATISTIC_START_ADDR+3, 1);
        OSMutexPost(MutexFlash);

        if((flash_flag==FLASH_FLAG || flash_flag != back_flash_flag)) //1、初次上电 flash_flag 和 back_flash_flag 相等，flash_flag != FLASH_FLAG
        {															//2、正常上电时flash_flag满足，3、打卡备份区域被擦除掉电时，flash_flag != FLASH_FLAG
        															//而 flash_flag != back_flash_flag 满足
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_ReadBuffer((uint8_t *)&updateinfo, UPDATE_INFO_START_ADDR + 1, sizeof(updateinfo));
			if(is_lost_data_flag != 0xBB){//如果is_lost_data_flag 被擦除，则用备份去替换
		        sFLASH_ReadBuffer(&flash_flag, BACKUP_STATISTIC_START_ADDR, 1);
		        sFLASH_ReadBuffer((uint8_t *)&F4_COUNT, BACKUP_STATISTIC_START_ADDR+1, 2);
		        sFLASH_ReadBuffer((uint8_t *)&is_lost_data_flag, BACKUP_STATISTIC_START_ADDR+3, 1);
				sFLASH_ReadBuffer((uint8_t *)&stat_buffer , BACKUP_STATISTIC_START_ADDR+4, sizeof(stat_buffer));
			}else{
	            sFLASH_ReadBuffer((uint8_t *)&stat_buffer , STATISTIC_START_ADDR+4, sizeof(stat_buffer));
			}
			OSMutexPost(MutexFlash);
			F4_DTU_COUNT = stat_buffer.F4_DTU_COUNT;
        }else{

			flash_flag = 0xBB;
			F4_COUNT = 0;
			is_lost_data_flag = 0xBB;
            OSMutexPend(MutexFlash , 0 , &err);
			sFLASH_EraseSubSector(USERHABIT_START_ADDR);
            sFLASH_EraseSubSector(STATISTIC_START_ADDR);
            sFLASH_EraseSubSector(BACKUP_STATISTIC_START_ADDR);
            memset((uint8_t *)&stat_buffer,0,sizeof(stat_buffer));
            stat_buffer.usr_hbt_time = 168;
	        stat_buffer.stat_data_time = 720;
			stat_buffer.config_flag = 0;
			stat_buffer.config_time = 30;
            sFLASH_WriteBuffer(&flash_flag ,STATISTIC_START_ADDR,1);
            sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,STATISTIC_START_ADDR+1,2);
            sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,STATISTIC_START_ADDR+3,1);
            sFLASH_WriteBuffer((uint8_t *)&stat_buffer , STATISTIC_START_ADDR+4 , sizeof(stat_buffer));
            sFLASH_WriteBuffer(&flash_flag ,BACKUP_STATISTIC_START_ADDR,1);
            sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,BACKUP_STATISTIC_START_ADDR+1,2);
            sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,BACKUP_STATISTIC_START_ADDR+3,1);
            sFLASH_WriteBuffer((uint8_t *)&stat_buffer , BACKUP_STATISTIC_START_ADDR+4 , sizeof(stat_buffer));
            OSMutexPost(MutexFlash);
            ee_flag = 0xBB;
            updateinfo.update_success_flag = 1;
			updateinfo.begin_recover_time = 5;
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
            sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
            sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
            sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
            OSMutexPost(MutexFlash);
        }

}

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
//        IWDG_SetReload(0xC35);                                                  //设置重载参数
		IWDG_SetReload(0xFFF);												  //设置重载参数
        IWDG_ReloadCounter();                                                   //喂狗
        IWDG_Enable();                                                          //开启看门狗
}

void diable_IWDG(void)
{
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);                           //进行写操作使能
        IWDG_SetPrescaler(IWDG_Prescaler_256);                                   //设置分频系数
        IWDG_SetReload(0xC35);                                                  //设置重载参数
        IWDG_ReloadCounter();
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
void  Dev_Param_Init(void)
{
    uint8_t err;
    /**************************************************/
    LedValue.ConSta = LED_STA_OFF;
    LedValue.ErrSta = LED_STA_OFF;
    LedValue.Signal = 0;

    /**************************************************/
    memset(&ACParam   , 0 ,sizeof(ACParam));//待定
    /**************************************************/
    GprsParam.EraserFlag = 0;
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_ReadBuffer(&GprsParam.EraserFlag , GPRS_INFO_START_ADDR , 1);
    OSMutexPost(MutexFlash);
    if(GprsParam.EraserFlag == FLASH_FLAG){
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_ReadBuffer((uint8_t *)&GprsParam , GPRS_INFO_START_ADDR , sizeof(GprsParam));
        sFLASH_ReadBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
        sFLASH_ReadBuffer((uint8_t *)&Receive_Real_Page , RECEIVE_PAGE_START_ADDR , sizeof(Receive_Real_Page));
        var.buttonPush = var.Hyaline =var.onlineTest = var.projDebug = var.s_update= 0;
        OSMutexPost(MutexFlash);
    }else{
        GprsParam.EraserFlag = 0xBB;
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(GPRS_INFO_START_ADDR);
        sFLASH_EraseSubSector(VAR_START_ADDR);
        sFLASH_WriteBuffer((uint8_t *)&GprsParam , GPRS_INFO_START_ADDR , sizeof(GprsParam));
        memset((uint8_t *)&var,0,sizeof(var));
        sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
        sFLASH_EraseSubSector(RECEIVE_PAGE_START_ADDR);
        sFLASH_WriteBuffer((uint8_t *)&Receive_Real_Page , RECEIVE_PAGE_START_ADDR , sizeof(Receive_Real_Page));
        OSMutexPost(MutexFlash);
    }

    g_Upload.EraserFlag = 0;
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_ReadBuffer(&g_Upload.EraserFlag , ERR_INFO_START_ADDR , 1);
    OSMutexPost(MutexFlash);
    if(g_Upload.EraserFlag == FLASH_FLAG){
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_ReadBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
        OSMutexPost(MutexFlash);
		g_Upload.RealDataLen = 0;
		g_Upload.TakeDataLen = 0;
		g_Upload.BigDataLen = 0;
		g_Upload.SendBigDataLen = 0;
		g_Upload.ErrDataLen = 0;
		g_Upload.BigDataLen = 0;
		g_Upload.PowerDataLen = 0;
		g_Upload.data_type = 0;
        if(g_Upload.SendFlag){
            if(g_Upload.OutErr){
                var.outdoorErr = 1;
            }else if(g_Upload.InErr){
                var.indoorErr  = 1;
            }
        }
        g_Upload.writeFlashAddr = ((g_Upload.writeFlashAddr / SECTOR_SIZE))*SECTOR_SIZE;
    }else{
        g_Upload.EraserFlag = 0xBB;
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
        sFLASH_WriteBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType) - G_UPLOAD_BUFFER_LEN*1024);
        OSMutexPost(MutexFlash);
    }
    /**************************************************/
}


/*******************************************************
* Function Name: 	softReset
* Purpose: 		软复位
* Params :              无
* Return: 		无
* Limitation: 	        无
*******************************************************/
void softReset( void )
{
	    uint8_t err = 0;
  		uint8_t flash_flag = 0xBB;
		stat_buffer.dtu_param_buff[DTU_GET_ELEC_COUNT]++;
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
        sFLASH_WriteBuffer(&flash_flag ,STATISTIC_START_ADDR,1);
        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,STATISTIC_START_ADDR+1,2);
        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,STATISTIC_START_ADDR+3,1);
        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,STATISTIC_START_ADDR+4,sizeof(stat_buffer));
        OSMutexPost(MutexFlash);
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

