# Uploading to MKS TFT28 V4.0 (STM32F107VCT6) — all the options

> **Serial Monitor does not upload anything.** It is just a terminal that opens
> a COM port *after* the upload. What you actually want is the **UART
> bootloader**, which is a different thing that happens to use the same wires.

The STM32F107 has a **factory bootloader burned into ROM** (system memory).
It cannot be erased, so it is always available as a rescue path. Per ST's
**AN2606**, the STM32F105xx/107xx bootloader supports **four** interfaces:

| Interface | Pins | Notes |
|---|---|---|
| **USART1** | PA9 = TX, PA10 = RX | on this board PA9 goes to the WiFi header |
| **USART2 (remapped)** | **PD5 = TX, PD6 = RX** | ✅ the **AUX-1 / printer connector** — the practical choice |
| CAN2 (remapped) | PB5 = RX, PB6 = TX | PB6 is the EEPROM SCL here — not usable |
| **DFU (USB OTG FS)** | PA11 = D−, PA12 = D+ | needs HSE = 8 / 14.7456 / **25 MHz** |

---

## Option A — Serial upload over USART2 (PD5 / PD6)  ← recommended without ST-Link

### 1. Hardware

```
USB-TTL adapter (3.3 V logic!)      MKS TFT28
------------------------------      ---------
GND                          <-->   GND
TXD                          <-->   PD6   (MCU RX)
RXD                          <-->   PD5   (MCU TX)
```
PD5/PD6 are on the printer serial connector. **Use a 3.3 V adapter** —
a 5 V one can damage the MCU.

### 2. Enter the bootloader

Set **BOOT0 = 1**, BOOT1 = 0, then reset/power-cycle the board.
Look for a BOOT0 jumper or a test pad on the PCB; if there is none, pull the
BOOT0 pin to 3.3 V through a 10 kΩ resistor. After flashing, put BOOT0 back to 0.

### 3. Arduino IDE

| Tools menu | Value |
|---|---|
| Board | Generic STM32F1 series |
| Board part number | Generic F107VCTx |
| **Upload method** | **STM32CubeProgrammer (Serial)** |
| Port | your USB-TTL COM port |

Press Upload. Bootloader UART format is fixed at **8 bits, EVEN parity,
1 stop bit** and the baud rate is auto-detected — CubeProgrammer handles this.

### Manual equivalent

```bash
STM32_Programmer_CLI -c port=COM5 br=115200 -w firmware.bin 0x08000000 -v -rst
# or with the classic tool:
stm32flash -w firmware.bin -v -g 0x0 /dev/ttyUSB0
```

---

## Option B — DFU over the board's own USB port (no extra hardware!)

The board already has USB on PA11/PA12. With **BOOT0 = 1** the ROM bootloader
enumerates as a **DFU device** and you can flash straight over that USB cable.

⚠️ **Condition:** DFU on the F105/F107 requires an external crystal of exactly
**8 MHz, 14.7456 MHz or 25 MHz**. Check the crystal marking on the board.
If it is something else, DFU will not start and you must use serial or SWD.

| Tools menu | Value |
|---|---|
| Upload method | **STM32CubeProgrammer (DFU)** |

```bash
dfu-util -a 0 -s 0x08000000:leave -D firmware.bin
```

---

## Option C — SWD with ST-Link V2

The most reliable: works regardless of BOOT0, crystal, or a bricked flash,
and gives you a debugger. See [../Arduino/README.md](../Arduino/README.md).

---

## Which should you pick?

| Situation | Use |
|---|---|
| You already have the ST-Link V2 | **SWD** — fastest, debuggable, can't lock you out |
| No ST-Link, USB-TTL available | Serial / USART2 |
| No ST-Link, no USB-TTL, crystal is 8/14.7456/25 MHz | DFU over USB |
| Board bricked / RDP set | SWD only |

You said you already have an ST-Link V2 — **stick with SWD**. Serial is worth
setting up as a backup path, because if you ever flash something that
immediately reconfigures the SWD pins (PA13/PA14) as GPIO, SWD can become hard
to attach, and BOOT0 = 1 is then your way back in.

---

## Using the Serial Monitor for what it *is* good at

Once your own firmware is running, print debug over the same PD5/PD6 pins:

```cpp
// Arduino IDE: Tools -> U(S)ART support -> Enabled (generic 'Serial')
HardwareSerial SerialAUX(PD6, PD5);   // RX, TX

void setup() {
  SerialAUX.begin(115200);
  SerialAUX.println("TFT28 alive");
  lcdInit();
  SerialAUX.println("LCD init done");
}
```
Open Serial Monitor at 115200 on the USB-TTL port. Very useful for finding out
whether the code even reaches `lcdInit()` when the screen stays black.

---

### Reminder
Any of these methods writing to `0x08000000` **destroys the stock MKS
bootloader and firmware**. Back it up before the first write.
