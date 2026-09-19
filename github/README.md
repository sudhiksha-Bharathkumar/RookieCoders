# TrustSeal

TrustSeal is a reusable smart tamper-evident delivery system for food and parcel logistics.

The prototype combines:
- ESP32 DevKit
- MPU6050 motion sensing
- LDR light sensing
- Normally-closed tamper switch
- Wi-Fi + NTP time
- Flask backend
- Customer QR verification page

## Repository structure

```text
TrustSeal/
├── README.md
├── .gitignore
├── frontend/
│   ├── index.html
│   ├── verify.html
│   ├── style.css
│   ├── verify.css
│   ├── main.js
│   └── verify.js
├── backend/
│   ├── app.py
│   └── requirements.txt
└── firmware/
    └── trustseal_esp32/
        └── trustseal_esp32.ino
```

## 1. Hardware

### Required

| Component | Purpose |
|---|---|
| ESP32 DevKit | Controller + Wi-Fi |
| MPU6050 | Detect movement/acceleration |
| LDR + 10kΩ | Detect unexpected light exposure |
| Normally-closed tamper switch | Detect physical opening |
| LED(s), optional | Local status indication |

The firmware also contains optional support for a second switch/reed input because the original prototype logic included it. If you do not have that component, set `USE_SECOND_SWITCH` to `false` in the firmware.

### Example ESP32 pins

| Signal | GPIO |
|---|---:|
| Tamper switch | 25 |
| Optional second switch/reed | 26 |
| LDR analog output | 34 |
| Green LED | 32 |
| Red LED | 33 |
| MPU6050 SDA | 21 |
| MPU6050 SCL | 22 |

Check your exact ESP32 board and wiring before powering the circuit.

## 2. Firmware libraries

Install these from the Arduino IDE Library Manager:

- Adafruit MPU6050
- Adafruit Unified Sensor
- ArduinoJson

`WiFi.h`, `HTTPClient.h`, `Wire.h`, `WiFiUdp.h`, and `NTPClient.h` are used by the firmware. Depending on your Arduino ESP32 setup, install `NTPClient` from the Library Manager if it is not already available.

## 3. Backend setup

Open a terminal in `backend/`:

```bash
python -m venv .venv
```

Windows:

```bash
.venv\Scripts\activate
```

macOS/Linux:

```bash
source .venv/bin/activate
```

Install dependencies:

```bash
pip install -r requirements.txt
```

Run:

```bash
python app.py
```

The API runs on:

```text
http://127.0.0.1:5000
```

## 4. Frontend setup

Do not open the HTML files directly with `file://`.

Use VS Code Live Server, another static server, or deploy the `frontend/` folder to Netlify/GitHub Pages.

For local development, the verification page uses:

```text
http://127.0.0.1:5000
```

for API requests.

## 5. ESP32 configuration

Open:

```text
firmware/trustseal_esp32/trustseal_esp32.ino
```

Change:

```cpp
const char* ssid = "YOUR_WIFI_NETWORK";
const char* password = "YOUR_WIFI_PASSWORD";
const char* API_BASE_URL = "http://YOUR_COMPUTER_IP:5000";
```

Example:

```cpp
const char* API_BASE_URL = "http://192.168.1.10:5000";
```

Important: `127.0.0.1` means the ESP32 itself, so do not use `127.0.0.1` in the ESP32 firmware.

The computer running Flask and the ESP32 should be on the same Wi-Fi network for local testing.

## 6. ESP32 API endpoints

The firmware calls:

### GET

```text
/api/status?device_id=TS-ESP32-001
```

Example response:

```json
{
  "device_id": "TS-ESP32-001",
  "active": true
}
```

### POST

```text
/api/sensor-data
```

Example payload:

```json
{
  "device_id": "TS-ESP32-001",
  "current_time": "15:03:19",
  "is_active": true,
  "tamper_score": 65,
  "is_tampered": true,
  "initial_tamper_time": "15:03:19",
  "sensor": {
    "tamper_switch": true,
    "second_switch": false,
    "light": 2450,
    "motion": 14.2
  }
}
```

## 7. Sensor-fusion logic

The prototype uses a weighted tamper score.

Default weights:

```text
Tamper switch       +50
Second switch       +20
Light threshold     +15
Motion threshold    +15
```

Default trigger:

```text
tamper score >= 65
```

This means the prototype expects the physical tamper signal plus at least one additional signal before declaring a tamper event.

These values are prototype thresholds. Calibrate them against your actual enclosure and environment before treating them as production detection criteria.

## 8. QR verification

The customer QR should identify the TrustSeal/package and open:

```text
https://YOUR-FRONTEND-DOMAIN/verify.html?id=TS-1048
```

Do not put customer secrets, OTPs, passwords, or permanent credentials inside the QR code.

QR identity and customer authorization are separate:

```text
QR = identify the package/seal
OTP/authentication = authorize handover
```

The current prototype uses a demo confirmation endpoint. Production should add authentication, HTTPS, replay protection, rate limiting, persistent storage, and proper device authorization.

## 9. Current prototype flow

```text
Restaurant
   ↓
Attach TrustSeal
   ↓
Activate device
   ↓
ESP32 checks activation status
   ↓
Sensors monitor package
   ↓
ESP32 sends events to backend
   ↓
Delivery partner transports package
   ↓
Customer scans QR
   ↓
Verification page loads package journey
   ↓
Customer confirms receipt
   ↓
Seal becomes available for return/reset
```

## 10. GitHub

Recommended repository name:

```text
trustseal
```

Initial commands:

```bash
git init
git add .
git commit -m "Initial TrustSeal prototype"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/trustseal.git
git push -u origin main
```

## Important prototype limitation

The backend currently stores demo state in memory. Restarting Flask resets the data.

The current firmware also sends HTTP requests rather than production HTTPS requests.

For a production version, move package/device state to a database and secure the ESP32-to-backend connection.
