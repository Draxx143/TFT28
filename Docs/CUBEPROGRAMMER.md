# Using STM32CubeProgrammer with the MKS TFT28 V4.0

STM32CubeProgrammer (STM32CubeProg) is ST's official flashing tool. It comes in
two flavours that do exactly the same thing:

* **GUI** — `STM32CubeProgrammer.exe`
* **CLI** — `STM32_Programmer_CLI` (in the `bin/` folder — add it to `PATH`)

It speaks **SWD/JTAG (ST-Link)**, **UART**, **USB DFU**, SPI, I2C and CAN.
This is also the tool the Arduino IDE calls behind the scenes, so using it
directly just gives you more control.

---

## 0. Get a binary to flash

**From this repo (bare-metal build):**
```
build/tft28.bin        -> program at 0x08000000
build/tft28.hex        -> address is inside the file
build/tft28.elf        -> also accepted
```

**From Arduino IDE:** `Sketch → Export Compiled Binary`. The `.bin`/`.hex`
appear in a `build/STMicroelectronics.stm32.GenF1/` folder next to the sketch.

---

## 1. Connect (SWD / ST-Link V2)

```
ST-Link V2        MKS TFT28 JTAG header
----------        ---------------------
GND     <------>  GND
SWDIO   <------>  PA13
SWCLK   <------>  PA14
RESET   <------>  NRST     (optional, but makes "Under reset" mode work)
```
Power the board from its own supply. **Do not** connect the ST-Link 3.3 V pin.
BOOT0 = 0, BOOT1 = 0.

### GUI
Top-right dropdown → **ST-LINK** → **Connect**.

Settings that matter:

| Field | Value |
|---|---|
| Port | SWD |
| Frequency | 4000 kHz (drop to 1000 if unstable) |
| Mode | **Normal**; use **Under reset** if the MCU refuses to connect |
| Reset mode | Hardware reset (needs the NRST wire) |
| Shared | off |

On success the log shows the device ID and flash size:
```
Device ID     : 0x418          <- STM32F105/F107 connectivity line
Flash size    : 256 KBytes
```

### CLI
```bash
STM32_Programmer_CLI -c port=SWD freq=4000 mode=UR
```

---

## 2. BACK UP THE STOCK FIRMWARE FIRST

Do this before you write anything. The MKS bootloader + firmware are closed
source; once erased there is no official way to get them back.

### GUI
Set **Address** `0x08000000`, **Size** `0x40000` (256 KB), press **Read**,
then the **Save As** icon → `mks_tft28_stock.bin`.

### CLI
```bash
STM32_Programmer_CLI -c port=SWD -r32 0x08000000 0x40000 -o mks_tft28_stock.bin
```

Verify the file is 262144 bytes and is not all `FF` or all `00`.

---

## 3. Read protection (RDP) — read this before connecting

Go to the **OB** (Option Bytes) tab and look at **RDP**.

| RDP value | Meaning | What happens |
|---|---|---|
| `AA` | Level 0, no protection | you can read + write freely ✅ |
| `BB` (or anything else) | Level 1 | reading flash is blocked; **lowering RDP back to AA triggers a full mass erase** — the stock firmware is destroyed |

So: if RDP is already set, **the backup is impossible** and un-protecting the
chip wipes it. Decide consciously before pressing anything.

To un-protect (this WILL erase everything):
OB tab → set **RDP = AA** → **Apply** → power-cycle the board.

CLI:
```bash
STM32_Programmer_CLI -c port=SWD -ob rdp=0xAA
```

---

## 4. Erase and program

### GUI
1. **Erasing & Programming** tab.
2. **File path** → pick `tft28.bin` (or the Arduino `.bin`).
3. **Start address**: `0x08000000` — **required for `.bin` files**.
   (For `.hex` / `.elf` the address is embedded; the field is ignored.)
4. Tick **Verify programming** and **Run after programming**.
5. **Start Programming**.

> Full chip erase first: **Erasing & Programming → Full chip erase**
> (or the **Erase flash memory** button in the Memory tab).

### CLI
```bash
# program + verify + reset and run
STM32_Programmer_CLI -c port=SWD -w build/tft28.bin 0x08000000 -v -rst

# hex file (no address needed)
STM32_Programmer_CLI -c port=SWD -w build/tft28.hex -v -rst

# full chip erase
STM32_Programmer_CLI -c port=SWD -e all
```

---

## 5. The other connection modes

### UART (no ST-Link)
BOOT0 = 1, reset, then a **3.3 V** USB-TTL on **PD5 (MCU TX) / PD6 (MCU RX)**:

```bash
STM32_Programmer_CLI -c port=COM5 br=115200 -w build/tft28.bin 0x08000000 -v -rst
```
GUI: dropdown → **UART**, pick the port, Parity = **Even**, then Connect.
Put BOOT0 back to 0 afterwards.

### USB DFU (the board's own USB port)
BOOT0 = 1, reset, plug the USB cable. Requires the board crystal to be
**8 / 14.7456 / 25 MHz**.
```bash
STM32_Programmer_CLI -c port=USB1 -w build/tft28.bin 0x08000000 -v -rst
```
GUI: dropdown → **USB**, Port = `USB1`.

---

## Handy CLI reference

| Task | Command |
|---|---|
| List ST-Links | `STM32_Programmer_CLI -l st-link` |
| List USB DFU devices | `STM32_Programmer_CLI -l usb` |
| Connect + show info | `STM32_Programmer_CLI -c port=SWD` |
| Read 256 KB to file | `-c port=SWD -r32 0x08000000 0x40000 -o dump.bin` |
| Program + verify + run | `-c port=SWD -w fw.bin 0x08000000 -v -rst` |
| Mass erase | `-c port=SWD -e all` |
| Read option bytes | `-c port=SWD -ob displ` |
| Remove read protection | `-c port=SWD -ob rdp=0xAA` |
| Just reset and run | `-c port=SWD -rst` |

---

## Troubleshooting

| Message | Cause / fix |
|---|---|
| `No STM32 target found` | wiring, board not powered, or SWD pins remapped — try **mode=UR** (Under reset) with the NRST wire connected |
| `Error: Data read failed` | RDP level 1 is active — see section 3 |
| `Cannot read memory... target not halted` | use Under reset mode, or hold NRST while connecting |
| Connects once, then never again | your firmware disabled SWD or put the MCU to sleep early — connect with **Under reset**, or BOOT0 = 1 to boot the ROM bootloader instead |
| `ST-LINK firmware upgrade required` | run **STLinkUpgrade** from the CubeProgrammer install folder |
| Programs fine but nothing runs | `.bin` written to the wrong address — it must be `0x08000000` |
