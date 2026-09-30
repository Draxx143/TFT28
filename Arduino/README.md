# Arduino IDE + ST-Link V2 — MKS TFT28 V4.0 (STM32F107VCT6)

Sketch: [`TFT28_Circle/TFT28_Circle.ino`](TFT28_Circle/TFT28_Circle.ino)

---

## 1. Install the STM32 core

Arduino IDE 2.x → **File → Preferences → Additional boards manager URLs**, add:

```
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

Then **Tools → Board → Boards Manager** → search `STM32` → install
**“STM32 MCU based boards”** (by STMicroelectronics).

> Use the **official STM32duino core**, not the old Roger Clark core — the
> latter has no STM32F107 variant.

## 2. Install STM32CubeProgrammer

Download from st.com and install. The Arduino core calls it to talk to the
ST-Link. On Windows add its `bin/` folder to `PATH` if the IDE can't find it.
Also install the ST-Link USB driver (STSW-LINK009) on Windows.

## 3. Tools menu settings

| Setting | Value |
|---|---|
| Board | **Generic STM32F1 series** |
| Board part number | **Generic F107VCTx** |
| Upload method | **STM32CubeProgrammer (SWD)** |
| USB support (if available) | None |
| U(S)ART support | Enabled (generic 'Serial') |
| Optimize | Smallest (-Os) |
| C Runtime Library | Newlib Nano (default) |

## 4. Wiring

```
ST-Link V2          MKS TFT28 JTAG header
----------          ---------------------
GND        <----->  GND
SWDIO      <----->  PA13
SWCLK      <----->  PA14
RESET      <----->  NRST      (optional but helps)
```

* Power the board from **its own supply** (or the printer connector).
* **Do not** connect the ST-Link 3.3 V pin.
* BOOT0 = 0, BOOT1 = 0.
* Disconnect the board from the printer while flashing.

## 5. Back up the stock firmware FIRST

Flashing from Arduino writes to `0x08000000` and **destroys the MKS
bootloader + firmware**. There is no public source to restore it.

```bash
# stlink-tools
st-flash read mks_tft28_backup.bin 0x08000000 0x40000

# or STM32CubeProgrammer CLI
STM32_Programmer_CLI -c port=SWD -r32 0x08000000 0x40000 -o backup.bin
```

If the chip has **read protection (RDP level 1)** enabled, the first
connection will trigger a mass erase — the backup will be impossible and the
original firmware is gone. Know that before you start.

## 6. Upload

Open the sketch → **Upload** (arrow button). You should get a dark blue
screen with an orange circle in the middle, a white outline, four coloured
corner squares, and the circle blinking orange/cyan every 0.7 s.

---

## Why no Adafruit_GFX / TFT_eSPI / UTFT?

Those libraries expect either an SPI panel or an FSMC/8080 memory-mapped bus.
The **STM32F107 has no FSMC**, so the 16-bit bus must be bit-banged. The
sketch does that directly on the registers (`GPIOE->ODR`, `GPIOB->BSRR`) —
`digitalWrite()` per data line would be ~50x slower.

Once this works, `Adafruit_GFX` can be layered on top by subclassing
`Adafruit_GFX` and implementing `drawPixel()` / `fillRect()` with the
functions in this sketch.

## Troubleshooting

| Symptom | Fix |
|---|---|
| `Error: No STM32 target found` | check SWD wiring, board powered, BOOT0 = 0 |
| Upload OK but screen black, backlight off | PD14 not reached — check the sketch actually runs (blink PA2 buzzer or a pin) |
| Backlight on, screen white | init sequence mismatch: panel may be ILI9328 — try `0x0010=0x14B0`, `0x0012=0x008E`, `0x0013=0x0C00`, `0x0029=0x0015` |
| Garbage pixels / noise | WR strobe too fast — add `__NOP(); __NOP();` inside `wrStrobe()` |
| Colours look swapped (red↔blue) | clear the BGR bit: `R03h = 0x1028` |
| Image rotated 90° or mirrored | change `R03h` AM bit and `R60h` GS bit, and the swap in `lcdWindow()` |
| `'GPIOE' was not declared` | wrong board part number selected — must be an F107 variant |
