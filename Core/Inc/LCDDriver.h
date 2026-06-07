#ifndef __LCD_DRIVER_H__
#define __LCD_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#ifndef CCMRAM
#define CCMRAM
#endif

#define RED     0xF800U
#define GREEN   0x07E0U
#define BLUE    0x001FU
#define WHITE   0xFFFFU
#define BLACK   0x0000U
#define YELLOW  0xFFE0U
#define GRAY0   0xEF7DU
#define GRAY1   0x8410U
#define GRAY2   0x4208U

#define LCD_CTRLB    GPIOB
#define LCD_SCL      GPIO_PIN_3
#define LCD_SDA      GPIO_PIN_5
#define LCD_RST      GPIO_PIN_15
#define LCD_RS       GPIO_PIN_12

#define LCD_SDA_SET  (LCD_CTRLB->BSRR = LCD_SDA)
#define LCD_SDA_CLR  (LCD_CTRLB->BRR = LCD_SDA)
#define LCD_SCL_SET  (LCD_CTRLB->BSRR = LCD_SCL)
#define LCD_SCL_CLR  (LCD_CTRLB->BRR = LCD_SCL)
#define LCD_RST_SET  (LCD_CTRLB->BSRR = LCD_RST)
#define LCD_RST_CLR  (LCD_CTRLB->BRR = LCD_RST)
#define LCD_RS_SET   (LCD_CTRLB->BSRR = LCD_RS)
#define LCD_RS_CLR   (LCD_CTRLB->BRR = LCD_RS)

#define X_MAX_PIXEL  240U
#define Y_MAX_PIXEL  240U

void Lcd_Init(void);
void Lcd_Clear(uint16_t color);
void Lcd_FillRect(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);
void LCDshowDate(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t add);
void LCDshowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t add);
void LCDshowDot(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t add);
void LCDShowNum(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t num);
void LCDShowFnum(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_DRIVER_H__ */
