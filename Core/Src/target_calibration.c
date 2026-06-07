#include "target_calibration.h"

volatile TargetCalibration_t g_target_calibration = {0};

static int32_t ClampValue(int32_t value, int32_t min_value, int32_t max_value)
{
  if (value > max_value)
  {
    return max_value;
  }

  if (value < min_value)
  {
    return min_value;
  }

  return value;
}

static int32_t StepToward(int32_t current_value, int32_t target_value, int32_t step)
{
  if (target_value > (current_value + step))
  {
    return current_value + step;
  }

  if (target_value < (current_value - step))
  {
    return current_value - step;
  }

  return target_value;
}

void TargetCalibration_UpdateInstant(void)
{
  int32_t vadj_target;
  int32_t iadj_target;

  g_target_calibration.vy_calibrated =
      (((int32_t)g_sample_processed.vy_raw * TARGET_CAL_VY_K) >> 12) + TARGET_CAL_VY_B;

  /*
   * 当前最小恢复仅保留“LCD/电位器输入模式”的目标值路径：
   * - VadjAvg -> 电压目标
   * - IadjAvg -> 电流目标
   *
   * 先不恢复：
   * - 串口下发目标值路径
   * - BUCK/BOOST 模式切换状态机
   * - 完整 DF/CtrValue 结构
   */
  vadj_target = ((g_sample_processed.vadj_avg * 6963) >> 12) - 200;
  if (vadj_target < 0)
  {
    vadj_target = 0;
  }

  iadj_target = ((g_sample_processed.iadj_avg * 3072) >> 12) - 200;
  if (iadj_target < 0)
  {
    iadj_target = 0;
  }

  g_target_calibration.vadj_target_instant = vadj_target;
  g_target_calibration.iadj_target_instant = iadj_target;
  g_target_calibration.current_limit_mode = 1U; /* 当前最小恢复默认按 BUCK 上限路径观察 */
}

void TargetCalibration_UpdateStepped(void)
{
  g_target_calibration.vref_target = StepToward(
      g_target_calibration.vref_target,
      g_target_calibration.vadj_target_instant,
      TARGET_VREF_STEP);
  g_target_calibration.vref_target = ClampValue(
      g_target_calibration.vref_target,
      TARGET_MIN_VREF,
      TARGET_MAX_VREF);

  g_target_calibration.iref_target = StepToward(
      g_target_calibration.iref_target,
      g_target_calibration.iadj_target_instant,
      TARGET_IREF_STEP);
  g_target_calibration.iref_target = ClampValue(
      g_target_calibration.iref_target,
      TARGET_MIN_IREF,
      TARGET_MAX_IREF_BUCK);
}
