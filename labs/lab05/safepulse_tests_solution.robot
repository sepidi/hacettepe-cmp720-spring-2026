*** Comments ***
# SafePulse Lab 5 - Automated Testing & Validation (SOLUTION)
# Week 6: Evaluation, Validation & Testing for Embedded Systems
#
# Run with:
#   renode-test safepulse_tests_solution.robot

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

Check Alarm Pin
    [Documentation]    Read GPIOD_ODR bit 14 (PD14 red LED = alarm output).
    ...                0 = OFF (no alarm), 1 = ON (alarm active)
    [Arguments]        ${expected}
    ${odr}=    Execute Command    sysbus ReadDoubleWord 0x40020C14
    ${odr}=    Strip String    ${odr}
    ${bit}=    Evaluate    (int('${odr}', 0) >> 14) & 1
    Should Be Equal As Integers    ${bit}    ${expected}
    ...    Alarm GPIO PD14: expected=${expected} but got=${bit} (ODR=${odr})

*** Test Cases ***
# ===========================================================
# TEST 1: System Boot and Sensor Detection
# Requirement: FR-07 (startup self-test)
# ===========================================================
System Should Boot Successfully
    [Documentation]     Verify startup banner and sensor detection
    ${tester}=          Create Terminal Tester    ${UART}
    Set Sensor Values   72    97    36    8
    Start Emulation

    Wait For Line On Uart       SafePulse v0.5          testerId=${tester}
    Wait For Line On Uart       Sensor detected         testerId=${tester}


# ===========================================================
# TEST 2: Normal Operation - Periodic Sampling
# Requirement: FR-01 (200ms sampling), FR-03 (1Hz logging)
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
# Requirement: FR-02 (alarm within 100ms when SpO2 < 90%)
# Requirement: NFR-01 (alarm latency <= 100ms)
#
# Tests the LOCK-FREE GPIO alarm path (processing task),
# NOT the UART logging path. After boot, the alarm assertion
# uses only deterministic virtual time + direct GPIO reads —
# no UART dependency. We advance 500ms (enough for one 200ms
# sensor cycle + processing) and check PD14 directly.
# ===========================================================
Should Trigger Alarm On Critical SpO2
    [Documentation]     Verify GPIO alarm fires when SpO2 drops below 90%.
    ...                 Tests the lock-free GPIO path with NO UART dependency.
    ${tester}=          Create Terminal Tester    ${UART}    timeout=15
    Set Sensor Values   72    97    36    8
    Start Emulation

    # Boot sync — only UART dependency, just confirms system is alive
    Wait For Line On Uart       Sensor detected         testerId=${tester}

    # Pre-condition: alarm GPIO must be LOW in normal state
    Execute Command    pause
    Check Alarm Pin    0

    # Inject critical SpO2 and advance exactly 500ms of virtual time
    # (enough for one 200ms sensor cycle + processing, with margin)
    Set Sensor Values   72    85    36    8
    Execute Command    emulation RunFor "0:0:0.5"

    # Assertion: GPIO alarm pin must now be HIGH
    Check Alarm Pin    1


# ===========================================================
# TEST 4: Warning State Detection
# Requirement: FR-05 (warning when SpO2 90-94%)
# ===========================================================
Should Enter Warning State On Low SpO2
    [Documentation]     Verify WARNING state when SpO2 is 90-94%
    ${tester}=          Create Terminal Tester    ${UART}    timeout=15
    Set Sensor Values   72    93    36    8
    Start Emulation

    Wait For Line On Uart       WARNING             testerId=${tester}      timeout=10

# ===========================================================
# TEST 5: Alarm Recovery
# Requirement: FR-04 (alarm deactivation on recovery)
# ===========================================================
Should Clear Alarm On Recovery
    [Documentation]     Verify alarm clears when vitals return to normal
    ${tester}=          Create Terminal Tester    ${UART}    timeout=20
    Set Sensor Values   120    85    37    8
    Start Emulation

    Wait For Line On Uart       CRITICAL            testerId=${tester}      timeout=10

    Set Sensor Values   72    97    36    8

    Wait For Line On Uart       NORMAL              testerId=${tester}      timeout=15

# ===========================================================
# TEST 6: Edge Case - SpO2 at Exact Threshold
# Validates: Boundary behavior at SpO2 = 90%
# ===========================================================
Should Handle SpO2 At Threshold Boundary
    [Documentation]     Test behavior at exact SpO2 = 90% boundary
    ${tester}=          Create Terminal Tester    ${UART}    timeout=15
    Set Sensor Values   72    90    36    8
    Start Emulation

    Wait For Line On Uart       WARNING             testerId=${tester}      timeout=10

    Set Sensor Values   72    89    36    8
    Wait For Line On Uart       CRITICAL            testerId=${tester}      timeout=10

# ===========================================================
# TEST 7: High Temperature Critical Alert
# Validates: Temperature threshold checking
# ===========================================================
Should Trigger Alarm On Critical Temperature
    [Documentation]     Verify alarm fires when temperature exceeds 40.0 C
    ${tester}=          Create Terminal Tester    ${UART}    timeout=15
    Set Sensor Values   72    97    40    5
    Start Emulation

    Wait For Line On Uart       CRITICAL            testerId=${tester}      timeout=10

# ===========================================================
# TEST 8: Full Deterioration Scenario
# Validates: End-to-end from normal -> critical -> recovery
# ===========================================================
Should Complete Full Deterioration Scenario
    [Documentation]     Run built-in scenario and verify state transitions
    ${tester}=          Create Terminal Tester    ${UART}    timeout=120
    Enable Scenario
    Start Emulation

    Wait For Line On Uart       NORMAL              testerId=${tester}      timeout=15
    Wait For Line On Uart       CRITICAL            testerId=${tester}      timeout=45
    Wait For Line On Uart       WARNING             testerId=${tester}      timeout=45
    Wait For Line On Uart       NORMAL              testerId=${tester}      timeout=60
