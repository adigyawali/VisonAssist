# VisionAssist

VisionAssist is a prototype aid for navigation assistance of visually impaired people or indoor spaces when sight or attention is limited such as night time. An ultrasonic module sweeps for nearby obstacles and missing ground, gives simple tones so you know when to pause or redirect, and can mirror the readings on a dashboard.

- Power it on and it immediately starts scanning ahead and downward, pulsing a buzzer faster as you get closer to an obstacle and emitting a steady warning for drop‑offs.
- A browser dashboard can connect over Bluetooth to show live distances, the sensor sweep angle, and voice readouts of the current alert.


## How it works
- Hardware: built around an Arduino Nano 33 BLE, two ultrasonic rangefinders for forward and trench detection, a small servo to sweep the forward sensor, and a piezo buzzer for feedback.
- Firmware: the Arduino sketch smooths distance readings, triggers obstacle/drop-off alerts, and advertises a BLE service that shares JSON with the latest distances and servo angle.
- Dashboard: the Next.js app in `Interfaceapp/` uses Web Bluetooth to subscribe to those values, renders distance bars, and provides voice alerts in the browser.


## Repository map
- `hardware/` – Arduino sketches for obstacle and trench detection plus servo sweep.
- `Interfaceapp/` – Web dashboard (Next.js) that pairs over BLE and voices alerts.
- `scripts/` – Utility scripts and supporting packages.
