/*
  SAVVIP Nano 33 BLE — 2x HC-SR04 over BLE
  Sends "fwd_cm,down_cm" as a text notification ~10 times/sec.

  Pins (HC-SR04 powered from 3.3V):
    Forward:  TRIG→D5, ECHO→D7, VCC→3.3V, GND→GND
    Downward: TRIG→D6, ECHO→D8, VCC→3.3V, GND→GND
*/

#include <ArduinoBLE.h>

// Ultrasonic pins
const int TRIG_F = 5, ECHO_F = 7;  // Forward
const int TRIG_D = 6, ECHO_D = 8;  // Downward

// BLE: custom service + notify characteristic
BLEService savvipSvc("12345678-1234-1234-1234-1234567890ab");
BLECharacteristic distChar("12345678-1234-1234-1234-1234567890ac",
                           BLERead | BLENotify, 40);

long read_cm(int trig, int echo) {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long us = pulseIn(echo, HIGH, 30000);   // ~30 ms timeout
  if (us == 0) return -1;
  long cm = us / 58; if (cm > 500) cm = 500;
  return cm;
}

void setup() {
  Serial.begin(115200);
  BLE.begin();

  BLE.setLocalName("SAVVIP");
  BLE.setAdvertisedService(savvipSvc);
  savvipSvc.addCharacteristic(distChar);
  BLE.addService(savvipSvc);

  distChar.writeValue("-, -");
  BLE.advertise();

  Serial.println("SAVVIP BLE started. Advertising as SAVVIP.");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    while (central.connected()) {
      long f = read_cm(TRIG_F, ECHO_F);
      long d = read_cm(TRIG_D, ECHO_D);

      char buf[32];
      if (f < 0 && d < 0)       snprintf(buf, sizeof(buf), "-,-");
      else if (f < 0)           snprintf(buf, sizeof(buf), "-,%ld", d);
      else if (d < 0)           snprintf(buf, sizeof(buf), "%ld,-", f);
      else                      snprintf(buf, sizeof(buf), "%ld,%ld", f, d);

      distChar.writeValue((const uint8_t*)buf, strlen(buf));
      delay(100); // ~10 Hz
    }
  } else {
    delay(100);
  }
}
