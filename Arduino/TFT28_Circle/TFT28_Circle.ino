/*
 * TFT28_Circle.ino
 * ----------------------------------------------------------------------
 * MKS TFT28 V4.0  (STM32F107VCT6)  -  draw a circle on the LCD
 * Arduino IDE 2.x + official STM32duino core, flashed with ST-Link V2.
 *
 * Tools menu:
 *   Board            : Generic STM32F1 series
 *   Board part number: Generic F107VCTx
 *   Upload method    : STM32CubeProgrammer (SWD)
 *   USB support      : None
 *   Optimize         : Smallest (-Os)
 *
 * Wiring (ST-Link V2 -> board JTAG header):
 *   GND -> GND,  SWDIO -> PA13,  SWCLK -> PA14,  (RESET -> NRST optional)
 *   Power the board from its own supply. Do NOT connect ST-Link 3.3 V.
 *
 * !! Flashing overwrites the stock MKS bootloader. Back it up first. !!
 *
 * Panel: ILI9325 / ILI9328 class, 16-bit parallel bus, bit-banged.
 *   DB0..DB15 = PE0..PE15      CS = PC8    RS = PD13
 *   WR = PB14                  RD = PD15   BACKLIGHT = PD14
 * ----------------------------------------------------------------------
 */

/* ---------------- pins ---------------- */
#define LCD_CS_PIN      PC8
#define LCD_RS_PIN      PD13
#define LCD_WR_PIN      PB14
#define LCD_RD_PIN      PD15
#define LCD_BL_PIN      PD14

#define TOUCH_CS_PIN    PC9    // keep de-asserted
#define SD_CS_PIN       PD11   // keep de-asserted
#define FLASH_CS_PIN    PB9    // keep de-asserted

/* Bit masks used for the fast register writes */
#define M_RS   (1u << 13)      // PD13
#define M_WR   (1u << 14)      // PB14

/* ---------------- screen ---------------- */
#define LCD_WIDTH   320        // landscape
#define LCD_HEIGHT  240

#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define ORANGE  0xFD20

/* ======================================================================
 *  Low level bus  -  direct register access, digitalWrite is far too slow
 * ====================================================================== */

static inline void wrStrobe() {
  GPIOB->BSRR = M_WR << 16;    // WR low
  GPIOB->BSRR = M_WR;          // WR high -> panel latches the bus
}

static inline void lcdCmd(uint16_t c) {
  GPIOD->BSRR = M_RS << 16;    // RS = 0 -> index register
  GPIOE->ODR  = c;
  wrStrobe();
}

static inline void lcdData(uint16_t d) {
  GPIOD->BSRR = M_RS;          // RS = 1 -> data
  GPIOE->ODR  = d;
  wrStrobe();
}

static inline void lcdReg(uint16_t reg, uint16_t val) {
  lcdCmd(reg);
  lcdData(val);
}

/* Same pixel n times: bus stays stable, only WR toggles. */
static void lcdRepeat(uint16_t d, uint32_t n) {
  GPIOD->BSRR = M_RS;
  GPIOE->ODR  = d;
  while (n--) {
    GPIOB->BSRR = M_WR << 16;
    GPIOB->BSRR = M_WR;
  }
}

/* ======================================================================
 *  Addressing  (panel is natively 240x320, we run it landscape)
 * ====================================================================== */
static void lcdWindow(int x1, int y1, int x2, int y2) {
  uint16_t px1 = y1, px2 = y2;   // screen X <-> panel Y
  uint16_t py1 = x1, py2 = x2;

  lcdReg(0x50, px1);
  lcdReg(0x51, px2);
  lcdReg(0x52, py1);
  lcdReg(0x53, py2);
  lcdReg(0x20, px1);
  lcdReg(0x21, py1);
  lcdCmd(0x22);                  // write to GRAM
}

/* ======================================================================
 *  Drawing
 * ====================================================================== */
void lcdFillScreen(uint16_t color) {
  lcdWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
  lcdRepeat(color, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
}

void lcdFillRect(int x, int y, int w, int h, uint16_t color) {
  if (w <= 0 || h <= 0) return;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
  if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
  if (w <= 0 || h <= 0) return;

  lcdWindow(x, y, x + w - 1, y + h - 1);
  lcdRepeat(color, (uint32_t)w * h);
}

void lcdPixel(int x, int y, uint16_t color) {
  if (x < 0 || y < 0 || x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
  lcdWindow(x, y, x, y);
  lcdData(color);
}

/* midpoint circle - outline */
void lcdDrawCircle(int cx, int cy, int r, uint16_t color) {
  int x = 0, y = r, d = 3 - 2 * r;
  while (x <= y) {
    lcdPixel(cx + x, cy + y, color);  lcdPixel(cx - x, cy + y, color);
    lcdPixel(cx + x, cy - y, color);  lcdPixel(cx - x, cy - y, color);
    lcdPixel(cx + y, cy + x, color);  lcdPixel(cx - y, cy + x, color);
    lcdPixel(cx + y, cy - x, color);  lcdPixel(cx - y, cy - x, color);
    if (d < 0) d += 4 * x + 6;
    else       d += 4 * (x - y--) + 10;
    x++;
  }
}

/* filled circle - horizontal spans, much faster than per-pixel */
void lcdFillCircle(int cx, int cy, int r, uint16_t color) {
  int x = 0, y = r, d = 3 - 2 * r;
  while (x <= y) {
    lcdFillRect(cx - x, cy - y, 2 * x + 1, 1, color);
    lcdFillRect(cx - x, cy + y, 2 * x + 1, 1, color);
    lcdFillRect(cx - y, cy - x, 2 * y + 1, 1, color);
    lcdFillRect(cx - y, cy + x, 2 * y + 1, 1, color);
    if (d < 0) d += 4 * x + 6;
    else       d += 4 * (x - y--) + 10;
    x++;
  }
}

/* ======================================================================
 *  Init
 * ====================================================================== */
static void lcdGpioInit() {
  /* Whole port E as the data bus. Written directly because Arduino's
     pinMode() would be 16 slow calls and may not pick max slew rate.
     0x3 per nibble = output push-pull, 50 MHz.                        */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIOE->CRL = 0x33333333;
  GPIOE->CRH = 0x33333333;
  GPIOE->ODR = 0x0000;

  pinMode(LCD_CS_PIN, OUTPUT);
  pinMode(LCD_RS_PIN, OUTPUT);
  pinMode(LCD_WR_PIN, OUTPUT);
  pinMode(LCD_RD_PIN, OUTPUT);
  pinMode(LCD_BL_PIN, OUTPUT);

  /* Park the other chip selects high so they don't interfere */
  pinMode(TOUCH_CS_PIN, OUTPUT);  digitalWrite(TOUCH_CS_PIN, HIGH);
  pinMode(SD_CS_PIN,    OUTPUT);  digitalWrite(SD_CS_PIN,    HIGH);
  pinMode(FLASH_CS_PIN, OUTPUT);  digitalWrite(FLASH_CS_PIN, HIGH);

  digitalWrite(LCD_CS_PIN, HIGH);
  digitalWrite(LCD_WR_PIN, HIGH);
  digitalWrite(LCD_RD_PIN, HIGH);
  digitalWrite(LCD_BL_PIN, LOW);   // backlight off until the panel is ready
}

void lcdInit() {
  lcdGpioInit();
  delay(50);

  /* No LCD reset GPIO on this board - the panel shares the board reset. */
  digitalWrite(LCD_CS_PIN, LOW);   // CS stays low for the whole session

  const uint16_t R01h = 0x0000;    // SS=0, SM=0
  const uint16_t R03h = 0x1038;    // BGR=1, I/D=11, AM=1  (landscape)
  const uint16_t R60h = 0xA700;    // GS=1, 320 lines

  lcdReg(0x0001, R01h);
  lcdReg(0x0002, 0x0700);
  lcdReg(0x0003, R03h);
  lcdReg(0x0004, 0x0000);
  lcdReg(0x0008, 0x0202);
  lcdReg(0x0009, 0x0000);
  lcdReg(0x000A, 0x0000);

  /* power on */
  lcdReg(0x0010, 0x0000);
  lcdReg(0x0011, 0x0007);
  lcdReg(0x0012, 0x0000);
  lcdReg(0x0013, 0x0000);
  delay(200);

  lcdReg(0x0010, 0x1590);
  lcdReg(0x0011, 0x0227);
  delay(50);
  lcdReg(0x0012, 0x009C);
  delay(50);
  lcdReg(0x0013, 0x1900);
  lcdReg(0x0029, 0x0023);
  lcdReg(0x002B, 0x000E);
  delay(50);

  /* gamma */
  lcdReg(0x0030, 0x0007);  lcdReg(0x0031, 0x0707);
  lcdReg(0x0032, 0x0006);  lcdReg(0x0035, 0x0704);
  lcdReg(0x0036, 0x1F04);  lcdReg(0x0037, 0x0004);
  lcdReg(0x0038, 0x0000);  lcdReg(0x0039, 0x0706);
  lcdReg(0x003C, 0x0701);  lcdReg(0x003D, 0x000F);
  delay(50);

  /* full-panel window */
  lcdReg(0x0050, 0x0000);  lcdReg(0x0051, 239);
  lcdReg(0x0052, 0x0000);  lcdReg(0x0053, 319);

  lcdReg(0x0060, R60h);
  lcdReg(0x0061, 0x0001);

  lcdReg(0x0090, 0x0010);
  lcdReg(0x0092, 0x0000);
  lcdReg(0x0095, 0x0110);
  lcdReg(0x0097, 0x0000);

  lcdReg(0x0007, 0x0133);          // display ON
  delay(50);

  lcdFillScreen(BLACK);
  digitalWrite(LCD_BL_PIN, HIGH);  // backlight on
}

/* ======================================================================
 *  Sketch
 * ====================================================================== */
void setup() {
  lcdInit();

  lcdFillScreen(RGB565(0, 0, 40));                          // dark blue

  lcdFillCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 70, ORANGE); // the circle
  lcdDrawCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 70, WHITE);
  lcdDrawCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 71, WHITE);

  /* corner markers - confirm orientation and full addressability */
  lcdFillRect(0, 0, 10, 10, RED);
  lcdFillRect(LCD_WIDTH - 10, 0, 10, 10, GREEN);
  lcdFillRect(0, LCD_HEIGHT - 10, 10, 10, BLUE);
  lcdFillRect(LCD_WIDTH - 10, LCD_HEIGHT - 10, 10, 10, YELLOW);
}

void loop() {
  /* blink the circle so you can see the CPU is alive */
  delay(700);
  lcdFillCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 69, CYAN);
  delay(700);
  lcdFillCircle(LCD_WIDTH / 2, LCD_HEIGHT / 2, 69, ORANGE);
}
