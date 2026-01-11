# 🦐 ShrimpHub Reef Project: ESP32 Tasks Overview

## Team Info

| Field   | Details                                               |
|---------|-------------------------------------------------------|
| TEAM_ID | swastiktheyeagerists                                  |
| REEF_ID | swastikb06theyeagerists                               |
| Members | Swastik Bandyopadhyay, Saptarshi Sen                |

---

## Project Overview

This repository contains the ESP32 implementations for **Task 1, 2, and 3** of the ShrimpHub Reef challenge.  
The project handles:

- **LED heartbeat simulation** (Task 1)
- **Distress signal handling with priority** (Task 2)
- **Button-window synchronization** (Task 3)  

All tasks use **MQTT communication, FreeRTOS scheduling, and OLED visual feedback**.

---

## 📌 Table of Contents

1. [Task 1: Timing Keeper](#task-1-timing-keeper)
2. [Task 2: Priority Guardian](#task-2-priority-guardian)
3. [Task 3: Window Synchronizer](#task-3-window-synchronizer)
4. [Key Design Principles](#key-design-principles)
5. [Timing Accuracy & LED Indicators](#timing-accuracy--led-indicators)
6. [Video Links](#video-links)
7. [Build Instructions](#build-instructions)
8. [Collaborators Note](#collaborators-note)

---

## Task 1: Timing Keeper

**Objective:** Simulate reef RGB LED heartbeat patterns with precise timing using MQTT messages.

### Key Components

#### 1️⃣ WiFi Setup
```cpp
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
```
- Connects the ESP32 to WiFi.
- Ensures the device is online before MQTT communication.

#### 2️⃣ MQTT Client & Callback
```cpp
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload, length);

    JsonArray redArr = doc["red"];
    JsonArray greenArr = doc["green"];
    xQueueOverwrite(timingQueue, &timings);
}
```
- Receives LED timing updates from MQTT topics.
- Parses JSON arrays for **Red** and **Green** LEDs.
- Updates FreeRTOS queue for thread-safe access.

#### 3️⃣ LED Task (PWM Control)
```cpp
for (int i = 0; i < active.length; i++) {
    ledcWrite(RED_PIN, 0);
    vTaskDelayUntil(&lastWake, active.red[i] / portTICK_PERIOD_MS);
    ledcWrite(RED_PIN, 255);
    ...
}
```
- Drives LEDs with **precise, non-blocking timing**.
- Uses `vTaskDelayUntil` for ±5ms accuracy.

#### 4️⃣ Display Task
```cpp
display.println("Task 1 Running");
display.println("Listening to reef");
display.display();
```
- OLED shows task status.
- Non-blocking display updates for debugging and judging.

#### 5️⃣ FreeRTOS Architecture

| Task        | Priority | Purpose                        |
| ----------- | -------- | ------------------------------ |
| mqttTask    | 2        | Handles MQTT subscription/loop |
| ledTask     | 3        | Drives LED timing precisely    |
| displayTask | 1        | Updates OLED with task info    |

---

## Task 2: Priority Guardian

**Objective:** Handle background data streaming and high-priority distress messages.

### Key Components

#### 1️⃣ Rolling Average (Background Task)
```cpp
float sum = 0;
for (int i = 0; i < bufferCount; i++) sum += rollingBuffer[i];
lastRollingAvg = sum / bufferCount;
```
- Maintains rolling buffer of last 10 MQTT data values.
- Updates OLED display and Serial monitor.

#### 2️⃣ Distress Handling (High-Priority Task)
```cpp
if (top == TEAM_ID && msg.equals("CHALLENGE")) {
    distressTimestamp = millis();
    xTaskNotify(distressTaskHandle, 0, eNoAction);
}
```
- Preempts lower-priority tasks.
- Ensures **response within 250ms**.

#### 3️⃣ MQTT Acknowledgment
```cpp
snprintf(payload, sizeof(payload),
         "{\"status\":\"ACK\",\"timestamp_ms\":%lu}", ackTimestamp);
mqttClient.publish(REEF_ID, payload);
```
- Sends acknowledgment with timestamp to reef.
- LED indicators show preemption status.

#### 4️⃣ FreeRTOS Task Priorities

| Task           | Priority | Purpose                    |
| -------------- | -------- | -------------------------- |
| distressTask   | 3        | Respond to distress events |
| mqttTask       | 2        | Maintain MQTT connection   |
| backgroundTask | 1        | Rolling average updates    |

---

## Task 3: Window Synchronizer

**Objective:** Synchronize a physical button press with ephemeral “window open” MQTT signals.

### Key Components

#### 1️⃣ Window Detection via MQTT
```cpp
if (top == WINDOW_CODE && msg.indexOf("OPEN") >= 0) {
    windowOpen = true;
    windowOpenTime = millis();
}
```
- Sets `windowOpen` flag when MQTT OPEN signal received.
- Records timestamp for synchronization checks.

#### 2️⃣ Button Press Task
```cpp
if ((millis() - lastDebounce) > 20 && lastState == HIGH && current == LOW) {
    buttonPressTime = millis();
    if (windowOpen && abs((long)(buttonPressTime - windowOpenTime)) <= 50) {
        syncCount++;
        mqttClient.publish("cagedmonkey/listener", payload);
    }
}
```
- Implements **debouncing** (20ms) to prevent false triggers.
- Sync success occurs only if **button pressed within ±50ms** of window.

#### 3️⃣ LED & Display Feedback

| Condition       | RED LED | GREEN LED | OLED          |
| --------------- | ------- | --------- | ------------- |
| Window Closed   | HIGH    | LOW       | "NO WINDOW"   |
| Window Open     | LOW     | HIGH      | "WINDOW OPEN" |
| Sync Successful | LOW     | Blink     | "SYNC OK"     |

#### 4️⃣ Timing Precision
- Button-window sync: ±50ms  
- MQTT loop: 10ms delay  
- Debouncing: 20ms

---

## 🔑 Key Design Principles

- **Non-blocking RTOS tasks** for precise LED and timing control.  
- **MQTT-driven architecture** allows dynamic configuration from reef.  
- **OLED + LEDs** provide instant feedback for debugging and judging.  
- **Priority handling:** distress events preempt background tasks.  
- **Rolling average buffers** allow smooth live data monitoring.

---

## ⏱ Timing Accuracy & LED Indicators

| Task   | Action                   | LED Timing / Precision |
| ------ | ------------------------ | ---------------------- |
| Task 1 | LED heartbeat simulation | ±5ms PWM accuracy      |
| Task 2 | Distress acknowledgment  | ≤250ms response        |
| Task 3 | Button-window sync       | ±50ms tolerance        |

---

## ⚡ Summary

This project demonstrates:

- **Advanced FreeRTOS scheduling**  
- **MQTT message handling**  
- **Real-time LED and button control**  
- **Robust OLED feedback**  
- **Precise timing for reef synchronization tasks**

---

## Video Links

| Task | Link |
|------|------|
| Task 1 | [Link](https://drive.google.com/file/d/1anmkQ3GOU4SCLUYkj7CeXR2Jud-J4BVX/view?usp=sharing) |
| Task 2 | [Link](https://drive.google.com/file/d/1YHR6NzI-a6CDL9OU2KYA-gtokjC26OYn/view?usp=sharing) |

---

## Build Instructions

1. Install **Arduino IDE** or **PlatformIO**.
2. Include required libraries:  
   - `WiFi.h`, `PubSubClient.h`, `Wire.h`, `Adafruit_GFX.h`, `Adafruit_SSD1306.h`, `ArduinoJson.h`
3. Update `config.h` or hardcoded **WiFi/MQTT credentials**.
4. Upload Task 1, Task 2, or Task 3 sketch to ESP32.

---

## Collaborators Note

- Ensure **OLED connections (SDA=21, SCL=22)** are correct.  
- Use **FreeRTOS task priorities** to prevent timing conflicts.  
- Test each task **independently before integrating**.  
- Check **MQTT topics** and **REEF_ID/TEAM_ID** consistency.
