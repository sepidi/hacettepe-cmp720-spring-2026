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
#   0x05 - Scenario control (write 0x01 to start deterioration)
#
# Protocol:
#   Read register:  START + addr(W) + reg + RESTART + addr(R) + data + STOP
#   Write register: START + addr(W) + reg + data + STOP

# ----------------------------------------------------------------
# Get a reference to the DummyI2CSlave peripheral
# ----------------------------------------------------------------
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
# Deterioration scenario timeline  (tick -> SpO2 value)
# ----------------------------------------------------------------
SCENARIO_TIMELINE = [
    (0,   97), (10,  96), (20,  95), (25,  94), (30,  93),
    (35,  92), (40,  91), (45,  90), (50,  88), (55,  85),
    (60,  83), (70,  80), (80,  85), (90,  88), (100, 91),
    (110, 93), (120, 95), (130, 97), (140, 97),
]

# ----------------------------------------------------------------
# Sensor state
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
# Scenario helpers (linear interpolation along timeline)
# ----------------------------------------------------------------
def get_scenario_spo2(tick):
    if tick >= SCENARIO_TIMELINE[-1][0]:
        return SCENARIO_TIMELINE[-1][1]
    for i in range(len(SCENARIO_TIMELINE) - 1):
        t0, v0 = SCENARIO_TIMELINE[i]
        t1, v1 = SCENARIO_TIMELINE[i + 1]
        if t0 <= tick < t1:
            frac = float(tick - t0) / float(t1 - t0)
            return int(v0 + frac * (v1 - v0))
    return 97

def get_scenario_hr(tick):
    spo2 = get_scenario_spo2(tick)
    if spo2 >= 95:
        return 72
    if spo2 >= 90:
        return 72 + (95 - spo2) * 4
    return 72 + (95 - spo2) * 5

def get_scenario_temp(tick):
    spo2 = get_scenario_spo2(tick)
    if spo2 >= 95:
        return (36, 8)
    if spo2 >= 90:
        return (37, 2)
    return (37, 8)

# ----------------------------------------------------------------
# I2C event handler: firmware wrote byte(s) to the sensor
#
#   Single byte  [reg]       -> register selection (for upcoming read)
#   Two bytes    [reg, val]  -> register write (scenario control)
# ----------------------------------------------------------------
def on_data_received(data):
    raw = list(data)
    if len(raw) == 0:
        return
    reg = raw[0] & 0xFF
    sensor_state["selected_reg"] = reg

    # Handle a register write (address + value)
    if len(raw) >= 2 and reg == REG_SCENARIO:
        val = raw[1] & 0xFF
        if val == 0x01:
            sensor_state["scenario_active"] = True
            sensor_state["scenario_tick"]   = 0
        elif val == 0x00:
            sensor_state["scenario_active"] = False
            sensor_state["scenario_tick"]   = 0
            sensor_state["heart_rate"] = 72
            sensor_state["spo2"]       = 97
            sensor_state["temp_int"]   = 36
            sensor_state["temp_frac"]  = 8

# ----------------------------------------------------------------
# I2C event handler: firmware requested a read from the sensor
#
# Advance the scenario on each heart-rate read (register 0x00),
# which is always the first register read in a sampling cycle.
# Then enqueue the response byte for the selected register.
# ----------------------------------------------------------------
def on_read_requested(count):
    reg = sensor_state["selected_reg"]

    # Advance the deterioration scenario once per sampling cycle
    if reg == REG_HEART_RATE and sensor_state["scenario_active"]:
        sensor_state["scenario_tick"] += 1
        tick = sensor_state["scenario_tick"]
        sensor_state["spo2"]       = get_scenario_spo2(tick)
        sensor_state["heart_rate"] = get_scenario_hr(tick)
        ti, tf = get_scenario_temp(tick)
        sensor_state["temp_int"]  = ti
        sensor_state["temp_frac"] = tf

    # Look up value for the selected register
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

    # Enqueue the response byte(s)
    for _ in range(count):
        sensor.EnqueueResponseByte(val & 0xFF)

# ----------------------------------------------------------------
# Wire up the event handlers
# ----------------------------------------------------------------
sensor.DataReceived      += on_data_received
sensor.ReadRequested     += on_read_requested
