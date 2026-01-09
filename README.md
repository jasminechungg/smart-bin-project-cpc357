SMART CITY WASTE COMPACTOR SYSTEM
https://img.shields.io/badge/License-MIT-blue.svg

1️⃣ Project Overview
The Smart City Smart Bin Monitoring System is a prototype IoT solution for monitoring waste bins in urban areas. The system combines:

ESP32-based smart bin firmware to monitor fill levels and compress trash automatically

Streamlit-based web dashboard to visualize real-time bin status, location, and alerts

Firebase Realtime Database for cloud data storage

Key goals:

Monitor bin fill levels and location in real-time

Automate trash compression using a linear actuator

Alert operators when bins are full or jammed

Provide a user-friendly dashboard for city waste management

This project focuses on dry waste bins (e.g., paper) and can be extended to multiple bins in a city.

2️⃣ Features
Live IoT Monitoring: Track one live smart bin + 11 simulated neighbor bins

Automatic Compression: Actuator compresses waste when bin reaches 60% full

Safety Features:

LED warning during compression/retraction (blinking)

Solid LED alert when bin is full/jammed

Real-time Dashboard: Visualizes bin fill %, system state, and location

Firebase Integration: Cloud storage for historical and real-time data

GPS Tracking: Monitors bin location

3️⃣ Hardware Requirements
Component	Description
ESP32 (Maker Feather AIoT S3)	Microcontroller
Ultrasonic Sensor	HC-SR04 or compatible (measure fill level)
Linear Actuator	For compression mechanism
LED	Visual indicator of system state
Jumper Wires / Breadboard	For prototyping
Power Supply	5V suitable for ESP32 & actuator
4️⃣ Software Requirements
Component	Version / Notes
Arduino IDE	1.8+ or PlatformIO
Python	3.9+
Streamlit	Latest (e.g., 1.24)
Firebase Account	Realtime Database enabled
Python Libraries:	streamlit, firebase_admin, pandas, numpy, pydeck, geopy
5️⃣ Repository Structure
text
smart-bin-project-cpc357/
├── app.py                    # Streamlit dashboard
├── .gitignore
├── README.md
├── arduino/
│   └── smart_bin.ino         # ESP32 firmware
├── assets/
│   └── wiring_diagram.png    # Hardware connection diagram
└── requirements.txt          # Python dependencies
6️⃣ Installation & Setup
Step 1: Clone Repository
bash
git clone https://github.com/jasminechungg/smart-bin-project-cpc357.git
cd smart-bin-project-cpc357
Step 2: Install Python Libraries
bash
pip install streamlit firebase_admin pandas numpy pydeck geopy
Step 3: Firebase Setup
Create a Firebase Realtime Database

Download service account key JSON

Place the JSON in your project folder (e.g., smartbin-key.json)

Update app.py with your Firebase credentials:

python
FIREBASE_DB_URL = "https://<your-project-id>.firebaseio.com"
SERVICE_ACCOUNT_KEY = "smartbin-key.json"
Step 4: Arduino Setup
Open arduino/smart_bin.ino in Arduino IDE

Install required Arduino libraries:

bash
# Arduino Library Manager
Firebase ESP Client
TinyGPSPlus
Update Arduino code with your credentials:

API_KEY

DATABASE_URL

Connect ESP32 via USB

Select board & port in Arduino IDE

Upload the code to ESP32

Step 5: Run the Dashboard
bash
streamlit run app.py
Open browser → http://localhost:8501

⚠️ Security Note: Never commit your Firebase keys to public repositories. Mask or use .gitignore.

7️⃣ How It Works
ESP32 Firmware
Measures fill % using ultrasonic sensor

Controls actuator: compresses bin at 60% fill, retracts after 15s

LED Indicator:

Blinking during compression/retraction

Solid ON if bin is full/jammed

Sends data to Firebase: fill %, state, GPS location

Streamlit Dashboard
Fetches data from Firebase

Displays bins on interactive map (PyDeck)

Shows metrics, alerts, and fill levels

Provides overview of neighbor bins for context

Hardware Wiring Diagram
https://assets/wiring_diagram.png

8️⃣ Libraries Used
Python
bash
streamlit
firebase_admin
pandas
numpy
pydeck
geopy
Arduino
bash
WiFi
Firebase_ESP_Client
TinyGPSPlus
9️⃣ Notes & Recommendations
Recommendations:
Ensure ESP32 has stable Wi-Fi for real-time updates

Keep Arduino firmware thresholds configurable:

cpp
TRIGGER_PERCENTAGE and SAFETY_PERCENTAGE
Adjust ultrasonic sensor filtering if the environment is noisy

For multiple bins: deploy multiple ESP32 devices, each with unique Firebase paths

Bash Commands for Setup:
bash
# Create assets folder if it doesn't exist
mkdir -p assets

# Move your wiring diagram into the folder
mv /path/to/wiring_diagram.png assets/wiring_diagram.png

# Stage the new file and README
git add assets/wiring_diagram.png README.md

# Commit the changes
git commit -m "Add wiring diagram and update README"

# Push to GitHub
git push
🔟 License
text
MIT License - feel free to use for educational purposes
