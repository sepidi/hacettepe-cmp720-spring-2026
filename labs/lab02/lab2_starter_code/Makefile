# SafePulse Lab 2 - Makefile
# Build bare-metal firmware for STM32F4 (ARM Cortex-M4)

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS += -nostdlib -nostartfiles
CFLAGS += -Wall -Wextra -O1 -g
CFLAGS += -T stm32f4.ld

SRC = startup.s main.c
TARGET = build/safepulse

all: $(TARGET).elf
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin

clean:
	rm -rf build/

# Verbose build (use: make V=1)
ifdef V
  CFLAGS += -v
endif

.PHONY: all clean
