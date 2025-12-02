// utils/state.ts
export interface procData {
  obstacle: number;   // m
  trench: number;     // m
  angle: number;      // degrees
  msg: string;
}

type Listener = (data: procData) => void;

let current: procData = {
  obstacle: 0,
  trench: 0,
  angle: 0,
  msg: "waiting...",
};

let listeners: Listener[] = [];

export function getValue() {
  return current;
}

export function onValueChange(cb: Listener) {
  listeners.push(cb);
  return () => {
    listeners = listeners.filter((f) => f !== cb);
  };
}

export function emitValue(data: procData) {
  current = data;
  listeners.forEach((f) => f(data));
}
