#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Приклад оновленої конфігурації пінів
#define PIN_TRIGGER      22  // Червоний LED
#define PIN_STATUS_LED   23  // Зелений LED (Замість GPIO 26)
#define PIN_EMERGENCY_SW 18  // Фізична кнопка (Вхід з Pull-Up)
#define PIN_ADC_SENSE    34  // Вхід АЦП (Залишаємо GPIO 34, бо 18/19/22/23 не мають АЦП в режимі Wi-Fi)

// --- Часові інтервали (в мілісекундах) ---
constexpr unsigned long ARMING_TIMEOUT_MS = 5 * 60 * 1000UL; // 5 хвилин таймер безпеки
constexpr unsigned long FIRING_DURATION_MS = 3000UL;         // 3 секунди тривалість імітації
constexpr unsigned long SENSOR_CHECK_INTERVAL_MS = 100UL;    // Опитування датчиків кожні 100 мс

// --- Межі напруги живлення (у вольтах, для Fail-Safe) ---
constexpr float VOLTAGE_MIN_THRESHOLD = 3.0f;
constexpr float VOLTAGE_MAX_THRESHOLD = 3.6f;

#endif // CONFIG_H