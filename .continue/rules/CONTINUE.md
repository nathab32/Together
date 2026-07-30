# Together Project Guide

## Project Overview
**Together** is a collaborative communication device designed to help friends stay connected. Inspired by Tamagotchi, it focuses on shared experiences and physical interaction through an ESP32-based hardware interface.

### Key Technologies
- **Hardware:** ESP32 (ESP32-D0WD-V3)
- **Framework:** Arduino (via PlatformIO)
- **Audio:** `arduino-audio-tools` for I2S communication, PDM microphones, and FLAC/WAV processing.
- **Networking:** WiFi and MQTT (`ArduinoMqttClient`) for real-time communication.
- **Libraries:** OneButton for tactile interaction.

---

## Getting Started

### Prerequisites
- [VS Code](https://code.visualstudio.com/) with [PlatformIO IDE extension](https://platformio.org/platformio-ide).
- ESP32 Development Board.

### Installation
1. Clone the repository.
2. Open the project in VS Code (PlatformIO will automatically detect `platformio.ini`).
3. Create a `src/Credentials.h` file (if not present) with your WiFi and MQTT credentials:
   ```cpp
   #define SSID "Your_WiFi_SSID"
   #define PASS "Your_WiFi_Password"
   #define SERVER "MQTT_Server_IP"
   #define MQTT_USER "Username"
   #define MQTT_PASS "Password"
   ```
4. Build and Upload using the PlatformIO toolbar.

---

## Project Structure
- **`src/`**: Main source code.
  - `main.cpp`: Entry point. Contains experimental code for recording, MQTT messaging, and audio streaming.
  - `Audio.cpp/h`: Wrapper class for audio functionality (Mic, Speaker, Sine Generator, Encoded Streams).
  - `Credentials.h`: (User-provided) Network credentials.
- **`lib/`**: External libraries (e.g., `arduino-audio-tools`, `ArduinoMqttClient`).
- **`include/`**: Header files.
- **`platformio.ini`**: Project configuration and dependencies.

---

## Development Workflow

### Coding Standards
- Use **I2S** for audio interfacing.
- Follow the **StreamCopy** pattern from `arduino-audio-tools` for data transfer.
- Implement non-blocking logic using `OneButton` for UI interactions.

### Audio Pipeline
- **Input:** PDM Microphone connected to pins 18 (Data) and 19 (WS).
- **Output:** I2S MAX98357A (or similar) Amp using pins 25 (BCLK), 32 (DIN), 33 (LRC).
- **Processing:** Supports `FLAC` and `WAV` decoding.

---

## Key Concepts
- **Collaborative Interaction:** The device is designed for peer-to-peer or server-mediated interaction (streaks, voice notes).
- **MQTT Audio:** Audio is packetized and sent over MQTT for real-time voice communication.
- **Low-Power:** Designed for battery-operated use (ESP32 deep sleep considerations).

---

## Common Tasks

### Adding a New Button Interaction
1. Define a `OneButton` instance: `OneButton myBtn(PIN);`
2. Attach a callback: `myBtn.attachClick(myCallback);`
3. Call `myBtn.tick();` in the `loop()`.

### Changing Audio Format
- Update the `AudioInfo` object in `Audio.cpp` or `main.cpp`.
- Ensure the encoder/decoder matches (e.g., `FLACDecoder` vs `WAVDecoder`).

---

## Troubleshooting
- **No Audio:** Check `MAX_MODE` (Pin 23) is set to `HIGH` to enable the amplifier.
- **WiFi Connection Fails:** Verify `Credentials.h` and 2.4GHz network compatibility.
- **I2S Noise:** Ensure pins 18/19 (PDM) and 25/32/33 (I2S) are correctly wired and not shared with other peripherals.

---

## References
- [Arduino-Audio-Tools Documentation](https://github.com/pschatzmann/arduino-audio-tools)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
