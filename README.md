# IoT-Based Food Monitoring System in Cold Storage

An IoT-based system using **ESP8266** (compatible with ESP32) to monitor and maintain food quality in cold storage environments by measuring **temperature**, **humidity**, and **gas levels**. The system is capable of automatic and manual control of **ACs**, **exhaust fans**, and can send alerts via **SMS using Twilio API** and **Blynk IoT app**.

---

## 📦 Features

- 🌡️ **Temperature Monitoring** using DHT22
- 💧 **Humidity Monitoring**
- 🧪 **Methane/Gas Detection**
- 🟢 **Freshness Evaluation** (Fresh / Slightly Spoiled / Spoiled)
- 🌀 **Exhaust Fan Auto-Control**
- ❄️ **AC Control** (Auto + Manual)
- 🚨 **Real-Time Alerts via Blynk & SMS (Twilio)**
- 🌐 **Local Web Dashboard (via ESP WebServer)**

---

## 🔧 Hardware Requirements

- ESP8266 / ESP32 Development Board  
- DHT22 Temperature and Humidity Sensor  
- MQ Gas Sensor (connected to A0)  
- 2 x AC Relays (AC_1 & AC_2 control)  
- Exhaust Fan (via relay)  
- LED for Gas Alert  
- Power Supply  
- Wi-Fi Access Point  

---

## 🧪 Sensors and Controls

| Sensor/Component | Description                         |
|------------------|-------------------------------------|
| DHT22            | Measures temperature and humidity   |
| MQ Gas Sensor    | Detects gas concentration (e.g. methane) |
| AC_1 / AC_2      | AC control (manual/auto)            |
| Exhaust Fan      | Turns ON/OFF based on humidity      |
| LED              | Blinks on gas detection             |

---

## 📲 IoT Integration

- **Blynk App** (with virtual pins):
  - `V0` → Alert Text  
  - `V1` → Manual AC1 Control  
  - `V2` → Temperature  
  - `V3` → Humidity  
  - `V4` → Gas Sensor Reading  
  - `V5` → Freshness Status  
  - `V6` → Manual AC2 Control  
  - `V7` → Manual Exhaust Control  
  - `V9` → AC1 Status  
  - `V8` → Raw Gas Value  

- **Twilio API** for SMS alerts when:
  - Gas detected
  - High temperature/humidity
  - Manual AC/Exhaust toggles

---

## 🌐 Web Interface (Local Access)

- `/` – Simple HTML welcome page  
- `/data` – Returns sensor readings in JSON:
```json
{
  "temp": 27.40,
  "humidity": 49.60,
  "gas": 120
}
