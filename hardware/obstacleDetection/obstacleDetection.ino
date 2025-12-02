#include <ArduinoBLE.h>
#include <Servo.h>

// === Pin layout ===
#define OBST_TRIG 5
#define OBST_ECHO 7
#define TRENCH_TRIG 6
#define TRENCH_ECHO 8
#define BUZZER 4
#define SERVO_PIN 10

// === BLE setup ===
#define SERVICE_UUID "c1d0a000-1234-4abc-bbbb-1234567890ab"
#define CHAR_UUID    "c1d0a001-1234-4abc-bbbb-1234567890ab"
BLEService visionService(SERVICE_UUID);
BLEStringCharacteristic distChar(CHAR_UUID, BLERead | BLENotify, 128);

Servo scanServo;

// === Constants (cm) ===
const float alpha = 0.2;
const float mountHeight = 100.0;
const float obstacleTrigger = 120.0; // start reacting at 120 cm
const float trenchLimit = 110.0;     // ground missing beyond 110 cm → trench

// === Globals ===
float obstSmooth = 0;
float trenchSmooth = 0;

// === Distance function (returns -1 if no echo) ===
float getDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long start = micros();
  while (digitalRead(echoPin) == LOW) {
    BLE.poll();
    if (micros() - start > 30000) return -1;  // timeout
  }

  unsigned long echoStart = micros();
  while (digitalRead(echoPin) == HIGH) {
    BLE.poll();
    if (micros() - echoStart > 30000) return -1;
  }

  unsigned long duration = micros() - echoStart;
  return duration * 0.0343 / 2.0;  // cm
}

// === Setup ===
void setup() {
  Serial.begin(9600);

  pinMode(OBST_TRIG, OUTPUT);
  pinMode(OBST_ECHO, INPUT);
  pinMode(TRENCH_TRIG, OUTPUT);
  pinMode(TRENCH_ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);

  scanServo.attach(SERVO_PIN);
  scanServo.write(150);

  if (!BLE.begin()) {
    Serial.println("BLE init failed");
    while (1);
  }

  BLE.setLocalName("VisionAssist");
  BLE.setAdvertisedService(visionService);
  visionService.addCharacteristic(distChar);
  BLE.addService(visionService);

  distChar.writeValue("{\"status\":\"ready\"}");
  BLE.advertise();
  Serial.println("Advertising as VisionAssist...");
}

// === Main loop ===
void loop() {
  static int angle = 150;
  static int step = -5;

  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to: ");
    Serial.println(central.address());

    while (central.connected()) {
      // === Servo sweep ===
      scanServo.write(angle);
      angle += step;
      if (angle <= 30 || angle >= 150) step = -step;

      // === Read sensors ===
      float obstRaw = getDistanceCM(OBST_TRIG, OBST_ECHO);
      float trenchRaw = getDistanceCM(TRENCH_TRIG, TRENCH_ECHO);

      // ignore invalid (-1) readings
      if (obstRaw > 0) {
        if (obstSmooth == 0) obstSmooth = obstRaw;
        obstSmooth = alpha * obstRaw + (1 - alpha) * obstSmooth;
      }
      if (trenchRaw > 0) {
        if (trenchSmooth == 0) trenchSmooth = trenchRaw;
        trenchSmooth = alpha * trenchRaw + (1 - alpha) * trenchSmooth;
      }

      // === Detection logic ===
      bool obstacleDetected = (obstSmooth > 0 && obstSmooth < obstacleTrigger);
      bool trenchDetected = (trenchRaw == -1 || trenchSmooth > trenchLimit);

      // === Adaptive buzzer ===
      if (obstacleDetected) {
        // closer → higher frequency and faster pulse
        int freq = map((int)obstSmooth, 120, 20, 200, 2000);
        freq = constrain(freq, 200, 2000);
        int beepDur = map((int)obstSmooth, 120, 20, 300, 50);

        tone(BUZZER, freq);
        delay(beepDur);
        noTone(BUZZER);
        delay(beepDur);
      }
      else if (trenchDetected) {
        // constant warning for trench / missing ground
        tone(BUZZER, 800);
        delay(150);
        noTone(BUZZER);
        delay(150);
      }
      else {
        noTone(BUZZER);
      }

      // === BLE JSON output ===
      char jsonBuffer[128];
      snprintf(jsonBuffer, sizeof(jsonBuffer),
        "{\"angle\":%d,\"obstacleCM\":%.1f,\"trenchCM\":%.1f}",
        angle, obstSmooth, trenchSmooth);
      distChar.writeValue(jsonBuffer);
      Serial.println(jsonBuffer);

      BLE.poll();
      delay(100);
    }

    Serial.println("Disconnected");
    noTone(BUZZER);
    BLE.advertise();
  }

  BLE.poll();
}
