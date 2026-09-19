# TrustSeal Hackathon Demo

## Before the demo

1. Start Flask.
2. Start the frontend with Live Server.
3. Put the ESP32 and laptop on the same Wi-Fi.
4. Set the laptop LAN IP in the firmware.
5. Upload the firmware.
6. Confirm the Serial Monitor shows a Wi-Fi connection.
7. Activate `TS-ESP32-001` using the backend.
8. Open the customer verification page.

## Demo sequence

### 1. Show the website

Open:

```text
frontend/index.html
```

Explain the problem and TrustSeal system.

### 2. Show the physical prototype

Point out:

- ESP32
- MPU6050
- LDR
- tamper switch

### 3. Activate the seal

Call:

```text
POST /api/device/TS-ESP32-001/activate
```

The green LED should indicate the active state.

### 4. Trigger tamper

Open or release the tamper switch and/or expose the LDR to light while moving the package.

The firmware combines sensor signals into a tamper score.

### 5. Show backend data

The ESP32 sends sensor data to:

```text
POST /api/sensor-data
```

### 6. Customer scan

Use the QR code for:

```text
https://YOUR-FRONTEND-DOMAIN/verify.html?id=TS-1048
```

For a local phone demo, your laptop's LAN IP can be used instead of localhost, but both devices must be reachable on the same network.

### 7. Confirm receipt

The current prototype has a demo confirmation button.

For a production design, replace that action with customer authentication/OTP.

## Suggested pitch line

"TrustSeal does not rely on one sensor. It correlates physical opening, light exposure and motion to reconstruct what happened to the package."
