"use client";
import { useEffect } from "react";

interface VoiceAlertProps {
  alert: string;
}

export default function VoiceAlert({ alert }: VoiceAlertProps) {
  useEffect(() => {
    if (alert && alert !== "clear") {
      const msg = new SpeechSynthesisUtterance(alert);
      speechSynthesis.speak(msg);
    }
  }, [alert]);

  return null;
}
