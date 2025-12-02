// utils/ble.ts
import { emitValue } from "./state";

const NUS_SERVICE = "c1d0a000-1234-4abc-bbbb-1234567890ab";
const NUS_TX = "c1d0a001-1234-4abc-bbbb-1234567890ab";

export async function connectBLE() {
  try {
    console.log("Requesting Bluetooth device...");
    const device = await navigator.bluetooth.requestDevice({
      filters: [{ services: [NUS_SERVICE] }],
      optionalServices: [NUS_SERVICE],
    });

    const server = await device.gatt?.connect();
    if (!server) throw new Error("Failed to connect GATT");

    const service = await server.getPrimaryService(NUS_SERVICE);
    const txChar = await service.getCharacteristic(NUS_TX);

    txChar.addEventListener("characteristicvaluechanged", (event: Event) => {
      const target = event.target as BluetoothRemoteGATTCharacteristic;
      const val = target.value;
      if (!val) return;

      const value = new TextDecoder().decode(val);
      try {
        const json = JSON.parse(value);

        const obstacle = json.obstacleCM ? json.obstacleCM / 100 : 0;
        const trench = json.trenchCM ? json.trenchCM / 100 : 0;
        const angle = json.angle ?? 0;

        const msg =
          obstacle < 0.5 ? "obstacle ahead" :
          trench < 0.5 ? "trench ahead" :
          "clear";

        emitValue({ obstacle, trench, angle, msg });
      } catch {
        console.warn("Invalid BLE data:", value);
      }
    });

    await txChar.startNotifications();
    console.log("Connected and listening to BLE notifications");
  } catch (err) {
    console.error("BLE connection failed:", err);
    console.log("Running in simulation mode...");
    simulateData();
  }
}

function simulateData() {
  setInterval(() => {
    const data = {
      obstacle: Math.random() * 2,
      trench: Math.random() * 2,
      angle: Math.random() * 180,
      msg: Math.random() < 0.3 ? "obstacle ahead" : "clear",
    };
    emitValue(data);
  }, 1000);
}
