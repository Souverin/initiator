#include "FSM.h"

StateMachine::StateMachine()
    : currentState(SystemState::INIT),
      initState(InitState::START),
      failReason(FailSafeReason::NONE),
      armingStartMs(0),
      firingStartMs(0),
      voltageMonitor(PIN_ADC_SENSE),
      physicalButton(PIN_EMERGENCY_SW),
      dummySensor("TiltSensor_Sim"),
      indicator(PIN_STATUS_LED),
      pwmTrigger(PIN_TRIGGER) {}

void StateMachine::initHardware() {
    voltageMonitor.init();
    physicalButton.init();
    dummySensor.init();
    indicator.init();
    pwmTrigger.init();
}

void StateMachine::update() {
    physicalButton.update();
    voltageMonitor.readVoltage();

    // Фізична кнопка діє як Emergency Stop
    if (physicalButton.wasPressed()) {
        Serial.println("[BTN] Physical Emergency Stop pressed!");
        if (currentState == SystemState::ARMING || currentState == SystemState::ARMED) {
            transitionTo(SystemState::SAFE);
        }
    }

    // Перевірка безпеки
    if (currentState != SystemState::INIT && currentState != SystemState::FAIL_SAFE) {
        checkContinuousSafety();
    }

    // Оновлення FSM станів
    switch (currentState) {
        case SystemState::INIT:      handleInit(); break;
        case SystemState::SAFE:      handleSafe(); break;
        case SystemState::ARMING:    handleArming(); break;
        case SystemState::ARMED:     handleArmed(); break;
        case SystemState::FIRING:    handleFiring(); break;
        case SystemState::FAIL_SAFE: handleFailSafe(); break;
    }

    // Оновлення світлозвукової індикації
    indicator.update(currentState);
}

void StateMachine::checkContinuousSafety() {
    // Зчитуємо напругу (метод враховує прапорець симуляції isSimulated)
    float v = voltageMonitor.readVoltage();

    if (!voltageMonitor.isVoltageValid()) {
        if (v < VOLTAGE_MIN_THRESHOLD) {
            triggerFailSafe(FailSafeReason::LOW_VOLTAGE);
        } else {
            triggerFailSafe(FailSafeReason::HIGH_VOLTAGE);
        }
        return;
    }

    if (!dummySensor.readData()) {
        triggerFailSafe(FailSafeReason::SENSOR_FAULT);
        return;
    }
}

bool StateMachine::canTransitionTo(SystemState newState) const {
    if (newState == SystemState::FAIL_SAFE) return true;

    switch (currentState) {
        case SystemState::INIT:
            return (newState == SystemState::SAFE);
        case SystemState::SAFE:
            return (newState == SystemState::ARMING);
        case SystemState::ARMING:
            return (newState == SystemState::SAFE || newState == SystemState::ARMED);
        case SystemState::ARMED:
            return (newState == SystemState::SAFE || newState == SystemState::FIRING);
        case SystemState::FIRING:
            return (newState == SystemState::SAFE);
        case SystemState::FAIL_SAFE:
            return false;
        default:
            return false;
    }
}

void StateMachine::transitionTo(SystemState newState) {
    if (!canTransitionTo(newState)) {
        triggerFailSafe(FailSafeReason::INVALID_COMMAND);
        return;
    }

    if (newState == SystemState::ARMING) {
        armingStartMs = millis();
    } else if (newState == SystemState::FIRING) {
        firingStartMs = millis();
    } else if (newState == SystemState::SAFE) {
        pwmTrigger.disarmPin();
    }

    currentState = newState;
    Serial.print("[FSM] State changed to: ");
    Serial.println(systemStateToString(currentState));
}

void StateMachine::triggerFailSafe(FailSafeReason reason) {
    pwmTrigger.disarmPin();
    failReason = reason;
    currentState = SystemState::FAIL_SAFE;

    Serial.print("[FAIL-SAFE CRITICAL] Reason: ");
    Serial.println(failSafeReasonToString(failReason));
}

bool StateMachine::commandStart() {
    if (currentState == SystemState::SAFE) {
        transitionTo(SystemState::ARMING);
        return true;
    }
    return false;
}

bool StateMachine::commandStop() {
    if (currentState == SystemState::ARMING || currentState == SystemState::ARMED) {
        resetFaults(); // Скидаємо інжектовані помилки при знятті
        transitionTo(SystemState::SAFE);
        return true;
    }
    return false;
}

bool StateMachine::commandFire() {
    if (currentState == SystemState::ARMED) {
        transitionTo(SystemState::FIRING);
        return true;
    }
    return false;
}

void StateMachine::handleInit() {
    switch (initState) {
        case InitState::START:
            Serial.println("[POST] Starting hardware self-tests...");
            initState = InitState::CHECK_FS;
            break;

        case InitState::CHECK_FS:
            Serial.println("[POST] Checking File System... OK");
            initState = InitState::CHECK_ESP_ID;
            break;

        case InitState::CHECK_ESP_ID:
            Serial.print("[POST] ESP Chip ID: ");
            Serial.println((uint32_t)ESP.getEfuseMac(), HEX);
            initState = InitState::CHECK_BUTTON;
            break;

        case InitState::CHECK_BUTTON:
            if (physicalButton.isRawPressed()) {
                Serial.println("[POST ERROR] Button is stuck on boot!");
                initState = InitState::FAILED;
                failReason = FailSafeReason::BUTTON_STUCK;
            } else {
                Serial.println("[POST] Button check... OK");
                initState = InitState::CHECK_VOLTAGE;
            }
            break;

        case InitState::CHECK_VOLTAGE:
            voltageMonitor.readVoltage();
            if (!voltageMonitor.isVoltageValid()) {
                Serial.println("[POST ERROR] Voltage out of bounds!");
                initState = InitState::FAILED;
                failReason = FailSafeReason::LOW_VOLTAGE;
            } else {
                Serial.print("[POST] Voltage check: ");
                Serial.print(voltageMonitor.getVoltage());
                Serial.println("V... OK");
                initState = InitState::CHECK_SENSORS;
            }
            break;

        case InitState::CHECK_SENSORS:
            if (!dummySensor.init() || !dummySensor.readData()) {
                Serial.println("[POST ERROR] Sensor initialization failed!");
                initState = InitState::FAILED;
                failReason = FailSafeReason::SENSOR_FAULT;
            } else {
                Serial.println("[POST] Abstract Sensors check... OK");
                initState = InitState::COMPLETE;
            }
            break;

        case InitState::COMPLETE:
            Serial.println("[POST] All Self-Tests PASSED successfully.");
            transitionTo(SystemState::SAFE);
            break;

        case InitState::FAILED:
            triggerFailSafe(failReason);
            break;
    }
}

void StateMachine::handleSafe() {
    pwmTrigger.disarmPin();
}

void StateMachine::handleArming() {
    pwmTrigger.disarmPin();
    if (millis() - armingStartMs >= ARMING_TIMEOUT_MS) {
        transitionTo(SystemState::ARMED);
    }
}

void StateMachine::handleArmed() {
    pwmTrigger.disarmPin();
}

void StateMachine::handleFiring() {
    pwmTrigger.setTriggerState(currentState, true);
    if (millis() - firingStartMs >= FIRING_DURATION_MS) {
        pwmTrigger.disarmPin();
        transitionTo(SystemState::SAFE);
    }
}

void StateMachine::handleFailSafe() {
    pwmTrigger.disarmPin();
}

unsigned long StateMachine::getArmingRemainingTimeMs() const {
    if (currentState != SystemState::ARMING) return 0;
    unsigned long elapsed = millis() - armingStartMs;
    if (elapsed >= ARMING_TIMEOUT_MS) return 0;
    return ARMING_TIMEOUT_MS - elapsed;
}