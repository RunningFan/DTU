/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_IT_H
#define __STM32F10x_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void Reset_Handler             (void);// Reset Handler
void NMI_Handler               (void);// NMI Handler
void HardFault_Handler         (void);// Hard Fault Handler
void MemManage_Handler         (void);// MPU Fault Handler
void BusFault_Handler          (void);// Bus Fault Handler
void UsageFault_Handler        (void);// Usage Fault Handler
void SVC_Handler               (void);// SVCall Handler
void DebugMon_Handler          (void);// Debug Monitor Handler
void PendSV_Handler            (void);// PendSV Handler
void SysTick_Handler           (void);// SysTick Handler
// External Interrupts
void WWDG_IRQHandler           (void);// Window Watchdog
void PVD_IRQHandler            (void);// PVD through EXTI Line detect
void TAMPER_IRQHandler         (void);// Tamper
void RTC_IRQHandler            (void);// RTC
void FLASH_IRQHandler          (void);// Flash
void RCC_IRQHandler            (void);// RCC
void EXTI0_IRQHandler          (void);// EXTI Line 0
void EXTI1_IRQHandler          (void);// EXTI Line 1
void EXTI2_IRQHandler          (void);// EXTI Line 2
void EXTI3_IRQHandler          (void);// EXTI Line 3
void EXTI4_IRQHandler          (void);// EXTI Line 4
void DMA1_Channel1_IRQHandler  (void);// DMA1 Channel 1
void DMA1_Channel2_IRQHandler  (void);// DMA1 Channel 2
void DMA1_Channel3_IRQHandler  (void);// DMA1 Channel 3
void DMA1_Channel4_IRQHandler  (void);// DMA1 Channel 4
void DMA1_Channel5_IRQHandler  (void);// DMA1 Channel 5
void DMA1_Channel6_IRQHandler  (void);// DMA1 Channel 6
void DMA1_Channel7_IRQHandler  (void);// DMA1 Channel 7
void ADC1_2_IRQHandler         (void);// ADC1 & ADC2
void USB_HP_CAN1_TX_IRQHandler (void);// USB High Priority or CAN1 TX
void USB_LP_CAN1_RX0_IRQHandler(void);// USB Low  Priority or CAN1 RX0
void CAN1_RX1_IRQHandler       (void);// CAN1 RX1
void CAN1_SCE_IRQHandler       (void);// CAN1 SCE
void EXTI9_5_IRQHandler        (void);// EXTI Line 9..5
void TIM1_BRK_IRQHandler       (void);// TIM1 Break
void TIM1_UP_IRQHandler        (void);// TIM1 Update
void TIM1_TRG_COM_IRQHandler   (void);// TIM1 Trigger and Commutation
void TIM1_CC_IRQHandler        (void);// TIM1 Capture Compare
void TIM2_IRQHandler           (void);// TIM2
void TIM3_IRQHandler           (void);// TIM3
void TIM4_IRQHandler           (void);// TIM4
void I2C1_EV_IRQHandler        (void);// I2C1 Event
void I2C1_ER_IRQHandler        (void);// I2C1 Error
void I2C2_EV_IRQHandler        (void);// I2C2 Event
void I2C2_ER_IRQHandler        (void);// I2C2 Error
void SPI1_IRQHandler           (void);// SPI1
void SPI2_IRQHandler           (void);// SPI2
void USART1_IRQHandler         (void);// USART1
void USART2_IRQHandler         (void);// USART2
void USART3_IRQHandler         (void);// USART3
void EXTI15_10_IRQHandler      (void);// EXTI Line 15..10
void RTCAlarm_IRQHandler       (void);// RTC Alarm through EXTI Line
void USBWakeUp_IRQHandler      (void);// USB Wakeup from suspend
void TIM8_BRK_IRQHandler       (void);// TIM8 Break
void TIM8_UP_IRQHandler        (void);// TIM8 Update
void TIM8_TRG_COM_IRQHandler   (void);// TIM8 Trigger and Commutation
void TIM8_CC_IRQHandler        (void);// TIM8 Capture Compare
void ADC3_IRQHandler           (void);// ADC3
void FSMC_IRQHandler           (void);// FSMC
void SDIO_IRQHandler           (void);// SDIO
void TIM5_IRQHandler           (void);// TIM5
void SPI3_IRQHandler           (void);// SPI3
void UART4_IRQHandler          (void);// UART4
void UART5_IRQHandler          (void);// UART5
void TIM6_IRQHandler           (void);// TIM6
void TIM7_IRQHandler           (void);// TIM7
void DMA2_Channel1_IRQHandler  (void);// DMA2 Channel1
void DMA2_Channel2_IRQHandler  (void);// DMA2 Channel2
void DMA2_Channel3_IRQHandler  (void);// DMA2 Channel3
void DMA2_Channel4_5_IRQHandler(void);// DMA2 Channel4 & Channel5

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
