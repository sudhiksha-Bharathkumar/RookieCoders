# TrustSeal Hardware Notes

## MPU6050

Typical I2C connections:

```text
ESP32  →  MPU6050

3.3V   →  VCC
GND    →  GND
GPIO21 →  SDA
GPIO22 →  SCL
```

## LDR

The LDR should be wired as a voltage divider so its analog output can be read by GPIO34.

A common prototype arrangement is:

```text
3.3V
 |
LDR
 |
 +------ GPIO34
 |
10kΩ
 |
GND
```

The exact direction of the voltage change depends on the divider arrangement.

## Normally-closed tamper switch

The firmware uses:

```cpp
pinMode(TAMPER_PIN, INPUT_PULLUP);
```

The supplied prototype logic treats:

```text
HIGH = triggered/open
LOW  = normal/closed
```

Confirm this behavior with your exact switch and wiring. Mechanical switch behavior can vary depending on whether COM/NC/NO terminals are used.

## LEDs

```text
Green LED → GPIO32
Red LED   → GPIO33
```

Use appropriate current-limiting resistors for LEDs.

## Important

Do not connect 5V signals directly to ESP32 GPIO pins.

Calibrate the LDR and motion thresholds with the actual package/enclosure before using the system for real deliveries.
