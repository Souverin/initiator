#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

class ButtonHandler {
public:
    ButtonHandler(uint8_t pin, unsigned long debounceMs = 50)
        : buttonPin(pin), debounceInterval(debounceMs), lastState(HIGH), 
          currentState(HIGH), lastDebounceTime(0), pressedEvent(false) {}

    void init() {
        pinMode(buttonPin, INPUT_PULLUP);
        bool rawInitial = digitalRead(buttonPin);
        Serial.printf("[HW LOG] Button initialized on GPIO %d (INPUT_PULLUP). Initial raw state: %d (%s)\n", 
                      buttonPin, rawInitial, rawInitial == LOW ? "PRESSED" : "RELEASED");
        lastState = rawInitial;
        currentState = rawInitial;
    }

    // Головний оновлювач стану в loop()
    void update() {
        bool reading = digitalRead(buttonPin);

        // 1. Лог будь-якої миттєвої зміни сирого стану
        if (reading != lastState) {
            Serial.printf("[HW LOG] Raw state edge on GPIO %d: %d -> %d at %lu ms\n", 
                          buttonPin, lastState, reading, millis());
            lastDebounceTime = millis();
        }

        // 2. Логіка debounce
        if ((millis() - lastDebounceTime) > debounceInterval) {
            if (reading != currentState) {
                currentState = reading;
                Serial.printf("[HW LOG] Debounced state settled on GPIO %d: %s\n", 
                              buttonPin, currentState == LOW ? "PRESSED (LOW)" : "RELEASED (HIGH)");
                
                // Натискання при Pull-Up — це перехід у LOW
                if (currentState == LOW) {
                    pressedEvent = true;
                    Serial.printf("[HW LOG] Event triggered: BUTTON_PRESSED on GPIO %d\n", buttonPin);
                }
            }
        }
        lastState = reading;
    }

    // Перевірка чи була кнопка натиснута (одноразовий прапорець)
    bool wasPressed() {
        if (pressedEvent) {
            pressedEvent = false;
            return true;
        }
        return false;
    }

    // Поточний фізичний стан
    bool isRawPressed() const {
        return digitalRead(buttonPin) == LOW;
    }

private:
    uint8_t buttonPin;
    unsigned long debounceInterval;
    bool lastState;
    bool currentState;
    unsigned long lastDebounceTime;
    bool pressedEvent;
};

#endif // BUTTON_HANDLER_H