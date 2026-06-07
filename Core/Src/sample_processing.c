#include "sample_processing.h"

volatile SampleProcessed_t g_sample_processed = {0};

void SampleProcessing_Update(void)
{
  static uint32_t vx_sum = 0U;
  static uint32_t ix_sum = 0U;
  static uint32_t vy_sum = 0U;
  static uint32_t iy_sum = 0U;
  static uint32_t vadj_sum = 0U;
  static uint32_t iadj_sum = 0U;
  static uint32_t temp_sum = 0U;

  static uint16_t offset_count = 0U;
  static uint32_t ix_offset_sum = 0U;
  static uint32_t iy_offset_sum = 0U;
  static uint32_t il_offset_sum = 0U;

  g_sample_processed.il_raw = (int32_t)g_sample_semantic_raw.il_raw;
  g_sample_processed.vy_raw = (int32_t)g_sample_semantic_raw.vy_raw;
  g_sample_processed.iy_raw = (int32_t)g_sample_semantic_raw.iy_raw;
  g_sample_processed.vx_raw = (int32_t)g_sample_semantic_raw.vx_raw;
  g_sample_processed.ix_raw = (int32_t)g_sample_semantic_raw.ix_raw;
  g_sample_processed.vadj_raw = (int32_t)g_sample_semantic_raw.vadj_raw;
  g_sample_processed.iadj_raw = (int32_t)g_sample_semantic_raw.iadj_raw;
  g_sample_processed.temp_raw = (int32_t)g_sample_semantic_raw.temp_raw;

  /*
   * 偏置初始化框架：参考旧工程 StateMInit() 的 256 次累积思路，
   * 这里只恢复“偏置累积 + 完成标志 + 去偏置结果观测”，
   * 不恢复状态机、不恢复 BUCK/BOOST 方向翻转。
   */
  if (offset_count < 256U)
  {
    offset_count++;
    ix_offset_sum += (uint32_t)g_sample_processed.ix_raw;
    iy_offset_sum += (uint32_t)g_sample_processed.iy_raw;
    il_offset_sum += (uint32_t)g_sample_processed.il_raw;

    if (offset_count == 256U)
    {
      g_sample_processed.ix_offset = (int32_t)(ix_offset_sum >> 8);
      g_sample_processed.iy_offset = (int32_t)(iy_offset_sum >> 8);
      g_sample_processed.il_offset = (int32_t)(il_offset_sum >> 8);
      g_sample_processed.offset_ready = 1U;
    }
  }

  g_sample_processed.offset_sample_count = offset_count;

  if (g_sample_processed.offset_ready != 0U)
  {
    g_sample_processed.ix_zero_based = g_sample_processed.ix_raw - g_sample_processed.ix_offset;
    g_sample_processed.iy_zero_based = g_sample_processed.iy_raw - g_sample_processed.iy_offset;
    g_sample_processed.il_zero_based = g_sample_processed.il_raw - g_sample_processed.il_offset;
  }
  else
  {
    g_sample_processed.ix_zero_based = 0;
    g_sample_processed.iy_zero_based = 0;
    g_sample_processed.il_zero_based = 0;
  }

  /*
   * 最小平均值层：只保留旧工程的轻量一阶平滑框架。
   * 当前阶段仍不引入：
   * - BUCK/BOOST 电流方向翻转
   * - 电压校准系数
   * - 温度摄氏度换算
   * - 控制目标处理
   */
  vx_sum = vx_sum + (uint32_t)g_sample_processed.vx_raw - (vx_sum >> 6);
  g_sample_processed.vx_avg = (int32_t)(vx_sum >> 6);

  ix_sum = ix_sum + (uint32_t)g_sample_processed.ix_raw - (ix_sum >> 6);
  g_sample_processed.ix_avg = (int32_t)(ix_sum >> 6);

  vy_sum = vy_sum + (uint32_t)g_sample_processed.vy_raw - (vy_sum >> 6);
  g_sample_processed.vy_avg = (int32_t)(vy_sum >> 6);

  iy_sum = iy_sum + (uint32_t)g_sample_processed.iy_raw - (iy_sum >> 6);
  g_sample_processed.iy_avg = (int32_t)(iy_sum >> 6);

  vadj_sum = vadj_sum + (uint32_t)g_sample_processed.vadj_raw - (vadj_sum >> 8);
  {
    extern uint8_t s_vadj_serial_override;
    static uint8_t s_vadj_serial_override_prev = 0U;
    static int32_t s_vadj_override_adc_anchor = 0;
    int32_t adc_new = (int32_t)(vadj_sum >> 8);

    if (s_vadj_serial_override != 0)
    {
      /*
       * 串口接管模式：记录接管瞬间的 ADC 平均值作为锚点。
       * 之后只要旋钮带动 ADC 平均值相对锚点变化 >= 10，
       * 就退出串口接管，恢复旋钮控制。
       */
      int32_t diff;

      if (s_vadj_serial_override_prev == 0U)
      {
        s_vadj_override_adc_anchor = adc_new;
      }

      diff = adc_new - s_vadj_override_adc_anchor;
      if (diff < 0) diff = -diff;
      if (diff >= 10)
      {
        s_vadj_serial_override = 0U;
        g_sample_processed.vadj_avg = adc_new;
      }
      /* 差值 < 10，不更新 vadj_avg，保持串口值 */
    }
    else
    {
      /* 正常模式：ADC 自由更新，无门槛 */
      g_sample_processed.vadj_avg = adc_new;
    }

    s_vadj_serial_override_prev = s_vadj_serial_override;
  }

  iadj_sum = iadj_sum + (uint32_t)g_sample_processed.iadj_raw - (iadj_sum >> 8);
  g_sample_processed.iadj_avg = (int32_t)(iadj_sum >> 8);

  temp_sum = temp_sum + (uint32_t)g_sample_processed.temp_raw - (temp_sum >> 8);
  g_sample_processed.temp_avg = (int32_t)(temp_sum >> 8);

  /*
   * ILAvg 当前仍保守处理：先保留 raw 直通，避免误认为已完成完整电流处理链。
   */
  g_sample_processed.il_avg = g_sample_processed.il_raw;
}
