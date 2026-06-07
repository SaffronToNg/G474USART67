#ifndef __MINIMAL_STATE_MACHINE_H__
#define __MINIMAL_STATE_MACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "power_mode.h"
#include "sample_processing.h"
#include "target_calibration.h"

typedef enum
{
  STATE_INIT = 0,
  STATE_WAIT = 1,
  STATE_RISE = 2,
  STATE_RUN = 3,
  STATE_ERR = 4
} MinimalState_t;

typedef struct
{
  MinimalState_t state;
  uint16_t wait_counter;
  uint8_t offset_capture_complete;
  uint8_t output_enabled_request;
  int32_t vref_snapshot;
  int32_t iref_snapshot;
  uint8_t rise_stage;
  int32_t rise_vref_start;
  int32_t rise_iref_start;
  int32_t rise_q1_max_duty;
  int32_t rise_q2_max_duty;
  int32_t rise_q1_duty;
  int32_t rise_q1_applied_duty;
  int32_t rise_q2_duty;
  int32_t rise_q2_applied_duty;
  uint16_t rise_q1_counter;
  uint16_t rise_q2_counter;
  uint8_t pwm_enable_request;
  uint8_t q1_handover_ready;
  uint8_t q2_handover_ready;
  uint8_t ta1_output_enabled;
  uint8_t ta2_output_enabled;
  uint8_t loop_control_enable;
  uint8_t q1_closed_loop_enable;
  PowerMode_t selected_mode;
  uint8_t mode_confirmed;
} MinimalStateMachine_t;

extern volatile MinimalStateMachine_t g_minimal_state_machine;

void MinimalStateMachine_Init(void);
void MinimalStateMachine_Tick5ms(void);
void MinimalStateMachine_RunControlTick(void);
const char *MinimalStateMachine_StateName(MinimalState_t state);

#ifdef __cplusplus
}
#endif

#endif /* __MINIMAL_STATE_MACHINE_H__ */
