#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_WIFI_NETWORK";
const char* password = "YOUR_WIFI_PASSWORD";
const char* API_BASE_URL = "http://192.168.1.10:5000";
const char* DEVICE_ID = "TS-ESP32-001";

const int TAMPER_PIN = 25;
const int SECOND_SWITCH_PIN = 26;
const bool USE_SECOND_SWITCH = false;
const int LDR_AO_PIN = 34;
const int GREEN_LED = 32;
const int RED_LED = 33;

const int LIGHT_THRESH = 2000;
const float MOTION_THRESH = 12.0;
const int TAMPER_TRIGGER_SCORE = 65;

bool isSystemActive = false;
bool hasBeenTampered = false;
String initialTamperTime = "N/A";

Adafruit_MPU6050 mpu;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

String apiUrl(const String& path) {
  return String(API_BASE_URL) + path;
}

String currentTime() {
  timeClient.update();
  return timeClient.getFormattedTime();
}

void setLEDs() {
  if (hasBeenTampered) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(GREEN_LED, isSystemActive ? HIGH : LOW);
    digitalWrite(RED_LED, LOW);
  }
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 60) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
}

void checkActivationStatus() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(apiUrl("/api/status?device_id=") + DEVICE_ID);
  http.setTimeout(2500);

  int code = http.GET();
  if (code > 0) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, http.getString()) == DeserializationError::Ok) {
      bool newActive = doc["active"] | false;
      if (!newActive && isSystemActive) {
        hasBeenTampered = false;
        initialTamperTime = "N/A";
      }
      isSystemActive = newActive;
      setLEDs();
    }
  }
  http.end();
}

void sendData(int score, int ldrValue, float motion) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(apiUrl("/api/sensor-data"));
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<512> doc;
  doc["device_id"] = DEVICE_ID;
  doc["current_time"] = currentTime();
  doc["is_active"] = isSystemActive;
  doc["tamper_score"] = score;
  doc["is_tampered"] = hasBeenTampered;
  doc["initial_tamper_time"] = hasBeenTampered ? initialTamperTime : "";

  JsonObject sensor = doc.createNestedObject("sensor");
  sensor["ldr"] = ldrValue;
  sensor["motion"] = motion;
  sensor["tamper_switch"] = digitalRead(TAMPER_PIN) == HIGH;
  sensor["second_switch"] = USE_SECOND_SWITCH && digitalRead(SECOND_SWITCH_PIN) == HIGH;

  String payload;
  serializeJson(doc, payload);
  http.POST(payload);
  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(TAMPER_PIN, INPUT_PULLUP);
  if (USE_SECOND_SWITCH) pinMode(SECOND_SWITCH_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found.");
    while (true) {
      digitalWrite(RED_LED, !digitalRead(RED_LED));
      delay(300);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  connectWiFi();
  timeClient.begin();
  timeClient.update();
  setLEDs();
}

void loop() {
  static unsigned long lastStatus = 0;
  static unsigned long lastPost = 0;
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  if (now - lastStatus >= 3000) {
    lastStatus = now;
    checkActivationStatus();
  }

  int ldr = analogRead(LDR_AO_PIN);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  bool tamper = digitalRead(TAMPER_PIN) == HIGH;
  bool second = USE_SECOND_SWITCH && digitalRead(SECOND_SWITCH_PIN) == HIGH;

  int score = 0;
  if (tamper) score += 50;
  if (second) score += 20;
  if (ldr > LIGHT_THRESH) score += 15;
  if (abs(a.acceleration.z) > MOTION_THRESH || abs(a.acceleration.x) > MOTION_THRESH) score += 15;

  float motion = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  if (isSystemActive && !hasBeenTampered && score >= TAMPER_TRIGGER_SCORE) {
    hasBeenTampered = true;
    initialTamperTime = currentTime();
    setLEDs();
    Serial.println("TAMPER DETECTED");
  }

  if (now - lastPost >= 3000) {
    lastPost = now;
    sendData(score, ldr, motion);
  }

  delay(250);
}
