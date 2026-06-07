#include "minimal_state_machine.h"
#include "hrtim.h"
#include "lowrate_loop_calc.h"
#include "power_mode.h"

#define RISE_MIN_DUTY 100
#define RISE_MAX_DUTY 30767
#define RISE_DUTY_STEP 300
#define TA1_SAFE_MAX_DUTY_Q15 30767
#define HRTIM_PERIOD_COUNTS 23039
#define HRTIM_ADC_TRIGGER_MAX 7000
#define HRTIM_Q2_OFFSET_COUNTS 921
#define HRTIM_Q2_END_MAX_COUNTS 22120

static void MinimalStateMachine_ApplyTa1Duty(void)
{
  int32_t applied_q1_duty = g_minimal_state_machine.rise_q1_duty;
  int32_t comp1;
  int32_t comp4;

  if (applied_q1_duty < RISE_MIN_DUTY)
  {
    applied_q1_duty = RISE_MIN_DUTY;
  }

  if (applied_q1_duty > TA1_SAFE_MAX_DUTY_Q15)
  {
    applied_q1_duty = TA1_SAFE_MAX_DUTY_Q15;
  }

  g_minimal_state_machine.rise_q1_applied_duty = applied_q1_duty;

  comp1 = (applied_q1_duty * HRTIM_PERIOD_COUNTS) >> 15;
  if (comp1 < 1)
  {
    comp1 = 1;
  }
  if (comp1 > HRTIM_PERIOD_COUNTS)
  {
    comp1 = HRTIM_PERIOD_COUNTS;
  }

  comp4 = comp1 >> 1;
  if (comp4 < 1)
  {
    comp4 = 1;
  }
  if (comp4 > HRTIM_ADC_TRIGGER_MAX)
  {
    comp4 = HRTIM_ADC_TRIGGER_MAX;
  }

  __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, (uint32_t)comp1);
  __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_4, (uint32_t)comp4);
}

static void MinimalStateMachine_ApplyTa2Window(void)
{
  int32_t applied_q1_duty = g_minimal_state_machine.rise_q1_applied_duty;
  int32_t applied_q2_duty = g_minimal_state_machine.rise_q2_duty;
  int32_t comp1;
  int32_t comp2;
  int32_t comp3;

  if (applied_q2_duty < RISE_MIN_DUTY)
  {
    applied_q2_duty = RISE_MIN_DUTY;
  }

  if (applied_q2_duty > g_minimal_state_machine.rise_q2_max_duty)
  {
    applied_q2_duty = g_minimal_state_machine.rise_q2_max_duty;
  }

  if (applied_q2_duty > RISE_MAX_DUTY)
  {
    applied_q2_duty = RISE_MAX_DUTY;
  }

  g_minimal_state_machine.rise_q2_applied_duty = applied_q2_duty;

  comp1 = (applied_q1_duty * HRTIM_PERIOD_COUNTS) >> 15;
  if (comp1 < 1)
  {
    comp1 = 1;
  }
  if (comp1 > HRTIM_PERIOD_COUNTS)
  {
    comp1 = HRTIM_PERIOD_COUNTS;
  }

  comp2 = comp1 + HRTIM_Q2_OFFSET_COUNTS;
  if (comp2 > HRTIM_Q2_END_MAX_COUNTS)
  {
    comp2 = HRTIM_Q2_END_MAX_COUNTS;
  }

  comp3 = comp2 + ((applied_q2_duty * HRTIM_PERIOD_COUNTS) >> 15);
  if (comp3 > HRTIM_Q2_END_MAX_COUNTS)
  {
    comp3 = HRTIM_Q2_END_MAX_COUNTS;
  }

  __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, (uint32_t)comp2);
  __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, (uint32_t)comp3);
}

static void MinimalStateMachine_UpdateIndicators(void)
{
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);

  switch (g_minimal_state_machine.state)
  {
  case STATE_INIT:
    HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
    break;

  case STATE_WAIT:
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
    break;

  case STATE_RISE:
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
    break;

  case STATE_RUN:
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
    break;

  case STATE_ERR:
    HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
    break;

  default:
    break;
  }
}

volatile MinimalStateMachine_t g_minimal_state_machine = {
    STATE_INIT,
    0U,
    0U,
    0U,
    0,
    0,
    0U,
    0,
    0,
    RISE_MIN_DUTY,
    RISE_MIN_DUTY,
    RISE_MIN_DUTY,
    RISE_MIN_DUTY,
    RISE_MIN_DUTY,
    RISE_MIN_DUTY,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    POWER_MODE_BUCK,
    0U};

void MinimalStateMachine_Init(void)
{
  g_minimal_state_machine.state = STATE_INIT;
  g_minimal_state_machine.wait_counter = 0U;
  g_minimal_state_machine.offset_capture_complete = 0U;
  g_minimal_state_machine.output_enabled_request = 0U;
  g_minimal_state_machine.vref_snapshot = 0;
  g_minimal_state_machine.iref_snapshot = 0;
  g_minimal_state_machine.rise_stage = 0U;
  g_minimal_state_machine.rise_vref_start = 0;
  g_minimal_state_machine.rise_iref_start = 0;
  g_minimal_state_machine.rise_q1_max_duty = RISE_MIN_DUTY;
  g_minimal_state_machine.rise_q2_max_duty = RISE_MIN_DUTY;
  g_minimal_state_machine.rise_q1_duty = RISE_MIN_DUTY;
  g_minimal_state_machine.rise_q1_applied_duty = RISE_MIN_DUTY;
  g_minimal_state_machine.rise_q2_duty = RISE_MIN_DUTY;
  g_minimal_state_machine.rise_q2_applied_duty = RISE_MIN_DUTY;
  g_minimal_state_machine.rise_q1_counter = 0U;
  g_minimal_state_machine.rise_q2_counter = 0U;
  g_minimal_state_machine.pwm_enable_request = 0U;
  g_minimal_state_machine.q1_handover_ready = 0U;
  g_minimal_state_machine.q2_handover_ready = 0U;
  g_minimal_state_machine.ta1_output_enabled = 0U;
  g_minimal_state_machine.ta2_output_enabled = 0U;
  g_minimal_state_machine.loop_control_enable = 0U;
  g_minimal_state_machine.q1_closed_loop_enable = 0U;
  g_minimal_state_machine.selected_mode = POWER_MODE_BUCK;
  g_minimal_state_machine.mode_confirmed = 0U;
  PowerMode_Reset();
  MinimalStateMachine_UpdateIndicators();
}

void MinimalStateMachine_Tick5ms(void)
{
  switch (g_minimal_state_machine.state)
  {
  case STATE_INIT:
    g_minimal_state_machine.vref_snapshot = 0;
    g_minimal_state_machine.iref_snapshot = 0;

    if (g_sample_processed.offset_ready != 0U)
    {
      g_minimal_state_machine.offset_capture_complete = 1U;
      g_minimal_state_machine.wait_counter = 0U;
      g_minimal_state_machine.state = STATE_WAIT;
    }
    break;

  case STATE_WAIT:
    if (g_minimal_state_machine.wait_counter < 100U)
    {
      g_minimal_state_machine.wait_counter++;
    }

    g_minimal_state_machine.vref_snapshot = g_target_calibration.vref_target;
    g_minimal_state_machine.iref_snapshot = g_target_calibration.iref_target;

    if (g_minimal_state_machine.wait_counter >= 100U)
    {
      if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
      {
        PowerMode_Request(POWER_MODE_BUCK);
        g_minimal_state_machine.selected_mode = POWER_MODE_BUCK;
        g_minimal_state_machine.mode_confirmed = 1U;
        g_minimal_state_machine.output_enabled_request = 1U;
      }
      else if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
      {
        PowerMode_Request(POWER_MODE_BOOST);
        g_minimal_state_machine.selected_mode = POWER_MODE_BOOST;
        g_minimal_state_machine.mode_confirmed = 1U;
        g_minimal_state_machine.output_enabled_request = 1U;
      }
    }

    if ((g_minimal_state_machine.offset_capture_complete != 0U) &&
        (g_minimal_state_machine.output_enabled_request != 0U) &&
        (g_minimal_state_machine.mode_confirmed != 0U))
    {
      PowerMode_Lock();
      g_minimal_state_machine.rise_stage = 0U;
      g_minimal_state_machine.state = STATE_RISE;
    }
    break;

  case STATE_RISE:
    if (g_minimal_state_machine.rise_stage == 0U)
    {
      g_minimal_state_machine.rise_vref_start = 0;
      g_minimal_state_machine.rise_iref_start = 0;
      g_minimal_state_machine.rise_q1_max_duty = RISE_MIN_DUTY;
      g_minimal_state_machine.rise_q2_max_duty = RISE_MIN_DUTY;
      g_minimal_state_machine.rise_q1_duty = RISE_MIN_DUTY;
      g_minimal_state_machine.rise_q1_applied_duty = RISE_MIN_DUTY;
      g_minimal_state_machine.rise_q2_duty = RISE_MIN_DUTY;
      g_minimal_state_machine.rise_q2_applied_duty = RISE_MIN_DUTY;
      g_minimal_state_machine.rise_q1_counter = 0U;
      g_minimal_state_machine.rise_q2_counter = 0U;
      g_minimal_state_machine.pwm_enable_request = 0U;
      g_minimal_state_machine.q1_handover_ready = 0U;
      g_minimal_state_machine.q2_handover_ready = 0U;
      g_minimal_state_machine.ta1_output_enabled = 0U;
      g_minimal_state_machine.ta2_output_enabled = 0U;
      g_minimal_state_machine.loop_control_enable = 0U;
      g_minimal_state_machine.q1_closed_loop_enable = 0U;
      LowRateLoopCalc_Reset();
      MinimalStateMachine_ApplyTa1Duty();
      MinimalStateMachine_ApplyTa2Window();
      g_minimal_state_machine.rise_stage = 1U;
    }
    else if (g_minimal_state_machine.rise_stage == 1U)
    {
      if (PowerMode_GetActive() == POWER_MODE_BOOST)
      {
        if (g_minimal_state_machine.pwm_enable_request == 0U)
        {
          g_minimal_state_machine.pwm_enable_request = 1U;
          g_minimal_state_machine.q2_handover_ready = 1U;
        }

        if ((g_minimal_state_machine.q2_handover_ready != 0U) &&
            (g_minimal_state_machine.ta2_output_enabled == 0U))
        {
          if (HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA2) == HAL_OK)
          {
            g_minimal_state_machine.ta2_output_enabled = 1U;
          }
        }

        if (g_minimal_state_machine.rise_q2_max_duty < RISE_MAX_DUTY)
        {
          g_minimal_state_machine.rise_q2_counter++;
          g_minimal_state_machine.rise_q2_max_duty =
              (int32_t)g_minimal_state_machine.rise_q2_counter * RISE_DUTY_STEP;

          if (g_minimal_state_machine.rise_q2_max_duty > RISE_MAX_DUTY)
          {
            g_minimal_state_machine.rise_q2_max_duty = RISE_MAX_DUTY;
            g_minimal_state_machine.q1_handover_ready = 1U;
          }
        }
        else if ((g_minimal_state_machine.q1_handover_ready != 0U) &&
                 (g_minimal_state_machine.ta1_output_enabled == 0U))
        {
          if (HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1) == HAL_OK)
          {
            g_minimal_state_machine.ta1_output_enabled = 1U;
          }
        }
        else if (g_minimal_state_machine.ta1_output_enabled != 0U)
        {
          g_minimal_state_machine.rise_q1_duty = 32767 - g_minimal_state_machine.rise_q2_applied_duty;
          if (g_minimal_state_machine.rise_q1_duty > g_minimal_state_machine.rise_q1_max_duty)
          {
            g_minimal_state_machine.rise_q1_duty = g_minimal_state_machine.rise_q1_max_duty;
          }

          MinimalStateMachine_ApplyTa2Window();
          MinimalStateMachine_ApplyTa1Duty();

          if (g_minimal_state_machine.rise_q1_max_duty < RISE_MAX_DUTY)
          {
            g_minimal_state_machine.rise_q1_counter++;
            g_minimal_state_machine.rise_q1_max_duty =
                (int32_t)g_minimal_state_machine.rise_q1_counter * RISE_DUTY_STEP;

            if (g_minimal_state_machine.rise_q1_max_duty > RISE_MAX_DUTY)
            {
              g_minimal_state_machine.rise_q1_max_duty = RISE_MAX_DUTY;
            }
          }
          else
          {
            g_minimal_state_machine.rise_stage = 2U;
            LowRateLoopCalc_SeedFromDuty(g_minimal_state_machine.rise_q2_applied_duty);
            g_minimal_state_machine.loop_control_enable = 1U;
            g_minimal_state_machine.q1_closed_loop_enable = 1U;
            g_minimal_state_machine.state = STATE_RUN;
          }
        }
      }
      else
      {
        if (g_minimal_state_machine.pwm_enable_request == 0U)
        {
          g_minimal_state_machine.pwm_enable_request = 1U;
          g_minimal_state_machine.q1_handover_ready = 1U;
        }

        if ((g_minimal_state_machine.q1_handover_ready != 0U) &&
            (g_minimal_state_machine.ta1_output_enabled == 0U))
        {
          if (HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1) == HAL_OK)
          {
            g_minimal_state_machine.ta1_output_enabled = 1U;
          }
        }

        if (g_minimal_state_machine.rise_q1_max_duty < RISE_MAX_DUTY)
        {
          g_minimal_state_machine.rise_q1_counter++;
          g_minimal_state_machine.rise_q1_max_duty =
              (int32_t)g_minimal_state_machine.rise_q1_counter * RISE_DUTY_STEP;

          if (g_minimal_state_machine.rise_q1_max_duty > RISE_MAX_DUTY)
          {
            g_minimal_state_machine.rise_q1_max_duty = RISE_MAX_DUTY;
            g_minimal_state_machine.q2_handover_ready = 1U;
          }

          g_minimal_state_machine.rise_q1_duty = g_minimal_state_machine.rise_q1_max_duty;
          MinimalStateMachine_ApplyTa1Duty();
        }
        else if ((g_minimal_state_machine.q2_handover_ready != 0U) &&
                 (g_minimal_state_machine.ta2_output_enabled == 0U))
        {
          if (HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA2) == HAL_OK)
          {
            g_minimal_state_machine.ta2_output_enabled = 1U;
          }
        }
        else if (g_minimal_state_machine.ta2_output_enabled != 0U)
        {
          g_minimal_state_machine.rise_q2_duty = 32767 - g_minimal_state_machine.rise_q1_applied_duty;
          if (g_minimal_state_machine.rise_q2_duty > g_minimal_state_machine.rise_q2_max_duty)
          {
            g_minimal_state_machine.rise_q2_duty = g_minimal_state_machine.rise_q2_max_duty;
          }

          MinimalStateMachine_ApplyTa2Window();

          if (g_minimal_state_machine.rise_q2_max_duty < RISE_MAX_DUTY)
          {
            g_minimal_state_machine.rise_q2_counter++;
            g_minimal_state_machine.rise_q2_max_duty =
                (int32_t)g_minimal_state_machine.rise_q2_counter * RISE_DUTY_STEP;

            if (g_minimal_state_machine.rise_q2_max_duty > RISE_MAX_DUTY)
            {
              g_minimal_state_machine.rise_q2_max_duty = RISE_MAX_DUTY;
            }
          }
          else
          {
            g_minimal_state_machine.rise_stage = 2U;
            LowRateLoopCalc_SeedFromDuty(g_minimal_state_machine.rise_q1_applied_duty);
            g_minimal_state_machine.loop_control_enable = 1U;
            g_minimal_state_machine.q1_closed_loop_enable = 1U;
            g_minimal_state_machine.state = STATE_RUN;
          }
        }
      }
    }
    break;

  case STATE_RUN:
  case STATE_ERR:
  default:
    break;
  }

  MinimalStateMachine_UpdateIndicators();
}

void MinimalStateMachine_RunControlTick(void)
{
  int32_t q1_clamped;
  int32_t q2_clamped;

  if ((g_minimal_state_machine.state != STATE_RUN) ||
      (g_minimal_state_machine.loop_control_enable == 0U) ||
      (g_minimal_state_machine.q1_closed_loop_enable == 0U))
  {
    return;
  }

  if (PowerMode_GetActive() == POWER_MODE_BOOST)
  {
    q2_clamped = g_lowrate_loop_calc.q2_duty_cmd;
    if (q2_clamped < RISE_MIN_DUTY)
    {
      q2_clamped = RISE_MIN_DUTY;
    }
    if (q2_clamped > g_minimal_state_machine.rise_q2_max_duty)
    {
      q2_clamped = g_minimal_state_machine.rise_q2_max_duty;
    }
    if (q2_clamped > RISE_MAX_DUTY)
    {
      q2_clamped = RISE_MAX_DUTY;
    }

    g_minimal_state_machine.rise_q2_duty = q2_clamped;
    MinimalStateMachine_ApplyTa2Window();

    q1_clamped = 32767 - g_minimal_state_machine.rise_q2_applied_duty;
    if (q1_clamped < RISE_MIN_DUTY)
    {
      q1_clamped = RISE_MIN_DUTY;
    }
    if (q1_clamped > g_minimal_state_machine.rise_q1_max_duty)
    {
      q1_clamped = g_minimal_state_machine.rise_q1_max_duty;
    }
    if (q1_clamped > TA1_SAFE_MAX_DUTY_Q15)
    {
      q1_clamped = TA1_SAFE_MAX_DUTY_Q15;
    }

    g_minimal_state_machine.rise_q1_duty = q1_clamped;
    MinimalStateMachine_ApplyTa1Duty();
    return;
  }

  q1_clamped = g_lowrate_loop_calc.q1_duty_cmd;
  if (q1_clamped < RISE_MIN_DUTY)
  {
    q1_clamped = RISE_MIN_DUTY;
  }
  if (q1_clamped > TA1_SAFE_MAX_DUTY_Q15)
  {
    q1_clamped = TA1_SAFE_MAX_DUTY_Q15;
  }

  g_minimal_state_machine.rise_q1_duty = q1_clamped;
  MinimalStateMachine_ApplyTa1Duty();

  q2_clamped = 32767 - g_minimal_state_machine.rise_q1_applied_duty;
  if (q2_clamped < RISE_MIN_DUTY)
  {
    q2_clamped = RISE_MIN_DUTY;
  }
  if (q2_clamped > g_minimal_state_machine.rise_q2_max_duty)
  {
    q2_clamped = g_minimal_state_machine.rise_q2_max_duty;
  }

  g_minimal_state_machine.rise_q2_duty = q2_clamped;
  MinimalStateMachine_ApplyTa2Window();
}

const char *MinimalStateMachine_StateName(MinimalState_t state)
{
  switch (state)
  {
  case STATE_INIT:
    return "Init";
  case STATE_WAIT:
    return "Wait";
  case STATE_RISE:
    return "Rise";
  case STATE_RUN:
    return "Run";
  case STATE_ERR:
    return "Err";
  default:
    return "Unknown";
  }
}
