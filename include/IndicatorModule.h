#ifndef INDICATOR_MODULE_H
#define INDICATOR_MODULE_H

#include <Arduino.h>
#include "types.h"

class IndicatorModule {
public:
    IndicatorModule(uint8_t pin) : ledPin(pin), lastToggleMs(0), ledState(false) {}

    void init() {
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);
    }

    // Головний оновлювач індикації в loop()
    void update(SystemState currentState) {
        unsigned long currentMs = millis();

        switch (currentState) {
            case SystemState::INIT:
                // Швидке блимання: 100 мс ON / 100 мс OFF
                if (currentMs - lastToggleMs >= 100) {
                    lastToggleMs = currentMs;
                    ledState = !ledState;
                    digitalWrite(ledPin, ledState ? HIGH : LOW);
                }
                break;

            case SystemState::SAFE:
                // Рідкісний спалах: 50 мс ON / 1950 мс OFF
                if (ledState && (currentMs - lastToggleMs >= 50)) {
                    ledState = false;
                    lastToggleMs = currentMs;
                    digitalWrite(ledPin, LOW);
                } else if (!ledState && (currentMs - lastToggleMs >= 1950)) {
                    ledState = true;
                    lastToggleMs = currentMs;
                    digitalWrite(ledPin, HIGH);
                }
                break;

            case SystemState::ARMING:
                // Часте блимання: 500 мс ON / 500 мс OFF
                if (currentMs - lastToggleMs >= 500) {
                    lastToggleMs = currentMs;
                    ledState = !ledState;
                    digitalWrite(ledPin, ledState ? HIGH : LOW);
                }
                break;

            case SystemState::ARMED:
                // Постійне світіння (або імпульси 100 мс ON / 100 мс OFF)
                digitalWrite(ledPin, HIGH);
                ledState = true;
                break;

            case SystemState::FIRING:
                // Безперервний біп / світіння під час підриву
                digitalWrite(ledPin, HIGH);
                ledState = true;
                break;

            case SystemState::FAIL_SAFE:
                // Аварійний стробоскоп: 100 мс ON / 100 мс OFF
                if (currentMs - lastToggleMs >= 100) {
                    lastToggleMs = currentMs;
                    ledState = !ledState;
                    digitalWrite(ledPin, ledState ? HIGH : LOW);
                }
                break;
        }
    }

private:
    uint8_t ledPin;
    unsigned long lastToggleMs;
    bool ledState;
};

#endif // INDICATOR_MODULE_H