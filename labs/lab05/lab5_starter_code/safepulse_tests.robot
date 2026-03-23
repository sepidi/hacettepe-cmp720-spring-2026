*** Comments ***
# SafePulse Lab 5 - Automated Testing & Validation
# Week 6: Evaluation, Validation & Testing for Embedded Systems
#
# The sensor model (vitals_sensor.py) provides an I2C interface
# for the firmware and a sensor_state dictionary for test control.
# Use Set Sensor Values to inject exact readings — they won't be
# overwritten by the firmware.
#
# Run with:
#   renode-test safepulse_tests.robot

*** Settings ***
Resource        ${RENODEKEYWORDS} 
Library         String
Library         Collections
Suite Setup     Setup
Suite Teardown  Teardown
Test Setup      SafePulse Test Setup
Test Teardown   Test Teardown

*** Variables ***
${UART}         sysbus.usart2

*** Keywords ***
SafePulse Test Setup
    Reset Emulation
    Execute Command             mach create "SafePulse"
    Execute Command             machine LoadPlatformDescription @platforms/boards/stm32f4_discovery.repl
    Execute Command             machine LoadPlatformDescriptionFromString "vitalsSensor: Mocks.DummyI2CSlave @ i2c1 0x55"
    Execute Command             sysbus.cpu PerformanceInMips 168
    Execute Command             sysbus LoadELF @${CURDIR}/build/zephyr/zephyr.elf
    Execute Command             include @${CURDIR}/vitals_sensor.py
    Execute Command             logLevel -1 sysbus.usart2

Set Sensor Values
    [Documentation]             Inject specific values into the sensor model.
    [Arguments]                 ${hr}    ${spo2}    ${temp_int}    ${temp_frac}
    Execute Command             python "sensor_state['heart_rate'] = ${hr}"
    Execute Command             python "sensor_state['spo2'] = ${spo2}"
    Execute Command             python "sensor_state['temp_int'] = ${temp_int}"
    Execute Command             python "sensor_state['temp_frac'] = ${temp_frac}"

Enable Scenario
    [Documentation]             Enable the built-in deterioration scenario
    Execute Command             python "sensor_state['scenario_active'] = True"
    Execute Command             python "sensor_state['scenario_tick'] = 0"


*** Test Cases ***

# ===========================================================
# TEST 1: System Boot and Sensor Detection (GIVEN)
# Requirement: FR-07 (startup self-test)
# ===========================================================
System Should Boot Successfully
    [Documentation]     Verify startup banner and sensor detection
    ${tester}=          Create Terminal Tester    ${UART}
    Set Sensor Values   72    97    36    8
    Start Emulation

    Wait For Line On Uart       SafePulse v0.5      testerId=${tester}
    Wait For Line On Uart       Sensor detected     testerId=${tester}


# ===========================================================
# TEST 2: Normal Operation - Periodic Sampling
# Validates: FR-01 (200ms sampling) and FR-03 (1Hz logging).
# Note that FR-01 is indirectly verified: if sensor data appears and
# updates in the logs, sampling must be occurring.
#
# The Zephyr firmware logs vitals in this format:
#   [NNNN ms][sample_id] HR=72 bpm, SpO2=97%, Temp=36.8 C, State=NORMAL ...
#
# The [NNNN ms] prefix is the Zephyr k_uptime timestamp.
# In NORMAL state the log timer fires at 1 Hz, so two
# consecutive vitals lines should have timestamps ~1000ms apart.
#
# Strategy:
#   1. Use defaultPauseEmulation=true so virtual time freezes
#      the instant a UART line matches — this gives exact timestamps.
#   2. Use a regex with capture groups to extract both the
#      timestamp and the vitals from each log line.
#   3. Compute the delta between two consecutive timestamps
#      and assert it falls within the expected range.
#
# TODO:
#   a) Create a terminal tester with defaultPauseEmulation=true
#      and a generous timeout (e.g. 30s).
#   b) Set normal sensor values and start emulation.
#   c) Use Wait For Line On Uart with the provided VITALS_REGEX
#      to capture the first vitals log line.
#   d) Resume emulation (Start Emulation) and capture a second line.
#   e) Extract timestamps from capture group 1 using the
#      helper below, compute the delta, and assert ~1000ms ± 200ms.
#
# Useful keywords:
#   Wait For Line On Uart    <regex>    treatAsRegex=true
#   Start Emulation
#   Evaluate                 <python expression>
#   Should Be True           <condition>
# ===========================================================

Should Report Vitals Periodically
    [Documentation]     Verify periodic UART output with vital signs.
    ...                 The Zephyr firmware prints with format:
    ...                 [NNNN ms][sample_id] HR=... State=...
    ...                 In NORMAL state the log timer runs at 1 Hz,
    ...                 so consecutive timestamps should differ by ~1000ms.

    # Use defaultPauseEmulation so emulation pauses the instant a match occurs
    ${tester}=          Create Terminal Tester    ${UART}    timeout=30    defaultPauseEmulation=true
    Set Sensor Values   72    97    36    8
    Start Emulation

    # Wait for first vitals log line — group 1 = timestamp, groups 2-5 = vitals
    ${line1}=           Wait For Line On Uart    \\[(\\d+)\\s*ms\\].*?HR=(\\d+).*?SpO2=(\\d+).*?Temp=(\\d+\\.?\\d*).*?State=(\\w+)  treatAsRegex=true    testerId=${tester}

    # Resume emulation and wait for the next vitals log line
    Start Emulation
    ${line2}=           Wait For Line On Uart    \\[(\\d+)\\s*ms\\].*?HR=(\\d+).*?SpO2=(\\d+).*?Temp=(\\d+\\.?\\d*).*?State=(\\w+)  treatAsRegex=true    testerId=${tester}

    # Extract timestamps directly from capture group 1
    ${has_groups}=      Evaluate    "groups" in $line1
    ${groups_key}=      Set Variable If    ${has_groups}    groups    Groups
    ${ts1}=             Set Variable    ${line1["${groups_key}"][0]}
    ${ts2}=             Set Variable    ${line2["${groups_key}"][0]}

    ${delta_ms}=        Evaluate    int('${ts2}') - int('${ts1}')
    # In NORMAL state the log timer fires at 1 Hz, so expect ~1000ms ± 200ms
    Should Be True    ${delta_ms} >= 800 and ${delta_ms} <= 1200
    ...    FR-03 violated: log period was ${delta_ms}ms, expected ~1000ms


# ===========================================================
# TEST 3: Critical SpO2 Alarm
# Validates: FR-02, NFR-01
#
# TODO: 
#       a) Start with normal values, wait for a log line to 
#          confirm the system has processed the new sensor 
#          values.
#       b) Pre-condition: alarm GPIO must be LOW in normal state
#       c) Inject critical SpO2
#       d) advance 500ms of virtual time (enough for one 
#          200ms sensor cycle + processing, with a margin)
#       e) Assert: the GPIO alarm pin must now be HIGH
# ===========================================================
Should Trigger Alarm On Critical SpO2
    [Documentation]     Verify alarm fires when SpO2 drops below 90%
    # TODO: Write this test

    Fail    Not implemented yet

# ===========================================================
# TEST 4: Warning State Detection
# Validates: FR-05
#
# TODO: Set SpO2 to a warning-level value (90-94) and verify
#       "WARNING" appears on UART.
# ===========================================================
Should Enter Warning State On Low SpO2
    [Documentation]     Verify WARNING state when SpO2 is 90-94%
    # TODO: Write this test

    Fail    Not implemented yet


# ===========================================================
# TEST 5: Alarm Recovery (CHALLENGE)
# Validates: FR-04
#
# TODO: Start with critical values, wait for CRITICAL,
#       then inject normal values and wait for NORMAL.
# ===========================================================
Should Clear Alarm On Recovery
    [Documentation]     Verify alarm clears when vitals return to normal
    # TODO: Write this test

    Fail    Not implemented yet


# ===========================================================
# TEST 6: Edge Case - SpO2 at Exact Threshold
# Validates: Boundary behavior at SpO2 = 90%
#
# TODO: SpO2 = 90 should be WARNING (code uses < 90 for CRITICAL).
#       SpO2 = 89 should trigger CRITICAL. Test both.
# ===========================================================
Should Handle SpO2 At Threshold Boundary
    [Documentation]     Test behavior at exact SpO2 = 90% boundary
    # TODO: Write this test

    Fail    Not implemented yet
