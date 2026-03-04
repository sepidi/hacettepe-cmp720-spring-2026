#include <stdint.h>

/* ============================================================
 * Register Definitions for STM32F4 -- Week 3 - Lab 2
 *
 * All base addresses and offsets are from the STM32F4
 * Reference Manual (RM0090):
 *   - RCC registers: Section 7 (register map at end of section)
 *   - GPIO registers: Section 8 (register map at end of section)
 *   - USART registers: Section 30 (register map at end of section)
 * ============================================================ */

/* Reset and Clock Control (RCC) -- RM0090 Section 7 */
#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x44))

/* GPIO Port D (LEDs on STM32F4-Discovery) -- RM0090 Section 8 */
#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR       (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

/* USART1 -- RM0090 Section 30 */
#define USART1_BASE     0x40011000
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

/* Bit definitions -- bit positions from RM0090 register descriptions */
/* See FAQ for more clarifications */
#define USART_SR_TXE    (1 << 7)   /* Transmit data register empty */
#define USART_CR1_UE    (1 << 13)  /* USART enable */
#define USART_CR1_TE    (1 << 3)   /* Transmitter enable */

#define LED_GREEN_PIN   12  /* PD12 = Green LED on STM32F4-Discovery */

/* ============================================================
 * Helper Functions (provided)
 * ============================================================ */

/* Simple busy-wait delay (NOT suitable for real-time!) */
void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 8400; i++) {
        __asm__("nop");
    }
}

/* Send a single character over UART */
void uart_putc(char c) {
    while (!(USART1_SR & USART_SR_TXE));  /* Wait until TX empty */
    USART1_DR = c;
}

/* Send a string over UART */
void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

/* Convert integer to decimal string (simple implementation) */
void uart_put_int(int val) {
    char buf[12];
    int i = 0;
    if (val < 0) { uart_putc('-'); val = -val; }
    if (val == 0) { uart_putc('0'); return; }
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (i > 0) { uart_putc(buf[--i]); }
}

/* ============================================================
 * TODO 1: Initialize UART1
 *
 * Reference: RM0090 Section 30 (USART) -- register map at end
 *            RM0090 Section 7  (RCC)   -- APB2ENR register
 *
 * Steps:
 *   a) Enable USART1 clock via RCC_APB2ENR (bit 4).
 *      Look up "RCC_APB2ENR" in RM0090 Section 7 to verify
 *      which bit corresponds to USART1EN.
 *
 *   b) Set baud rate register (BRR).
 *      For a peripheral clock of 84 MHz and 115200 baud:
 *        BRR = 84,000,000 / 115,200 = 729 = 0x2D9
 *      (See RM0090 Section 30.3.4 for the BRR formula.)
 *
 *   c) Enable the USART and its transmitter by writing to CR1.
 *      You need two bits set: UE (bit 13) enables the whole
 *      USART, and TE (bit 3) enables the transmitter.
 *      To set multiple bits in one write, combine masks with
 *      the bitwise OR operator (|). For example:
 *        USART_CR1_UE = (1 << 13) = 0x2000 # alredy done in definitions
 *        USART_CR1_TE = (1 << 3)  = 0x0008 # alredy done in definitions
 *        USART_CR1_UE | USART_CR1_TE = 0x2008
 *      Write the combined value to USART1_CR1.
 *      Note: Use direct assignment (=) rather than |= so CR1
 *      contains only these bits (others stay 0). This selects
 *      default settings: 8 data bits, no parity
 *      (see RM0090 Section 30.6.4).
 * ============================================================ */
void uart_init(void) {
    // TODO: Your code here
}

/* ============================================================
 * TODO 2: Initialize GPIO for LED
 *
 * Reference: RM0090 Section 8 (GPIO) -- MODER and ODR registers
 *            RM0090 Section 7 (RCC)  -- AHB1ENR register
 *
 * Steps:
 *   a) Enable GPIOD clock via RCC_AHB1ENR (bit 3).
 *      Look up "RCC_AHB1ENR" in RM0090 Section 7 to verify
 *      which bit corresponds to GPIODEN.
 *
 *   b) Configure PD12 as a general-purpose output in GPIOD_MODER.
 *      Background (RM0090 Section 8.4.1): MODER uses 2 bits per pin.
 *      Pin 0 uses bits [1:0], pin 1 uses [3:2], ... pin N uses
 *      bits [2N+1:2N]. Modes: 00=input, 01=output, 10=AF, 11=analog.
 *      For PD12, bits are [25:24] since 12 * 2 = 24. Write 01 there.
 *
 *      Use a clear-then-set pattern:
 *      Step 1 - Clear bits [25:24] to 00
 *      Step 2 - Set output mode (01):
 *
 *      Why two steps? If the field was 11 (analog), |= 01 would 
 *      keep it 11. Clearing first guarantees the correct value 
 *      for any prior state.
 * ============================================================ */
void led_init(void) {
    // TODO: Your code here
}

/* ============================================================
 * TODO 3: Toggle LED
 *
 * Reference: RM0090 Section 8 (GPIO) -- ODR register
 *
 * XOR the bit for PD12 in GPIOD_ODR to toggle the pin.
 * XOR is the standard read-modify-write pattern for toggling
 * a single bit without affecting others.
 * ============================================================ */
void led_toggle(void) {
    // TODO: Your code here
}

/* ============================================================
 * Main Function
 * ============================================================ */
int main(void) {
    /* Initialize peripherals */
    uart_init();
    led_init();

    /* Startup banner */
    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  SafePulse v0.1 - System Online\r\n");
    uart_puts("  Patient Vital Signs Monitor\r\n");
    uart_puts("========================================\r\n");
    uart_puts("\r\n");

    /* Simulated patient data (hardcoded for this week) */
    int heart_rate = 72;      /* BPM */
    int spo2 = 97;            /* percent */
    int temperature = 368;    /* 36.8 C (x10 for fixed-point) */

    int count = 0;

    /* Main loop: blink LED and print data at ~1Hz */
    while (1) {
        led_toggle();

        /* Print current "readings" */
        uart_puts("[");
        uart_put_int(count);
        uart_puts("] HR=");
        uart_put_int(heart_rate);
        uart_puts(" bpm, SpO2=");
        uart_put_int(spo2);
        uart_puts("%, Temp=");
        uart_put_int(temperature / 10);
        uart_puts(".");
        uart_put_int(temperature % 10);
        uart_puts(" C, State=NORMAL\r\n");

        count++;

        /* Delay ~1 second using busy-wait */
        delay_ms(1000);
    }

    return 0;  /* Never reached */
}
