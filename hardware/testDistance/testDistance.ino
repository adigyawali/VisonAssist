#include <ArduinoBLE.h>

#define LEFT_TRIG 5
#define LEFT_ECHO 7
#define RIGHT_TRIG 6
#define RIGHT_ECHO 8
#define LED 9

#define SERVICE_UUID "c1d0a000-1234-4abc-bbbb-1234567890ab"
#define CHAR_UUID    "c1d0a001-1234-4abc-bbbb-1234567890ab"

BLEService visionService(SERVICE_UUID);
BLEStringCharacteristic distChar(CHAR_UUID, BLERead | BLENotify, 64);

// smoothing factor: 0.0–1.0 (higher = faster response, lower = smoother)
const float alpha = 0.2;
float leftSmooth = 0;
float rightSmooth = 0;

// === distance ===
float getDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long start = micros();
  while (digitalRead(echoPin) == LOW) {
    BLE.poll();
    if (micros() - start > 30000) return -1;
  }

  unsigned long echoStart = micros();
  while (digitalRead(echoPin) == HIGH) {
    BLE.poll();
    if (micros() - echoStart > 30000) return -1;
  }

  unsigned long duration = micros() - echoStart;
  return duration * 0.0343 / 2.0;
}

// === setup ===
void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT);
  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT);
  pinMode(LED, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("BLE failed to start");
    while (1);
  }

  BLE.setLocalName("VisionAssist");
  BLE.setDeviceName("VisionAssist");
  BLE.setAdvertisedService(visionService);
  visionService.addCharacteristic(distChar);
  BLE.addService(visionService);

  distChar.writeValue("{\"status\":\"ready\"}");
  BLE.advertise();
  Serial.println("Advertising as VisionAssist...");
}

// === loop ===
void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to: ");
    Serial.println(central.address());

    while (central.connected()) {
      float leftRaw = getDistanceCM(LEFT_TRIG, LEFT_ECHO);
      float rightRaw = getDistanceCM(RIGHT_TRIG, RIGHT_ECHO);

      // initialize smoothers on first valid read
      if (leftSmooth == 0 && leftRaw > 0) leftSmooth = leftRaw;
      if (rightSmooth == 0 && rightRaw > 0) rightSmooth = rightRaw;

      // exponential smoothing
      if (leftRaw > 0)
        leftSmooth = alpha * leftRaw + (1 - alpha) * leftSmooth;
      if (rightRaw > 0)
        rightSmooth = alpha * rightRaw + (1 - alpha) * rightSmooth;

      digitalWrite(LED, (leftSmooth > 0 && leftSmooth < 15) ||
                            (rightSmooth > 0 && rightSmooth < 15));

      char jsonBuffer[80];
      snprintf(jsonBuffer, sizeof(jsonBuffer),
               "{\"left\":%.1f,\"right\":%.1f}", leftSmooth, rightSmooth);
      distChar.writeValue(jsonBuffer);
      Serial.println(jsonBuffer);

      BLE.poll();
      delay(100);
    }

    Serial.println("Disconnected");
    BLE.advertise();
  }

  BLE.poll();
}
