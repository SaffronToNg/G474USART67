#ifndef __TARGET_CALIBRATION_H__
#define __TARGET_CALIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "sample_processing.h"

#define TARGET_MAX_VREF    3012
#define TARGET_MIN_VREF    0
#define TARGET_VREF_STEP   6
#define TARGET_MAX_IREF    621
#define TARGET_MAX_IREF_BUCK 1242
#define TARGET_MIN_IREF    0
#define TARGET_IREF_STEP   25

#define TARGET_CAL_VY_K    4096
#define TARGET_CAL_VY_B    0

typedef struct
{
  int32_t vy_calibrated;
  int32_t vadj_target_instant;
  int32_t iadj_target_instant;
  int32_t vref_target;
  int32_t iref_target;
  uint8_t current_limit_mode;
} TargetCalibration_t;

extern volatile TargetCalibration_t g_target_calibration;

void TargetCalibration_UpdateInstant(void);
void TargetCalibration_UpdateStepped(void);

#ifdef __cplusplus
}
#endif

#endif /* __TARGET_CALIBRATION_H__ */
