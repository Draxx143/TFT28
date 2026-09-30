/**
 * lcd.c — ILI9325/ILI9328 driver, 16-bit parallel bus bit-banged on GPIOE.
 *
 * Board: MKS TFT28 V4.0 (STM32F107VCT6)
 *   DB0..DB15 = PE0..PE15
 *   CS = PC8   RS = PD13   WR = PB14   RD = PD15   BL = PD14
 *
 * The STM32F107 has NO FSMC, so every bus cycle is done in software.
 * Register/init sequence follows the known-good open-source MKS-TFT firmware.
 */

#include "lcd.h"
#include "board_pins_bare.h"
#include "stm32f107_min.h"

/* ------------------------------------------------------------------ *
 *  Low level bus
 * ------------------------------------------------------------------ */

/* One write strobe: WR low then high. The GPIO writes themselves are
 * slower than the panel's minimum pulse width, so no extra NOPs needed. */
static inline void wr_strobe(void)
{
  GPIOB->BSRR = (uint32_t)LCD_WR_PIN << 16;  /* WR = 0 */
  GPIOB->BSRR = (uint32_t)LCD_WR_PIN;        /* WR = 1 */
}

static inline void lcd_write_cmd(uint16_t cmd)
{
  GPIOD->BSRR = (uint32_t)LCD_RS_PIN << 16;  /* RS = 0 -> index/command */
  GPIOE->ODR  = cmd;
  wr_strobe();
}

static inline void lcd_write_data(uint16_t data)
{
  GPIOD->BSRR = (uint32_t)LCD_RS_PIN;        /* RS = 1 -> data */
  GPIOE->ODR  = data;
  wr_strobe();
}

static inline void lcd_write_reg(uint16_t reg, uint16_t val)
{
  lcd_write_cmd(reg);
  lcd_write_data(val);
}

/* Repeat the same pixel n times (fast fill) */
static void lcd_write_data_repeat(uint16_t data, uint32_t n)
{
  GPIOD->BSRR = (uint32_t)LCD_RS_PIN;        /* RS = 1 */
  GPIOE->ODR  = data;                        /* bus stays stable */
  while (n--) {
    GPIOB->BSRR = (uint32_t)LCD_WR_PIN << 16;
    GPIOB->BSRR = (uint32_t)LCD_WR_PIN;
  }
}

/* ------------------------------------------------------------------ *
 *  Crude blocking delay. Core runs on HSI = 8 MHz (no PLL configured),
 *  so ~8000 cycles per ms; the loop body is ~4 cycles.
 * ------------------------------------------------------------------ */
void delay_ms(uint32_t ms)
{
  volatile uint32_t n = ms * 2000U;
  while (n--) { __asm__ volatile(""); }
}

/* ------------------------------------------------------------------ *
 *  GPIO setup
 * ------------------------------------------------------------------ */
static void lcd_gpio_init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN |
                  RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;

  /* Whole port E = data bus, push-pull 50 MHz */
  GPIOE->CRL = 0x33333333U;
  GPIOE->CRH = 0x33333333U;
  GPIOE->ODR = 0x0000U;

  /* Control lines */
  gpio_config(GPIOC, 8,  GPIO_CFG_OUT_PP_50MHZ);  /* LCD CS   */
  gpio_config(GPIOD, 13, GPIO_CFG_OUT_PP_50MHZ);  /* LCD RS   */
  gpio_config(GPIOB, 14, GPIO_CFG_OUT_PP_50MHZ);  /* LCD WR   */
  gpio_config(GPIOD, 15, GPIO_CFG_OUT_PP_50MHZ);  /* LCD RD   */
  gpio_config(GPIOD, 14, GPIO_CFG_OUT_PP_50MHZ);  /* Backlight*/

  /* Park the other chip selects HIGH so nothing else drives the SPI1
   * bus / interferes: touch CS (PC9), SD CS (PD11), SPI-Flash CS (PB9). */
  gpio_config(GPIOC, 9,  GPIO_CFG_OUT_PP_50MHZ);
  gpio_config(GPIOD, 11, GPIO_CFG_OUT_PP_50MHZ);
  gpio_config(GPIOB, 9,  GPIO_CFG_OUT_PP_50MHZ);
  gpio_set(GPIOC, TOUCH_CS_PIN);
  gpio_set(GPIOD, SD_CS_PIN);
  gpio_set(GPIOB, FLASH_CS_PIN);

  /* Idle levels: CS high (inactive), WR/RD high, backlight off for now */
  gpio_set(GPIOC, LCD_CS_PIN);
  gpio_set(GPIOB, LCD_WR_PIN);
  gpio_set(GPIOD, LCD_RD_PIN);
  gpio_clear(GPIOD, LCD_BL_PIN);
}

void lcd_backlight(int on)
{
  if (on) gpio_set(GPIOD, LCD_BL_PIN);
  else    gpio_clear(GPIOD, LCD_BL_PIN);
}

/* ------------------------------------------------------------------ *
 *  Addressing
 *
 *  The panel is natively 240(x) x 320(y). We run it in landscape by
 *  setting AM=1 (address counter increments in the other direction) and
 *  swapping the coordinates here: screen X -> panel Y, screen Y -> panel X.
 * ------------------------------------------------------------------ */
static void lcd_set_window(int x1, int y1, int x2, int y2)
{
  /* landscape -> panel coordinate swap */
  uint16_t px1 = (uint16_t)y1, px2 = (uint16_t)y2;
  uint16_t py1 = (uint16_t)x1, py2 = (uint16_t)x2;

  lcd_write_reg(0x50, px1);   /* window horizontal start */
  lcd_write_reg(0x51, px2);   /* window horizontal end   */
  lcd_write_reg(0x52, py1);   /* window vertical   start */
  lcd_write_reg(0x53, py2);   /* window vertical   end   */

  lcd_write_reg(0x20, px1);   /* GRAM address X */
  lcd_write_reg(0x21, py1);   /* GRAM address Y */
  lcd_write_cmd(0x22);        /* write to GRAM  */
}

/* ------------------------------------------------------------------ *
 *  Init
 * ------------------------------------------------------------------ */
void lcd_init(void)
{
  lcd_gpio_init();
  delay_ms(50);

  /* No hardware reset pin on this board: the panel shares the board reset. */
  gpio_clear(GPIOC, LCD_CS_PIN);      /* CS active low, kept low the whole time */

  /* Landscape (SwapXY) orientation values */
  const uint16_t R01h = 0x0000;       /* SS=0, SM=0                          */
  const uint16_t R03h = 0x1038;       /* BGR=1, I/D[1:0]=11, AM=1            */
  const uint16_t R60h = 0xA700;       /* GS=1, NL[5:0]=0x27 (320 lines)      */

  lcd_write_reg(0x0001, R01h);
  lcd_write_reg(0x0002, 0x0700);      /* LCD driving waveform control        */
  lcd_write_reg(0x0003, R03h);        /* Entry mode                          */
  lcd_write_reg(0x0004, 0x0000);      /* Resizing control                    */
  lcd_write_reg(0x0008, 0x0202);      /* Front/back porch                    */
  lcd_write_reg(0x0009, 0x0000);
  lcd_write_reg(0x000A, 0x0000);

  /* ---- Power on sequence ---- */
  lcd_write_reg(0x0010, 0x0000);
  lcd_write_reg(0x0011, 0x0007);
  lcd_write_reg(0x0012, 0x0000);
  lcd_write_reg(0x0013, 0x0000);
  delay_ms(200);

  lcd_write_reg(0x0010, 0x1590);      /* SAP, BT[3:0], AP, DSTB, SLP, STB    */
  lcd_write_reg(0x0011, 0x0227);
  delay_ms(50);
  lcd_write_reg(0x0012, 0x009C);
  delay_ms(50);
  lcd_write_reg(0x0013, 0x1900);
  lcd_write_reg(0x0029, 0x0023);      /* VCOMH                               */
  lcd_write_reg(0x002B, 0x000E);      /* frame rate ~112 fps                 */
  delay_ms(50);

  /* ---- Gamma ---- */
  lcd_write_reg(0x0030, 0x0007);
  lcd_write_reg(0x0031, 0x0707);
  lcd_write_reg(0x0032, 0x0006);
  lcd_write_reg(0x0035, 0x0704);
  lcd_write_reg(0x0036, 0x1F04);
  lcd_write_reg(0x0037, 0x0004);
  lcd_write_reg(0x0038, 0x0000);
  lcd_write_reg(0x0039, 0x0706);
  lcd_write_reg(0x003C, 0x0701);
  lcd_write_reg(0x003D, 0x000F);
  delay_ms(50);

  /* ---- Window = full panel ---- */
  lcd_write_reg(0x0050, 0x0000);
  lcd_write_reg(0x0051, 239);
  lcd_write_reg(0x0052, 0x0000);
  lcd_write_reg(0x0053, 319);

  lcd_write_reg(0x0060, R60h);
  lcd_write_reg(0x0061, 0x0001);

  lcd_write_reg(0x0090, 0x0010);      /* Panel interface control 1           */
  lcd_write_reg(0x0092, 0x0000);
  lcd_write_reg(0x0095, 0x0110);
  lcd_write_reg(0x0097, 0x0000);

  lcd_write_reg(0x0007, 0x0133);      /* Display ON                          */
  delay_ms(50);

  lcd_fill_screen(BLACK);
  lcd_backlight(1);
}

/* ------------------------------------------------------------------ *
 *  Drawing primitives
 * ------------------------------------------------------------------ */
void lcd_fill_screen(uint16_t color)
{
  lcd_set_window(0, 0, LCD_W - 1, LCD_H - 1);
  lcd_write_data_repeat(color, (uint32_t)LCD_W * LCD_H);
}

void lcd_fill_rect(int x, int y, int w, int h, uint16_t color)
{
  if (w <= 0 || h <= 0) return;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > LCD_W) w = LCD_W - x;
  if (y + h > LCD_H) h = LCD_H - y;
  if (w <= 0 || h <= 0) return;

  lcd_set_window(x, y, x + w - 1, y + h - 1);
  lcd_write_data_repeat(color, (uint32_t)w * h);
}

void lcd_draw_pixel(int x, int y, uint16_t color)
{
  if (x < 0 || y < 0 || x >= LCD_W || y >= LCD_H) return;
  lcd_set_window(x, y, x, y);
  lcd_write_data(color);
}

/* Bresenham / midpoint circle outline */
void lcd_draw_circle(int cx, int cy, int r, uint16_t color)
{
  int x = 0, y = r, d = 3 - 2 * r;
  while (x <= y) {
    lcd_draw_pixel(cx + x, cy + y, color);
    lcd_draw_pixel(cx - x, cy + y, color);
    lcd_draw_pixel(cx + x, cy - y, color);
    lcd_draw_pixel(cx - x, cy - y, color);
    lcd_draw_pixel(cx + y, cy + x, color);
    lcd_draw_pixel(cx - y, cy + x, color);
    lcd_draw_pixel(cx + y, cy - x, color);
    lcd_draw_pixel(cx - y, cy - x, color);
    if (d < 0) d += 4 * x + 6;
    else       d += 4 * (x - y--) + 10;
    x++;
  }
}

/* Filled circle: draw horizontal spans, much faster than per-pixel */
void lcd_fill_circle(int cx, int cy, int r, uint16_t color)
{
  int x = 0, y = r, d = 3 - 2 * r;
  while (x <= y) {
    lcd_fill_rect(cx - x, cy - y, 2 * x + 1, 1, color);
    lcd_fill_rect(cx - x, cy + y, 2 * x + 1, 1, color);
    lcd_fill_rect(cx - y, cy - x, 2 * y + 1, 1, color);
    lcd_fill_rect(cx - y, cy + x, 2 * y + 1, 1, color);
    if (d < 0) d += 4 * x + 6;
    else       d += 4 * (x - y--) + 10;
    x++;
  }
}
