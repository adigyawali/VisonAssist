
# Vision Assist — Pi4 Client (WebSocket JPEG Sender)

Captures frames from the Pi Camera (Picamera2 preferred; OpenCV fallback), optionally verifies an **ArUco** tag is visible,
JPEG-encodes at a fixed size, and sends **only the latest frame** over WebSocket to your server.

Default endpoint: `wss://vision-assist.layne-pitman.com/cameye` (change in `config.json`).

## Features
- **Low-latency ring-buffer**: capture and network run on separate tasks; old frames are dropped.
- **Picamera2** (NV12 → RGB) at 640×480, 8–10 FPS; switches to `cv2.VideoCapture(0)` if Picamera2 isn't available.
- **Local ArUco check** (DICT_5X5_100): warns if the tag disappears for N frames (useful during setup).
- **Configurable JPEG quality** and FPS.
- Clean shutdown on SIGINT/SIGTERM.

## Install (Pi OS Bookworm recommended)
```bash
sudo apt update
sudo apt install -y python3-pip python3-opencv
# Picamera2 (if not already present)
sudo apt install -y python3-picamera2 libcamera-apps
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
```

> If you use OpenCV-only fallback, ensure your USB cam or legacy stack is accessible as /dev/video0.

## Configure
Edit `config.json`:
```json
{
  "ws_url": "wss://vision-assist.layne-pitman.com/cameye",
  "width": 640,
  "height": 480,
  "fps": 10,
  "jpeg_quality": 60,
  "aruco_check": true,
  "aruco_dict": "DICT_5X5_100",
  "aruco_warn_after_missing": 10
}
```

## Run
```bash
source .venv/bin/activate
python sender.py
```

## Systemd (optional)
Create `/etc/systemd/system/vision-assist-client.service` with the template in `systemd/vision-assist-client.service` (edit paths),
then:
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now vision-assist-client
```

## Protocol (matches your GPU server)
Sends a JSON text message per frame:
```json
{
  "type": "frame",
  "seq": 123,
  "t_ns": 1731170000000000,
  "jpeg_b64": "<BASE64 JPEG>",
  "imu": []
}
```
Server replies with JSON detections/nearest/free-space. This client prints server responses if received.
