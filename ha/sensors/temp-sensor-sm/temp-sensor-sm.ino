#include <limits.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"

#if USE_RGB_LED
#include <utility/wifi_drv.h>
#endif

// ================= GLOBALS =================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= TIMERS =================
unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;
unsigned long lastTempRead = 0;

// ================= SENSOR STATE =================
bool converting = false;
unsigned long conversionStart = 0;

// ================= LED STATE MACHINE =================
enum LedState {
  LED_STATE_NORMAL,
  LED_STATE_WIFI_CONNECTING,
  LED_STATE_MQTT_CONNECTING,
  LED_STATE_ERROR
};

LedState currentLedState = LED_STATE_NORMAL;

// Priority resolver
LedState resolveLedState(bool wifiOk, bool mqttOk, bool error) {
  if (error) return LED_STATE_ERROR;
  if (!wifiOk) return LED_STATE_WIFI_CONNECTING;
  if (!mqttOk) return LED_STATE_MQTT_CONNECTING;
  return LED_STATE_NORMAL;
}

// ================= LED DRIVER =================
unsigned long ledTimer = 0;
int blinkCount = 0;
bool ledOn = false;

enum LedColor { LED_GREEN, LED_RED };

void setLed(uint8_t brightness, LedColor color = LED_RED) {
#if USE_RGB_LED
  WiFiDrv::analogWrite(LED_R_PIN, color == LED_RED   ? brightness : 0);
  WiFiDrv::analogWrite(LED_G_PIN, color == LED_GREEN ? brightness : 0);
  WiFiDrv::analogWrite(LED_B_PIN, 0);
#else
  analogWrite(LED_PIN, brightness);
#endif
}

void updateLed() {
  unsigned long now = millis();

  switch (currentLedState) {

    case LED_STATE_NORMAL: {
      unsigned long period = (unsigned long)LED_FADE_DURATION * 4;
      float t = (float)(now % period) / (float)period;
      float s = (t < 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
      s = s * s;
      setLed((uint8_t)(255.0f * s), LED_GREEN);
      break;
    }

    case LED_STATE_WIFI_CONNECTING:
      // fast blink
      if (now - ledTimer > 200) {
        ledTimer = now;
        ledOn = !ledOn;
        setLed(ledOn ? 255 : 0);
      }
      break;

    case LED_STATE_MQTT_CONNECTING:
      // medium blink
      if (now - ledTimer > 500) {
        ledTimer = now;
        ledOn = !ledOn;
        setLed(ledOn ? 255 : 0);
      }
      break;

    case LED_STATE_ERROR:
      // burst blinking
      if (blinkCount < LED_ERROR_BLINK_COUNT * 2) {
        if (now - ledTimer > LED_ERROR_BLINK_TIME) {
          ledTimer = now;
          ledOn = !ledOn;
          setLed(ledOn ? 255 : 0);
          blinkCount++;
        }
      } else {
        // pause after burst
        setLed(0);
        if (now - ledTimer > 2000) {
          blinkCount = 0;
          ledTimer = now;
        }
      }
      break;
  }
}

// ================= WIFI =================
bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void connectWiFi() {
  if (wifiConnected()) return;

  unsigned long now = millis();
  if (now - lastWifiAttempt < WIFI_RETRY_INTERVAL) return;

  lastWifiAttempt = now;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// ================= MQTT =================
bool mqttConnected() {
  return mqttClient.connected();
}

void connectMQTT() {
  if (!wifiConnected()) return;
  if (mqttConnected()) return;

  unsigned long now = millis();
  if (now - lastMqttAttempt < MQTT_RETRY_INTERVAL) return;

  lastMqttAttempt = now;

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.connect(MQTT_CLIENT_ID);
}

// ================= SENSOR =================
int stabilise(int raw) {
  static int last = INT_MIN;
  if (last == INT_MIN || abs(raw - last) >= HYSTERESIS_THRESHOLD) {
    last = raw;
  }
  return last;
}

void readTemperature() {
  unsigned long now = millis();

  if (!converting && now - lastTempRead > TEMP_INTERVAL) {
    lastTempRead = now;
    sensors.requestTemperatures();
    conversionStart = now;
    converting = true;
#if DEBUG_SERIAL
    Serial.println("Converting...");
#endif
  }

  if (converting && now - conversionStart > SENSOR_DELAY) {
    float temp = sensors.getTempCByIndex(0);
    converting = false;

#if DEBUG_SERIAL
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.println(" \xC2\xB0""C");
#endif

    if (mqttConnected()) {
      int value = stabilise((int)(temp * 100));
      char payload[8];
      snprintf(payload, sizeof(payload), "%d", value);
      bool ok = mqttClient.publish(MQTT_TOPIC, payload);
#if DEBUG_SERIAL
      Serial.print(ok ? "MQTT sent" : "MQTT failed");
      Serial.print(": ");
      Serial.print(MQTT_TOPIC);
      Serial.print(" = ");
      Serial.println(value);
#endif
    }
#if DEBUG_SERIAL
    else {
      Serial.println("MQTT: not connected");
    }
#endif
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

#if USE_RGB_LED
  WiFiDrv::pinMode(LED_R_PIN, OUTPUT);
  WiFiDrv::pinMode(LED_G_PIN, OUTPUT);
  WiFiDrv::pinMode(LED_B_PIN, OUTPUT);
#else
  pinMode(LED_PIN, OUTPUT);
#endif

  sensors.begin();
  sensors.setResolution(SENSOR_RESOLUTION);
  sensors.setWaitForConversion(false);
}

// ================= LOOP =================
void loop() {
  bool wifiOk = wifiConnected();
  bool mqttOk = mqttConnected();
  bool error = false;

  // Connections
  connectWiFi();
  connectMQTT();

  if (mqttOk) {
    mqttClient.loop();
  }

  // Sensor
  if (wifiOk && mqttOk) {
    readTemperature();
  }

  // Example error condition (extend as needed)
  if (!wifiOk && millis() > 30000) {
    error = true;
  }

  // Resolve LED state (priority-based)
  currentLedState = resolveLedState(wifiOk, mqttOk, error);

  // Update LED
  updateLed();
}