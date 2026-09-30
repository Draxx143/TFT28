/**
 * lcd.h — ILI9325/ILI9328 driver for MKS TFT28 V4.0 (16-bit GPIO parallel bus)
 */
#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>

/* Landscape 320 x 240 */
#define LCD_W 320
#define LCD_H 240

/* RGB565 colours */
#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFD20

void lcd_init(void);
void lcd_backlight(int on);

void lcd_fill_screen(uint16_t color);
void lcd_fill_rect(int x, int y, int w, int h, uint16_t color);
void lcd_draw_pixel(int x, int y, uint16_t color);

void lcd_draw_circle(int cx, int cy, int r, uint16_t color);
void lcd_fill_circle(int cx, int cy, int r, uint16_t color);

void delay_ms(uint32_t ms);

#endif /* LCD_DRIVER_H */
