<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=venom&color=0:000000,50:00111a,100:003344&height=220&section=header&text=IRUTESAM&fontSize=55&fontColor=00E6FF&animation=twinkling&fontAlignY=35&desc=DARK%20VENOM%20INTERFACE&descAlignY=60&descAlign=50"/>
</p>
# 💀 IRUTESAM — Dark Venom v1.0

IRUTESAM is an ESP32-based IR hacking toolkit with a custom hacker-style touchscreen UI.  
This version (v1.0) focuses on **IR capture, analysis, storage, and replay** using a 2.8" TFT display.

> Capture ➜ Save ➜ Replay ➜ Control

---

## ⚡ Features (v1.0)

- 📡 IR Signal Capture (all major protocols)
- 🔁 IR Replay (protocol + RAW fallback)
- 💾 Save signals to SD card
- 📂 Load / delete saved signals
- 📊 Live waveform visualization
- 🖥️ Advanced hacker-style UI (Dark Venom theme)
- 👆 Touchscreen navigation (XPT2046)

---

## 🧠 Hardware Used

- ESP32 DevKit V1
- ILI9341 2.8" TFT (240x320)
- XPT2046 Touch Controller
- IR Receiver + IR LED
- SD Card Module

---

## 📦 Required Libraries

Install via Arduino Library Manager:

- `TFT_eSPI` (by Bodmer)
- `IRremoteESP8266`
- `XPT2046_Touchscreen`
- `SD` (built-in)
- `SPI` (built-in)

---

## ⚙️ TFT_eSPI Setup (IMPORTANT)

Edit this file:
TFT_eSPI/User_Setup.h

### Set Driver:
```cpp
#define ILI9341_2_DRIVER
#define TFT_RGB_ORDER TFT_BGR

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18

#define TFT_CS   17
#define TFT_DC   16
#define TFT_RST  -1

#define TOUCH_CS 21
```
## 🔌 Pin Configuration

### 🖥️ TFT + Touch (Shared SPI)

| Signal   | GPIO |
|----------|------|
| MOSI     | 23   |
| MISO     | 19   |
| SCK      | 18   |
| TFT_CS   | 17   |
| DC       | 16   |
| TOUCH_CS | 21   |

---

### 💾 SD Card (Separate SPI)

| Signal | GPIO |
|--------|------|
| MOSI   | 12   |
| MISO   | 13   |
| SCK    | 14   |
| CS     | 27   |

---

### 📡 IR Module

| Signal | GPIO |
|--------|------|
| IR RX  | 15   |
| IR TX  | 4    |

## 🔥 How It Works

### 📡 IR Capture
- Uses `IRrecv`
- Converts raw buffer → microseconds
- Stores protocol + raw data

---

### 💾 IR Save
- Saved as `.ir` file on SD card  
- File format:

protocol,value,bits,rawLen
raw1,raw2,raw3...****

---

### 🔁 IR Replay
- First tries protocol-based sending  
- Falls back to RAW signal if needed  

---

## 🖥️ UI System

- Custom rendering engine using `TFT_eSPI`  
- Dark Venom hacker-style theme  
- Boot animation + hex graphics  
- Menu, dialog, and waveform UI screens  

---

## 👆 Touch System

- Gesture-based controls:
  - Tap → Select  
  - Hold → Open  
  - Double tap → Fast action  
- Smart zone detection:
  - UP / DOWN / SELECT  

---
## 🚀 Upload Instructions

### 🛠️ Board
ESP32 Dev Module

### 📦 Partition Scheme
Default 4MB

### ⬆️ Upload
- Connect ESP32 via USB  
- Select correct COM port  
- Click Upload in Arduino IDE



