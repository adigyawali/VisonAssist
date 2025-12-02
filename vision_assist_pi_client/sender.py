
import os, sys, json, time, asyncio, base64, signal, threading
import numpy as np
import cv2
import websockets

# Try Picamera2 first
USE_PICAMERA2 = False
try:
    from picamera2 import Picamera2
    USE_PICAMERA2 = True
except Exception:
    USE_PICAMERA2 = False

with open("config.json","r") as f:
    CFG = json.load(f)

WS_URL = CFG.get("ws_url", "wss://vision-assist.layne-pitman.com/cameye")
W, H = int(CFG.get("width", 640)), int(CFG.get("height", 480))
FPS = int(CFG.get("fps", 10))
JPEG_Q = int(CFG.get("jpeg_quality", 60))

ARUCO_CHECK = bool(CFG.get("aruco_check", True))
ARUCO_DICT_NAME = CFG.get("aruco_dict", "DICT_5X5_100")
ARUCO_WARN_AFTER = int(CFG.get("aruco_warn_after_missing", 10))

# ArUco dictionary
ARUCO_DICT = None
ARUCO_DET = None
if ARUCO_CHECK:
    try:
        adict = getattr(cv2.aruco, ARUCO_DICT_NAME)
        ARUCO_DICT = cv2.aruco.getPredefinedDictionary(adict)
        params = cv2.aruco.DetectorParameters()
        ARUCO_DET = cv2.aruco.ArucoDetector(ARUCO_DICT, params)
    except Exception as e:
        print("[WARN] OpenCV ArUco not available; continuing without local tag check.")
        ARUCO_CHECK = False

# Shared latest frame (BGR)
latest_lock = threading.Lock()
latest_frame = None
latest_time_ns = 0
stop_flag = False

def handle_signal(sig, frame):
    global stop_flag
    stop_flag = True
signal.signal(signal.SIGINT, handle_signal)
signal.signal(signal.SIGTERM, handle_signal)

def capture_loop():
    global latest_frame, latest_time_ns, stop_flag
    if USE_PICAMERA2:
        print("[INFO] Using Picamera2")
        cam = Picamera2()
        cfg = cam.create_preview_configuration(main={"format": "RGB888", "size": (W, H)})
        cam.configure(cfg)
        cam.start()
        # Warm up
        time.sleep(0.2)
        frame_interval = 1.0 / max(FPS,1)
        next_t = time.time()
        while not stop_flag:
            arr = cam.capture_array()  # RGB888
            img = cv2.cvtColor(arr, cv2.COLOR_RGB2BGR)
            now_ns = time.time_ns()
            with latest_lock:
                latest_frame = img
                latest_time_ns = now_ns
            # simple FPS pacing to avoid flooding
            next_t += frame_interval
            sleep = next_t - time.time()
            if sleep > 0: time.sleep(sleep)
        cam.stop()
    else:
        print("[INFO] Using OpenCV VideoCapture(0)")
        cap = cv2.VideoCapture(0)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, W)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, H)
        cap.set(cv2.CAP_PROP_FPS, FPS)
        while not stop_flag:
            ret, frame = cap.read()
            if not ret:
                time.sleep(0.02)
                continue
            now_ns = time.time_ns()
            with latest_lock:
                latest_frame = frame
                latest_time_ns = now_ns
        cap.release()

def aruco_present(img_bgr):
    if not ARUCO_CHECK or ARUCO_DET is None:
        return True
    corners, ids, _ = ARUCO_DET.detectMarkers(img_bgr)
    return ids is not None and len(ids) > 0

async def sender_loop():
    global stop_flag
    print(f"[INFO] Connecting to {WS_URL}")
    seq = 0
    missing_count = 0
    async for ws in websockets.connect(WS_URL, max_size=8*1024*1024, ping_interval=10, ping_timeout=10):
        try:
            print("[INFO] WebSocket connected")
            while not stop_flag:
                # get latest
                with latest_lock:
                    img = None if latest_frame is None else latest_frame.copy()
                    t_ns = latest_time_ns
                if img is None:
                    await asyncio.sleep(0.01)
                    continue
                # Optional local ArUco check
                if ARUCO_CHECK:
                    if aruco_present(img):
                        missing_count = 0
                    else:
                        missing_count += 1
                        if missing_count == ARUCO_WARN_AFTER:
                            print("[WARN] ArUco tag not seen for several frames – check placement/lighting.")
                # JPEG encode
                ok, buf = cv2.imencode(".jpg", img, [int(cv2.IMWRITE_JPEG_QUALITY), JPEG_Q])
                if not ok:
                    await asyncio.sleep(0.01)
                    continue
                b64 = base64.b64encode(buf).decode("ascii")
                msg = {
                    "type": "frame",
                    "seq": seq,
                    "t_ns": int(t_ns) if t_ns else int(time.time()*1e9),
                    "jpeg_b64": b64,
                    "imu": []  # TODO: hook up IMU here if needed
                }
                await ws.send(json.dumps(msg))
                # Optionally read one response (non-blocking-ish)
                try:
                    resp = await asyncio.wait_for(ws.recv(), timeout=0.001)
                    # Print the nearest for visibility
                    # print(resp)  # uncomment to log full JSON
                except asyncio.TimeoutError:
                    pass
                seq += 1
                # throttle to approx FPS (capture already paces, but network loop should also rest)
                await asyncio.sleep(1.0/max(FPS,1))
        except Exception as e:
            print(f"[ERR] WebSocket error: {e}. Reconnecting soon...")
            await asyncio.sleep(1.0)

def main():
    t = threading.Thread(target=capture_loop, daemon=True)
    t.start()
    try:
        asyncio.run(sender_loop())
    finally:
        global stop_flag
        stop_flag = True
        t.join(timeout=2.0)

if __name__ == "__main__":
    main()
