#include <stdint.h>

/* ============================================================
 * Register Definitions for STM32F4 -- Week 4 - Lab 3
 * ============================================================ */

/* Reset and Clock Control (RCC) */
#define RCC_BASE        0x40023800
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x44))

/* GPIO Port B (I2C1 pins: PB6=SCL, PB7=SDA) */
#define GPIOB_BASE      0x40020400
#define GPIOB_MODER     (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER    (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_OSPEEDR   (*(volatile uint32_t *)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR     (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRL      (*(volatile uint32_t *)(GPIOB_BASE + 0x20))

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

/* I2C1 (RM0090 Section 27.6) */
#define I2C1_BASE       0x40005400
#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_OAR1       (*(volatile uint32_t *)(I2C1_BASE + 0x08))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20))

/* TIM2 (General-purpose 32-bit timer) */
#define TIM2_BASE       0x40000000
#define TIM2_CR1        (*(volatile uint32_t *)(TIM2_BASE + 0x00))
#define TIM2_DIER       (*(volatile uint32_t *)(TIM2_BASE + 0x0C))
#define TIM2_SR         (*(volatile uint32_t *)(TIM2_BASE + 0x10))
#define TIM2_CNT        (*(volatile uint32_t *)(TIM2_BASE + 0x24))
#define TIM2_PSC        (*(volatile uint32_t *)(TIM2_BASE + 0x28))
#define TIM2_ARR        (*(volatile uint32_t *)(TIM2_BASE + 0x2C))

/* NVIC (Nested Vectored Interrupt Controller) */
#define NVIC_ISER0      (*(volatile uint32_t *)0xE000E100)
#define NVIC_ISER1      (*(volatile uint32_t *)0xE000E104)

/* ============================================================
 * Bit Definitions
 * ============================================================ */

/* USART */
#define USART_SR_TXE    (1 << 7)
#define USART_CR1_UE    (1 << 13)
#define USART_CR1_TE    (1 << 3)

/* TIM */
#define TIM_CR1_CEN     (1 << 0)
#define TIM_DIER_UIE    (1 << 0)
#define TIM_SR_UIF      (1 << 0)

/* I2C CR1 bits */
#define I2C_CR1_PE      (1 << 0)
#define I2C_CR1_START   (1 << 8)
#define I2C_CR1_STOP    (1 << 9)
#define I2C_CR1_ACK     (1 << 10)
#define I2C_CR1_SWRST   (1 << 15)

/* I2C SR1 bits */
#define I2C_SR1_SB      (1 << 0)
#define I2C_SR1_ADDR    (1 << 1)
#define I2C_SR1_BTF     (1 << 2)
#define I2C_SR1_RXNE    (1 << 6)
#define I2C_SR1_TXE     (1 << 7)

/* GPIO / LED */
#define LED_GREEN_PIN   12
#define LED_RED_PIN     14

/* Vital Signs Sensor - I2C Interface */
#define SENSOR_I2C_ADDR     0x55

#define SENSOR_REG_HR       0x00
#define SENSOR_REG_SPO2     0x01
#define SENSOR_REG_TEMP_INT 0x02
#define SENSOR_REG_TEMP_FRC 0x03
#define SENSOR_REG_ID       0x04
#define SENSOR_REG_SCENARIO 0x05

/* Critical Thresholds */
#define SPO2_WARNING    94
#define SPO2_CRITICAL   90
#define TEMP_WARNING    380
#define TEMP_CRITICAL   400
#define HR_WARNING_LOW    50   /* Bradycardia warning */
#define HR_WARNING_HIGH   120  /* Tachycardia warning */
#define HR_CRITICAL_LOW   40   /* Severe bradycardia */
#define HR_CRITICAL_HIGH  150  /* Severe tachycardia */

/* System State */
typedef enum {
    STATE_NORMAL,
    STATE_WARNING,
    STATE_CRITICAL,
    STATE_SENSOR_FAULT
} system_state_t;

volatile system_state_t current_state = STATE_NORMAL;
volatile uint8_t  latest_hr = 0;
volatile uint8_t  latest_spo2 = 0;
volatile uint8_t  latest_temp_int = 0;
volatile uint8_t  latest_temp_frac = 0;
volatile uint32_t sample_count = 0;
volatile uint8_t  new_data_flag = 0;

/* ============================================================
 * UART Functions (from Week 3)
 * ============================================================ */
void uart_init(void) {
    RCC_APB2ENR |= (1 << 4);
    USART1_BRR = 0x2D9;
    USART1_CR1 = USART_CR1_UE | USART_CR1_TE;
}

void uart_putc(char c) {
    while (!(USART1_SR & USART_SR_TXE));
    USART1_DR = c;
}

void uart_puts(const char *s) {
    while (*s) { uart_putc(*s++); }
}

void uart_put_int(int val) {
    char buf[12];
    int i = 0;
    if (val < 0) { uart_putc('-'); val = -val; }
    if (val == 0) { uart_putc('0'); return; }
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (i > 0) { uart_putc(buf[--i]); }
}

/* ============================================================
 * GPIO Functions
 * ============================================================ */
void gpio_init(void) {
    RCC_AHB1ENR |= (1 << 3);
    GPIOD_MODER &= ~((3 << (LED_GREEN_PIN * 2)) | (3 << (LED_RED_PIN * 2)));
    GPIOD_MODER |=  ((1 << (LED_GREEN_PIN * 2)) | (1 << (LED_RED_PIN * 2)));
    GPIOD_ODR &= ~(1 << LED_RED_PIN);
}

void alarm_on(void) {
    GPIOD_ODR |= (1 << LED_RED_PIN);
}

void alarm_off(void) {
    GPIOD_ODR &= ~(1 << LED_RED_PIN);
}

/* ============================================================
 * I2C1 Driver (provided complete)
 * ============================================================ */
void i2c_init(void) {
    RCC_AHB1ENR |= (1 << 1);   /* GPIOB clock */
    RCC_APB1ENR |= (1 << 21);  /* I2C1 clock  */

    /* PB6 (SCL) and PB7 (SDA) as AF4, open-drain, pull-up */
    GPIOB_MODER  &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_MODER  |=  ((2 << (6 * 2)) | (2 << (7 * 2)));
    GPIOB_OTYPER |=  ((1 << 6) | (1 << 7));
    GPIOB_OSPEEDR |= ((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_PUPDR &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_PUPDR |=  ((1 << (6 * 2)) | (1 << (7 * 2)));
    GPIOB_AFRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB_AFRL |=  ((4U   << 24) | (4U   << 28));

    /* Software reset then configure */
    I2C1_CR1 |= I2C_CR1_SWRST;
    I2C1_CR1 &= ~I2C_CR1_SWRST;

    I2C1_CR2 = 42;       /* APB1 = 42 MHz */
    I2C1_CCR = 210;      /* 100 kHz standard mode */
    I2C1_TRISE = 43;     /* Max rise time */

    I2C1_CR1 |= I2C_CR1_PE;  /* Enable */
}

uint8_t i2c_read_register(uint8_t dev_addr, uint8_t reg_addr) {
    /* Write phase: send register address */
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));

    I2C1_DR = (uint32_t)(dev_addr << 1);
    while (!(I2C1_SR1 & I2C_SR1_ADDR));
    (void)I2C1_SR2;

    I2C1_DR = reg_addr;
    while (!(I2C1_SR1 & I2C_SR1_BTF));

    /* Read phase: repeated start, read one byte */
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));

    I2C1_DR = (uint32_t)(dev_addr << 1) | 1;
    while (!(I2C1_SR1 & I2C_SR1_ADDR));

    I2C1_CR1 &= ~I2C_CR1_ACK;
    (void)I2C1_SR2;
    I2C1_CR1 |= I2C_CR1_STOP;

    while (!(I2C1_SR1 & I2C_SR1_RXNE));
    return (uint8_t)I2C1_DR;
}

void i2c_write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t value) {
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));

    I2C1_DR = (uint32_t)(dev_addr << 1);
    while (!(I2C1_SR1 & I2C_SR1_ADDR));
    (void)I2C1_SR2;

    I2C1_DR = reg_addr;
    while (!(I2C1_SR1 & I2C_SR1_TXE));

    I2C1_DR = value;
    while (!(I2C1_SR1 & I2C_SR1_BTF));

    I2C1_CR1 |= I2C_CR1_STOP;
}

/* ============================================================
 * SOLUTION 1: Read Vital Signs from Sensor (via I2C)
 * ============================================================ */
uint8_t read_vitals(void) {
    if (i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_ID) != 0xA5) {
        return 0;
    }

    latest_hr        = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_HR);
    latest_spo2      = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_SPO2);
    latest_temp_int  = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_TEMP_INT);
    latest_temp_frac = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_TEMP_FRC);

    return 1;
}

/* ============================================================
 * SOLUTION 2: Check Thresholds and Update System State
 * ============================================================ */
void check_thresholds(void) {
    uint16_t temp_tenths = (uint16_t)latest_temp_int * 10 + latest_temp_frac;

    /* CRITICAL check first (safety priority) */
    if (latest_spo2 < SPO2_CRITICAL
        || temp_tenths > TEMP_CRITICAL
        || latest_hr < HR_CRITICAL_LOW
        || latest_hr > HR_CRITICAL_HIGH) {
        if (current_state != STATE_CRITICAL) {
            current_state = STATE_CRITICAL;
            alarm_on();
        }
        return;
    }

    /* WARNING check */
    if (latest_spo2 <= SPO2_WARNING
        || temp_tenths >= TEMP_WARNING
        || latest_hr < HR_WARNING_LOW
        || latest_hr > HR_WARNING_HIGH) {
        if (current_state == STATE_CRITICAL) {
            alarm_off();
        }
        current_state = STATE_WARNING;
        return;
    }

    /* All normal */
    if (current_state == STATE_CRITICAL) { // Bad bad implementation
       alarm_off();                        // shame on you Selma hoca
    }
    current_state = STATE_NORMAL;
}

/* ============================================================
 * SOLUTION 3: Configure TIM2 for 200ms periodic interrupt
 * ============================================================ */
void timer_init(void) {
    RCC_APB1ENR |= (1 << 0);
    TIM2_PSC = 8399;
    TIM2_ARR = 1999;
    TIM2_DIER |= TIM_DIER_UIE;
    NVIC_ISER0 |= (1 << 28);
    TIM2_CR1 |= TIM_CR1_CEN;
}

/* ============================================================
 * SOLUTION 4: TIM2 Interrupt Service Routine
 * ============================================================ */
void TIM2_IRQHandler(void) {
    TIM2_SR &= ~TIM_SR_UIF;

    if (!read_vitals()) {
        current_state = STATE_SENSOR_FAULT;
        alarm_on();
        new_data_flag = 1;
        sample_count++;
        return;
    }

    check_thresholds();
    sample_count++;
    new_data_flag = 1;
}

/* ============================================================
 * State name helper
 * ============================================================ */
const char* state_name(system_state_t s) {
    switch (s) {
        case STATE_NORMAL:       return "NORMAL";
        case STATE_WARNING:      return "WARNING";
        case STATE_CRITICAL:     return "CRITICAL";
        case STATE_SENSOR_FAULT: return "SENSOR_FAULT";
        default:                 return "UNKNOWN";
    }
}

/* ============================================================
 * Main Function
 * ============================================================ */
int main(void) {
    uart_init();
    gpio_init();
    i2c_init();

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  SafePulse v0.3 - I2C Sensor & Timing\r\n");
    uart_puts("  Patient Vital Signs Monitor\r\n");
    uart_puts("========================================\r\n");

    uint8_t dev_id = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_ID);
    if (dev_id != 0xA5) {
        uart_puts("[ERROR] Sensor not detected on I2C bus! Check wiring.\r\n");
        while (1);
    }
    uart_puts("[OK] Vital signs sensor detected (ID=0xA5) on I2C1\r\n");

    i2c_write_register(SENSOR_I2C_ADDR, SENSOR_REG_SCENARIO, 0x01);
    uart_puts("[INFO] Patient scenario started\r\n");
    uart_puts("\r\n");

    timer_init();

    uart_puts("[INFO] TIM2 sampling started at 200ms interval\r\n");
    uart_puts("\r\n");

    uint32_t last_printed = 0;

    while (1) {
        if (new_data_flag) {
            new_data_flag = 0;

            if (sample_count - last_printed >= 5) {
                last_printed = sample_count;

                uart_puts("[");
                uart_put_int(sample_count);
                uart_puts("] HR=");
                uart_put_int(latest_hr);
                uart_puts(" bpm, SpO2=");
                uart_put_int(latest_spo2);
                uart_puts("%, Temp=");
                uart_put_int(latest_temp_int);
                uart_puts(".");
                uart_put_int(latest_temp_frac);
                uart_puts(" C, State=");
                uart_puts(state_name(current_state));
                uart_puts("\r\n");
            }
        }
    }

    return 0;
}
