#include "webserver_desl.h"
#include <driver/ledc.h>
#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Adafruit_ADS1015.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

WebServer_DESL webServer;
Adafruit_ADS1015 ads1015;
StaticJsonDocument<512> doc;

// ADC 
int16_t in_volt;
int16_t in_amp;
int16_t out_volt;
int16_t out_amp;
int count = 0;
int count_two = 0;
bool adc_connected = false;
unsigned long lastPrintTime = 0;

// PWM configuration
volatile bool bipolar_pwm_state = false;  // global
volatile bool unipolar_pwm_state = false;  // global 
const int pwmPin1 = 13;
const int pwmPin4 = 16;
const int pwmPin2 = 14;
const int pwmPin3 = 15;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int pwmFreq = 5000; // PWM frequency in Hz
const int pwmResolution = 8; // PWM resolution (8, 10, 12, or 15 bits)
//const int sinTableSize = 100; 
//float sinTable[sinTableSize]; // table for the sine wave cycle
int sinPWM[]={1,2,5,7,10,12,15,17,19,22,24,27,30,32,34,37,39,42,
44,47,49,52,54,57,59,61,64,66,69,71,73,76,78,80,83,85,88,90,92,94,97,99,
101,103,106,108,110,113,115,117,119,121,124,126,128,130,132,134,136,138,140,142,144,146,
148,150,152,154,156,158,160,162,164,166,168,169,171,173,175,177,178,180,182,184,185,187,188,190,192,193,
195,196,198,199,201,202,204,205,207,208,209,211,212,213,215,216,217,219,220,221,222,223,224,225,226,227,
228,229,230,231,232,233,234,235,236,237,237,238,239,240,240,241,242,242,243,243,244,244,245,245,246,246,
247,247,247,248,248,248,248,249,249,249,249,249,255,255,255,255,249,249,249,249,249,248,
248,248,248,247,247,247,246,246,245,245,244,244,243,243,242,242,241,240,240,239,238,237,237,236,235,234,
233,232,231,230,229,228,227,226,225,224,223,222,221,220,219,217,216,215,213,212,211,209,208,207,205,204,
202,201,199,198,196,195,193,192,190,188,187,185,184,182,180,178,177,175,173,171,169,168,166,164,162,160,
158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,126,124,121,119,117,115,113,110,108,106,
103,101,99,97,94,92,90,88,85,83,80,78,76,73,71,69,66,64,61,59,57,54,52,49,47,44,42,39,37,34,32,30,
27,24,22,19,17,15,12,10,7,5,2,1};
int newSinPWM[]={1,5,10,15,19,24,30,34,39,
44,49,54,59,64,69,73,78,83,88,92,97,
101,106,110,115,119,124,128,132,136,140,144,
148,152,156,160,164,168,171,175,178,182,185,188,192,
195,198,201,204,207,209,212,215,217,220,222,224,226,
228,230,232,234,236,237,239,240,242,243,244,245,246,
247,247,248,248,249,249,255,255,249,249,249,
248,248,247,246,245,244,243,242,241,240,238,237,235,
233,231,229,227,225,223,221,219,216,213,211,208,205,
202,199,196,193,190,187,184,180,177,173,169,166,162,
158,154,150,146,142,138,134,130,126,121,117,113,108,
103,99,94,90,85,80,76,71,66,61,57,52,47,42,37,32,
27,22,17,12,7,2};

// Functions 
void readOrFakeSensorValues() {
  if (adc_connected) {
    doc["in_volt"] = (String)ads1015.readADC_SingleEnded(0) + " V";
    doc["in_amp"] = (String)ads1015.readADC_SingleEnded(1) + " A";
    doc["out_volt"] = (String)ads1015.readADC_SingleEnded(2) + " V";
    doc["out_amp"] = (String)ads1015.readADC_SingleEnded(3) + " A";
  } else {
    count++;
    count_two = count + 5;
    if (count == 10) {
      count = 0;
    }
    doc["in_volt"] = (String)count + " V";
    doc["in_amp"] = (String)count + " A";
    doc["out_volt"] = (String)count + " V";
    doc["out_amp"] = (String)count + " A";
  }
}

// RTOS Tasks
void taskWebServer(void *parameter) {
    while (1) {
        // Listen to clients and handle their requests (clients can run and stop the PWM signal)
        webServer.loop();
        esp_task_wdt_reset();  // Reset the watchdog timer
        // Delay to avoid excessive loop iteration
        delay(10);
    }
}

void taskSinPWM(void *parameter) {
  while (1) {
    // Bipoar PWM
    if (bipolar_pwm_state) {
      for (int i = 0; i < sizeof(sinPWM) / sizeof(sinPWM[0]); i++) {
        int dutyCycle1 = sinPWM[i];
        int dutyCycle2 = 255 - sinPWM[i];  // Complementary signal
        ledcWrite(pwmChannel1, dutyCycle1);
        ledcWrite(pwmChannel2, dutyCycle2);
        vTaskDelay(pdMS_TO_TICKS(7));
      }
      esp_task_wdt_reset();  // Reset the watchdog timer
      for (int i = sizeof(sinPWM) / sizeof(sinPWM[0]) - 1; i >= 0; i--) {
        int dutyCycle1 = sinPWM[i];
        int dutyCycle2 = 255 - sinPWM[i];  // Complementary signal
        ledcWrite(pwmChannel1, dutyCycle1);
        ledcWrite(pwmChannel2, dutyCycle2);
        vTaskDelay(pdMS_TO_TICKS(7));
      }
    }
    esp_task_wdt_reset();  // Reset the watchdog timer

    // Unipolar PWM
    if (unipolar_pwm_state) {
      ledcWrite(pwmChannel2, 0);
      for (int i = 0; i < sizeof(newSinPWM) / sizeof(newSinPWM[0]); i++) {
        int dutyCycle1 = newSinPWM[i];
        ledcWrite(pwmChannel1, dutyCycle1);
        vTaskDelay(pdMS_TO_TICKS(7));
      }
      esp_task_wdt_reset();  // Reset the watchdog timer
      for (int i = sizeof(newSinPWM) / sizeof(newSinPWM[0]) - 1; i >= 0; i--) {
        int dutyCycle1 = newSinPWM[i];
        ledcWrite(pwmChannel1, dutyCycle1);
        vTaskDelay(pdMS_TO_TICKS(7));
      }
      esp_task_wdt_reset();  // Reset the watchdog timer
      ledcWrite(pwmChannel1, 0);
      for (int i = 0; i < sizeof(newSinPWM) / sizeof(newSinPWM[0]); i++) {
        int dutyCycle2 = newSinPWM[i];
        ledcWrite(pwmChannel2, dutyCycle2);
        vTaskDelay(pdMS_TO_TICKS(7));
      }
      esp_task_wdt_reset();  // Reset the watchdog timer
      for (int i = sizeof(newSinPWM) / sizeof(newSinPWM[0]) - 1; i >= 0; i--) {
        int dutyCycle2 = newSinPWM[i];
        ledcWrite(pwmChannel2, dutyCycle2);
        vTaskDelay(pdMS_TO_TICKS(7));
      }
    }
    esp_task_wdt_reset();  // Reset the watchdog timer

    if (!unipolar_pwm_state && !bipolar_pwm_state) {
      ledcWrite(pwmChannel1, 0);
      ledcWrite(pwmChannel2, 0);
      esp_task_wdt_reset();
    }
  }
}

void setup() {
  // Serial communication
  Serial.begin(115200);

  // Web server
  webServer.begin();
  xTaskCreatePinnedToCore(taskWebServer, "Web Server Task", 4096, NULL, 2, NULL, 1);

  // ADC
  ads1015.begin();

  // PWM
  // Channel 1
  ledcSetup(pwmChannel1, pwmFreq, pwmResolution);
  ledcAttachPin(pwmPin1, pwmChannel1);
  ledcAttachPin(pwmPin4, pwmChannel1);
  // Channel 2
  ledcSetup(pwmChannel2, pwmFreq, pwmResolution);
  ledcAttachPin(pwmPin2, pwmChannel2);
  ledcAttachPin(pwmPin3, pwmChannel2);
  xTaskCreate(taskSinPWM, "Sinewave Bipolar PWM Task", 4096, NULL, 1, NULL);
}

void loop() {
    // Read sensor values or fake them
    readOrFakeSensorValues();
    
    // Send the JSON object to all connected clients
    webServer.broadcast_doc(doc);
    doc.clear();

    delay(100);
}