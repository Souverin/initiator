#ifndef PWM_TRIGGER_H
#define PWM_TRIGGER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class PWMTrigger {
public:
    PWMTrigger(uint8_t pin) : triggerPin(pin), isTriggerActive(false) {}

    void init() {
        pinMode(triggerPin, OUTPUT);
        disarmPin(); // Гарантоване знеструмлення при старті
    }

    // Дозволено активувати ТІЛЬКИ в стані FIRING
    void setTriggerState(SystemState currentState, bool active) {
        if (currentState == SystemState::FIRING && active) {
            armAndFire();
        } else {
            disarmPin();
        }
    }

    void disarmPin() {
        digitalWrite(triggerPin, LOW);
        isTriggerActive = false;
    }

    bool isActive() const { return isTriggerActive; }

private:
    uint8_t triggerPin;
    bool isTriggerActive;

    void armAndFire() {
        // Симуляція генерації PWM підриву (або просто HIGH сигнал)
        digitalWrite(triggerPin, HIGH);
        isTriggerActive = true;
    }
};

#endif // PWM_TRIGGER_H