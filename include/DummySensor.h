#ifndef DUMMY_SENSOR_H
#define DUMMY_SENSOR_H

#include "ISensor.h"
#include <Arduino.h>

class DummySensor : public ISensor {
public:
    DummySensor(const char* name) : sensorName(name), healthy(true), simulatedError(false) {}

    bool init() override {
        healthy = true;
        return true;
    }

    bool readData() override {
        // Якщо увімкнено штучну помилку — датчик стає нездоровим
        if (simulatedError) {
            healthy = false;
        }
        return healthy;
    }

    bool isHealthy() const override { return healthy; }
    const char* getName() const override { return sensorName; }

    // Метод для тестування Fail-Safe логіки
    void injectError(bool enable) { simulatedError = enable; }

private:
    const char* sensorName;
    bool healthy;
    bool simulatedError;
};

#endif // DUMMY_SENSOR_H