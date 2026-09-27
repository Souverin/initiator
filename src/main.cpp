#include <Arduino.h>
#include "config.h"
#include "FSM.h"
#include "WebServerModule.h"

StateMachine fsm;
WebServerModule webServer(80, fsm);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    // Ініціалізація периферії
    fsm.initHardware();

    // Ініціалізація Веб-сервера та Wi-Fi
    webServer.init();

    Serial.println("\n=== Detonator Initiator Simulator Fully Initialized ===");
    Serial.println("Connect to Wi-Fi SSID: 'ESP32_Initiator_Sim' (Pass: 12345678)");
    Serial.println("Open Web UI at: http://192.168.4.1");
}

void loop() {
    // Головний однопотоковий цикл
    fsm.update();
}