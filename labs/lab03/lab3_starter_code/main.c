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
#define TIM_CR1_CEN     (1 << 0)   /* Counter enable */
#define TIM_DIER_UIE    (1 << 0)   /* Update interrupt enable */
#define TIM_SR_UIF      (1 << 0)   /* Update interrupt flag */

/* I2C CR1 bits */
#define I2C_CR1_PE      (1 << 0)    /* Peripheral enable */
#define I2C_CR1_START   (1 << 8)    /* Generate START condition */
#define I2C_CR1_STOP    (1 << 9)    /* Generate STOP condition */
#define I2C_CR1_ACK     (1 << 10)   /* Acknowledge enable */
#define I2C_CR1_SWRST   (1 << 15)   /* Software reset */

/* I2C SR1 bits */
#define I2C_SR1_SB      (1 << 0)    /* Start bit generated */
#define I2C_SR1_ADDR    (1 << 1)    /* Address sent / matched */
#define I2C_SR1_BTF     (1 << 2)    /* Byte transfer finished */
#define I2C_SR1_RXNE    (1 << 6)    /* Data register not empty (read) */
#define I2C_SR1_TXE     (1 << 7)    /* Data register empty (write) */

/* GPIO / LED */
#define LED_GREEN_PIN   12  /* PD12 = Green LED */
#define LED_RED_PIN     14  /* PD14 = Red LED (ALARM) */

/* ============================================================
 * Vital Signs Sensor --- I2C Interface
 *
 * The sensor is connected to I2C1 (PB6=SCL, PB7=SDA) at
 * 7-bit address 0x55.  Each internal register is selected
 * by writing a 1-byte register address, then reading 1 byte.
 *
 * Register Map:
 *   0x00 - Heart Rate (BPM), uint8, read-only
 *   0x01 - SpO2 (%), uint8, read-only
 *   0x02 - Temperature integer part (C), uint8, read-only
 *   0x03 - Temperature fractional part (tenths), uint8, read-only
 *   0x04 - Device ID (should return 0xA5), read-only
 *   0x05 - Scenario control: write 0x01 to start deterioration
 * ============================================================ */
#define SENSOR_I2C_ADDR     0x55

#define SENSOR_REG_HR       0x00
#define SENSOR_REG_SPO2     0x01
#define SENSOR_REG_TEMP_INT 0x02
#define SENSOR_REG_TEMP_FRC 0x03
#define SENSOR_REG_ID       0x04
#define SENSOR_REG_SCENARIO 0x05

/* ============================================================
 * Critical Thresholds (from Week 2 requirements)
 * ============================================================ */
#define SPO2_WARNING    94
#define SPO2_CRITICAL   90
#define TEMP_WARNING    380   /* 38.0 C in tenths */
#define TEMP_CRITICAL   400   /* 40.0 C in tenths */
#define HR_WARNING_LOW    50   /* Bradycardia warning */
#define HR_WARNING_HIGH   120  /* Tachycardia warning */
#define HR_CRITICAL_LOW   40   /* Severe bradycardia */
#define HR_CRITICAL_HIGH  150  /* Severe tachycardia */

/* ============================================================
 * System State
 * ============================================================ */
typedef enum {
    STATE_NORMAL,
    STATE_WARNING,
    STATE_CRITICAL,
    STATE_SENSOR_FAULT
} system_state_t;

/* Global state (volatile because modified in ISR) */
volatile system_state_t current_state = STATE_NORMAL;
volatile uint8_t  latest_hr = 0;
volatile uint8_t  latest_spo2 = 0;
volatile uint8_t  latest_temp_int = 0;
volatile uint8_t  latest_temp_frac = 0;
volatile uint32_t sample_count = 0;
volatile uint8_t  new_data_flag = 0;  /* Set in ISR, cleared in main */

/* ============================================================
 * UART Functions (from Week 3 - provided complete)
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
 * GPIO Functions (from Week 3 - provided, extended for alarm)
 *
 * STM32F4-Discovery LED pin assignments:
 *   PD12 = Green LED (extra output pin if needed)
 *   PD14 = Red LED   (alarm output - safety critical)
 *
 * GPIOD_MODER: 2 bits per pin select the pin mode
 *   00 = Input, 01 = Output, 10 = Alt Function, 11 = Analog
 *   Pin 12 -> bits [25:24],  Pin 14 -> bits [29:28]
 *
 * GPIOD_ODR: 1 bit per pin controls output level
 *   1 = HIGH (LED on),  0 = LOW (LED off)
 * ============================================================ */
void gpio_init(void) {
    RCC_AHB1ENR |= (1 << 3);  /* Enable GPIOD clock */
    /* PD12 (green LED) and PD14 (red LED / alarm) as output */
    GPIOD_MODER &= ~((3 << (LED_GREEN_PIN * 2)) | (3 << (LED_RED_PIN * 2)));
    GPIOD_MODER |=  ((1 << (LED_GREEN_PIN * 2)) | (1 << (LED_RED_PIN * 2)));
    /* Start with alarm OFF */
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
 *
 * These functions implement the low-level I2C protocol for
 * communicating with the vital signs sensor.  The same code
 * works on real STM32F4 hardware and in Renode simulation.
 *
 * Reference: RM0090, Section 27.3.3 (I2C master mode)
 * ============================================================ */

/**
 * i2c_init - Configure I2C1 in standard mode (100 kHz)
 *
 * Pin assignment (STM32F4-Discovery compatible):
 *   PB6 = I2C1_SCL  (Alternate Function 4)
 *   PB7 = I2C1_SDA  (Alternate Function 4)
 *
 * Clock assumptions:
 *   APB1 = 42 MHz (used by I2C peripheral)
 */
void i2c_init(void) {
    /* Enable peripheral clocks */
    RCC_AHB1ENR |= (1 << 1);   /* GPIOB clock */
    RCC_APB1ENR |= (1 << 21);  /* I2C1 clock  */

    /* PB6 (SCL) and PB7 (SDA): AF mode, open-drain, high-speed, pull-up */
    GPIOB_MODER  &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_MODER  |=  ((2 << (6 * 2)) | (2 << (7 * 2)));
    GPIOB_OTYPER |=  ((1 << 6) | (1 << 7));
    GPIOB_OSPEEDR |= ((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_PUPDR &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_PUPDR |=  ((1 << (6 * 2)) | (1 << (7 * 2)));

    /* Select AF4 (I2C1) for PB6 and PB7 */
    GPIOB_AFRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB_AFRL |=  ((4U   << 24) | (4U   << 28));

    /* Software reset then configure */
    I2C1_CR1 |= I2C_CR1_SWRST;
    I2C1_CR1 &= ~I2C_CR1_SWRST;

    I2C1_CR2 = 42;       /* APB1 clock = 42 MHz */
    I2C1_CCR = 210;      /* 100 kHz: 42MHz / (2 * 100kHz) */
    I2C1_TRISE = 43;     /* (1000ns * 42MHz) + 1 */

    I2C1_CR1 |= I2C_CR1_PE;  /* Enable I2C peripheral */
}

/**
 * i2c_read_register - Read one byte from a sensor register via I2C
 *
 * Performs: START -> addr(W) -> reg -> RESTART -> addr(R) -> data -> STOP
 *
 * @param dev_addr  7-bit I2C device address
 * @param reg_addr  Register address to read
 * @return          The byte value read
 */
uint8_t i2c_read_register(uint8_t dev_addr, uint8_t reg_addr) {
    /* Write phase: send register address */
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));

    I2C1_DR = (uint32_t)(dev_addr << 1);
    while (!(I2C1_SR1 & I2C_SR1_ADDR));
    (void)I2C1_SR2;  /* Clear ADDR by reading SR2 */

    I2C1_DR = reg_addr;
    while (!(I2C1_SR1 & I2C_SR1_BTF));

    /* Read phase: repeated start, read one byte */
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));

    I2C1_DR = (uint32_t)(dev_addr << 1) | 1;
    while (!(I2C1_SR1 & I2C_SR1_ADDR));

    /* Single-byte reception: disable ACK before clearing ADDR */
    I2C1_CR1 &= ~I2C_CR1_ACK;
    (void)I2C1_SR2;
    I2C1_CR1 |= I2C_CR1_STOP;

    while (!(I2C1_SR1 & I2C_SR1_RXNE));
    return (uint8_t)I2C1_DR;
}

/**
 * i2c_write_register - Write one byte to a sensor register via I2C
 *
 * Performs: START -> addr(W) -> reg -> data -> STOP
 */
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
 * TODO 1: Read Vital Signs from Sensor (via I2C)
 *
 * Use the provided i2c_read_register() function to read all
 * sensor data and store in the corresponding global variables.
 *
 * Steps:
 *   IMPORTANT: Always verify the sensor is present BEFORE
 *   reading data.  If the sensor is disconnected or faulty,
 *   i2c_read_register() will return garbage -- not 0xA5.
 *   
 *   a) Read SENSOR_REG_ID and verify it equals 0xA5
 *      - If not, return 0 (sensor fault)
 *   b) Read SENSOR_REG_HR -> latest_hr
 *   c) Read SENSOR_REG_SPO2 -> latest_spo2
 *   d) Read SENSOR_REG_TEMP_INT -> latest_temp_int
 *   e) Read SENSOR_REG_TEMP_FRC -> latest_temp_frac
 *   f) Return 1 (success)
 *
 * Hint: Each read is one call:
 *   value = i2c_read_register(SENSOR_I2C_ADDR, register);
 * ============================================================ */
uint8_t read_vitals(void) {
    // TODO: Your code here
    return 0;
}

/* ============================================================
 * TODO 2: Check Thresholds and Update System State
 *
 * Available global variables:
 *   - latest_hr         : most recent heart rate (uint8)
 *   - latest_spo2       : most recent SpO2 reading (uint8)
 *   - latest_temp_int   : temperature integer part (uint8)
 *   - latest_temp_frac  : temperature fractional part in tenths (uint8)
 *   - current_state     : system state (system_state_t enum)
 *
 * Available functions:
 *   - alarm_on()        : activates red LED (PD14)
 *   - alarm_off()       : deactivates red LED (PD14)
 *
 * Tip: Combine temperature into tenths for easy comparison:
 *   uint16_t temp_tenths = (uint16_t)latest_temp_int * 10
 *                          + latest_temp_frac;
 *   Example: 38.5 C -> temp_tenths = 385
 *
 * Rules (from Week 2 specification) - check in priority order:
 *
 *   1. CRITICAL (highest priority, check first):
 *      SpO2 < 90  OR  temp > 40.0 C  OR  HR < 40  OR  HR > 150
 *      -> Set current_state = STATE_CRITICAL
 *      -> Call alarm_on() IMMEDIATELY (safety-critical!)
 *
 *   2. WARNING:
 *      SpO2 <= 94  OR  temp >= 38.0 C  OR  HR < 50  OR  HR > 120
 *      -> Set current_state = STATE_WARNING
 *      -> If previous state was CRITICAL, call alarm_off()
 *
 *   3. NORMAL (all values in safe range):
 *      -> Set current_state = STATE_NORMAL
 *      -> If previous state was CRITICAL, call alarm_off()
 * ============================================================ */
void check_thresholds(void) {
    // TODO: Your code here
}

/* ============================================================
 * TODO 3: Configure TIM2 for 200ms periodic interrupt
 *
 * TIM2 runs at 84 MHz (APB1 timer clock).
 * We want an interrupt every 200ms.
 *
 * How STM32 timers work:
 *
 *   The timer has a counter that increments at each "tick".
 *   The tick rate is determined by the prescaler:
 *
 *     tick_rate = timer_clock / (PSC + 1)
 *
 *   The counter counts from 0 up to the auto-reload value.
 *   When it reaches ARR, it resets to 0 and fires an interrupt:
 *
 *     interrupt_period = (ARR + 1) / tick_rate
 *
 *   Both PSC and ARR use "value - 1" because the hardware
 *   counts inclusively from 0.  For example, PSC = 2 means
 *   the hardware divides by 3 (counts 0, 1, 2 then ticks).
 *
 * Timer math for 200ms:
 *
 *   Step 1 - Choose prescaler to get a convenient tick rate:
 *     PSC + 1 = 8400
 *     tick_rate = 84,000,000 / 8400 = 10,000 Hz  (one tick = 0.1 ms)
 *
 *   Step 2 - Choose auto-reload for desired period:
 *     ARR + 1 = 2000
 *     period = 2000 / 10,000 = 0.2 s = 200 ms  ✓
 *
 *   Verification:
 *     84,000,000 / 8400 / 2000 = 5 Hz = 200 ms period  ✓
 *
 * Available registers:
 *   - RCC_APB1ENR  : peripheral clock enable (bit 0 = TIM2)
 *   - TIM2_PSC     : prescaler register
 *   - TIM2_ARR     : auto-reload register
 *   - TIM2_DIER    : interrupt enable (bit 0 = UIE)
 *   - TIM2_CR1     : control register (bit 0 = CEN, counter enable)
 *   - NVIC_ISER0   : interrupt set-enable (bit 28 = TIM2)
 *
 * Steps:
 *   a) Enable TIM2 clock: set bit 0 of RCC_APB1ENR
 *   b) Set prescaler: TIM2_PSC = 8399      (divides 84MHz to 10kHz)
 *   c) Set auto-reload: TIM2_ARR = 1999    (2000 ticks at 10kHz = 200ms)
 *   d) Enable update interrupt: TIM2_DIER |= TIM_DIER_UIE
 *   e) Enable TIM2 in NVIC: NVIC_ISER0 |= (1 << 28)
 *   f) Start counting: TIM2_CR1 |= TIM_CR1_CEN
 *
 * IMPORTANT: Enable the clock (step a) FIRST! The other
 * registers are inaccessible until the peripheral clock is on.
 * ============================================================ */
void timer_init(void) {
    // TODO: Your code here
}

/* ============================================================
 * TODO 4: TIM2 Interrupt Service Routine
 *
 * This function is called by hardware every 200ms.
 * It MUST be named TIM2_IRQHandler to match the vector table
 * entry in startup.s.
 * 
 * Why clear the flag FIRST?
 *
 *   When TIM2 overflows, it sets UIF in TIM2_SR.  The NVIC
 *   sees this and calls our ISR.  But the NVIC does NOT clear
 *   UIF for us — only our code can do that.
 *
 *   When the ISR returns, the NVIC re-checks UIF.  If it's
 *   still set, the NVIC immediately calls the ISR again,
 *   trapping the CPU in an infinite loop where the main loop
 *   never runs.  Clearing UIF first prevents this.
 *
 * Steps:
 *   a) Clear the update interrupt flag (UIF in TIM2_SR)
 *      IMPORTANT: Clear FIRST to avoid re-entry!
 *   b) Call read_vitals()
 *      - If it returns 0: sensor fault, set state accordingly
 *   c) Call check_thresholds() to update state and alarm
 *   d) Increment sample_count
 *   e) Set new_data_flag = 1 so main loop knows to print
 * ============================================================ */
void TIM2_IRQHandler(void) {
    // TODO: Your code here
}

/* ============================================================
 * State name helper (provided)
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
    /* Initialize all peripherals */
    uart_init();
    gpio_init();
    i2c_init();

    /* Startup banner */
    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  SafePulse v0.3 - I2C Sensor & Timing\r\n");
    uart_puts("  Patient Vital Signs Monitor\r\n");
    uart_puts("========================================\r\n");

    /* Verify sensor is present on the I2C bus */
    uint8_t dev_id = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_ID);
    if (dev_id != 0xA5) {
        uart_puts("[ERROR] Sensor not detected on I2C bus! Check wiring.\r\n");
        while (1);  /* Halt */
    }
    uart_puts("[OK] Vital signs sensor detected (ID=0xA5) on I2C1\r\n");

    /* Start the deterioration scenario so students can observe
     * the alarm triggering during the lab session */
    i2c_write_register(SENSOR_I2C_ADDR, SENSOR_REG_SCENARIO, 0x01);
    uart_puts("[INFO] Patient scenario started\r\n");
    uart_puts("\r\n");

    /* Initialize timer LAST (starts generating interrupts) */
    timer_init();

    uart_puts("[INFO] TIM2 sampling started at 200ms interval\r\n");
    uart_puts("\r\n");

    /* Main loop: print data when ISR sets the flag
     * Note: The main loop no longer controls sampling timing!
     * The timer ISR handles all real-time-critical work.
     * This loop only handles non-critical display updates. 
     *
     * DESIGN NOTE: The Week 2 specification requires different
     * display refresh rates depending on system state:
     *   - NORMAL:   1 Hz (once per second)
     *   - WARNING:  2 Hz (twice per second)
     *   - CRITICAL: 2 Hz (twice per second)
     *
     * This week's implementation uses a fixed 1 Hz rate for
     * simplicity (print every 5th sample at 200ms = 1 second).
     * State-dependent refresh rates require tracking elapsed
     * time per state, which is better handled with RTOS timers.
     * This will be implemented in Week 5 (Zephyr RTOS).
     *
     * The safety-critical alarm logic is NOT affected by this
     * simplification -- alarm_on() fires immediately in the ISR
     * regardless of the display refresh rate.
     */
    uint32_t last_printed = 0;

    
    while (1) {
        if (new_data_flag) {
            new_data_flag = 0;

            /* Only print at ~1Hz (every 5th sample at 200ms) */
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

        /* LED heartbeat toggle in main loop (non-critical) */
        /* The real work is done in the ISR */
    }

    return 0;
}