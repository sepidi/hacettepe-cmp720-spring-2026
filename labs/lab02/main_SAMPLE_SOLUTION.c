#include <stdint.h>

/* ============================================================
 * Register Definitions for STM32F4
 * ============================================================ */

/* Reset and Clock Control (RCC) */
#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x44))

/* GPIO Port D (LEDs on STM32F4-Discovery) */
#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR       (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

/* USART1 */
#define USART1_BASE     0x40011000
#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

/* Bit definitions */
#define USART_SR_TXE    (1 << 7)   /* Transmit data register empty */
#define USART_CR1_UE    (1 << 13)  /* USART enable */
#define USART_CR1_TE    (1 << 3)   /* Transmitter enable */

#define LED_GREEN_PIN   12  /* PD12 = Green LED */

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
 * SOLUTION: Initialize UART1
 * ============================================================ */
void uart_init(void) {
    /* a) Enable USART1 clock (bit 4 of APB2ENR) */
    RCC_APB2ENR |= (1 << 4);

    /* b) Set baud rate: 84MHz / 115200 = 729 (0x2D9) */
    USART1_BRR = 0x2D9;

    /* c) Enable USART (UE) and Transmitter (TE) */
    USART1_CR1 = USART_CR1_UE | USART_CR1_TE;
}

/* ============================================================
 * SOLUTION: Initialize GPIO for LED
 * ============================================================ */
void led_init(void) {
    /* a) Enable GPIOD clock (bit 3 of AHB1ENR) */
    RCC_AHB1ENR |= (1 << 3);

    /* b) Set PD12 to output mode (bits 25:24 = 01)
     *    First clear the bits, then set to output */
    GPIOD_MODER &= ~(3 << (LED_GREEN_PIN * 2));  /* Clear */
    GPIOD_MODER |=  (1 << (LED_GREEN_PIN * 2));   /* Set output */
}

/* ============================================================
 * SOLUTION: Toggle LED
 * ============================================================ */
void led_toggle(void) {
    GPIOD_ODR ^= (1 << LED_GREEN_PIN);
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
