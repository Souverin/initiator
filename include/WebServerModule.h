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

        // Головна сторінка
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            Serial.println("[WEB LOG] GET / - Serving INDEX_HTML");
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
            Serial.println("\n[WEB LOG] ---> Incoming POST request to /api/cmd");

            if (request->hasParam("cmd", true)) {
                String cmd = request->getParam("cmd", true)->value();
                Serial.printf("[WEB LOG] Received command parameter 'cmd': '%s'\n", cmd.c_str());
                Serial.printf("[WEB LOG] Current FSM State before cmd: %s\n", systemStateToString(fsm.getCurrentState()));

                bool success = false;

                if (cmd == "start") {
                    success = fsm.commandStart();
                } else if (cmd == "stop") {
                    success = fsm.commandStop();
                } else if (cmd == "fire") {
                    success = fsm.commandFire();
                } else {
                    Serial.printf("[WEB LOG] Unknown command received: '%s'\n", cmd.c_str());
                }

                if (success) {
                    Serial.printf("[WEB LOG] Command '%s' ACCEPTED by FSM. New state: %s\n", 
                                  cmd.c_str(), systemStateToString(fsm.getCurrentState()));
                    request->send(200, "text/plain", "OK");
                } else {
                    Serial.printf("[WEB LOG] ERROR: Command '%s' REJECTED by FSM!\n", cmd.c_str());
                    request->send(400, "text/plain", "Command rejected by Fail-Safe/FSM rules");
                }
            } else {
                Serial.println("[WEB LOG] ERROR: Missing 'cmd' parameter in POST request!");
                request->send(400, "text/plain", "Missing cmd param");
            }
        });

        // Тестовий ендпоінт штучної помилки (Fail-Safe Inject)
        server.on("/api/test/fault", HTTP_POST, [this](AsyncWebServerRequest *request) {
            Serial.println("\n[WEB LOG] ---> Incoming POST request to /api/test/fault (Injecting Fault)");
            Serial.printf("[WEB LOG] FSM State before fault: %s\n", systemStateToString(fsm.getCurrentState()));

            fsm.injectVoltageFault(2.5f); // Просідання напруги < 3.0V
            fsm.update();

            Serial.printf("[WEB LOG] Fault injected. FSM State after fault: %s (Reason: %s)\n", 
                          systemStateToString(fsm.getCurrentState()), 
                          failSafeReasonToString(fsm.getFailSafeReason()));

            request->send(200, "text/plain", "Fault Injected");
        });

        // Обробка неіснуючих маршрутів (404)
        server.onNotFound([](AsyncWebServerRequest *request) {
            Serial.printf("[WEB LOG] WARN: 404 Not Found -> Method: %s, URL: %s\n", 
                          request->methodToString(), request->url().c_str());
            request->send(404, "text/plain", "Not found");
        });

        server.begin();
        Serial.println("[WEB] Server started.");
    }

private:
    AsyncWebServer server;
    StateMachine& fsm;
};

#endif // WEB_SERVER_MODULE_H