# MKS TFT28 V4.0 — Pin Map (verified)

MCU: **STM32F107VCT6** (LQFP100, Connectivity Line, 256 KB Flash / 64 KB RAM)

Reference used for verification: `robotsrulz/MKS-TFT` (`Inc/mks_conf.h`, `Src/main.c`,
`Src/stm32f1xx_hal_msp.c`, guarded by `STM32F107xC && MKS_TFT`) — the de-facto open-source
firmware for MKS TFT28/32 V4.x.

## 1. LCD — 16-bit parallel, bit-banged GPIO (NOT FSMC)

> STM32F107 has **no FSMC**. The whole bus is toggled by software on GPIOE.

| Signal | Pin | Note |
|---|---|---|
| DB0..DB15 | **PE0 .. PE15** | whole port E = data bus, push-pull, HIGH speed |
| CS  | **PC8**  | active low |
| RS (D/CX) | **PD13** | 0 = command, 1 = data |
| WR  | **PB14** | active low, write strobe |
| RD  | **PD15** | active low |
| BACKLIGHT | **PD14** | push-pull; HIGH = on |

LCD **RESET is not on a GPIO** on this board — it is tied to the board/NRST reset net.
(The `LCD_RESET` on PE1 you may see in that firmware belongs to the *CZMINI / STM32F103* variant,
not to the MKS TFT28.)

Panel: 320x240, ILI9325/ILI9328-class controller, 65k colors.

## 2. Touch — XPT2046 / ADS7843 on SPI3 (remapped)

| Signal | Pin | Note |
|---|---|---|
| TP_CS   | **PC9**  | `TOUCH_nCS` |
| TP_SCK  | **PC10** | SPI3_SCK  (AFIO remap) |
| TP_MISO | **PC11** | SPI3_MISO (AFIO remap) |
| TP_MOSI | **PC12** | SPI3_MOSI (AFIO remap) |
| TP_INT (PENIRQ) | **PC5** | `TOUCH_DI`, input, EXTI9_5 |

Needs `__HAL_AFIO_REMAP_SPI3_ENABLE()` (and JTAG-DP disabled / SWD-only, because PB3/PB4 belong to JTAG).

## 3. SPI1 bus — shared by SD card and SPI Flash

| Signal | Pin |
|---|---|
| SCK  | **PA5** |
| MISO | **PA6** |
| MOSI | **PA7** |
| SD_CS | **PD11** |
| SD_CARD_DETECT | **PB15** (input, pull-up, EXTI15_10) |
| FLASH_CS (W25Qxx, 8 MB) | **PB9** |

⚠️ SD and SPI Flash are on the **same** SPI1 bus — never assert both CS lines at once.

## 4. EEPROM — AT24C16 on I2C1

| Signal | Pin |
|---|---|
| SCL | **PB6** |
| SDA | **PB7** |

## 5. Serial ports

| Port | Pins | Use |
|---|---|---|
| USART2 (remapped) | **PD5 = TX**, **PD6 = RX** | printer / host link (AUX-1) |
| USART3 (remapped) | **PD8 = TX**, **PD9 = RX** | **WiFi module** (ESP8266 header) |
| PA8 | GPIO | WiFi RESET (board-level) |
| PA9 | GPIO input, EXTI | `WIFI_DI` — WiFi data-available / handshake, **not** USART1_TX in the reference firmware |

## 6. USB (OTG FS, device/host for U-disk)

| Signal | Pin |
|---|---|
| USB_DM | **PA11** |
| USB_DP | **PA12** |

## 7. Misc / extras (were missing from your list)

| Signal | Pin | Note |
|---|---|---|
| BUZZER | **PA2** | `SPEAKER` |
| FILAMENT_DET | **PB0** | input, EXTI0 rising |
| POWER_DET | **PB1** | input, EXTI1 rising |

## 8. Debug

| Signal | Pin |
|---|---|
| SWDIO | **PA13** |
| SWCLK | **PA14** |
| NRST  | NRST |

Boot mode: BOOT0 = 0, BOOT1 = 0.

---

## Review of the pin map you posted

✅ Correct: all LCD data/control pins, backlight, touch CS/SCK/MISO/MOSI/INT,
SD (PA5/PA6/PA7/PD11/PB15), Flash CS PB9, EEPROM PB6/PB7, USB PA11/PA12,
UART PD5/PD6, SWD PA13/PA14.

⚠️ Needs fixing / clarifying:

1. **WiFi section is muddled.** The WiFi UART is **USART3 remapped: PD8 = TX (MCU→ESP), PD9 = RX (MCU←ESP)**.
   Your lines "PA9 ─ TX / PA10 ─ RX" plus "PD8 ─ WIFI_RX / PD9 ─ WIFI_TX" list the WiFi UART twice.
   PA9 is used as an **input** (`WIFI_DI`) in the reference firmware; PA10 is not used for WiFi.
2. **Flash and SD share SPI1** — worth writing it as one bus with two chip selects, not two sections.
3. **No LCD RESET pin** exists; don't allocate a GPIO for it.
4. **Missing:** buzzer PA2, filament detect PB0, power detect PB1.
5. Note that PD5/PD6 is USART2 **remapped** and PD8/PD9 is USART3 **remapped** — both need AFIO remap enabled.
6. Note SPI3 remap requires JTAG disabled (SWD only), otherwise PB3/PB4 conflict.
