#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <TinyGPSPlus.h>

// Firebase helpers
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ================= WIFI & FIREBASE CONFIG =================
#define WIFI_SSID "Zzz"
#define WIFI_PASSWORD "kembangcomel"

#define API_KEY "AIzaSyAKVVvW9IInCuPnromQVLlIIZLlxKUWLtQ"
#define DATABASE_URL "https://smartbin-3dced-default-rtdb.asia-southeast1.firebasedatabase.app"

// ================= PIN DEFINITIONS =================
// Motor control pins
#define IN3_PIN 9
#define IN4_PIN 10

// Ultrasonic sensor pins
#define TRIG_PIN 6
#define ECHO_PIN 5

// Alert LED pin
#define LED_PIN A5   // Red LED

// GPS pins (Maker Feather AIoT S3)
#define GPS_RX_PIN 16
#define GPS_TX_PIN 15
#define GPS_BAUD 9600

// ================= BIN CONFIGURATION =================
const float BIN_HEIGHT_CM = 18.0;

// Logic thresholds
const float TRIGGER_PERCENTAGE = 60.0;   // Start compression
const float SAFETY_PERCENTAGE  = 95.0;   // Overflow safety

// Motor cycle duration
const unsigned long CYCLE_DURATION_MS = 15000;

// ================= ULTRASONIC FILTER SETTINGS =================
#define NUM_SAMPLES 7
#define MIN_VALID_CM 2.0
#define MAX_VALID_CM 50.0

// ================= LED BLINK SETTINGS =================
const unsigned long LED_BLINK_INTERVAL = 500; // ms
unsigned long lastBlinkTime = 0;
bool ledState = false;

// ================= SYSTEM STATES =================
enum SystemState {
  STATE_IDLE,       // Normal
  STATE_EXTENDING,  // Compressing
  STATE_RETRACTING, // Returning
  STATE_JAMMED      // Full / Error
};

SystemState currentState = STATE_IDLE;
unsigned long stateStartTime = 0;

// ================= GLOBAL VARIABLES =================
float currentDistanceCm = 0.0;
float lastValidDistanceCm = BIN_HEIGHT_CM;
float currentFillPercentage = 0.0;

// ================= FIREBASE & GPS OBJECTS =================
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
unsigned long sendDataPrevMillis = 0;

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// ================= FUNCTION PROTOTYPES =================
float readDistanceFiltered();
float calculatePercentage(float distanceCm);
void handleAutomationLogic();
void updateLED();
void extendActuator();
void retractActuator();
void stopMotor();
void printStatus();
String getStateString();
void readGPS();

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== SMART BIN SYSTEM START ===");

  // Motor pins
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  stopMotor();

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // LED OFF at startup

  // GPS
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  // WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi Connected");

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("System Ready");
}

// ================= LOOP =================
void loop() {
  // 1. Read GPS
  readGPS();

  // 2. Read ultrasonic sensor
  float newDistance = readDistanceFiltered();

  // Update fill only when motor is not moving
  if (currentState == STATE_IDLE || currentState == STATE_JAMMED) {
    currentDistanceCm = newDistance;
    currentFillPercentage = calculatePercentage(currentDistanceCm);
  }

  // 3. LED handling
  updateLED();

  // 4. Print status
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    printStatus();
  }

  // 5. Automation logic
  handleAutomationLogic();

  // 6. Firebase update (every 5s)
  if (Firebase.ready() && signupOK && millis() - sendDataPrevMillis > 5000) {
    sendDataPrevMillis = millis();

    Firebase.RTDB.setFloat(&fbdo, "/smartbin/emptySpaceCm", currentDistanceCm);
    Firebase.RTDB.setFloat(&fbdo, "/smartbin/fillPercentage", currentFillPercentage);
    Firebase.RTDB.setString(&fbdo, "/smartbin/state", getStateString());

    Firebase.RTDB.setBool(
      &fbdo,
      "/smartbin/compressionActive",
      (currentState == STATE_EXTENDING || currentState == STATE_RETRACTING)
    );

    if (gps.location.isValid()) {
      Firebase.RTDB.setFloat(&fbdo, "/smartbin/location/lat", gps.location.lat());
      Firebase.RTDB.setFloat(&fbdo, "/smartbin/location/lng", gps.location.lng());
    }
  }
}

// ================= LED LOGIC =================
void updateLED() {
  unsigned long now = millis();

  switch (currentState) {
    case STATE_IDLE:
      digitalWrite(LED_PIN, LOW);
      break;

    case STATE_EXTENDING:
    case STATE_RETRACTING:
      if (now - lastBlinkTime >= LED_BLINK_INTERVAL) {
        lastBlinkTime = now;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
      }
      break;

    case STATE_JAMMED:
      digitalWrite(LED_PIN, HIGH); // SOLID ON
      break;
  }
}

// ================= AUTOMATION LOGIC =================
void handleAutomationLogic() {
  unsigned long now = millis();

  switch (currentState) {
    case STATE_IDLE:
      if (currentFillPercentage >= TRIGGER_PERCENTAGE &&
          currentFillPercentage < SAFETY_PERCENTAGE) {
        currentState = STATE_EXTENDING;
        stateStartTime = now;
        extendActuator();
      } 
      else if (currentFillPercentage >= SAFETY_PERCENTAGE) {
        currentState = STATE_JAMMED;
        stopMotor();
      }
      break;

    case STATE_EXTENDING:
      if (now - stateStartTime >= CYCLE_DURATION_MS) {
        currentState = STATE_RETRACTING;
        stateStartTime = now;
        retractActuator();
      }
      break;

    case STATE_RETRACTING:
      if (now - stateStartTime >= CYCLE_DURATION_MS) {
        stopMotor();
        delay(2000);

        float checkFill = calculatePercentage(readDistanceFiltered());
        currentState = (checkFill >= TRIGGER_PERCENTAGE) ? STATE_JAMMED : STATE_IDLE;
      }
      break;

    case STATE_JAMMED:
      stopMotor();
      if (currentFillPercentage < TRIGGER_PERCENTAGE) {
        currentState = STATE_IDLE;
      }
      break;
  }
}

// ================= SENSOR & HELPERS =================
float readDistanceFiltered() {
  float readings[NUM_SAMPLES];
  int count = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) continue;

    float distance = duration * 0.0343 / 2;
    if (distance >= MIN_VALID_CM && distance <= MAX_VALID_CM) {
      readings[count++] = distance;
    }
    delay(15);
  }

  if (count == 0) return lastValidDistanceCm;

  for (int i = 0; i < count - 1; i++) {
    for (int j = i + 1; j < count; j++) {
      if (readings[j] < readings[i]) {
        float t = readings[i];
        readings[i] = readings[j];
        readings[j] = t;
      }
    }
  }

  lastValidDistanceCm = readings[count / 2];
  return lastValidDistanceCm;
}

float calculatePercentage(float emptySpaceCm) {
  emptySpaceCm = constrain(emptySpaceCm, 0, BIN_HEIGHT_CM);
  return ((BIN_HEIGHT_CM - emptySpaceCm) / BIN_HEIGHT_CM) * 100.0;
}

void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}

void printStatus() {
  Serial.print("State: ");
  Serial.print(getStateString());
  Serial.print(" | Fill: ");
  Serial.print(currentFillPercentage, 1);
  Serial.println("%");
}

String getStateString() {
  switch (currentState) {
    case STATE_IDLE: return "IDLE";
    case STATE_EXTENDING: return "EXTENDING";
    case STATE_RETRACTING: return "RETRACTING";
    case STATE_JAMMED: return "JAMMED";
    default: return "UNKNOWN";
  }
}

// ================= MOTOR CONTROL =================
void extendActuator() {
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
}

void retractActuator() {
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, HIGH);
}

void stopMotor() {
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}
