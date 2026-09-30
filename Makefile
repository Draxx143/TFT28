# MKS TFT28 V4.0 (STM32F107VCT6) - bare-metal LCD demo

TARGET  = tft28
BUILD   = build

PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE    = $(PREFIX)size

SRCS = Core/Src/main.c Core/Src/lcd.c Core/Src/startup_stm32f107.c
INCS = -ICore/Inc

CPU    = -mcpu=cortex-m3 -mthumb
CFLAGS = $(CPU) $(INCS) -Os -g3 -Wall -Wextra -ffunction-sections -fdata-sections \
         -fno-common -std=c11
LDFLAGS = $(CPU) -TSTM32F107VCT6.ld -nostdlib -Wl,--gc-sections \
          -Wl,-Map=$(BUILD)/$(TARGET).map

OBJS = $(addprefix $(BUILD)/,$(notdir $(SRCS:.c=.o)))
vpath %.c $(sort $(dir $(SRCS)))

all: $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).hex

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD):
	mkdir -p $@

# Flash with ST-Link (needs stlink-tools or OpenOCD)
flash: $(BUILD)/$(TARGET).bin
	st-flash write $(BUILD)/$(TARGET).bin 0x08000000

flash-openocd: $(BUILD)/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
	  -c "program $(BUILD)/$(TARGET).elf verify reset exit"

clean:
	rm -rf $(BUILD)

.PHONY: all clean flash flash-openocd
