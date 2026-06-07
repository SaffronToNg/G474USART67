#ifndef __POWER_MODE_H__
#define __POWER_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum
{
  POWER_MODE_BUCK = 1,
  POWER_MODE_BOOST = 2
} PowerMode_t;

typedef struct
{
  PowerMode_t requested_mode;
  PowerMode_t active_mode;
  uint8_t selection_locked;
} PowerModeContext_t;

extern volatile PowerModeContext_t g_power_mode;

void PowerMode_Reset(void);
void PowerMode_Request(PowerMode_t mode);
void PowerMode_Lock(void);
PowerMode_t PowerMode_GetActive(void);
uint8_t PowerMode_IsLocked(void);

#ifdef __cplusplus
}
#endif

#endif /* __POWER_MODE_H__ */
