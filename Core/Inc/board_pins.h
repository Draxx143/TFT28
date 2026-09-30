/**
 * board_pins.h — MKS TFT28 V4.0 (STM32F107VCT6)
 *
 * Verified against robotsrulz/MKS-TFT (STM32F107xC && MKS_TFT build).
 * See Docs/PINMAP.md for the full table and notes.
 */
#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include "stm32f1xx_hal.h"

/* ------------------------------------------------------------------ */
/* LCD: 16-bit parallel bus, software (GPIO) driven. No FSMC on F107.  */
/* ------------------------------------------------------------------ */
#define LCD_DATA_GPIO_Port        GPIOE          /* PE0..PE15 = DB0..DB15 */
#define LCD_DATA_ALL_Pins         (0xFFFFU)

#define LCD_nCS_GPIO_Port         GPIOC
#define LCD_nCS_Pin               GPIO_PIN_8     /* PC8  */
#define LCD_RS_GPIO_Port          GPIOD
#define LCD_RS_Pin                GPIO_PIN_13    /* PD13 : 0=cmd 1=data */
#define LCD_nWR_GPIO_Port         GPIOB
#define LCD_nWR_Pin               GPIO_PIN_14    /* PB14 */
#define LCD_nRD_GPIO_Port         GPIOD
#define LCD_nRD_Pin               GPIO_PIN_15    /* PD15 */
#define LCD_BACKLIGHT_GPIO_Port   GPIOD
#define LCD_BACKLIGHT_Pin         GPIO_PIN_14    /* PD14 */
/* NOTE: no LCD reset GPIO on this board - tied to board NRST. */

#define LCD_WIDTH                 320
#define LCD_HEIGHT                240

/* ------------------------------------------------------------------ */
/* Touch: XPT2046/ADS7843 on SPI3 (AFIO remap PC10/PC11/PC12)          */
/* ------------------------------------------------------------------ */
#define TOUCH_nCS_GPIO_Port       GPIOC
#define TOUCH_nCS_Pin             GPIO_PIN_9     /* PC9  */
#define TOUCH_IRQ_GPIO_Port       GPIOC
#define TOUCH_IRQ_Pin             GPIO_PIN_5     /* PC5  (PENIRQ / TOUCH_DI) */
/* SPI3_SCK  = PC10, SPI3_MISO = PC11, SPI3_MOSI = PC12 (remapped)     */
#define TOUCH_SPI                 SPI3

/* ------------------------------------------------------------------ */
/* SPI1 bus: SD card + SPI Flash (shared!)                             */
/* ------------------------------------------------------------------ */
/* SPI1_SCK = PA5, SPI1_MISO = PA6, SPI1_MOSI = PA7                    */
#define STORAGE_SPI               SPI1

#define SDCARD_nCS_GPIO_Port      GPIOD
#define SDCARD_nCS_Pin            GPIO_PIN_11    /* PD11 */
#define SDCARD_DETECT_GPIO_Port   GPIOB
#define SDCARD_DETECT_Pin         GPIO_PIN_15    /* PB15, pull-up */

#define FLASH_nCS_GPIO_Port       GPIOB
#define FLASH_nCS_Pin             GPIO_PIN_9     /* PB9, W25Qxx 8MB */

/* ------------------------------------------------------------------ */
/* EEPROM AT24C16 on I2C1 (PB6 = SCL, PB7 = SDA)                       */
/* ------------------------------------------------------------------ */
#define EEPROM_I2C                I2C1
#define EEPROM_I2C_ADDRESS        (0xA0)

/* ------------------------------------------------------------------ */
/* Serial                                                              */
/* ------------------------------------------------------------------ */
/* USART2 remapped: PD5 = TX, PD6 = RX  -> printer / host (AUX-1)      */
#define HOST_UART                 USART2
/* USART3 remapped: PD8 = TX, PD9 = RX  -> WiFi module                 */
#define WIFI_UART                 USART3

#define WIFI_RESET_GPIO_Port      GPIOA
#define WIFI_RESET_Pin            GPIO_PIN_8     /* PA8 */
#define WIFI_DI_GPIO_Port         GPIOA
#define WIFI_DI_Pin               GPIO_PIN_9     /* PA9, input + EXTI */

/* ------------------------------------------------------------------ */
/* Misc                                                                */
/* ------------------------------------------------------------------ */
#define BUZZER_GPIO_Port          GPIOA
#define BUZZER_Pin                GPIO_PIN_2     /* PA2 */
#define FILAMENT_DI_GPIO_Port     GPIOB
#define FILAMENT_DI_Pin           GPIO_PIN_0     /* PB0 */
#define POWER_DI_GPIO_Port        GPIOB
#define POWER_DI_Pin              GPIO_PIN_1     /* PB1 */

/* USB OTG FS: PA11 = DM, PA12 = DP                                    */
/* SWD:        PA13 = SWDIO, PA14 = SWCLK, NRST                        */

#endif /* BOARD_PINS_H */
