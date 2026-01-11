#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

/* ===================== OLED OBJECT ===================== */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ===================== STRUCT ===================== */
struct LedTimings {
  int red[10];
  int green[10];
  int length;
};

/* ===================== GLOBALS ===================== */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
QueueHandle_t timingQueue;

/* ===================== WIFI SETUP ===================== */
void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" Connected");
}

/* ===================== MQTT CALLBACK ===================== */
void mqttCallback(char* topic, byte* payload, unsigned int length) {

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, payload, length)) {
    Serial.println("JSON parse failed");
    return;
  }

  JsonArray redArr   = doc["red"];
  JsonArray greenArr = doc["green"];

  if (redArr.isNull() || greenArr.isNull()) {
    Serial.println("Invalid timing payload");
    return;
  }

  LedTimings timings;
  timings.length = min((int)redArr.size(), (int)greenArr.size());

  for (int i = 0; i < timings.length; i++) {
    timings.red[i]   = redArr[i];
    timings.green[i] = greenArr[i];
  }

  xQueueOverwrite(timingQueue, &timings);

  Serial.print("Timing update received from topic: ");
  Serial.println(topic);
}

/* ===================== MQTT TASK ===================== */
void mqttTask(void* pv) {
  for (;;) {
    if (!mqttClient.connected()) {

      Serial.print("Connecting MQTT...");
      if (mqttClient.connect(TEAM_ID "-task1")) {

        Serial.println(" connected");

        mqttClient.subscribe(OFFICIAL_TIMING_TOPIC);
        mqttClient.subscribe(TEAM_TEST_TIMING_TOPIC);

        Serial.println("Subscribed to official + test topics");

      } else {
        Serial.println(" failed");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        continue;
      }
    }

    mqttClient.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/* ===================== LED HEART TASK ===================== */
void ledTask(void* pv) {

  ledcAttach(RED_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(GREEN_PIN, PWM_FREQ, PWM_RES);

  // Common anode → OFF = 255
  ledcWrite(RED_PIN, 255);
  ledcWrite(GREEN_PIN, 255);

  LedTimings active = { {1000}, {1000}, 1 };
  TickType_t lastWake;

  for (;;) {

    if (xQueueReceive(timingQueue, &active, 0) == pdTRUE) {
      Serial.println("New timing applied");
    }

    lastWake = xTaskGetTickCount();

    for (int i = 0; i < active.length; i++) {

      // RED
      ledcWrite(RED_PIN, 0);
      vTaskDelayUntil(&lastWake, active.red[i] / portTICK_PERIOD_MS);
      ledcWrite(RED_PIN, 255);
      vTaskDelayUntil(&lastWake, active.red[i] / portTICK_PERIOD_MS);

      // GREEN
      ledcWrite(GREEN_PIN, 0);
      vTaskDelayUntil(&lastWake, active.green[i] / portTICK_PERIOD_MS);
      ledcWrite(GREEN_PIN, 255);
      vTaskDelayUntil(&lastWake, active.green[i] / portTICK_PERIOD_MS);
    }
  }
}

/* ===================== DISPLAY TASK ===================== */
void displayTask(void* pv) {

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    vTaskDelete(NULL);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Task 1 Running");
  display.println("Listening to reef");
  display.display();

  for (;;) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

/* ===================== SETUP ===================== */
void setup() {
  Serial.begin(115200);

  timingQueue = xQueueCreate(1, sizeof(LedTimings));

  setupWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  xTaskCreatePinnedToCore(mqttTask, "MQTT", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(ledTask,  "LED",  4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(displayTask, "OLED", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // RTOS handles everything
}
