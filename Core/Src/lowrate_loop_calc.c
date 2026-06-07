#include "lowrate_loop_calc.h"

volatile LowRateLoopCalc_t g_lowrate_loop_calc = {0};

void LowRateLoopCalc_Reset(void)
{
  g_lowrate_loop_calc.feedback_vy = 0;
  g_lowrate_loop_calc.err = 0;
  g_lowrate_loop_calc.prop = 0;
  g_lowrate_loop_calc.inte = 0;
  g_lowrate_loop_calc.u0 = 0;
  g_lowrate_loop_calc.q1_duty_cmd = 0;
  g_lowrate_loop_calc.q2_duty_cmd = 0;
  g_lowrate_loop_calc.tick_count = 0U;
}

void LowRateLoopCalc_SeedFromDuty(int32_t q1_duty_seed)
{
  if (q1_duty_seed < 0)
  {
    q1_duty_seed = 0;
  }
  if (q1_duty_seed > 32767)
  {
    q1_duty_seed = 32767;
  }

  g_lowrate_loop_calc.feedback_vy = 0;
  g_lowrate_loop_calc.err = 0;
  g_lowrate_loop_calc.prop = 0;
  g_lowrate_loop_calc.inte = q1_duty_seed << 8;
  g_lowrate_loop_calc.u0 = q1_duty_seed;
  g_lowrate_loop_calc.q1_duty_cmd = q1_duty_seed;
  g_lowrate_loop_calc.q2_duty_cmd = 32767 - q1_duty_seed;
  g_lowrate_loop_calc.tick_count = 0U;
}

void LowRateLoopCalc_Tick(void)
{
  const int32_t kp = 100;
  const int32_t ki = 10;
  int32_t maxinte;

  if ((g_minimal_state_machine.state != STATE_RUN) ||
      (g_minimal_state_machine.loop_control_enable == 0U))
  {
    return;
  }

  g_lowrate_loop_calc.feedback_vy = g_target_calibration.vy_calibrated;
  g_lowrate_loop_calc.err =
      g_target_calibration.vref_target - g_lowrate_loop_calc.feedback_vy;
  g_lowrate_loop_calc.prop = g_lowrate_loop_calc.err * kp;
  g_lowrate_loop_calc.inte =
      g_lowrate_loop_calc.inte + g_lowrate_loop_calc.err * ki;

  maxinte = g_minimal_state_machine.rise_q1_max_duty << 8;
  if (g_lowrate_loop_calc.inte > maxinte)
  {
    g_lowrate_loop_calc.inte = maxinte;
  }
  else if (g_lowrate_loop_calc.inte < 0)
  {
    g_lowrate_loop_calc.inte = 0;
  }

  g_lowrate_loop_calc.u0 =
      (g_lowrate_loop_calc.prop + g_lowrate_loop_calc.inte) >> 8;
  g_lowrate_loop_calc.q1_duty_cmd = g_lowrate_loop_calc.u0;
  g_lowrate_loop_calc.q2_duty_cmd = 32767 - g_lowrate_loop_calc.q1_duty_cmd;
  g_lowrate_loop_calc.tick_count++;
}
