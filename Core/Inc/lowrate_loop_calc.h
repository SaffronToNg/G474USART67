#ifndef __LOWRATE_LOOP_CALC_H__
#define __LOWRATE_LOOP_CALC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "power_mode.h"
#include "minimal_state_machine.h"
#include "target_calibration.h"

typedef struct
{
  int32_t feedback_vy;
  int32_t err;
  int32_t prop;
  int32_t inte;
  int32_t u0;
  int32_t q1_duty_cmd;
  int32_t q2_duty_cmd;
  uint32_t tick_count;
} LowRateLoopCalc_t;

extern volatile LowRateLoopCalc_t g_lowrate_loop_calc;

void LowRateLoopCalc_Reset(void);
void LowRateLoopCalc_SeedFromDuty(int32_t q1_duty_seed);
void LowRateLoopCalc_Tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __LOWRATE_LOOP_CALC_H__ */
