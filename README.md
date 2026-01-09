 # 🏙️ Smart City Waste Compactor System

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![Python](https://img.shields.io/badge/Python-3.9%2B-yellow)
![Arduino](https://img.shields.io/badge/Platform-ESP32-green)

## 👥 Team Information
* **Course:** CPC357 - IOT ARCHITECTURE AND SMART APPLICATIONS
* **Members:**
  1. Ain Nabihah Binti Mahamad Chah Pari (162321)
  2. Jasmine Binti Mohd Shaiful Adli Chung (164191)

## 1️⃣ Project Overview

The **Smart City Smart Bin Monitoring System** is a **prototype IoT solution** for monitoring waste bins in urban areas. The system combines:

1.  **ESP32-based smart bin firmware** to monitor fill levels and compress trash automatically.
2.  **Streamlit-based web dashboard** to visualize real-time bin status, location, and alerts.
3.  **Firebase Realtime Database** for cloud data storage.

**Key Goals:**
* Monitor bin fill levels and location in real-time.
* Automate trash compression using a linear actuator.
* Alert operators when bins are full or jammed.
* Provide a user-friendly dashboard for city waste management.

> *Focused on **dry waste bins** (e.g., paper). Can be extended to multiple bins in a city.*

---

## 2️⃣ Features

* **Live IoT Monitoring**: Track one live smart bin + 11 simulated neighbor bins.
* **Automatic Compression**: Actuator compresses waste when bin reaches **60% full**.
* **Safety Features**:
    * **Blinking LED**: Warning during compression/retraction.
    * **Solid LED**: Alert when bin is full or jammed.
* **Real-time Dashboard**: Visualizes bin fill %, system state, and location.
* **Firebase Integration**: Cloud storage for historical and real-time data.
* **GPS Tracking**: Monitors bin location.

---

## 3️⃣ Hardware Requirements

| Component | Description |
| :--- | :--- |
| **ESP32** | Maker Feather AIoT S3 (or generic ESP32) |
| **Ultrasonic Sensor** | HC-SR04 or compatible (measure fill level) |
| **Linear Actuator** | For compression mechanism |
| **LED** | Visual indicator of system state |
| **Jumper Wires** | For prototyping |
| **Power Supply** | 5V suitable for ESP32 & actuator |

---

## 4️⃣ Software Requirements

| Component | Version / Notes |
| :--- | :--- |
| **Arduino IDE** | 1.8+ or PlatformIO |
| **Python** | 3.9+ |
| **Streamlit** | Latest (e.g., 1.24) |
| **Firebase Account** | Realtime Database enabled |
| **Python Libraries** | `streamlit`, `firebase_admin`, `pandas`, `numpy`, `pydeck`, `geopy` |

---

## 5️⃣ Repository Structure
```text
smart-bin-project-cpc357/
├── app.py                   # Streamlit dashboard
├── .gitignore               # Git ignore rules
├── README.md                # Project documentation
├── requirements.txt         # Python dependencies
├── arduino/
│   └── smart_bin.ino        # ESP32 firmware
└── wiring_diagram.png   # Hardware connection diagram
```
---

## 6️⃣ Installation & Setup

### Step 1: Clone Repository
```bash
git clone https://github.com/jasminechungg/smart-bin-project-cpc357.git
cd smart-bin-project-cpc357
```

### Step 2: Install Python Libraries
```bash
pip install streamlit firebase_admin pandas numpy pydeck geopy
```

### Step 3: Firebase Setup
1. Create a Firebase Realtime Database.

2. Download the service account key JSON file.

3. Place the JSON in your project folder (e.g., smartbin-key.json).

4. Update app.py with your Firebase credentials:

```bash
FIREBASE_DB_URL = "https://<your-project-id>.firebaseio.com"
SERVICE_ACCOUNT_KEY = "smartbin-key.json"
```

5. Update Arduino firmware with your credentials:

```bash
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "https://<your-project-id>.firebaseio.com"
```

> ⚠️ Security Note: Never commit your Firebase keys to public repositories. Use .gitignore or environment variables.

### Step 4: Arduino Setup
1. Open arduino/smart_bin.ino in Arduino IDE.

2. Install required Arduino libraries via Library Manager:

```text
Firebase ESP Client

TinyGPSPlus
```

3. Connect ESP32 via USB.

4. Select the correct board and port in Arduino IDE.

5. Upload the code to the ESP32.

### Step 5: Run the Dashboard

```bash
streamlit run app.py
```

Open browser at: http://localhost:8501

The dashboard shows:

 • Map with live + simulated bins

 • Fill %, system state, alerts

 • Real-time updates every 3 seconds

## 7️⃣ How It Works

**ESP32 Firmware
 • Measures fill % using ultrasonic sensor.

 • Controls actuator: compresses bin at 60% fill, retracts after 15s.

 • LED Indicator:

    • Blinking: during compression/retraction

    • Solid ON: if bin is full/jammed

 • Sends data to Firebase: fill %, state, GPS location.

**Streamlit Dashboard
 • Fetches data from Firebase.

 • Displays bins on an interactive map (PyDeck).

 • Shows metrics, alerts, and fill levels.

 • Provides overview of neighbor bins for context.

**Hardware Wiring Diagram
 • File: assets/wiring_diagram.png

## 8️⃣ Libraries Used

Python
```bash
streamlit
firebase_admin
pandas
numpy
pydeck
geopy
```

Arduino
```bash
WiFi
Firebase_ESP_Client
TinyGPSPlus
```

## 9️⃣ Notes & Recommendations
Ensure ESP32 has stable Wi-Fi for real-time updates.

Keep Arduino firmware thresholds configurable:

```bash
TRIGGER_PERCENTAGE
SAFETY_PERCENTAGE
```

Adjust ultrasonic sensor filtering if the environment is noisy.

For multiple bins: deploy multiple ESP32 devices, each with unique Firebase paths.

## 🔟 License
MIT License - feel free to use for educational purposes.
