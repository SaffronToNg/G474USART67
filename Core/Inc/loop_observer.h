#ifndef __LOOP_OBSERVER_H__
#define __LOOP_OBSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "minimal_state_machine.h"
#include "sample_processing.h"
#include "target_calibration.h"

typedef struct
{
  int32_t feedback_vy;
  int32_t loop_error;
  int32_t loop_prop;
  int32_t loop_inte;
  int32_t loop_u0;
  uint32_t loop_tick_count;
  uint32_t rep_event_count;
  uint8_t decimation_factor;
} LoopObserver_t;

extern volatile LoopObserver_t g_loop_observer;

void LoopObserver_Reset(void);
void LoopObserver_Tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __LOOP_OBSERVER_H__ */
