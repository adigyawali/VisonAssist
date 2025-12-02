import { useState, useEffect } from "react";

export interface SensorData {
  left: number;
  right: number;
  front: number;
  msg: string;
}

export default function useSensorData(): SensorData {
  const [data, setData] = useState<SensorData>({
    left: 0,
    right: 0,
    front: 0,
    msg: "waiting...",
  });

  useEffect(() => {
    const fetchData = async () => {
      try {
        const res = await fetch("http://localhost:3000/data/latest");
        if (!res.ok) return;
        const json = await res.json();

        // Arduino sends {"left":x,"right":x}, no front value → infer or keep 0
        const left = parseFloat(json.horizontal ?? 0);
        const right = parseFloat(json.height ?? 0);
        const front = (left + right) / 2; // optional derived field
        const msg =
          left < 0.5 || right < 0.5
            ? "stop"
            : left < 1.0
            ? "turn right"
            : "clear";

        setData({ left, right, front, msg });
      } catch (err) {
        console.error("Fetch error:", err);
      }
    };

    // Poll Flask every 500 ms for the latest BLE data
    const interval = setInterval(fetchData, 500);
    return () => clearInterval(interval);
  }, []);

  return data;
}
