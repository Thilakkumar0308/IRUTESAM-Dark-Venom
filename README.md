<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=venom&color=0:000000,50:00111a,100:003344&height=220&section=header&text=IRUTESAM&fontSize=55&fontColor=00E6FF&animation=twinkling&fontAlignY=35&desc=DARK%20VENOM%20INTERFACE&descAlignY=60&descAlign=50"/>
</p>

# 💀 IRUTESAM — Dark Venom v1.0

<p align="center">
  <a href="https://irutesam.zeal.wtf/">Preview of Firmware</a>
</p>

IRUTESAM is an ESP32-based IoT toolkit featuring IR analysis, WiFi scanning, NFC interaction, SD card storage, and a custom Dark Venom touchscreen UI.

Built for embedded experimentation, wireless analysis, automation, and hardware research.

> 📡 IR Capture ➜ Save ➜ Replay
> 📶 WiFi Scan ➜ Analyze ➜ Monitor
> 📱 NFC Detect ➜ Read UID ➜ Store

---

## ⚡ Features

* 📡 IR Signal Capture & Replay
* 💾 Save / Load IR Signals from SD Card
* 📂 Signal File Manager
* 📶 WiFi Network Scanner
* 🔐 WiFi Security Detection (Open / WEP / WPA / WPA2 / WPA3)
* 📱 PN532 NFC Tag Detection
* 🆔 NFC UID Reading & Storage
* 👆 XPT2046 Touch Navigation
* 🎨 Dark Venom Hacker-Style UI
* 💻 Real-time Embedded Interface

---

## 🧠 Hardware Used

* ESP32 DevKit V1
* ILI9341 2.8" TFT Display (240x320)
* XPT2046 Touch Controller
* PN532 NFC Module
* IR Receiver Module
* IR LED Transmitter
* SD Card Module

---

## 📦 Required Libraries

Install using Arduino Library Manager:

* TFT_eSPI
* IRremoteESP8266
* XPT2046_Touchscreen
* Adafruit PN532
* SD
* SPI

---

## ⚙️ TFT_eSPI Setup

Edit:

```cpp
TFT_eSPI/User_Setup.h
```

Configure:

```cpp
#define ILI9341_2_DRIVER
#define TFT_RGB_ORDER TFT_BGR

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18

#define TFT_CS   17
#define TFT_DC   16
#define TFT_RST  5

#define TOUCH_CS 25
```

---

## 🔌 Pin Configuration

### 🖥️ TFT Display (ILI9341)

| Signal  | GPIO |
| ------- | ---- |
| MOSI    | 23   |
| MISO    | 19   |
| SCK     | 18   |
| TFT_CS  | 17   |
| TFT_DC  | 16   |
| TFT_RST | 5    |
| TFT_BL  | 21   |

### 👆 Touch Controller (XPT2046)

| Signal    | GPIO |
| --------- | ---- |
| TOUCH_CS  | 25   |
| TOUCH_IRQ | 22   |

### 💾 SD Card (Separate SPI)

| Signal | GPIO |
| ------ | ---- |
| MOSI   | 12   |
| MISO   | 13   |
| SCK    | 14   |
| CS     | 27   |

### 📡 IR Module

| Signal | GPIO |
| ------ | ---- |
| IR_RX  | 15   |
| IR_TX  | 4    |

### 📱 NFC Module (PN532 UART)

| Signal   | GPIO |
| -------- | ---- |
| PN532_RX | 33   |
| PN532_TX | 32   |

---

## 🔥 Modules

### 📡 IR Module

* Capture IR signals
* Decode major protocols
* Store RAW timings
* Replay saved commands
* Protocol and RAW fallback support

### 📶 WiFi Module

* Scan nearby wireless networks
* Display RSSI values
* Detect security type
* Navigate available networks
* Real-time refresh support

### 📱 NFC Module

* Read PN532 NFC tags
* Capture UID information
* Store NFC records
* Fast tag identification

### 💾 SD Storage

* Save captured IR signals
* Load stored files
* Delete saved entries
* Persistent storage

---

## 🖥️ User Interface

* Dark Venom custom theme
* Touchscreen optimized layout
* Animated boot screen
* Status indicators
* Menu navigation system
* Embedded hacker-style design

---

## 👆 Touch Controls

| Action     | Function       |
| ---------- | -------------- |
| Tap        | Select         |
| Swipe Up   | Navigate Up    |
| Swipe Down | Navigate Down  |
| Back       | Return         |
| Select     | Open Menu Item |

---

## 📁 Firmware Structure

```text
IRUTESAM-v1.0/
├── IRUTESAM.ino
├── config.h
├── ir_module.cpp
├── ir_module.h
├── main_ui.h
├── nav_manager.cpp
├── nav_manager.h
├── nfc_module.cpp
├── nfc_module.h
├── sd_manager.cpp
├── sd_manager.h
├── tft_ui.cpp
├── tft_ui.h
├── touch_input.cpp
├── touch_input.h
├── wifi_module.cpp
└── wifi_module.h
```

---

## 🚀 Upload Instructions

### Board

```text
ESP32 Dev Module
```

### Partition Scheme

```text
Default 4MB with SPIFFS
```

### Upload

1. Connect ESP32 via USB
2. Select the correct COM port
3. Choose ESP32 Dev Module
4. Compile the firmware
5. Upload to ESP32
6. Reboot device

---

## ⚠️ Disclaimer

This project is intended for educational, research, and authorized testing purposes only.

Users are responsible for complying with local laws and regulations when using IR, NFC, and wireless functionality.

---

<p align="center">
Made with ❤️ using ESP32
</p>

