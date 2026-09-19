/*
  TrustSeal ESP32 Firmware
  ------------------------
  Prototype firmware for:
    - ESP32 DevKit
    - MPU6050
    - LDR
    - Normally-closed tamper switch
    - Optional second switch/reed input
    - Green/red status LEDs
    - Wi-Fi + NTP
    - Flask REST API

  IMPORTANT:
  1. Replace Wi-Fi credentials.
  2. Replace API_BASE_URL with the LAN IP of the computer running Flask.
  3. Do not use 127.0.0.1 as the ESP32 API address.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// -----------------------------------------------------------------------------
// NETWORK
// -----------------------------------------------------------------------------

const char* ssid = "YOUR_WIFI_NETWORK";
const char* password = "YOUR_WIFI_PASSWORD";

// Local example:
// const char* API_BASE_URL = "http://192.168.1.10:5000";

// Production:
// const char* API_BASE_URL = "https://your-api-domain.com";

const char* API_BASE_URL = "http://192.168.1.10:5000";

const char* DEVICE_ID = "TS-ESP32-001";

// -----------------------------------------------------------------------------
// HARDWARE
// -----------------------------------------------------------------------------

const int TAMPER_PIN = 25;

// Optional second switch/reed input.
// Set to false if your prototype does not have it.
const bool USE_SECOND_SWITCH = false;
const int SECOND_SWITCH_PIN = 26;

const int LDR_AO_PIN = 34;

const int GREEN_LED = 32;
const int RED_LED = 33;

// -----------------------------------------------------------------------------
// THRESHOLDS
// -----------------------------------------------------------------------------

const int LIGHT_THRESH = 2000;
const float MOTION_THRESH = 12.0;

// Prototype scoring:
// tamper switch = 50
// second switch = 20
// light = 15
// motion = 15
//
// 65 means the physical tamper signal + at least one other signal
// is normally needed.
const int TAMPER_TRIGGER_SCORE = 65;

// -----------------------------------------------------------------------------
// STATE
// -----------------------------------------------------------------------------

bool isSystemActive = false;
bool hasBeenTampered = false;

String initialTamperTime = "N/A";

unsigned long lastStatusCheck = 0;
unsigned long lastDataPost = 0;

const unsigned long STATUS_INTERVAL = 3000;
const unsigned long DATA_INTERVAL = 3000;

// -----------------------------------------------------------------------------
// OBJECTS
// -----------------------------------------------------------------------------

Adafruit_MPU6050 mpu;

WiFiUDP ntpUDP;

// India Standard Time: UTC + 5:30 = 19800 seconds
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

// -----------------------------------------------------------------------------
// HELPERS
// -----------------------------------------------------------------------------

String apiUrl(const String& path) {
  return String(API_BASE_URL) + path;
}

String getCurrentTime() {
  timeClient.update();
  return timeClient.getFormattedTime();
}

void setStatusLEDs() {
  if (hasBeenTampered) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    return;
  }

  digitalWrite(GREEN_LED, isSystemActive ? HIGH : LOW);
  digitalWrite(RED_LED, LOW);
}

bool isTamperSwitchTriggered() {
  /*
    With INPUT_PULLUP:
      HIGH = circuit open / switch released
      LOW  = circuit closed

    For a normally-closed tamper switch, adapt this logic to your
    exact mechanical wiring.

    Prototype convention retained from the supplied code:
      HIGH = triggered/open
  */
  return digitalRead(TAMPER_PIN) == HIGH;
}

bool isSecondSwitchTriggered() {
  if (!USE_SECOND_SWITCH) {
    return false;
  }

  return digitalRead(SECOND_SWITCH_PIN) == HIGH;
}

// -----------------------------------------------------------------------------
// Wi-Fi
// -----------------------------------------------------------------------------

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Wi-Fi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Wi-Fi connection failed.");
  }
}

// -----------------------------------------------------------------------------
// GET /api/status
// -----------------------------------------------------------------------------

void checkActivationStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;

  String url = apiUrl("/api/status?device_id=") + DEVICE_ID;

  http.begin(url);
  http.setTimeout(2500);

  int code = http.GET();

  if (code > 0) {
    String response = http.getString();

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      bool newActive = doc["active"] | false;

      /*
        Only clear the local tamper latch when the backend explicitly
        deactivates the device.
      */
      if (!newActive && isSystemActive) {
        hasBeenTampered = false;
        initialTamperTime = "N/A";
      }

      isSystemActive = newActive;
      setStatusLEDs();

      Serial.print("Backend active status: ");
      Serial.println(isSystemActive ? "ACTIVE" : "INACTIVE");
    } else {
      Serial.print("Status JSON error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("Status GET failed: ");
    Serial.println(code);
  }

  http.end();
}

// -----------------------------------------------------------------------------
// SENSOR FUSION
// -----------------------------------------------------------------------------

int calculateTamperScore(
  int ldrValue,
  float ax,
  float az,
  bool tamperSwitch,
  bool secondSwitch
) {
  int score = 0;

  if (tamperSwitch) {
    score += 50;
  }

  if (USE_SECOND_SWITCH && secondSwitch) {
    score += 20;
  }

  if (ldrValue > LIGHT_THRESH) {
    score += 15;
  }

  if (abs(ax) > MOTION_THRESH || abs(az) > MOTION_THRESH) {
    score += 15;
  }

  return score;
}

void monitorSensors(int& tamperScore, int& ldrValue, float& motionMagnitude) {
  ldrValue = analogRead(LDR_AO_PIN);

  sensors_event_t acceleration;
  sensors_event_t gyro;
  sensors_event_t temperature;

  mpu.getEvent(
    &acceleration,
    &gyro,
    &temperature
  );

  bool tamperSwitch = isTamperSwitchTriggered();
  bool secondSwitch = isSecondSwitchTriggered();

  tamperScore = calculateTamperScore(
    ldrValue,
    acceleration.acceleration.x,
    acceleration.acceleration.z,
    tamperSwitch,
    secondSwitch
  );

  motionMagnitude = sqrt(
    acceleration.acceleration.x * acceleration.acceleration.x +
    acceleration.acceleration.y * acceleration.acceleration.y +
    acceleration.acceleration.z * acceleration.acceleration.z
  );

  if (isSystemActive && !hasBeenTampered) {
    if (tamperScore >= TAMPER_TRIGGER_SCORE) {
      hasBeenTampered = true;
      initialTamperTime = getCurrentTime();

      Serial.println();
      Serial.println("================================");
      Serial.println("TAMPER DETECTED");
      Serial.print("Score: ");
      Serial.println(tamperScore);
      Serial.print("Time: ");
      Serial.println(initialTamperTime);
      Serial.println("================================");

      setStatusLEDs();
    }
  }
}

// -----------------------------------------------------------------------------
// POST /api/sensor-data
// -----------------------------------------------------------------------------

void sendSensorData(
  int tamperScore,
  int ldrValue,
  float motionMagnitude
) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;

  String url = apiUrl("/api/sensor-data");

  http.begin(url);
  http.setTimeout(3000);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<512> doc;

  doc["device_id"] = DEVICE_ID;
  doc["current_time"] = getCurrentTime();
  doc["is_active"] = isSystemActive;
  doc["tamper_score"] = tamperScore;
  doc["is_tampered"] = hasBeenTampered;
  doc["initial_tamper_time"] =
    hasBeenTampered ? initialTamperTime : "";

  JsonObject sensor = doc.createNestedObject("sensor");

  sensor["ldr"] = ldrValue;
  sensor["motion"] = motionMagnitude;
  sensor["tamper_switch"] = isTamperSwitchTriggered();
  sensor["second_switch"] = isSecondSwitchTriggered();

  String payload;
  serializeJson(doc, payload);

  int code = http.POST(payload);

  if (code > 0) {
    Serial.print("Sensor POST: ");
    Serial.println(code);
  } else {
    Serial.print("Sensor POST failed: ");
    Serial.println(code);
  }

  http.end();
}

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("==========================================");
  Serial.println("TRUSTSEAL ESP32");
  Serial.println("Smart Tamper-Evident Delivery Prototype");
  Serial.println("==========================================");

  // Switches
  pinMode(TAMPER_PIN, INPUT_PULLUP);

  if (USE_SECOND_SWITCH) {
    pinMode(SECOND_SWITCH_PIN, INPUT_PULLUP);
  }

  // LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // I2C
  Wire.begin(21, 22);

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not found.");
    Serial.println("Check SDA/SCL wiring and power.");
    while (true) {
      digitalWrite(RED_LED, !digitalRead(RED_LED));
      delay(300);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 initialized.");

  // Wi-Fi
  connectWiFi();

  // NTP
  timeClient.begin();
  timeClient.update();

  Serial.print("Current time: ");
  Serial.println(getCurrentTime());

  setStatusLEDs();
}

// -----------------------------------------------------------------------------
// LOOP
// -----------------------------------------------------------------------------

void loop() {
  unsigned long now = millis();

  // Reconnect if Wi-Fi drops.
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Check backend activation state.
  if (now - lastStatusCheck >= STATUS_INTERVAL) {
    lastStatusCheck = now;
    checkActivationStatus();
  }

  int tamperScore = 0;
  int ldrValue = 0;
  float motionMagnitude = 0;

  // Always read sensors locally.
  monitorSensors(
    tamperScore,
    ldrValue,
    motionMagnitude
  );

  // Send sensor state.
  if (now - lastDataPost >= DATA_INTERVAL) {
    lastDataPost = now;

    sendSensorData(
      tamperScore,
      ldrValue,
      motionMagnitude
    );
  }

  Serial.print("State: ");
  Serial.print(isSystemActive ? "ARMED" : "DEACTIVATED");

  Serial.print(" | Score: ");
  Serial.print(tamperScore);

  Serial.print(" | LDR: ");
  Serial.print(ldrValue);

  Serial.print(" | Motion: ");
  Serial.print(motionMagnitude, 2);

  Serial.print(" | Tampered: ");
  Serial.println(hasBeenTampered ? "YES" : "NO");

  delay(250);
}
