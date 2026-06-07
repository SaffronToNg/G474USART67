#include "LCDDriver.h"
#include "LCDdata.h"

static void SPI_WriteData(uint8_t data)
{
  uint8_t bit_index;

  for (bit_index = 0U; bit_index < 8U; ++bit_index)
  {
    if ((data & 0x80U) != 0U)
    {
      GPIOB->BSRR = GPIO_PIN_5;
    }
    else
    {
      GPIOB->BRR = GPIO_PIN_5;
    }

    GPIOB->BRR = GPIO_PIN_3;
    __NOP();
    GPIOB->BSRR = GPIO_PIN_3;
    __NOP();
    data <<= 1U;
  }
}

static void Lcd_WriteIndex(uint8_t index)
{
  LCD_RS_CLR;
  SPI_WriteData(index);
}

static void Lcd_WriteData(uint8_t data)
{
  LCD_RS_SET;
  SPI_WriteData(data);
}

static void LCD_WriteData_16Bit(uint16_t data)
{
  LCD_RS_SET;
  SPI_WriteData((uint8_t)(data >> 8));
  SPI_WriteData((uint8_t)data);
}

static void Lcd_SetRegion(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
  Lcd_WriteIndex(0x2AU);
  LCD_WriteData_16Bit(x_start);
  LCD_WriteData_16Bit(x_end);
  Lcd_WriteIndex(0x2BU);
  LCD_WriteData_16Bit(y_start);
  LCD_WriteData_16Bit(y_end);
  Lcd_WriteIndex(0x2CU);
}

static void Lcd_SetXY(uint16_t x, uint16_t y)
{
  Lcd_WriteIndex(0x2AU);
  LCD_WriteData_16Bit(x);
  Lcd_WriteIndex(0x2BU);
  LCD_WriteData_16Bit(y);
  Lcd_WriteIndex(0x2CU);
}

void Lcd_Init(void)
{
  LCD_SCL_SET;
  LCD_RST_CLR;
  HAL_Delay(100U);
  LCD_RST_SET;
  HAL_Delay(100U);

  Lcd_WriteIndex(0x11U);
  HAL_Delay(120U);

  Lcd_WriteIndex(0x3AU);
  Lcd_WriteData(0x05U);
  Lcd_WriteIndex(0xC5U);
  Lcd_WriteData(0x1AU);
  Lcd_WriteIndex(0x36U);
  Lcd_WriteData(0x00U);

  Lcd_WriteIndex(0xB2U);
  Lcd_WriteData(0x05U);
  Lcd_WriteData(0x05U);
  Lcd_WriteData(0x00U);
  Lcd_WriteData(0x33U);
  Lcd_WriteData(0x33U);

  Lcd_WriteIndex(0xB7U);
  Lcd_WriteData(0x05U);

  Lcd_WriteIndex(0xBBU);
  Lcd_WriteData(0x3FU);

  Lcd_WriteIndex(0xC0U);
  Lcd_WriteData(0x2CU);

  Lcd_WriteIndex(0xC2U);
  Lcd_WriteData(0x01U);

  Lcd_WriteIndex(0xC3U);
  Lcd_WriteData(0x0FU);

  Lcd_WriteIndex(0xC4U);
  Lcd_WriteData(0x20U);

  Lcd_WriteIndex(0xC6U);
  Lcd_WriteData(0x01U);

  Lcd_WriteIndex(0xD0U);
  Lcd_WriteData(0xA4U);
  Lcd_WriteData(0xA1U);

  Lcd_WriteIndex(0xE8U);
  Lcd_WriteData(0x03U);

  Lcd_WriteIndex(0xE9U);
  Lcd_WriteData(0x09U);
  Lcd_WriteData(0x09U);
  Lcd_WriteData(0x08U);

  Lcd_WriteIndex(0xE0U);
  Lcd_WriteData(0xD0U);
  Lcd_WriteData(0x05U);
  Lcd_WriteData(0x09U);
  Lcd_WriteData(0x09U);
  Lcd_WriteData(0x08U);
  Lcd_WriteData(0x14U);
  Lcd_WriteData(0x28U);
  Lcd_WriteData(0x33U);
  Lcd_WriteData(0x3FU);
  Lcd_WriteData(0x07U);
  Lcd_WriteData(0x13U);
  Lcd_WriteData(0x14U);
  Lcd_WriteData(0x28U);
  Lcd_WriteData(0x30U);

  Lcd_WriteIndex(0xE1U);
  Lcd_WriteData(0xD0U);
  Lcd_WriteData(0x05U);
  Lcd_WriteData(0x09U);
  Lcd_WriteData(0x09U);
  Lcd_WriteData(0x08U);
  Lcd_WriteData(0x03U);
  Lcd_WriteData(0x24U);
  Lcd_WriteData(0x32U);
  Lcd_WriteData(0x32U);
  Lcd_WriteData(0x3BU);
  Lcd_WriteData(0x14U);
  Lcd_WriteData(0x13U);
  Lcd_WriteData(0x28U);
  Lcd_WriteData(0x2FU);

  Lcd_WriteIndex(0x21U);
  Lcd_WriteIndex(0x29U);
}

void Lcd_FillRect(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
  uint32_t pixel_index;
  uint32_t pixel_count;

  if ((x_start >= X_MAX_PIXEL) || (y_start >= Y_MAX_PIXEL))
  {
    return;
  }

  if (x_end >= X_MAX_PIXEL)
  {
    x_end = X_MAX_PIXEL - 1U;
  }

  if (y_end >= Y_MAX_PIXEL)
  {
    y_end = Y_MAX_PIXEL - 1U;
  }

  if ((x_end < x_start) || (y_end < y_start))
  {
    return;
  }

  pixel_count = (uint32_t)(x_end - x_start + 1U) * (uint32_t)(y_end - y_start + 1U);
  Lcd_SetRegion(x_start, y_start, x_end, y_end);
  LCD_RS_SET;
  for (pixel_index = 0U; pixel_index < pixel_count; ++pixel_index)
  {
    SPI_WriteData((uint8_t)(color >> 8));
    SPI_WriteData((uint8_t)color);
  }
}

void Lcd_Clear(uint16_t color)
{
  Lcd_FillRect(0U, 0U, X_MAX_PIXEL - 1U, Y_MAX_PIXEL - 1U, color);
}

void LCDshowDate(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t add)
{
  uint16_t column;
  uint8_t bit_index = 0U;
  uint8_t temp = 0U;
  uint8_t data_index = 0U;

  Lcd_SetRegion(x, y, x + 15U, y + 31U);
  for (column = 0U; column < 64U; ++column)
  {
    temp = LCDdata[add][data_index];
    for (bit_index = 0U; bit_index < 8U; ++bit_index)
    {
      if ((temp & 0x01U) != 0U)
      {
        Lcd_WriteData((uint8_t)(fc >> 8));
        Lcd_WriteData((uint8_t)fc);
      }
      else
      {
        Lcd_WriteData((uint8_t)(bc >> 8));
        Lcd_WriteData((uint8_t)bc);
      }
      temp >>= 1U;
    }
    ++data_index;
  }
}

void LCDshowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t add)
{
  uint16_t column;
  uint8_t bit_index = 0U;
  uint8_t temp = 0U;
  uint8_t data_index = 0U;

  Lcd_SetRegion(x, y, x + 15U, y + 31U);
  for (column = 0U; column < 64U; ++column)
  {
    temp = LCDchar[add][data_index];
    for (bit_index = 0U; bit_index < 8U; ++bit_index)
    {
      if ((temp & 0x01U) != 0U)
      {
        Lcd_WriteData((uint8_t)(fc >> 8));
        Lcd_WriteData((uint8_t)fc);
      }
      else
      {
        Lcd_WriteData((uint8_t)(bc >> 8));
        Lcd_WriteData((uint8_t)bc);
      }
      temp >>= 1U;
    }
    ++data_index;
  }
}

void LCDshowDot(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t add)
{
  uint16_t column;
  uint8_t bit_index = 0U;
  uint8_t temp = 0U;
  uint8_t data_index = 0U;

  Lcd_SetRegion(x, y, x + 15U, y + 31U);
  for (column = 0U; column < 64U; ++column)
  {
    temp = LCDdot[add][data_index];
    for (bit_index = 0U; bit_index < 8U; ++bit_index)
    {
      if ((temp & 0x01U) != 0U)
      {
        Lcd_WriteData((uint8_t)(fc >> 8));
        Lcd_WriteData((uint8_t)fc);
      }
      else
      {
        Lcd_WriteData((uint8_t)(bc >> 8));
        Lcd_WriteData((uint8_t)bc);
      }
      temp >>= 1U;
    }
    ++data_index;
  }
}

void LCDShowNum(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t num)
{
  if (num <= 9U)
  {
    LCDshowDate(x, y, fc, bc, num);
  }
}

void LCDShowFnum(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t data)
{
  uint8_t digits[4] = {0U, 0U, 0U, 0U};

  digits[0] = (uint8_t)(data / 1000U);
  digits[1] = (uint8_t)((data - ((uint16_t)digits[0] * 1000U)) / 100U);
  digits[2] = (uint8_t)((data - ((uint16_t)digits[0] * 1000U) - ((uint16_t)digits[1] * 100U)) / 10U);
  digits[3] = (uint8_t)(data - ((uint16_t)digits[0] * 1000U) - ((uint16_t)digits[1] * 100U) - ((uint16_t)digits[2] * 10U));

  LCDShowNum(x, y, fc, bc, digits[0]);
  LCDShowNum(x + 16U, y, fc, bc, digits[1]);
  LCDshowDot(x + 32U, y, fc, bc, 2U);
  LCDShowNum(x + 48U, y, fc, bc, digits[2]);
  LCDShowNum(x + 64U, y, fc, bc, digits[3]);
}
