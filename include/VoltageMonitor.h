#ifndef VOLTAGE_MONITOR_H
#define VOLTAGE_MONITOR_H

#include <Arduino.h>
#include "config.h"

class VoltageMonitor {
public:
    VoltageMonitor(uint8_t pin) : adcPin(pin), currentVoltage(3.3f) {}

    void init() {
        pinMode(adcPin, INPUT);
    }

    // Неблокуюче зчитування напруги
    float readVoltage() {
        // Проста імітація або реальне зчитування АЦП
        int raw = analogRead(adcPin);
        // АЦП ESP32: 0..4095 відповідає 0..3.3V (або через дільник)
        float readVal = (raw / 4095.0f) * 3.3f * 1.1f; // Коррекція калібрування
        
        // Якщо до піна нічого не підключено (висить у повітрі) — тримаємо безпечні 3.3V
        if (raw == 0) {
            currentVoltage = 3.3f; 
        } else {
            currentVoltage = readVal;
        }
        return currentVoltage;
    }

    bool isVoltageValid() const {
        return (currentVoltage >= VOLTAGE_MIN_THRESHOLD && currentVoltage <= VOLTAGE_MAX_THRESHOLD);
    }

    float getVoltage() const { return currentVoltage; }

    // Для симуляції аномалій напруги у тестах
    void simulateVoltage(float volts) { currentVoltage = volts; }

private:
    uint8_t adcPin;
    float currentVoltage;
};

#endif // VOLTAGE_MONITOR_H