#include <webserver_desl.h>

WebServer_DESL::WebServer_DESL() : server(80), webSocket(1337) {}

void WebServer_DESL::begin() {
    // Make sure we can read the file system
    if (!SPIFFS.begin()) {
        Serial.println("Error mounting SPIFFS");
        while (1);
    }

    // Start access point
    WiFi.softAP(ssid, password);

    // Print our IP address
    Serial.println();
    Serial.println("AP running");
    Serial.print("My IP address: ");
    Serial.println(WiFi.softAPIP());

    // On HTTP request for root, provide index.html file
    server.on("/", HTTP_GET, std::bind(&WebServer_DESL::onIndexRequest, this, std::placeholders::_1));

    // On HTTP request for style sheet, provide style.css
    server.on("/styles.css", HTTP_GET, std::bind(&WebServer_DESL::onCSSRequest, this, std::placeholders::_1));

    // On HTTP request for app.js, provide app.js
    server.on("/app.js", HTTP_GET, std::bind(&WebServer_DESL::onAppJSRequest, this, std::placeholders::_1));

    // On HTTP request for highcharts.js, provide highcharts.js
    server.on("/highchart_desl.js", HTTP_GET, std::bind(&WebServer_DESL::onHighchartsJSRequest, this, std::placeholders::_1));

    // Handle requests for pages that do not exist
    server.onNotFound(std::bind(&WebServer_DESL::onPageNotFound, this, std::placeholders::_1));

    // Start web server
    server.begin();

    // Start WebSocket server and assign callback
    webSocket.begin();
    webSocket.onEvent(std::bind(&WebServer_DESL::onWebSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

void WebServer_DESL::loop() {
    // Look for and handle WebSocket data
    webSocket.loop();
}

void WebServer_DESL::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    // Handle WebSocket events
    if (type == WStype_TEXT) {
        // Parse the received JSON
        StaticJsonDocument<512> doc;
        deserializeJson(doc, payload, length);

        // Check the command and perform actions
        String command = doc["command"].as<String>();
        if (command == "some_command") {
            // Extract data and perform actions
            String value = doc["value"].as<String>();
            // Perform actions based on the received data
            Serial.println(value);
        } else if (command == "slider_state"){
           // Extract data and perform actions
            String value = doc["value"].as<String>();

            if (value == "Off") {
              pwmController.stop_bipolar_unipolar_pwm();                
            }
            else if (value == "Bipolar PWM On") {
              pwmController.run_bipolar_pwm();
            }
            else if (value == "Unipolar PWM On") {
              pwmController.run_unipolar_pwm();
            }
        }
        
    }
}

void WebServer_DESL::onIndexRequest(AsyncWebServerRequest *request) {
    IPAddress remote_ip = request->client()->remoteIP();
    Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
    request->send(SPIFFS, "/index.html", "text/html");
}

void WebServer_DESL::onCSSRequest(AsyncWebServerRequest *request) {
    IPAddress remote_ip = request->client()->remoteIP();
    Serial.println("[" + remote_ip.toString() +
                    "] HTTP GET request of " + request->url());
    request->send(SPIFFS, "/styles.css", "text/css");
    }

void WebServer_DESL::onAppJSRequest(AsyncWebServerRequest *request) {
    IPAddress remote_ip = request->client()->remoteIP();
    Serial.println("[" + remote_ip.toString() +
                    "] HTTP GET request of " + request->url());
    request->send(SPIFFS, "/app.js", "application/javascript");
}

void WebServer_DESL::onHighchartsJSRequest(AsyncWebServerRequest *request) {
    IPAddress remote_ip = request->client()->remoteIP();
    Serial.println("[" + remote_ip.toString() +
                    "] HTTP GET request of " + request->url());
    request->send(SPIFFS, "/highchart_desl.js", "application/javascript");
}

void WebServer_DESL::onPageNotFound(AsyncWebServerRequest *request) {
    IPAddress remote_ip = request->client()->remoteIP();
    Serial.println("[" + remote_ip.toString() +
                    "] HTTP GET request of " + request->url());
    request->send(404, "text/plain", "Not found");
}

void WebServer_DESL::broadcast_doc(StaticJsonDocument<512> doc) {
  serializeJson(doc, JSON_str);
  webSocket.broadcastTXT(JSON_str);
  JSON_str = "";
}