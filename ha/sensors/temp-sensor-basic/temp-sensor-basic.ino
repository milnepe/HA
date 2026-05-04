#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- USER CONFIG ----------
const char* ssid = "SSID";
const char* password = "KEY";

const char* mqtt_server = "192.168.1.60";
const int   mqtt_port = 1883;
const char* mqtt_topic = "house/bedroom-master/temp";

unsigned long intervalSeconds = 10;  // Default interval
// --------------------------------

// DS18B20 setup
#define ONE_WIRE_BUS 7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi & MQTT
WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long lastPublish = 0;

// ---------- FUNCTIONS ----------

void connectWiFi() {
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(2000);
  }
}

void connectMQTT() {
  while (!client.connected()) {
    String clientId = "Nano33IoT-";
    clientId += String(random(0xffff), HEX);

    if (!client.connect(clientId.c_str())) {
      delay(2000);
    }
  }
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(115200);
  delay(2000);

  sensors.begin();

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);
}

// ---------- LOOP ----------

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  unsigned long now = millis();

  if (now - lastPublish >= intervalSeconds * 1000UL) {
    lastPublish = now;

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC != DEVICE_DISCONNECTED_C) {
      int tempScaled = (int)(tempC * 100);

      char payload[16];
      snprintf(payload, sizeof(payload), "%d", tempScaled);

      client.publish(mqtt_topic, payload);

      Serial.print("Temperature: ");
      Serial.print(tempC);
      Serial.print(" °C -> ");
      Serial.println(payload);
    } else {
      Serial.println("Sensor disconnected");
    }
  }
}