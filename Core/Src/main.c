


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "hrtim.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "sample_semantics.h"
#include "sample_processing.h"
#include "power_mode.h"
#include "target_calibration.h"
#include "minimal_state_machine.h"
#include "lowrate_loop_calc.h"
#include "tft_debug_view.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "stm32g4xx_ll_system.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STAGE1_STATUS_PRINT_INTERVAL_MS 1000U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
__IO uint16_t g_adc1_dma_buffer[5] = {0};
__IO uint16_t g_adc2_dma_buffer[3] = {0};
__IO uint8_t g_usart3_rx_byte = 0;
__IO uint8_t g_usart3_last_rx_byte = 0;
__IO uint32_t g_tim2_irq_count = 0;
__IO uint32_t g_tim3_irq_count = 0;
__IO uint32_t g_hrtim_tima_rep_count = 0;
__IO uint32_t g_adc1_dma_cplt_count = 0;
__IO uint32_t g_adc2_dma_cplt_count = 0;
__IO uint32_t g_usart3_rx_event_count = 0;
__IO uint32_t g_usart3_rx_error_count = 0;
__IO uint8_t g_stage1_startup_complete = 0;
static uint32_t s_stage1_last_status_tick = 0;
__IO uint16_t g_uart3_print_div = 0;

/* USART3 命令接收状态：固定格式 V#### */
static uint8_t s_usart3_cmd_active = 0U;
static uint8_t s_usart3_cmd_digits = 0U;
static int32_t s_usart3_cmd_value = 0;

/* USART3 待应用命令：中断里只更新 pending，主循环再真正赋值 */
volatile uint8_t s_vadj_pending_flag = 0U;
volatile int32_t s_vadj_pending_value = 0;

/* vadj 串口接管标志：1=串口值生效，ADC变动<10不更新；0=ADC自由更新 */
uint8_t s_vadj_serial_override = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Stage1_StartSequence(void);
static void Stage1_PrintStatus(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Stage1_StartSequence(void)
{
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc1_dma_buffer, 5U) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)g_adc2_dma_buffer, 3U) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_IT_REP);

  if (HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_A) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_usart3_rx_byte, 1U) != HAL_OK)
  {
    Error_Handler();
  }

  MinimalStateMachine_Init();

  g_stage1_startup_complete = 1U;
  s_stage1_last_status_tick = HAL_GetTick();
}

static void Stage1_PrintStatus(void)
{
  char status_message[256];
  int message_length;

  if ((HAL_GetTick() - s_stage1_last_status_tick) < STAGE1_STATUS_PRINT_INTERVAL_MS)
  {
    return;
  }

  s_stage1_last_status_tick = HAL_GetTick();

  message_length = snprintf(
      status_message,
      sizeof(status_message),
      "U3_OK ST=%s RSTG=%u TA1=%u TA2=%u LOOP=%u RQ1A=%ld RQ2A=%ld VREF=%ld IREF=%ld LVY=%ld ERR=%ld U0=%ld Q1CMD=%ld Q2CMD=%ld LT=%lu\r\n",
      MinimalStateMachine_StateName(g_minimal_state_machine.state),
      (unsigned int)g_minimal_state_machine.rise_stage,
      (unsigned int)g_minimal_state_machine.ta1_output_enabled,
      (unsigned int)g_minimal_state_machine.ta2_output_enabled,
      (unsigned int)g_minimal_state_machine.loop_control_enable,
      (long)g_minimal_state_machine.rise_q1_applied_duty,
      (long)g_minimal_state_machine.rise_q2_applied_duty,
      (long)g_target_calibration.vref_target,
      (long)g_target_calibration.iref_target,
      (long)g_lowrate_loop_calc.feedback_vy,
      (long)g_lowrate_loop_calc.err,
      (long)g_lowrate_loop_calc.u0,
      (long)g_lowrate_loop_calc.q1_duty_cmd,
      (long)g_lowrate_loop_calc.q2_duty_cmd,
      (unsigned long)g_lowrate_loop_calc.tick_count);

  if ((message_length > 0) && ((size_t)message_length < sizeof(status_message)))
  {
    (void)HAL_UART_Transmit(&huart3, (uint8_t *)status_message, (uint16_t)message_length, 100U);
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  LL_DBGMCU_SetTracePinAssignment(LL_DBGMCU_TRACE_NONE);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_HRTIM1_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  Stage1_StartSequence();
  TftDebugView_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* 应用 USART3 中断里解析好的待写入值 */
    if (s_vadj_pending_flag != 0U)
    {
      s_vadj_serial_override = 1U;
      g_sample_processed.vadj_avg = s_vadj_pending_value;
      s_vadj_pending_flag = 0U;
    }

    /* 周期发送一次数据 */
    if (g_uart3_print_div >= 1000)
    {
      char tx_message[96];
      int tx_len;

      g_uart3_print_div = 0;

      tx_len = snprintf(tx_message,
                        sizeof(tx_message),
                        "M=%u,V%ld,%ld,%ld,%ld,%ld,RX=%lu,ER=%lu,LB=%c,VA=%ld,VI=%ld,PF=%u,PV=%ld\r\n",
                        (unsigned int)PowerMode_GetActive(),
                        (long)g_sample_processed.vx_avg,
                        (long)g_sample_processed.ix_avg,
                        (long)g_sample_processed.vy_avg,
                        (long)g_sample_processed.iy_avg,
                        (long)g_target_calibration.vref_target,
                        (unsigned long)g_usart3_rx_event_count,
                        (unsigned long)g_usart3_rx_error_count,
                        (char)g_usart3_last_rx_byte,
                        (long)g_sample_processed.vadj_avg,
                        (long)g_target_calibration.vadj_target_instant,
                        (unsigned int)s_vadj_pending_flag,
                        (long)s_vadj_pending_value);

      if ((tx_len > 0) && ((size_t)tx_len < sizeof(tx_message)))
      {
        (void)HAL_UART_Transmit(&huart3,
                                (uint8_t *)tx_message,
                                (uint16_t)tx_len,
                                1000U);
      }
    }

    TftDebugView_Update();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 18;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    g_tim2_irq_count++;
    SampleSemantic_UpdateFromBuffers();
    SampleProcessing_Update();
    TargetCalibration_UpdateInstant();
    TargetCalibration_UpdateStepped();
    LowRateLoopCalc_Tick();
    MinimalStateMachine_RunControlTick();
  }
  else if (htim->Instance == TIM3)
  {
    g_uart3_print_div++;
    g_tim3_irq_count++;
    MinimalStateMachine_Tick5ms();
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    g_adc1_dma_cplt_count++;
  }
  else if (hadc->Instance == ADC2)
  {
    g_adc2_dma_cplt_count++;
  }
}

void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx)
{
  if ((hhrtim->Instance == HRTIM1) && (TimerIdx == HRTIM_TIMERINDEX_TIMER_A))
  {
    g_hrtim_tima_rep_count++;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    uint8_t byte = g_usart3_rx_byte;
    g_usart3_last_rx_byte = byte;
    g_usart3_rx_event_count++;

    if (s_usart3_cmd_active == 0U)
    {
      if (byte == 'V')
      {
        s_usart3_cmd_active = 1U;
        s_usart3_cmd_digits = 0U;
        s_usart3_cmd_value = 0;
      }
    }
    else
    {
      if ((byte >= '0') && (byte <= '9'))
      {
        s_usart3_cmd_value = (s_usart3_cmd_value * 10) + (int32_t)(byte - '0');
        s_usart3_cmd_digits++;

        if (s_usart3_cmd_digits >= 4U)
        {
          if (s_usart3_cmd_value < 0)
          {
            s_usart3_cmd_value = 0;
          }
          else if (s_usart3_cmd_value > 4095)
          {
            s_usart3_cmd_value = 4095;
          }

          s_vadj_pending_value = s_usart3_cmd_value;
          s_vadj_pending_flag = 1U;

          s_usart3_cmd_active = 0U;
          s_usart3_cmd_digits = 0U;
          s_usart3_cmd_value = 0;
        }
      }
      else
      {
        /* 非法字节，直接丢弃当前命令 */
        s_usart3_cmd_active = 0U;
        s_usart3_cmd_digits = 0U;
        s_usart3_cmd_value = 0;
      }
    }

    if (HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_usart3_rx_byte, 1U) != HAL_OK)
    {
      g_usart3_rx_error_count++;
    }
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    g_usart3_rx_error_count++;
    (void)HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_usart3_rx_byte, 1U);
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
