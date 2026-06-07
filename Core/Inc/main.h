/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern __IO uint16_t g_adc1_dma_buffer[5];
extern __IO uint16_t g_adc2_dma_buffer[3];
extern __IO uint8_t g_usart3_rx_byte;
extern __IO uint8_t g_usart3_last_rx_byte;
extern __IO uint32_t g_tim2_irq_count;
extern __IO uint32_t g_tim3_irq_count;
extern __IO uint32_t g_hrtim_tima_rep_count;
extern __IO uint32_t g_adc1_dma_cplt_count;
extern __IO uint32_t g_adc2_dma_cplt_count;
extern __IO uint32_t g_usart3_rx_event_count;
extern __IO uint32_t g_usart3_rx_error_count;
extern __IO uint8_t g_stage1_startup_complete;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_VX_Pin GPIO_PIN_0
#define ADC_VX_GPIO_Port GPIOA
#define ADC_IX_Pin GPIO_PIN_1
#define ADC_IX_GPIO_Port GPIOA
#define ADC_VY_Pin GPIO_PIN_2
#define ADC_VY_GPIO_Port GPIOA
#define ADC_IY_Pin GPIO_PIN_3
#define ADC_IY_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define LED_G_Pin GPIO_PIN_13
#define LED_G_GPIO_Port GPIOC
#define LED_Y_Pin GPIO_PIN_14
#define LED_Y_GPIO_Port GPIOC
#define LED_R_Pin GPIO_PIN_15
#define LED_R_GPIO_Port GPIOC
#define KEY1_Pin GPIO_PIN_13
#define KEY1_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_14
#define KEY2_GPIO_Port GPIOB
#define KEY_SW_Pin GPIO_PIN_4
#define KEY_SW_GPIO_Port GPIOB

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
