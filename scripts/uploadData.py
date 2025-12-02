import asyncio
import threading
import requests
from flask import Flask, request, jsonify
from bleak import BleakScanner, BleakClient

# BLE identifiers
DEVICE_NAME = "VisionAssist"
SERVICE_UUID = "c1d0a000-1234-4abc-bbbb-1234567890ab"
CHAR_UUID = "c1d0a001-1234-4abc-bbbb-1234567890ab"

# Flask setup
app = Flask(__name__)

# --- Flask route to receive posted data (optional: for testing) ---
@app.route("/data", methods=["POST"])
def receive_data():
    data = request.json
    print("Received via POST:", data)
    return jsonify({"status": "ok"})


# --- Async BLE loop ---
async def ble_main():
    print("Scanning for VisionAssist...")
    device = None
    devices = await BleakScanner.discover(timeout=5)
    for d in devices:
        if d.name == DEVICE_NAME:
            device = d
            break

    if not device:
        print("Device not found. Make sure VisionAssist is advertising.")
        return

    print(f"Found device: {device.name} [{device.address}]")

    async with BleakClient(device.address) as client:
        await asyncio.sleep(1)
        print("Connected:", client.is_connected)

        def handle(_, data: bytearray):
            try:
                msg = data.decode("utf-8").strip()
                # Forward the JSON string to Flask endpoint
                requests.post("http://localhost:3000/data",
                              json={"payload": msg})
            except Exception as e:
                print("Error posting data:", e)

        await client.start_notify(CHAR_UUID, handle)
        print("Listening for BLE data... (Ctrl+C to stop)")

        try:
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            await client.stop_notify(CHAR_UUID)
            print("Stopped BLE notifications.")


# --- Run Flask + BLE together ---
def run_flask():
    app.run(host="0.0.0.0", port=3000, debug=False, use_reloader=False)


if __name__ == "__main__":
    # Run Flask server in a background thread
    flask_thread = threading.Thread(target=run_flask, daemon=True)
    flask_thread.start()

    # Run BLE client in asyncio event loop
    asyncio.run(ble_main())
