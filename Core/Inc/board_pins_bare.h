/**
 * board_pins_bare.h — bit masks for the bare-metal demo.
 * (board_pins.h holds the same map in HAL notation, for CubeMX projects.)
 * Board: MKS TFT28 V4.0 / STM32F107VCT6
 */
#ifndef BOARD_PINS_BARE_H
#define BOARD_PINS_BARE_H

/* LCD control */
#define LCD_CS_PIN     (1U << 8)    /* PC8  */
#define LCD_RS_PIN     (1U << 13)   /* PD13 */
#define LCD_WR_PIN     (1U << 14)   /* PB14 */
#define LCD_RD_PIN     (1U << 15)   /* PD15 */
#define LCD_BL_PIN     (1U << 14)   /* PD14 */
/* LCD data = PE0..PE15 (whole port E) */

/* Other chip selects we must keep de-asserted */
#define TOUCH_CS_PIN   (1U << 9)    /* PC9  */
#define SD_CS_PIN      (1U << 11)   /* PD11 */
#define FLASH_CS_PIN   (1U << 9)    /* PB9  */

/* Misc */
#define BUZZER_PIN     (1U << 2)    /* PA2  */

#endif /* BOARD_PINS_BARE_H */
