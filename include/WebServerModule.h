#ifndef WEB_SERVER_MODULE_H
#define WEB_SERVER_MODULE_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "FSM.h"
#include "web_index.h"

class WebServerModule {
public:
    WebServerModule(uint16_t port, StateMachine& stateMachine)
        : server(port), fsm(stateMachine) {}

    void init() {
        // Запуск точки доступу (SoftAP)
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ESP32_Initiator_Sim", "12345678");

        Serial.print("[WiFi] Access Point started. IP: ");
        Serial.println(WiFi.softAPIP());

        // Маршрути
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", INDEX_HTML);
        });

        // Ендпоінт статусу системи
        server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
            StaticJsonDocument<256> doc;
            doc["state"] = systemStateToString(fsm.getCurrentState());
            doc["init_state"] = (fsm.getCurrentState() == SystemState::INIT) ? (int)fsm.getInitState() : 0;
            doc["fail_reason"] = failSafeReasonToString(fsm.getFailSafeReason());
            doc["voltage"] = fsm.getSystemVoltage();
            doc["arming_time_ms"] = fsm.getArmingRemainingTimeMs();

            String jsonResponse;
            serializeJson(doc, jsonResponse);
            request->send(200, "application/json", jsonResponse);
        });

        // Ендпоінт прийому команд
        server.on("/api/cmd", HTTP_POST, [this](AsyncWebServerRequest *request) {
            if (request->hasParam("cmd", true)) {
                String cmd = request->getParam("cmd", true)->value();
                bool success = false;

                if (cmd == "start") success = fsm.commandStart();
                else if (cmd == "stop") success = fsm.commandStop();
                else if (cmd == "fire") success = fsm.commandFire();

                if (success) {
                    request->send(200, "text/plain", "OK");
                } else {
                    request->send(400, "text/plain", "Command rejected by Fail-Safe/FSM rules");
                }
            } else {
                request->send(400, "text/plain", "Missing cmd param");
            }
        });

        // Тестовий ендпоінт штучної помилки
        server.on("/api/test/fault", HTTP_POST, [this](AsyncWebServerRequest *request) {
            fsm.injectVoltageFault(2.5f); // Просідання напруги < 3.0V
            request->send(200, "text/plain", "Fault Injected");
        });

        server.begin();
        Serial.println("[WEB] Server started.");
    }

private:
    AsyncWebServer server;
    StateMachine& fsm;
};

#endif // WEB_SERVER_MODULE_H