/*
 * SafePulse Lab 4 - Zephyr RTOS Migration (FINAL + INSTRUMENTATION)
 * Week 5: System Software, RTOS, Scheduling & Real-Time Properties
 *
 * Design:
 *   - Processing task (priority 2): lock-free alarm path, mutex only for log data
 *   - Sensor task (priority 5): timer-driven 200ms I2C polling
 *   - Logging task (priority 8): sole UART writer, k_poll on timer + state change
 *   - Alarm fires within milliseconds of detection (well under 80ms budget)
 *   - Logging at 1 Hz normal, 2 Hz warning/critical, immediate on state transitions
 *
 * Instrumentation (k_uptime_get, ms resolution — Renode-compatible):
 *   - max_sched:  worst-case scheduling delay since last log (sensor post → processing start)
 *   - max_alarm:  worst-case alarm latency since last log (sensor post → GPIO write)
 *
 * NOTE: Uses k_uptime_get() instead of k_cycle_get_32() because Renode
 * does not model the DWT cycle counter (CYCCNT) in sync with virtual time.
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
    int64_t  sensor_stamp_ms;   /* k_uptime_get() when sensor posted msg */
} vitals_msg_t;

/* ============================================================
 * Kernel Objects
 * ============================================================ */

/* Alarm state — written only by processing task, lock-free */
volatile system_state_t current_state = STATE_NORMAL;

/* Log snapshot — protected by log_mutex */
K_MUTEX_DEFINE(log_mutex);
vitals_msg_t log_vitals;
system_state_t log_state;
uint32_t log_sched_delay_ms;
uint32_t log_proc_time_ms;
uint32_t log_alarm_latency_ms;  /* 0 when no alarm change this window */

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

/* Message queue: sensor → processing */
K_MSGQ_DEFINE(vitals_msgq, sizeof(vitals_msg_t), 8, 4);

/* Stack sizes */
#define SENSOR_STACK_SIZE     1024
#define PROCESSING_STACK_SIZE 1024
#define LOGGING_STACK_SIZE    1024

/* Priority assignment:
 *   Processing (alarm) = highest — acts on data
 *   Sensor             = middle  — acquires data
 *   Logging            = lowest  — reports data
 */
#define PROCESSING_PRIORITY   2
#define SENSOR_PRIORITY       5
#define LOGGING_PRIORITY      8

/* ============================================================
 * I2C Driver (same bare-metal driver from Week 4)
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
 * Sensor Task (priority 5)
 * Timer-driven at 200ms. Reads I2C, posts to message queue.
 * Stamps k_uptime_get() before posting for latency measurement.
 * No mutex, no UART.
 * ============================================================ */
void sensor_task_fn(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    uint32_t sample_id = 0;

    k_timer_start(&sensor_timer, K_MSEC(200), K_MSEC(200));

    while (1) {
        k_timer_status_sync(&sensor_timer);
        vitals_msg_t msg;

        if (i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_ID) == 0xA5) {
            msg.heart_rate = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_HR);
            msg.spo2       = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_SPO2);
            msg.temp_int   = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_TEMP_INT);
            msg.temp_frac  = i2c_read_register(SENSOR_I2C_ADDR, SENSOR_REG_TEMP_FRC);
            msg.sensor_ok  = 1;
        } else {
            memset(&msg, 0, sizeof(msg));
            msg.sensor_ok = 0;
        }
        msg.sample_id = sample_id;

        /* Stamp right before posting — this is the reference for all delays */
        msg.sensor_stamp_ms = k_uptime_get();
        k_msgq_put(&vitals_msgq, &msg, K_NO_WAIT);
        sample_id++;
    }
}

/* ============================================================
 * Processing Task (priority 2) — Safety-Critical
 * Lock-free alarm path: threshold check → GPIO, no blocking.
 * Mutex only for updating log snapshot (after alarm fires).
 * Measures: scheduling delay, processing time, alarm latency.
 * No UART access.
 * ============================================================ */
void processing_task_fn(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    while (1) {
        vitals_msg_t msg;
        k_msgq_get(&vitals_msgq, &msg, K_FOREVER);

        /* Scheduling delay: sensor post → processing start (ms) */
        int64_t proc_start = k_uptime_get();
        uint32_t sched_delay_ms = (uint32_t)(proc_start - msg.sensor_stamp_ms);

        /* === ALARM PATH — lock-free, no UART, no blocking === */
        system_state_t new_state;

        if (!msg.sensor_ok) {
            new_state = STATE_SENSOR_FAULT;
        } else {
            uint16_t temp_tenths = (uint16_t)msg.temp_int * 10 + msg.temp_frac;

            if (msg.spo2 < SPO2_CRITICAL
                || temp_tenths > TEMP_CRITICAL
                || msg.heart_rate < HR_CRITICAL_LOW
                || msg.heart_rate > HR_CRITICAL_HIGH) {
                new_state = STATE_CRITICAL;
            }
            else if (msg.spo2 <= SPO2_WARNING
                     || temp_tenths >= TEMP_WARNING
                     || msg.heart_rate < HR_WARNING_LOW
                     || msg.heart_rate > HR_WARNING_HIGH) {
                new_state = STATE_WARNING;
            }
            else {
                new_state = STATE_NORMAL;
            }
        }

        /* GPIO alarm — immediate, zero dependencies */
        uint32_t alarm_latency_ms = 0;
        if (new_state == STATE_CRITICAL || new_state == STATE_SENSOR_FAULT) {
            alarm_on();
            alarm_latency_ms = (uint32_t)(k_uptime_get() - msg.sensor_stamp_ms);
        } else if (current_state == STATE_CRITICAL
                   || current_state == STATE_SENSOR_FAULT) {
            alarm_off();
            alarm_latency_ms = (uint32_t)(k_uptime_get() - msg.sensor_stamp_ms);
        }

        /* Processing time */
        int64_t proc_end = k_uptime_get();
        uint32_t proc_time_ms = (uint32_t)(proc_end - proc_start);

        system_state_t old_state = current_state;
        current_state = new_state;

        /* === LOG PATH — mutex protected, after alarm === */
        /* Track MAXIMUM delays since last log read, not just latest. */
        k_mutex_lock(&log_mutex, K_FOREVER);
        log_vitals = msg;
        log_state = new_state;
        if (sched_delay_ms > log_sched_delay_ms) {
            log_sched_delay_ms = sched_delay_ms;
        }
        if (proc_time_ms > log_proc_time_ms) {
            log_proc_time_ms = proc_time_ms;
        }
        if (alarm_latency_ms > log_alarm_latency_ms) {
            log_alarm_latency_ms = alarm_latency_ms;
        }
        k_mutex_unlock(&log_mutex);

        /* Notify logging task on state transitions */
        if (new_state != old_state) {
            k_sem_give(&state_change_sem);
        }
    }
}

/* ============================================================
 * Logging Task (priority 8) — Non-Critical
 * Sole UART writer. Wakes on timer OR state change via k_poll.
 * 1 Hz normal, 2 Hz warning/critical, immediate on transitions.
 * Prints timing diagnostics with each log line.
 * ============================================================ */
void logging_task_fn(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    system_state_t prev_state = STATE_NORMAL;

    struct k_poll_event events[2] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
                                 K_POLL_MODE_NOTIFY_ONLY,
                                 &state_change_sem),
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
                                 K_POLL_MODE_NOTIFY_ONLY,
                                 &log_timer_signal),
    };

    k_timer_start(&log_timer, K_MSEC(1000), K_MSEC(1000));

    while (1) {
        k_poll(events, 2, K_FOREVER);

        /* Reset whichever event(s) fired */
        if (events[0].state == K_POLL_STATE_SEM_AVAILABLE) {
            k_sem_take(&state_change_sem, K_NO_WAIT);
            events[0].state = K_POLL_STATE_NOT_READY;
        }
        if (events[1].state == K_POLL_STATE_SIGNALED) {
            k_poll_signal_reset(&log_timer_signal);
            events[1].state = K_POLL_STATE_NOT_READY;
        }

        /* Snapshot under mutex — then reset max counters */
        k_mutex_lock(&log_mutex, K_FOREVER);
        vitals_msg_t snap = log_vitals;
        system_state_t snap_state = log_state;
        uint32_t sched_ms = log_sched_delay_ms;
        uint32_t proc_ms = log_proc_time_ms;
        uint32_t alarm_ms = log_alarm_latency_ms;
        log_sched_delay_ms = 0;
        log_proc_time_ms = 0;
        log_alarm_latency_ms = 0;
        k_mutex_unlock(&log_mutex);

        /* Report state transitions */
        if (snap_state != prev_state) {
            if (snap_state == STATE_CRITICAL) {
                printk("[ALARM] CRITICAL: SpO2=%d%%, HR=%d, alarm active\n",
                       snap.spo2, snap.heart_rate);
            } else if (snap_state == STATE_SENSOR_FAULT) {
                printk("[ALARM] SENSOR FAULT, alarm active\n");
            } else if (prev_state == STATE_CRITICAL
                       || prev_state == STATE_SENSOR_FAULT) {
                printk("[ALARM] Cleared — now %s\n", state_name(snap_state));
            }
            prev_state = snap_state;
        }

        /* Regular vitals log with timestamp and diagnostics */
        int64_t now = k_uptime_get();
        printk("[%lld ms][%d] HR=%d bpm, SpO2=%d%%, Temp=%d.%d C, State=%s"
               " [max_sched=%u ms, max_alarm=%u ms]\n",
               now, snap.sample_id, snap.heart_rate, snap.spo2,
               snap.temp_int, snap.temp_frac, state_name(snap_state),
               sched_ms, alarm_ms);

        /* Adjust logging rate based on state */
        if (snap_state == STATE_WARNING || snap_state == STATE_CRITICAL
            || snap_state == STATE_SENSOR_FAULT) {
            k_timer_start(&log_timer, K_MSEC(500), K_MSEC(500));
        } else {
            k_timer_start(&log_timer, K_MSEC(1000), K_MSEC(1000));
        }
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
    printk("[INFO] Alarm path: lock-free, <80ms budget\n");
    printk("[INFO] Logging: 1Hz normal, 2Hz warning/critical\n");
    printk("[INFO] Diagnostics: max_sched=worst scheduling delay, "
           "max_alarm=worst post-to-GPIO\n\n");

    return 0;
}