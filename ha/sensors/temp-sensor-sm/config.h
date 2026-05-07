#pragma once

// ================= FEATURE FLAGS =================
#define DEBUG_SERIAL true  // true = print temp readings to Serial

// ================= WIFI =================
#define WIFI_SSID     "SSID"
#define WIFI_PASSWORD "KEY"

// ================= MQTT =================
#define MQTT_SERVER     "192.168.1.60"
#define MQTT_PORT       1883
#define MQTT_TOPIC_BASE "sensor/temperature"

// ================= SENSOR =================
#define ONE_WIRE_BUS 7
#define HYSTERESIS_THRESHOLD 10  // centidegrees (0.10 °C)

// ================= LED (board-detected) =================
#if defined(ARDUINO_SAMD_MKRWIFI1010)
  #define USE_RGB_LED true
  #define LED_R_PIN   26   // NINA RGB via WiFiDrv
  #define LED_G_PIN   25
  #define LED_B_PIN   27
#elif defined(ARDUINO_SAMD_NANO_33_IOT)
  #define USE_RGB_LED false
  #define LED_PIN     6    // external LED on D6
#else
  #error "Unsupported board: add LED configuration"
#endif

// ================= TIMING =================
#define WIFI_RETRY_INTERVAL   5000
#define MQTT_RETRY_INTERVAL   5000
#define TEMP_INTERVAL         10000
#define SENSOR_RESOLUTION     9
#define SENSOR_DELAY          (94 << (SENSOR_RESOLUTION - 9))  // 9=94ms 10=188ms 11=376ms 12=752ms

// ================= LED PATTERNS =================
#define LED_FADE_DURATION     500
#define LED_WIFI_SOLID_TIME   1000
#define LED_ERROR_BLINK_TIME  300
#define LED_ERROR_BLINK_COUNT 5