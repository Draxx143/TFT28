/**
 * main.c — MKS TFT28 V4.0 "hello circle"
 *
 * Draws a filled circle in the middle of the 320x240 screen.
 * Runs straight off the 8 MHz HSI (no PLL) — keeps the first bring-up simple.
 */

#include "lcd.h"

int main(void)
{
  lcd_init();

  /* Dark blue background so a black screen clearly means "nothing works" */
  lcd_fill_screen(RGB565(0, 0, 40));

  /* The circle */
  lcd_fill_circle(LCD_W / 2, LCD_H / 2, 70, ORANGE);
  lcd_draw_circle(LCD_W / 2, LCD_H / 2, 70, WHITE);
  lcd_draw_circle(LCD_W / 2, LCD_H / 2, 71, WHITE);

  /* Corner markers: confirm orientation and that the full area is addressable */
  lcd_fill_rect(0, 0, 10, 10, RED);            /* top-left     */
  lcd_fill_rect(LCD_W - 10, 0, 10, 10, GREEN); /* top-right    */
  lcd_fill_rect(0, LCD_H - 10, 10, 10, BLUE);  /* bottom-left  */
  lcd_fill_rect(LCD_W - 10, LCD_H - 10, 10, 10, YELLOW);

  /* Slow blink of the circle so you can see the CPU is alive */
  for (;;) {
    delay_ms(700);
    lcd_fill_circle(LCD_W / 2, LCD_H / 2, 69, CYAN);
    delay_ms(700);
    lcd_fill_circle(LCD_W / 2, LCD_H / 2, 69, ORANGE);
  }
}
