# Temperature Stabilisation

The oscillation happens because `(int)(temp * 100)` truncates, so a reading hovering at e.g. 21.495°C flips between 2149 and 2150 on every read. Three approaches, each as a drop-in stabiliser function:

---

## 1. Hysteresis

Only accept a new value if it differs from the last published value by at least a threshold. Simplest, zero lag for real changes that exceed the band.

```cpp
#define HYSTERESIS_THRESHOLD 10  // centidegrees (0.10 °C)

int stabilise(int raw) {
  static int last = INT_MIN;
  if (last == INT_MIN || abs(raw - last) >= HYSTERESIS_THRESHOLD) {
    last = raw;
  }
  return last;
}
```

**Tradeoff:** Slow drift within the dead-band is invisible. Can get "stuck" if the true value creeps up in steps smaller than the threshold.

---

## 2. Exponential Moving Average (EMA)

Smooths readings continuously. Each new sample nudges the output by a fraction `alpha`. Never gets stuck, but always lags the true value slightly.

```cpp
#define EMA_ALPHA 0.2f  // 0.0=never changes, 1.0=no smoothing

int stabilise(int raw) {
  static float ema = -1.0f;
  if (ema < 0.0f) ema = (float)raw;
  ema += EMA_ALPHA * ((float)raw - ema);
  return (int)(ema + 0.5f);  // round rather than truncate
}
```

**Tradeoff:** Introduces lag — a sudden real change takes several readings to propagate fully. Alpha=0.2 means ~5 samples to reach 67% of a step change (~50 seconds at 10s intervals).

---

## 3. Confirmation Debounce

Only publishes a new value after it has been seen consistently for N consecutive readings. Ignores transient spikes entirely.

```cpp
#define DEBOUNCE_TOLERANCE 5  // centidegrees — readings within this count as "same"
#define DEBOUNCE_COUNT     3  // consecutive confirmations before accepting

int stabilise(int raw) {
  static int published  = INT_MIN;
  static int candidate  = INT_MIN;
  static int count      = 0;

  if (published == INT_MIN) { published = candidate = raw; return published; }

  if (abs(raw - candidate) <= DEBOUNCE_TOLERANCE) {
    if (++count >= DEBOUNCE_COUNT) {
      published  = candidate;
      count      = 0;
    }
  } else {
    candidate = raw;
    count     = 1;
  }
  return published;
}
```

**Tradeoff:** Adds latency of N × `TEMP_INTERVAL` (30s at defaults) before a genuine change is published. Robust against sensor glitches.

---

## Usage (same for all three)

Replace the two lines in `readTemperature()`:
```cpp
// before:
int value = (int)(temp * 100);

// after:
int value = stabilise((int)(temp * 100));
```

## Which to pick

| Approach | Best for | Watch out for |
|---|---|---|
| Hysteresis | Clean, predictable band | Creeping drift within band |
| EMA | Gradual trends, noisy sensors | Lag on sudden real changes |
| Debounce | Eliminating transient spikes | Slow to confirm real changes |

For a temperature sensor at 10s intervals, **hysteresis** is usually the right call — 0.10°C dead-band matches DS18B20's 9-bit resolution (±0.5°C accuracy, ~0.06°C step size) and has no lag.
