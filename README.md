# VisionAssist

VisionAssist is a prototype aid for navigating indoor spaces when sight or attention is limited. A small module sweeps for nearby obstacles and missing ground, gives simple tones so you know when to pause or redirect, and can mirror the readings on a lightweight dashboard.

- Power it on and it immediately starts scanning ahead and downward, pulsing a buzzer faster as you get closer to an obstacle and emitting a steady warning for drop‑offs.
- A browser dashboard can connect over Bluetooth to show live distances, the sensor sweep angle, and voice readouts of the current alert.
- A Raspberry Pi client can stream camera frames to a server if you want a remote view or computer-vision processing alongside the ultrasonic data.

## How it works (short version)
- Hardware: built around an Arduino Nano 33 BLE, two ultrasonic rangefinders for forward and trench detection, a small servo to sweep the forward sensor, and a piezo buzzer for feedback.
- Firmware: the Arduino sketch smooths distance readings, triggers obstacle/drop-off alerts, and advertises a BLE service that shares JSON with the latest distances and servo angle.
- Dashboard: the Next.js app in `Interfaceapp/` uses Web Bluetooth to subscribe to those values, renders distance bars, and provides voice alerts in the browser.
- Pi client: `vision_assist_pi_client/` optionally captures frames from a Pi camera and sends JPEGs over WebSocket to a server for additional processing.

## Repository map
- `hardware/` – Arduino sketches for obstacle and trench detection plus servo sweep.
- `Interfaceapp/` – Web dashboard (Next.js) that pairs over BLE and voices alerts.
- `vision_assist_pi_client/` – Raspberry Pi script that streams camera frames to a backend.
- `scripts/` – Utility scripts and supporting packages.
