/*
 * SafePulse Lab 4 - Zephyr RTOS Migration (Student Skeleton)
 * Week 5: System Software, RTOS, Scheduling & Real-Time Properties
 *
 * Architecture: Three cooperating tasks
 *   sensor_task     (Medium priority)  - Timer-driven 200ms I2C polling
 *   processing_task (HIGHEST priority) - Lock-free alarm path, mutex for log data
 *   logging_task    (Low priority)     - Event-driven UART, k_poll on timer + state change
 *
 * Data flow: sensor_task --[msgq]--> processing_task --[sem/mutex]--> logging_task
 *
 * Key design principles:
 *   - Alarm path is lock-free: threshold check -> GPIO write, no blocking
 *   - Mutex protects ONLY the log snapshot (written after alarm fires)
 *   - Logging wakes on timer OR state-change semaphore via k_poll
 *   - Cycle-accurate instrumentation for latency measurement
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

/* ============================================================
 * I2C1 Registers (RM0090 Section 27.6)
 * Same bare-metal driver from Week 4
 * ============================================================ */
#define GPIOB_BASE      0x40020400
#define GPIOB_MODER     (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER    (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_OSPEEDR   (*(volatile uint32_t *)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR     (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRL      (*(volatile uint32_t *)(GPIOB_BASE + 0x20))

#define I2C1_BASE       0x40005400
#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_OAR1       (*(volatile uint32_t *)(I2C1_BASE + 0x08))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20))

#define I2C_CR1_PE      (1 << 0)
#define I2C_CR1_START   (1 << 8)
#define I2C_CR1_STOP    (1 << 9)
#define I2C_CR1_ACK     (1 << 10)
#define I2C_CR1_SWRST   (1 << 15)
#define I2C_SR1_SB      (1 << 0)
#define I2C_SR1_ADDR    (1 << 1)
#define I2C_SR1_BTF     (1 << 2)
#define I2C_SR1_RXNE    (1 << 6)
#define I2C_SR1_TXE     (1 << 7)

/* ============================================================
 * Vital Signs Sensor - I2C Interface (same as Week 4)
 * ============================================================ */
#define SENSOR_I2C_ADDR     0x55

#define SENSOR_REG_HR       0x00
#define SENSOR_REG_SPO2     0x01
#define SENSOR_REG_TEMP_INT 0x02
#define SENSOR_REG_TEMP_FRC 0x03
#define SENSOR_REG_ID       0x04
#define SENSOR_REG_SCENARIO 0x05

/* ============================================================
 * Alarm GPIO (PD14 - Red LED)
 * ============================================================ */
#define GPIOD_BASE      0x40020C00
#define GPIOD_MODER     (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR       (*(volatile uint32_t *)(GPIOD_BASE + 0x14))
#define RCC_AHB1ENR     (*(volatile uint32_t *)(0x40023800 + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(0x40023800 + 0x40))
#define LED_RED_PIN     14

/* ============================================================
 * Thresholds (from Week 2 specification)
 * ============================================================ */
#define SPO2_WARNING        94
#define SPO2_CRITICAL       90
#define TEMP_WARNING        380
#define TEMP_CRITICAL       400
#define HR_WARNING_LOW      50
#define HR_WARNING_HIGH     120
#define HR_CRITICAL_LOW     40
#define HR_CRITICAL_HIGH    150

/* ============================================================
 * System State
 * ============================================================ */
typedef enum {
    STATE_NORMAL,
    STATE_WARNING,
    STATE_CRITICAL,
    STATE_SENSOR_FAULT
} system_state_t;

typedef struct {
    uint8_t  heart_rate;
    uint8_t  spo2;
    uint8_t  temp_int;
    uint8_t  temp_frac;
    uint32_t sample_id;
    uint8_t  sensor_ok;
    uint32_t sensor_cycles;     /* cycle count when sensor posted msg */
} vitals_msg_t;

/* ============================================================
 * Kernel Objects (provided)
 * ============================================================ */

/* Alarm state — written only by processing task, lock-free */
volatile system_state_t current_state = STATE_NORMAL;

/* Log snapshot — protected by log_mutex */
K_MUTEX_DEFINE(log_mutex);
vitals_msg_t log_vitals;
system_state_t log_state;
uint32_t log_sched_delay_us;
uint32_t log_proc_time_us;
uint32_t log_alarm_latency_us;  /* 0 when no alarm change this sample */

/* State change notification for logging task */
K_SEM_DEFINE(state_change_sem, 0, 1);

/* Log timer with poll signal — callback raises signal in ISR context.
 * Requires CONFIG_POLL=y in prj.conf */
struct k_poll_signal log_timer_signal = K_POLL_SIGNAL_INITIALIZER(log_timer_signal);

void log_timer_cb(struct k_timer *timer) {
    k_poll_signal_raise(&log_timer_signal, 0);
}

K_TIMER_DEFINE(log_timer, log_timer_cb, NULL);
K_TIMER_DEFINE(sensor_timer, NULL, NULL);

/* Message queue: sensor -> processing */
K_MSGQ_DEFINE(vitals_msgq, sizeof(vitals_msg_t), 8, 4);

/* Stack sizes */
#define SENSOR_STACK_SIZE     1024
#define PROCESSING_STACK_SIZE 1024
#define LOGGING_STACK_SIZE    1024

/* ============================================================
 * TODO 1: Define task priorities
 *
 * Zephyr: LOWER number = HIGHER priority
 * Preemptive priorities (>= 0) allow higher-priority threads
 * to preempt lower-priority ones at any point.
 *
 * Assign so that:
 *   - processing_task has HIGHEST priority (safety-critical alarm)
 *   - sensor_task has MEDIUM priority (periodic data acquisition)
 *   - logging_task has LOWEST priority (non-critical display)
 *
 * Leave gaps between numbers for future tasks.
 * ============================================================ */
#define PROCESSING_PRIORITY   5   /* TODO: Set appropriate priority */
#define SENSOR_PRIORITY       5   /* TODO: Set appropriate priority */
#define LOGGING_PRIORITY      5   /* TODO: Set appropriate priority */

/* ============================================================
 * I2C Driver (provided - same as Week 4)
 * ============================================================ */
static void i2c_init(void) {
    RCC_AHB1ENR |= (1 << 1);
    RCC_APB1ENR |= (1 << 21);
    GPIOB_MODER  &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_MODER  |=  ((2 << (6 * 2)) | (2 << (7 * 2)));
    GPIOB_OTYPER |=  ((1 << 6) | (1 << 7));
    GPIOB_OSPEEDR |= ((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_PUPDR &= ~((3 << (6 * 2)) | (3 << (7 * 2)));
    GPIOB_PUPDR |=  ((1 << (6 * 2)) | (1 << (7 * 2)));
    GPIOB_AFRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB_AFRL |=  ((4U   << 24) | (4U   << 28));
    I2C1_CR1 |= I2C_CR1_SWRST;
    I2C1_CR1 &= ~I2C_CR1_SWRST;
    I2C1_CR2 = 42;
    I2C1_CCR = 210;
    I2C1_TRISE = 43;
    I2C1_CR1 |= I2C_CR1_PE;
}

static uint8_t i2c_read_register(uint8_t dev_addr, uint8_t reg_addr) {
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));
    I2C1_DR = (uint32_t)(dev_addr << 1);
    while (!(I2C1_SR1 & I2C_SR1_ADDR));
    (void)I2C1_SR2;
    I2C1_DR = reg_addr;
    while (!(I2C1_SR1 & I2C_SR1_BTF));
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

static void i2c_write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t value) {
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
 * GPIO Helpers
 * ============================================================ */
static void alarm_gpio_init(void) {
    RCC_AHB1ENR |= (1 << 3);
    GPIOD_MODER &= ~(3 << (LED_RED_PIN * 2));
    GPIOD_MODER |=  (1 << (LED_RED_PIN * 2));
    GPIOD_ODR &= ~(1 << LED_RED_PIN);
}

static void alarm_on(void)  { GPIOD_ODR |=  (1 << LED_RED_PIN); }
static void alarm_off(void) { GPIOD_ODR &= ~(1 << LED_RED_PIN); }

static const char *state_name(system_state_t s) {
    switch (s) {
        case STATE_NORMAL:       return "NORMAL";
        case STATE_WARNING:      return "WARNING";
        case STATE_CRITICAL:     return "CRITICAL";
        case STATE_SENSOR_FAULT: return "SENSOR_FAULT";
        default:                 return "UNKNOWN";
    }
}

/* ============================================================
 * TODO 2: Implement sensor_task
 *
 * This task is timer-driven at 200ms using k_timer_status_sync().
 * Unlike k_msleep(), the timer approach keeps precise cadence
 * regardless of how long the I2C reads take.
 *
 * Steps (inside the loop):
 *   a) Wait for sensor_timer to expire: k_timer_status_sync(&sensor_timer)
 *   b) Read sensor ID via i2c_read_register() to check presence
 *   c) If OK (ID == 0xA5), read HR/SpO2/Temp into vitals_msg_t, set sensor_ok=1
 *      If not, memset msg to 0, set sensor_ok=0
 *   d) Set msg.sample_id
 *   e) Stamp cycle count: msg.sensor_cycles = k_cycle_get_32()
 *   f) Send via k_msgq_put(&vitals_msgq, &msg, K_NO_WAIT)
 *   g) Increment sample_id
 *
 * IMPORTANT: Start the timer BEFORE the loop with:
 *   k_timer_start(&sensor_timer, K_MSEC(200), K_MSEC(200));
 *
 * Why K_NO_WAIT on put? If the queue is full (processing task slow),
 * we drop the message rather than blocking—missing one 200ms sample
 * is safer than stalling the entire sampling loop.
 * ============================================================ */
void sensor_task_fn(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    uint32_t sample_id = 0;

    /* TODO: Start the sensor timer here */

    while (1) {
        /* TODO: Your code here */

        sample_id++;
    }
}

/* ============================================================
 * TODO 3: Implement processing_task
 *
 * This is the safety-critical core. The alarm path MUST be
 * lock-free: threshold check -> GPIO write, with NO mutex,
 * NO printk, NO blocking calls.
 *
 * Steps (inside the loop):
 *   a) Block on k_msgq_get(&vitals_msgq, &msg, K_FOREVER)
 *   b) Record scheduling delay:
 *        uint32_t proc_start = k_cycle_get_32();
 *        uint32_t sched_delay_us = k_cyc_to_us_floor32(proc_start - msg.sensor_cycles);
 *
 *   --- ALARM PATH (lock-free, no UART, no blocking) ---
 *   c) Determine new_state from sensor data:
 *      - sensor_ok == 0 -> STATE_SENSOR_FAULT
 *      - CRITICAL: SpO2<90 OR temp>40.0 OR HR<40 OR HR>150
 *      - WARNING:  SpO2<=94 OR temp>=38.0 OR HR<50 OR HR>120
 *      - Otherwise: STATE_NORMAL
 *   d) Fire alarm GPIO immediately:
 *      - CRITICAL or SENSOR_FAULT: alarm_on()
 *      - Transitioning OUT of CRITICAL/FAULT: alarm_off()
 *      - Measure alarm_latency_us after GPIO write
 *   e) Record proc_time_us, update current_state
 *
 *   --- LOG PATH (mutex protected, AFTER alarm fires) ---
 *   f) Lock log_mutex, write log_vitals/log_state/timing fields, unlock
 *   g) If state changed, notify logging: k_sem_give(&state_change_sem)
 *
 * WHY this separation matters: The alarm fires within microseconds
 * of data arrival. The mutex (for log data) is acquired AFTER the
 * GPIO write, so even if the logging task holds the mutex, the alarm
 * is never delayed.
 * ============================================================ */
void processing_task_fn(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    while (1) {
        /* TODO: Your code here */
    }
}

/* ============================================================
 * TODO 4: Implement logging_task
 *
 * This task is the SOLE UART writer. It wakes on EITHER:
 *   - A periodic timer (1Hz normal, 2Hz in warning/critical), OR
 *   - A state-change semaphore (for immediate transition reports)
 * using k_poll() to wait on both events simultaneously.
 *
 * Steps:
 *   a) Track prev_state (initialized to STATE_NORMAL)
 *   b) Set up k_poll_event array (provided below as a hint)
 *   c) Start the log timer: k_timer_start(&log_timer, K_MSEC(1000), K_MSEC(1000))
 *   d) Inside the loop:
 *      1. k_poll(events, 2, K_FOREVER)
 *      2. Reset whichever event(s) fired:
 *         - Semaphore: k_sem_take(&state_change_sem, K_NO_WAIT), reset state
 *         - Signal: k_poll_signal_reset(&log_timer_signal), reset state
 *      3. Lock log_mutex, snapshot all log_* variables, unlock
 *      4. If state changed from prev_state, print transition message
 *         (e.g., "[ALARM] CRITICAL: SpO2=...") and update prev_state
 *      5. Print regular vitals line with timing diagnostics:
 *         [timestamp][sample_id] HR=... [sched=X us, proc=Y us, alarm=Z us]
 *      6. Adjust timer rate: 500ms for warning/critical, 1000ms for normal
 *
 * HINT — k_poll event setup:
 *   struct k_poll_event events[2] = {
 *       K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
 *                                K_POLL_MODE_NOTIFY_ONLY,
 *                                &state_change_sem),
 *       K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
 *                                K_POLL_MODE_NOTIFY_ONLY,
 *                                &log_timer_signal),
 *   };
 * ============================================================ */
void logging_task_fn(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    system_state_t prev_state = STATE_NORMAL;

    /* TODO: Set up k_poll events and start log timer */

    while (1) {
        /* TODO: Your code here */
    }
}

/* ============================================================
 * Static Thread Definitions
 * ============================================================ */
K_THREAD_DEFINE(sensor_tid, SENSOR_STACK_SIZE,
                sensor_task_fn, NULL, NULL, NULL,
                SENSOR_PRIORITY, 0, 0);
K_THREAD_DEFINE(processing_tid, PROCESSING_STACK_SIZE,
                processing_task_fn, NULL, NULL, NULL,
                PROCESSING_PRIORITY, 0, 0);
K_THREAD_DEFINE(logging_tid, LOGGING_STACK_SIZE,
                logging_task_fn, NULL, NULL, NULL,
                LOGGING_PRIORITY, 0, 0);

/* ============================================================
 * Main
 * ============================================================ */
int main(void) {
    printk("\n========================================\n");
    printk("  SafePulse v0.5 - Zephyr RTOS\n");
    printk("  Patient Vital Signs Monitor\n");
    printk("  + Timing Instrumentation\n");
    printk("========================================\n\n");

    alarm_gpio_init();
    i2c_init();

    uint8_t dev_id = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_ID);
    if (dev_id != 0xA5) {
        printk("[ERROR] Sensor not detected on I2C bus!\n");
        return -1;
    }
    printk("[OK] Sensor detected (ID=0xA5) on I2C1\n");

    i2c_write_register(SENSOR_I2C_ADDR, SENSOR_REG_SCENARIO, 0x01);
    printk("[INFO] Patient scenario started\n");
    printk("[INFO] Priorities: processing=%d, sensor=%d, logging=%d\n",
           PROCESSING_PRIORITY, SENSOR_PRIORITY, LOGGING_PRIORITY);
    printk("[INFO] Alarm path: lock-free, <80us budget\n");
    printk("[INFO] Logging: 1Hz normal, 2Hz warning/critical\n");
    printk("[INFO] Diagnostics: sched=scheduling delay, "
           "proc=eval time, alarm=post-to-GPIO\n\n");

    return 0;
}