#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ================= USER CONFIG ================= */
#define WIFI_SSID     "Galaxy S23 FE B4B9"
#define WIFI_PASS     "S23FEpersonal"

#define MQTT_BROKER   "broker.mqttdashboard.com"
#define MQTT_PORT     1883

#define TEAM_ID       "swastiktheyeagerists"
#define REEF_ID       "swastikb06theyeagerists"

/* ================= HARDWARE ================= */
#define RED_LED_PIN    25
#define GREEN_LED_PIN  26

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_ADDR      0x3C

/* ================= GLOBALS ================= */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* Rolling average */
float rollingBuffer[10];
int bufferIndex = 0;
int bufferCount = 0;
volatile float lastRollingAvg = 0.0;

/* Distress timing */
volatile unsigned long distressTimestamp = 0;
volatile unsigned long ackTimestamp = 0;

/* FreeRTOS handles */
TaskHandle_t backgroundTaskHandle;
TaskHandle_t mqttTaskHandle;
TaskHandle_t distressTaskHandle;

/* ================= WIFI ================= */
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

/* ================= MQTT CALLBACK ================= */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  String msg = String((char*)payload);
  msg.trim();
  msg.toUpperCase();

  String top = String(topic);

  Serial.print("[MQTT] ");
  Serial.print(top);
  Serial.print(" → ");
  Serial.println(msg);

  if (top == TEAM_ID && msg.equals("CHALLENGE")) {
    distressTimestamp = millis();
    xTaskNotify(distressTaskHandle, 0, eNoAction);
  }

  if (top == "krillparadise/data/stream") {
    float value = msg.toFloat();
    rollingBuffer[bufferIndex] = value;
    bufferIndex = (bufferIndex + 1) % 10;
    if (bufferCount < 10) bufferCount++;
  }
}

/* ================= MQTT CONNECT ================= */
void connectMQTT() {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  while (!mqttClient.connected()) {
    String clientId = "ESP32_Task2_" + String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.print("MQTT connecting... ");
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("CONNECTED");
      mqttClient.subscribe("krillparadise/data/stream");
      mqttClient.subscribe(TEAM_ID);
    } else {
      Serial.print("FAILED rc=");
      Serial.println(mqttClient.state());
      delay(1000);
    }
  }
}

/* ================= TASK 1: BACKGROUND ================= */
void backgroundTask(void* pv) {
  while (true) {
    if (bufferCount > 0) {
      float sum = 0;
      for (int i = 0; i < bufferCount; i++) sum += rollingBuffer[i];

      lastRollingAvg = sum / bufferCount;

      Serial.print("[BACKGROUND] Avg = ");
      Serial.println(lastRollingAvg, 2);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("TASK 2");
      display.println("Status: NORMAL");
      display.print("Avg: ");
      display.println(lastRollingAvg, 2);
      display.display();
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/* ================= TASK 2: MQTT LOOP ================= */
void mqttTask(void* pv) {
  while (true) {
    if (!mqttClient.connected()) connectMQTT();
    mqttClient.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/* ================= TASK 3: DISTRESS ================= */
void distressTask(void* pv) {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);

  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("TASK 2");
    display.println("Status: DISTRESS");
    display.display();

    ackTimestamp = millis();

    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"status\":\"ACK\",\"timestamp_ms\":%lu}",
             ackTimestamp);

    mqttClient.publish(REEF_ID, payload);

    Serial.print("[DISTRESS] ");
    Serial.print(distressTimestamp);
    Serial.print(" → ");
    Serial.print(ackTimestamp);
    Serial.print(" (Δ=");
    Serial.print(ackTimestamp - distressTimestamp);
    Serial.println(" ms)");

    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  connectWiFi();
  connectMQTT();

  xTaskCreatePinnedToCore(backgroundTask, "Background", 4096, NULL, 1, &backgroundTaskHandle, 1);
  xTaskCreatePinnedToCore(mqttTask, "MQTT", 4096, NULL, 2, &mqttTaskHandle, 1);
  xTaskCreatePinnedToCore(distressTask, "Distress", 4096, NULL, 3, &distressTaskHandle, 0);
}

void loop() {}
