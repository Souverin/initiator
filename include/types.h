#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// Стани фази ініціалізації (POST)
enum class InitState {
    START,
    CHECK_FS,
    CHECK_ESP_ID,
    CHECK_BUTTON,
    CHECK_VOLTAGE,
    CHECK_SENSORS,
    COMPLETE,
    FAILED
};

// Основні робочі стани системи
enum class SystemState {
    INIT,
    SAFE,
    ARMING,
    ARMED,
    FIRING,
    FAIL_SAFE
};

// Причини виходу в аварійний режим
enum class FailSafeReason {
    NONE,
    FS_ERROR,
    LOW_VOLTAGE,
    HIGH_VOLTAGE,
    BUTTON_STUCK,
    SENSOR_FAULT,
    INVALID_COMMAND,
    MANUAL_ABORT
};

// Текстове представлення станів для логування та веб-інтерфейсу
inline const char* systemStateToString(SystemState state) {
    switch (state) {
        case SystemState::INIT:      return "INIT";
        case SystemState::SAFE:      return "SAFE";
        case SystemState::ARMING:    return "ARMING";
        case SystemState::ARMED:     return "ARMED";
        case SystemState::FIRING:    return "FIRING";
        case SystemState::FAIL_SAFE: return "FAIL_SAFE";
        default:                     return "UNKNOWN";
    }
}

inline const char* failSafeReasonToString(FailSafeReason reason) {
    switch (reason) {
        case FailSafeReason::NONE:            return "NONE";
        case FailSafeReason::FS_ERROR:        return "FS_ERROR";
        case FailSafeReason::LOW_VOLTAGE:     return "LOW_VOLTAGE";
        case FailSafeReason::HIGH_VOLTAGE:    return "HIGH_VOLTAGE";
        case FailSafeReason::BUTTON_STUCK:    return "BUTTON_STUCK";
        case FailSafeReason::SENSOR_FAULT:    return "SENSOR_FAULT";
        case FailSafeReason::INVALID_COMMAND: return "INVALID_COMMAND";
        case FailSafeReason::MANUAL_ABORT:    return "MANUAL_ABORT";
        default:                              return "UNKNOWN";
    }
}

#endif // TYPES_H