# SafePulse Lab 3 - Makefile
# Sensor Interfacing via I2C, Drivers & Hard Timing

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS += -nostdlib -nostartfiles
CFLAGS += -Wall -Wextra -O1 -g
CFLAGS += -T stm32f4.ld

SRC = startup.s main_solution.c
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

.PHONY: all clean
