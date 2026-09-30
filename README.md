# TFT28

Bare-metal firmware for the **MKS TFT28 V4.0** board (**STM32F107VCT6**).

Current milestone: **draw a circle on the LCD**.

## Two ways to build

* **Arduino IDE** (easiest) -> see [Arduino/README.md](Arduino/README.md), sketch in `Arduino/TFT28_Circle/`
* **Bare-metal Makefile** -> below

## Build

```bash
sudo apt install gcc-arm-none-eabi        # or the ARM GNU toolchain
make
```

Output: `build/tft28.bin`, `build/tft28.hex`, `build/tft28.elf` (~1.6 KB).

## Flash (SWD / ST-Link v2)

```
ST-LINK        MKS TFT28 JTAG header
-------        ---------------------
GND     <--->  GND
SWDIO   <--->  PA13
SWCLK   <--->  PA14
RESET   <--->  NRST   (optional)
```
Power the board from its own supply — **do not** connect the ST-Link 3.3 V pin.
BOOT0 = 0, BOOT1 = 0.

```bash
make flash              # stlink-tools
make flash-openocd      # OpenOCD
```

> ⚠️ **This overwrites the stock MKS bootloader** at `0x08000000`.
> Read it out first if you ever want to go back:
> `st-flash read backup_full.bin 0x08000000 0x40000`

## What the demo does

1. Configures GPIOE as the 16-bit data bus and PC8/PD13/PB14/PD15/PD14 as control lines.
2. Runs the ILI9325/ILI9328 power-on + gamma init sequence, landscape 320x240.
3. Dark-blue background, filled orange circle in the centre with a white outline,
   plus four coloured corner markers (orientation check).
4. Blinks the circle orange/cyan forever so you can see the CPU is alive.

Clock: plain **8 MHz HSI**, no PLL — intentionally simple for first bring-up.

## Layout

```
Core/Inc/lcd.h                 LCD driver API
Core/Inc/board_pins.h          pin map, HAL/CubeMX notation
Core/Inc/board_pins_bare.h     pin bit masks for this bare-metal build
Core/Inc/stm32f107_min.h       minimal RCC/GPIO register defs (no CMSIS needed)
Core/Src/lcd.c                 ILI9325/9328 driver, bit-banged 16-bit bus
Core/Src/main.c                the demo
Core/Src/startup_stm32f107.c   vector table + reset handler
STM32F107VCT6.ld               linker script
Docs/PINMAP.md                 verified pin map
Docs/UPLOAD_METHODS.md         SWD / UART bootloader / DFU
Docs/CUBEPROGRAMMER.md         STM32CubeProgrammer guide
```

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| Backlight on, screen white/blank | init sequence didn't take — panel may be ILI9328 or R61505; try `0x0010 = 0x14B0`, `0x0012 = 0x008E`, `0x0013 = 0x0C00` |
| Backlight off | PD14 not driven, or code never reached `lcd_init()` |
| Garbage / shifted image | GPIOE not fully configured, or WR strobe too fast — add `__NOP()` around the strobe |
| Colours inverted | BGR bit in R03h (`0x1038` -> `0x1028`) |
| Rotated 90° | swap in `lcd_set_window()` + the AM bit of R03h and GS of R60h |
