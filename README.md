# 🚀 Cosmic Runner ESP32 - Gesture-Controlled Space Game

![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D)
![License](https://img.shields.io/badge/license-MIT-green)
![BLE](https://img.shields.io/badge/BLE-Keyboard-purple)

A gesture-controlled space runner game using ESP32, LDR sensor, and Bluetooth Low Energy. Wave your hand over the sensor to control a rocket ship navigating through space obstacles!

## ✨ Features

### Hardware Side (ESP32)
- 📡 **BLE Keyboard Emulation** - ESP32 acts as wireless keyboard
- 💡 **LDR Light Sensor** - Detects hand gestures via light changes
- 🎯 **Auto-Calibration** - Automatically calibrates to ambient light on startup
- ⚡ **Dual LED Feedback** - Visual indicators for detection status
- 🛡️ **Cooldown Protection** - Prevents double-trigger with 150ms debounce
- 📊 **Serial Monitoring** - Real-time sensor value debugging

### Game Side (Web)
- 🌌 **Stunning Space Theme** - Cyberpunk neon aesthetic with parallax scrolling
- 🚀 **Smooth Physics** - Realistic rocket movement with gravity and rotation
- 💎 **Two Obstacle Types** - Rotating asteroids and energy crystals
- ✨ **Particle Effects** - Boost trails, explosions, and twinkling stars
- 🎯 **LDR Placement Guide** - Visual indicator showing optimal sensor position
- 📈 **Real-Time Statistics** - Avoidance rate, streak counter, obstacles passed
- 💾 **High Score Tracking** - Persistent storage using localStorage
- 📱 **Responsive Design** - Adapts to different screen sizes

## 🛠️ Hardware Requirements

- **ESP32 Development Board** (ESP32-DevKitC or similar)
- **LDR (Light Dependent Resistor)** - Any photoresistor
- **10kΩ Resistor** - For voltage divider circuit
- **2x LEDs** (any color)
- **2x 220Ω Resistors** - For LED current limiting
- **Breadboard & Jumper Wires**
- **USB Cable** - For programming and power

## 📐 Circuit Diagram

```
ESP32 Pin Configuration:
├─ GPIO 34 (ADC) ──┬── LDR ── VCC (3.3V)
│                  └── 10kΩ Resistor ── GND
├─ GPIO 2 ────── LED1 (+ 220Ω) ── GND
└─ GPIO 4 ────── LED2 (+ 220Ω) ── GND
```

### Wiring Instructions
1. **LDR Circuit**: Connect LDR between 3.3V and GPIO34, add 10kΩ resistor from GPIO34 to GND
2. **LED 1**: Connect anode to GPIO2 through 220Ω resistor, cathode to GND
3. **LED 2**: Connect anode to GPIO4 through 220Ω resistor, cathode to GND

## 💻 Software Requirements

### ESP32 Development
- [PlatformIO](https://platformio.org/) (VS Code extension)
- [Arduino Framework](https://www.arduino.cc/)
- **Library**: ESP32 BLE Keyboard by T-vK

### Game
- Modern web browser (Chrome, Firefox, Edge, Safari)
- Bluetooth support enabled

## 🚀 Getting Started

### 1. Flash ESP32 Firmware

```bash
# Clone the repository
git clone https://github.com/yourusername/Cosmic-Runner-ESP32.git
cd Cosmic-Runner-ESP32

# Open in PlatformIO (VS Code)
code .

# Build and upload
pio run --target upload

# Monitor serial output (optional)
pio device monitor
```

### 2. Pair Bluetooth Device

**Windows:**
1. Settings → Bluetooth & Devices → Add Device
2. Look for "ESP32 Dino"
3. Connect

**macOS/Linux:**
1. Bluetooth Settings → Add Device
2. Select "ESP32 Dino"
3. Pair

### 3. Launch the Game

1. Open `dinogame.html` in your browser
2. Allow keyboard input when prompted
3. Position your LDR sensor at the green target indicator on screen
4. Wave your hand to start playing!

## 🎮 How to Play

1. **Launch**: Open the game in your browser
2. **Calibration**: ESP32 auto-calibrates to ambient light (wait 1 second)
3. **Position Sensor**: Align LDR with the green pulsing target on screen
4. **Play**: Wave your hand over the LDR to make the rocket boost
5. **Avoid**: Navigate around asteroids and crystals
6. **Score**: Try to beat your high score!

### Controls
- 🖐️ **Hand Wave** (LDR) - Boost rocket upward
- ⌨️ **SPACE** - Manual boost (for testing)
- 🖱️ **Click** - Manual boost (for testing)

## 📊 Game Statistics

The game tracks:
- **Avoidance Rate**: Percentage of obstacles successfully dodged
- **Obstacles Passed**: Total count (X/Y format)
- **Current Streak**: Consecutive obstacles avoided
- **High Score**: Best score achieved (stored locally)

### Color Coding
- 🟢 **Green (≥90%)** - Excellent sensor placement!
- 🟡 **Yellow (70-89%)** - Good, fine-tune position
- 🔴 **Red (<70%)** - Adjust LDR alignment

## ⚙️ Configuration

### Adjust Sensitivity (main.cpp)
```cpp
int thresholdOffset = 200;      // Increase for less sensitivity
const unsigned long cooldownTime = 150;  // Debounce time in ms
```

### Game Difficulty (dinogame.html)
```javascript
let gameSpeed = 5;              // Initial speed (higher = harder)
const obstacleSpawnInterval = 90;  // Frames between obstacles
```


## 🎥 Demo Video

*Add link to demo video here*
