#include "loop_observer.h"

volatile LoopObserver_t g_loop_observer = {0, 0, 0, 0, 0, 0U, 0U, 64U};

void LoopObserver_Reset(void)
{
  g_loop_observer.feedback_vy = 0;
  g_loop_observer.loop_error = 0;
  g_loop_observer.loop_prop = 0;
  g_loop_observer.loop_inte = 0;
  g_loop_observer.loop_u0 = 0;
  g_loop_observer.loop_tick_count = 0U;
  g_loop_observer.rep_event_count = 0U;
  g_loop_observer.decimation_factor = 64U;
}

void LoopObserver_Tick(void)
{
  static int32_t inte = 0;
  int32_t err;
  int32_t prop;
  int32_t u0;
  int32_t maxinte;
  const int32_t kp = 100;
  const int32_t ki = 10;

  g_loop_observer.rep_event_count++;

  /*
   * 这是“最小控制环观测版”：
   * - 只恢复原工程 BUCK_Voltage_Position_PI 的核心计算链
   * - 只在 Run 状态下运行
   * - 先不接管真实 PWM compare
   * - 为减小高频中断负担，采用固定分频观测
   */
  if (g_minimal_state_machine.state != STATE_RUN)
  {
    return;
  }

  if ((g_loop_observer.rep_event_count % g_loop_observer.decimation_factor) != 0U)
  {
    return;
  }

  g_loop_observer.feedback_vy = g_target_calibration.vy_calibrated;

  err = g_target_calibration.vref_target - g_loop_observer.feedback_vy;
  prop = err * kp;
  inte = inte + err * ki;

  maxinte = g_minimal_state_machine.rise_q1_max_duty << 8;
  if (inte > maxinte)
  {
    inte = maxinte;
  }
  else if (inte < 0)
  {
    inte = 0;
  }

  u0 = (prop + inte) >> 8;

  g_loop_observer.loop_error = err;
  g_loop_observer.loop_prop = prop;
  g_loop_observer.loop_inte = inte;
  g_loop_observer.loop_u0 = u0;
  g_loop_observer.loop_tick_count++;
}
