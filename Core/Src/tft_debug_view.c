#include "tft_debug_view.h"

#include "LCDDriver.h"
#include "lowrate_loop_calc.h"
#include "minimal_state_machine.h"
#include "sample_processing.h"
#include "target_calibration.h"

#define TFT_DEBUG_UPDATE_INTERVAL_MS 200U
#define TFT_DEBUG_BOOT_FLASH_MS      180U
#define TFT_CHAR_WIDTH               16U
#define TFT_LINE_HEIGHT              36U
#define TFT_VALUE_X_SMALL            64U
#define TFT_VALUE_X_LARGE            96U

typedef struct
{
  uint16_t foreground;
  uint16_t background;
  const char *top_text;
  const char *bottom_text;
} TftBootPattern_t;

static const TftBootPattern_t s_boot_patterns[] = {
    {WHITE, RED, "RED", "TFT"},
    {BLACK, GREEN, "GREEN", "TFT"},
    {WHITE, BLUE, "BLUE", "TFT"},
    {BLACK, WHITE, "WHITE", "TFT"}};

static uint32_t s_last_update_tick = 0U;

static uint16_t TftDebugView_ColorForState(MinimalState_t state)
{
  switch (state)
  {
  case STATE_INIT:
    return YELLOW;
  case STATE_WAIT:
    return GREEN;
  case STATE_RISE:
    return BLUE;
  case STATE_RUN:
    return GREEN;
  case STATE_ERR:
    return RED;
  default:
    return WHITE;
  }
}

static void TftDebugView_DrawGlyph(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t background)
{
  if ((ch >= 'A') && (ch <= 'Z'))
  {
    LCDshowChar(x, y, color, background, (uint16_t)(ch - 'A'));
  }
  else if ((ch >= '0') && (ch <= '9'))
  {
    LCDshowDate(x, y, color, background, (uint16_t)(ch - '0'));
  }
  else if (ch == ':')
  {
    LCDshowDot(x, y, color, background, 1U);
  }
  else if (ch == '.')
  {
    LCDshowDot(x, y, color, background, 2U);
  }
  else
  {
    LCDshowDot(x, y, background, background, 0U);
  }
}

static void TftDebugView_DrawText(uint16_t x, uint16_t y, const char *text, uint16_t color, uint16_t background)
{
  uint16_t cursor_x = x;

  while (*text != '\0')
  {
    TftDebugView_DrawGlyph(cursor_x, y, *text, color, background);
    cursor_x = (uint16_t)(cursor_x + TFT_CHAR_WIDTH);
    ++text;
  }
}

static void TftDebugView_DrawUnsignedRaw(uint16_t x, uint16_t y, uint32_t value, uint8_t digits, uint16_t color, uint16_t background)
{
  uint8_t index;
  uint32_t divisor = 1U;

  for (index = 1U; index < digits; ++index)
  {
    divisor *= 10U;
  }

  for (index = 0U; index < digits; ++index)
  {
    LCDShowNum(x, y, color, background, (uint16_t)((value / divisor) % 10U));
    x = (uint16_t)(x + TFT_CHAR_WIDTH);
    divisor /= 10U;
  }
}

static void TftDebugView_DrawSignedRaw(uint16_t x, uint16_t y, int32_t value, uint8_t digits, uint16_t color, uint16_t background)
{
  uint32_t magnitude;
  uint32_t max_value = 1U;
  uint8_t index;

  for (index = 0U; index < digits; ++index)
  {
    max_value *= 10U;
  }
  max_value -= 1U;

  if (value < 0)
  {
    TftDebugView_DrawText(x, y, "N", color, background);
    magnitude = (uint32_t)(-value);
  }
  else
  {
    TftDebugView_DrawText(x, y, "P", color, background);
    magnitude = (uint32_t)value;
  }

  if (magnitude > max_value)
  {
    magnitude = max_value;
  }

  TftDebugView_DrawUnsignedRaw((uint16_t)(x + TFT_CHAR_WIDTH), y, magnitude, digits, color, background);
}

static void TftDebugView_DrawStateValue(uint16_t x, uint16_t y)
{
  const char *state_name = MinimalStateMachine_StateName(g_minimal_state_machine.state);
  uint16_t state_color = TftDebugView_ColorForState(g_minimal_state_machine.state);

  if (state_name[0] == 'I')
  {
    TftDebugView_DrawText(x, y, "INIT ", state_color, BLACK);
  }
  else if (state_name[0] == 'W')
  {
    TftDebugView_DrawText(x, y, "WAIT ", state_color, BLACK);
  }
  else if (state_name[0] == 'R')
  {
    if (state_name[1] == 'i')
    {
      TftDebugView_DrawText(x, y, "RISE ", state_color, BLACK);
    }
    else
    {
      TftDebugView_DrawText(x, y, "RUN  ", state_color, BLACK);
    }
  }
  else if (state_name[0] == 'E')
  {
    TftDebugView_DrawText(x, y, "ERR  ", state_color, BLACK);
  }
  else
  {
    TftDebugView_DrawText(x, y, "UNKN ", WHITE, BLACK);
  }
}

static void TftDebugView_ShowBootPattern(uint32_t boot_tick)
{
  uint32_t pattern_index = (boot_tick / TFT_DEBUG_BOOT_FLASH_MS) % (sizeof(s_boot_patterns) / sizeof(s_boot_patterns[0]));
  const TftBootPattern_t *pattern = &s_boot_patterns[pattern_index];

  Lcd_Clear(pattern->background);
  TftDebugView_DrawText(16U, 56U, pattern->top_text, pattern->foreground, pattern->background);
  TftDebugView_DrawText(64U, 120U, pattern->bottom_text, pattern->foreground, pattern->background);
}

static void TftDebugView_DrawMainLabels(void)
{
  TftDebugView_DrawText(0U, 0U, "MODE:", WHITE, BLACK);
  TftDebugView_DrawText(0U, TFT_LINE_HEIGHT, "STAT:", WHITE, BLACK);
  TftDebugView_DrawText(0U, 2U * TFT_LINE_HEIGHT, "VIN:", WHITE, BLACK);
  TftDebugView_DrawText(0U, 3U * TFT_LINE_HEIGHT, "VOUT:", WHITE, BLACK);
  TftDebugView_DrawText(0U, 4U * TFT_LINE_HEIGHT, "VPOT:", WHITE, BLACK);
  TftDebugView_DrawText(0U, 5U * TFT_LINE_HEIGHT, "RISE:", WHITE, BLACK);
}

static void TftDebugView_DrawModeValue(uint16_t x, uint16_t y)
{
  TftDebugView_DrawText(x, y, "BUCK ", GREEN, BLACK);
}

static void TftDebugView_DrawStateCompact(uint16_t x, uint16_t y)
{
  const char *state_name = MinimalStateMachine_StateName(g_minimal_state_machine.state);
  uint16_t state_color = TftDebugView_ColorForState(g_minimal_state_machine.state);

  if (state_name[0] == 'I')
  {
    TftDebugView_DrawText(x, y, "INIT ", state_color, BLACK);
  }
  else if (state_name[0] == 'W')
  {
    TftDebugView_DrawText(x, y, "WAIT ", state_color, BLACK);
  }
  else if (state_name[0] == 'R')
  {
    if (state_name[1] == 'i')
    {
      TftDebugView_DrawText(x, y, "RISE ", state_color, BLACK);
    }
    else
    {
      TftDebugView_DrawText(x, y, "RUN  ", state_color, BLACK);
    }
  }
  else if (state_name[0] == 'E')
  {
    TftDebugView_DrawText(x, y, "ERR  ", state_color, BLACK);
  }
  else
  {
    TftDebugView_DrawText(x, y, "UNKN ", WHITE, BLACK);
  }
}

static void TftDebugView_DrawMainValues(void)
{
  uint16_t vin_display = (uint16_t)((((int32_t)g_sample_processed.vx_avg) * 6800) >> 12);
  uint16_t vout_display = (uint16_t)((((int32_t)g_sample_processed.vy_avg) * 6800) >> 12);
  uint16_t vpot_display = (uint16_t)((g_target_calibration.vref_target * 6800) >> 12);

  TftDebugView_DrawModeValue(TFT_VALUE_X_SMALL, 0U);
  TftDebugView_DrawStateCompact(TFT_VALUE_X_SMALL, TFT_LINE_HEIGHT);
  LCDShowFnum(TFT_VALUE_X_SMALL, 2U * TFT_LINE_HEIGHT, WHITE, BLACK, vin_display);
  LCDShowFnum(TFT_VALUE_X_SMALL, 3U * TFT_LINE_HEIGHT, WHITE, BLACK, vout_display);
  LCDShowFnum(TFT_VALUE_X_SMALL, 4U * TFT_LINE_HEIGHT, WHITE, BLACK, vpot_display);
  TftDebugView_DrawUnsignedRaw(TFT_VALUE_X_SMALL, 5U * TFT_LINE_HEIGHT, (uint32_t)g_minimal_state_machine.rise_stage, 4U, WHITE, BLACK);
}

void TftDebugView_Init(void)
{
  uint32_t boot_start_tick;

  Lcd_Init();
  s_last_update_tick = 0U;

  boot_start_tick = HAL_GetTick();
  while ((HAL_GetTick() - boot_start_tick) < 1200U)
  {
    TftDebugView_ShowBootPattern(HAL_GetTick() - boot_start_tick);
  }

  Lcd_Clear(BLACK);
  TftDebugView_DrawMainLabels();
}

void TftDebugView_Update(void)
{
  uint32_t current_tick = HAL_GetTick();

  if ((current_tick - s_last_update_tick) < TFT_DEBUG_UPDATE_INTERVAL_MS)
  {
    return;
  }

  s_last_update_tick = current_tick;
  TftDebugView_DrawMainValues();
}
