#include "bsp.h"
#include "Compress.h"
#include "time.h"
#include "iap.h" 
#include "stm32f10x_flash.h"

int main(void)
{  
    BSP_Init();  
    
    if(dbgPrintf)(*dbgPrintf)("begin the skip!\r\n");
    sFLASH_ReadBuffer((uint8_t *)&updateinfo, UPDATE_INFO_START_ADDR + 1, sizeof(updateinfo)); 
    if(updatefuc(&updateinfo)){
    	softReset();
    }
//    if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
//    { 
 //       CloseIQHard();
        __disable_irq();   // 
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
        Jump_To_Application = (pFunction) JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) ApplicationAddress);
        Jump_To_Application();
//    }

//      SystemInit();
//      SCB->VTOR = FLASH_BASE|0x4000;
//      NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
//    __enable_irq();//开启总中断
//    OSInit();
//    OSTaskCreate((void (*) (void *)) AppStartTask,                              //指向任务代码的指针
//                 (void *) 0,                                                    //任务开始执行时，传递给任务的参数的指针
//                 (OS_STK *) &AppStartTaskStk[APP_TASK_START_STK_SIZE - 1] ,    //分配给任务的堆栈的栈顶指针   从顶向下递减
//                 (INT8U) APP_TASK_START_PRIO);                                  //分配给任务的优先级
//    OSStart();
//    return(0);
}



/******************* (C) COPYRIGHT 2015 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
