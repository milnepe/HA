# temp-sensor-sm Architecture

## Overview

An Arduino sketch for the **MKR WiFi 1010** (or Nano 33 IoT) that reads a DS18B20 temperature sensor over OneWire and publishes readings to an MQTT broker over WiFi. The "sm" stands for **state machine** — the design is structured around non-blocking, cooperative polling rather than blocking calls.

---

## Key Components

### `config.h` — All configuration in one place
- WiFi credentials, MQTT broker address/topic/client ID
- Hardware pins (OneWire bus, LED pins)
- Timing constants (retry intervals, sensor resolution/delay)
- Board detection via `#if defined(ARDUINO_SAMD_MKRWIFI1010)` to switch between the MKR1010's built-in RGB LED (driven via `WiFiDrv`) and an external LED on the Nano 33 IoT

### Connection layer (non-blocking)
- `connectWiFi()` and `connectMQTT()` are **rate-limited** using `millis()` timers (`lastWifiAttempt`, `lastMqttAttempt`). They return immediately if the retry interval hasn't elapsed — no `delay()` calls.
- MQTT depends on WiFi: `connectMQTT()` bails out early if WiFi isn't up.

### Sensor layer (two-phase async)
`readTemperature()` uses a two-phase non-blocking pattern to work around the DS18B20's conversion delay:
1. **Trigger**: calls `requestTemperatures()` and sets `converting = true`
2. **Collect**: waits `SENSOR_DELAY` (94ms at 9-bit resolution), then reads the result and publishes it as `temp * 100` (integer centidegrees) over MQTT

`setWaitForConversion(false)` is set in `setup()` so the library never blocks.

### LED state machine
A `LedState` enum drives visual feedback:

| State | Trigger | Pattern |
|---|---|---|
| `LED_STATE_NORMAL` | WiFi + MQTT connected | Green sine-wave fade |
| `LED_STATE_WIFI_CONNECTING` | No WiFi | Fast red blink (200ms) |
| `LED_STATE_MQTT_CONNECTING` | WiFi OK, no MQTT | Medium red blink (500ms) |
| `LED_STATE_ERROR` | No WiFi after 30s | Red burst (5 blinks, 2s pause) |

`resolveLedState()` applies a fixed priority (error > no-wifi > no-mqtt > normal) so the LED always reflects the most critical condition.

### `loop()` — cooperative scheduler
Each iteration of `loop()` calls all subsystems in order:
1. Attempt connections
2. Service the MQTT client (`mqttClient.loop()`)
3. Read/publish temperature (only when fully connected)
4. Resolve and update LED

No `delay()` anywhere — everything is driven by `millis()` comparisons, keeping the loop responsive.
