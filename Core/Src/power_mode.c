#include "power_mode.h"

volatile PowerModeContext_t g_power_mode = {
    POWER_MODE_BUCK,
    POWER_MODE_BUCK,
    0U};

void PowerMode_Reset(void)
{
  g_power_mode.requested_mode = POWER_MODE_BUCK;
  g_power_mode.active_mode = POWER_MODE_BUCK;
  g_power_mode.selection_locked = 0U;
}

void PowerMode_Request(PowerMode_t mode)
{
  if ((mode != POWER_MODE_BUCK) && (mode != POWER_MODE_BOOST))
  {
    return;
  }

  if (g_power_mode.selection_locked != 0U)
  {
    return;
  }

  g_power_mode.requested_mode = mode;
  g_power_mode.active_mode = mode;
}

void PowerMode_Lock(void)
{
  g_power_mode.selection_locked = 1U;
}

PowerMode_t PowerMode_GetActive(void)
{
  return g_power_mode.active_mode;
}

uint8_t PowerMode_IsLocked(void)
{
  return g_power_mode.selection_locked;
}
