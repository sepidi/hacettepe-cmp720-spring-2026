/* ============================================================
 * stm32f4.ld - Linker Script for STM32F4
 * SafePulse Lab Series
 *
 * Memory Layout:
 *   Flash: 0x08000000 - 0x080FFFFF (1MB)
 *   SRAM:  0x20000000 - 0x2001FFFF (128KB)
 *
 * DO NOT MODIFY - This file is provided for all labs.
 * ============================================================ */

/* Entry point */
ENTRY(Reset_Handler)

/* Stack at top of SRAM */
_estack = 0x20020000;

/* Memory regions */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* Interrupt vector table - must be at start of flash */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* Program code */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* Initialized data (stored in flash, copied to SRAM at startup) */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >SRAM AT> FLASH

    /* Uninitialized data (zeroed at startup) */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >SRAM

    /* Heap and stack */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        . = . + 0x400;  /* 1KB minimum heap */
        . = . + 0x800;  /* 2KB minimum stack */
        . = ALIGN(8);
    } >SRAM
}
