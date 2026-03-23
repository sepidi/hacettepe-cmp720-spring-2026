# -*- coding: utf-8 -*-
# SafePulse Vital Signs Sensor - Renode I2C Slave (IronPython 2)
# ==============================================================
# Hooks into Mocks.DummyI2CSlave on I2C1 at 7-bit address 0x55.
#
# I2C Register Map:
#   0x00 - Heart Rate (BPM), uint8, read-only
#   0x01 - SpO2 (%), uint8, read-only
#   0x02 - Temperature integer part (C), uint8, read-only
#   0x03 - Temperature fractional part (tenths), uint8, read-only
#   0x04 - Device ID (read-only, returns 0xA5)
#   0x05 - Scenario control (accepted on I2C but ignored;
#          scenario is controlled via sensor_state from Robot Framework)
#
# Protocol:
#   Read register:  START + addr(W) + reg + RESTART + addr(R) + data + STOP
#   Write register: START + addr(W) + reg + data + STOP
#
# Testing:
#   Sensor values are injected from Robot Framework by modifying
#   the sensor_state dictionary directly. The deterioration
#   scenario is activated only via Robot Framework (Enable Scenario
#   keyword), never by I2C writes from the firmware. This ensures
#   test-injected values are never overwritten unexpectedly.

sensor = monitor.Machine["sysbus.i2c1.vitalsSensor"]

# ----------------------------------------------------------------
# Register addresses
# ----------------------------------------------------------------
REG_HEART_RATE = 0x00
REG_SPO2       = 0x01
REG_TEMP_INT   = 0x02
REG_TEMP_FRAC  = 0x03
REG_DEVICE_ID  = 0x04
REG_SCENARIO   = 0x05

# ----------------------------------------------------------------
# Deterioration scenario stages
# Each stage lasts STAGE_LENGTH sampling ticks (~200ms each).
#   Normal -> Warning -> Critical -> Warning -> Normal
# ----------------------------------------------------------------
STAGE_LENGTH = 30

SCENARIO_STAGES = [
    # (spo2, hr, temp_int, temp_frac)
    (97, 72, 36, 8),   # Normal
    (93, 80, 37, 2),   # Warning
    (80, 97, 37, 8),   # Critical
    (93, 80, 37, 2),   # Warning (recovery)
    (97, 72, 36, 8),   # Normal  (recovered)
]

# ----------------------------------------------------------------
# Sensor state — modify from Robot Framework to inject values
# or control the scenario.
# ----------------------------------------------------------------
sensor_state = {
    "heart_rate": 72,
    "spo2": 97,
    "temp_int": 36,
    "temp_frac": 8,
    "scenario_active": False,
    "scenario_tick": 0,
    "selected_reg": 0x00,
}

# ----------------------------------------------------------------
# Scenario helper
# ----------------------------------------------------------------
def advance_scenario():
    sensor_state["scenario_tick"] += 1
    tick = sensor_state["scenario_tick"]
    stage = min(tick // STAGE_LENGTH, len(SCENARIO_STAGES) - 1)
    spo2, hr, ti, tf = SCENARIO_STAGES[stage]
    sensor_state["spo2"]       = spo2
    sensor_state["heart_rate"] = hr
    sensor_state["temp_int"]   = ti
    sensor_state["temp_frac"]  = tf

# ----------------------------------------------------------------
# I2C write handler: firmware selects a register for reading.
# Writes to REG_SCENARIO are accepted (no I2C error) but the
# scenario is NOT activated — only Robot Framework can do that.
# ----------------------------------------------------------------
def on_data_received(data):
    raw = list(data)
    if len(raw) == 0:
        return
    sensor_state["selected_reg"] = raw[0] & 0xFF

# ----------------------------------------------------------------
# I2C read handler: return the selected register value.
# Advances the scenario (if active) on each heart-rate read.
# ----------------------------------------------------------------
def on_read_requested(count):
    reg = sensor_state["selected_reg"]

    if reg == REG_HEART_RATE and sensor_state["scenario_active"]:
        advance_scenario()

    val = 0x00
    if reg == REG_HEART_RATE:
        val = sensor_state["heart_rate"]
    elif reg == REG_SPO2:
        val = sensor_state["spo2"]
    elif reg == REG_TEMP_INT:
        val = sensor_state["temp_int"]
    elif reg == REG_TEMP_FRAC:
        val = sensor_state["temp_frac"]
    elif reg == REG_DEVICE_ID:
        val = 0xA5

    for _ in range(count):
        sensor.EnqueueResponseByte(val & 0xFF)

# ----------------------------------------------------------------
# Wire up event handlers
# ----------------------------------------------------------------
sensor.DataReceived  += on_data_received
sensor.ReadRequested += on_read_requested