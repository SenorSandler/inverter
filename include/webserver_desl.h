#ifndef WEBSERVER_DESL_H
#define WEBSERVER_DESL_H

#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include "pwm_desl.h"

class WebServer_DESL {
public:
    WebServer_DESL();
    void begin();
    void loop();
    void broadcast_doc(StaticJsonDocument<512> doc);

private:
    // Constants
    const char *ssid = "DESL_ENDGAME";
    const char *password = "12345678";
    const int dns_port = 53;
    const int http_port = 80;
    const int ws_port = 1337;

    // Globals
    AsyncWebServer server;
    WebSocketsServer webSocket;
    Pwm_DESL pwmController; // Create an instance of Pwm_DESL
    String JSON_str = "";

    // Internal Functions
    void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t *payload, size_t length);
    void onIndexRequest(AsyncWebServerRequest *request);
    void onCSSRequest(AsyncWebServerRequest *request);
    void onPageNotFound(AsyncWebServerRequest *request);
    void onAppJSRequest(AsyncWebServerRequest *request);
    void onHighchartsJSRequest(AsyncWebServerRequest *request);
};

#endif
