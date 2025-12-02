"use client";

import { useEffect, useState } from "react";
import { onValueChange, getValue, type procData } from "@/utils/state";
import { connectBLE } from "@/utils/ble";
import VoiceAlert from "@/components/voiceAlert";
import DistanceBars from "@/components/distanceBars";

export default function Page() {
  const [data, setData] = useState<procData>(() => getValue());
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const off = onValueChange((value) => setData(value));
    return () => { if (typeof off === "function") off(); };
  }, []);

  async function handleConnect() {
    try {
      await connectBLE();
      setConnected(true);
    } catch (err) {
      console.error("BLE connect failed:", err);
    }
  }

  return (
    <main className="flex flex-col items-center justify-center min-h-screen gap-6 p-6">
      <h1 className="text-3xl font-bold">VisionAssist Dashboard</h1>

      {!connected && (
        <button
          className="px-4 py-2 rounded-xl bg-black text-white hover:bg-gray-800 transition"
          onClick={handleConnect}
        >
          Connect Bluetooth
        </button>
      )}

      <DistanceBars
        front={data.obstacle}
        left={data.trench}
        right={data.angle / 90} // normalize angle for bar
      />

      <div className="text-lg text-center">
        <p>Obstacle Distance: {Number.isFinite(data.obstacle) ? data.obstacle.toFixed(2) : "--"} m</p>
        <p>Trench Distance: {Number.isFinite(data.trench) ? data.trench.toFixed(2) : "--"} m</p>
        <p>Angle: {Number.isFinite(data.angle) ? data.angle.toFixed(0) : "--"}°</p>
        <p>Alert: {data.msg}</p>
      </div>

      <VoiceAlert alert={data.msg} />
    </main>
  );
}
