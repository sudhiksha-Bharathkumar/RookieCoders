# TrustSeal — Exact Theme Prototype

This repository uses the uploaded TrustSeal reference site's **same visual theme and styling language** for the TrustSeal prototype: the cream/paper/espresso/olive/honey/teal palette, League Spartan + Playfair Display + DM Mono typography, fixed navbar, hero treatment, rounded cards, timeline, sensor panel, and footer treatment.

The reference source uses those exact theme variables and typography in its stylesheet. The main site in this repository keeps that styling while changing the content to the TrustSeal B2B/product story.

## Two website experiences

### 1. Main website

```text
frontend/index.html
```

B2B/product story:
- Problem
- TrustSeal system
- Hardware
- Delivery journey
- Business model
- Sensor fusion
- Customer verification CTA

### 2. Customer verification website

```text
frontend/verify.html?id=TS-1048
```

Customer experience:
- TrustSeal ID
- Customer
- Order
- Package
- Restaurant
- Device
- Delivery timeline
- Receipt confirmation

## Run

### Backend

```bash
cd backend
pip install -r requirements.txt
python app.py
```

### Frontend

Use VS Code Live Server or another local static server.

Do NOT open the HTML from inside the ZIP archive.

## ESP32

Open:

```text
firmware/trustseal_esp32/trustseal_esp32.ino
```

Set Wi-Fi credentials and the computer's LAN IP:

```cpp
const char* API_BASE_URL = "http://192.168.1.10:5000";
```

The ESP32 and laptop must be reachable on the same network for local testing.

## QR

A production QR should point to:

```text
https://YOUR-FRONTEND-DOMAIN/verify.html?id=TS-1048
```

The QR identifies the package/seal. Customer authorization should be handled separately with OTP/authentication.

## GitHub

```bash
git init
git add .
git commit -m "TrustSeal exact theme prototype"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/trustseal.git
git push -u origin main
```

## Prototype limitations

The Flask backend is in-memory. Restarting it resets demo state.

The receipt confirmation endpoint is a prototype and should be replaced by authenticated OTP/customer verification for production.

The ESP32 prototype uses HTTP for local testing. Production should use HTTPS and device authentication.
