import axios from "axios";

// Exported variable: always stores the latest guidance
export let currentGuidance: string = "No guidance yet.";


// Promise is a placeholder for future results
// It is a future value that will be avaliable later after the asynchronous operation finishes.
export async function updateGuidance(left: number, right: number): Promise<void> {
  const prompt = `
Left sensor: ${left} cm. Right sensor: ${right} cm.
Write one calm, short, natural-sounding instruction for a visually impaired user.
Tone: clear, safe, concise.
Examples: "Clear path ahead." "Step right carefully." "Obstacle close on left."
Output only the instruction text.`;

  try {
    const res = await axios.post("http://localhost:11434/api/generate", {
      model: "llama3",
      prompt,
      stream: false,
    });

    currentGuidance = res.data.response.trim();
    console.log("Updated guidance:", currentGuidance);
  } catch (err) {
    console.error("LLM generation failed:", err);
  }
}

// Optional direct test
/*if (require.main === module) {
  updateGuidance(30, 60);
}*/

// Usage example in for other files.

// import { updateGuidance, currentGuidance } from "./generateGuidance";

// async function main() {
//   await updateGuidance(25, 50);
//   console.log("Current guidance from module:", currentGuidance);
// }

// main();



