#ifndef CONFIG_H
#define CONFIG_H

/* ===================== TEAM IDENTIFIERS ===================== */
#define TEAM_ID "swastiktheyeagerists"
#define REEF_ID "swastikb06theyeagerists"

/* ===================== OLED ===================== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

/* ===================== LED PINS ===================== */
#define RED_PIN    25
#define GREEN_PIN  26

#define RED_CH     0
#define GREEN_CH   1

#define PWM_FREQ  5000
#define PWM_RES   8   // 0–255

/* ===================== WIFI ===================== */
#define WIFI_SSID     "Galaxy S23 FE B4B9"
#define WIFI_PASSWORD "S23FEpersonal"

/* ===================== MQTT ===================== */
#define MQTT_SERVER "broker.mqttdashboard.com"
#define MQTT_PORT   1883

/* OFFICIAL reef topic (MANDATORY as per problem statement) */
#define OFFICIAL_TIMING_TOPIC "shrimpHub/led/timing/set"

/* Team-scoped topic (ONLY for public broker testing) */
#define TEAM_TEST_TIMING_TOPIC \
  "shrimpHub/" TEAM_ID "/task1/led/timing"

#endif
