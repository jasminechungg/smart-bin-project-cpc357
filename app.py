# ================= IMPORT LIBRARIES =================
# Streamlit for web dashboard
import streamlit as st

# Firebase Admin SDK for database access
import firebase_admin
from firebase_admin import credentials, db

# Utilities
import time
import random

# Data handling
import pandas as pd
import numpy as np

# Map visualization
import pydeck as pdk

# Reverse geocoding (lat/lng → address)
from geopy.geocoders import Nominatim
from geopy.extra.rate_limiter import RateLimiter


# ================= FIREBASE CONFIGURATION =================
# Firebase Realtime Database URL
FIREBASE_DB_URL = "https://smartbin-3dced-default-rtdb.asia-southeast1.firebasedatabase.app"

# Firebase service account key (should be ignored in .gitignore)
SERVICE_ACCOUNT_KEY = "smartbin-key.json"

# Initialize Firebase app (prevent duplicate initialization)
try:
    firebase_admin.get_app()
except ValueError:
    cred = credentials.Certificate(SERVICE_ACCOUNT_KEY)
    firebase_admin.initialize_app(cred, {"databaseURL": FIREBASE_DB_URL})

# Reference to smart bin data node
ref = db.reference("/smartbin")


# ================= STREAMLIT PAGE CONFIG =================
st.set_page_config(
    page_title="Smart City Waste Dashboard",
    layout="wide"
)

st.title("🗑️ Smart City Waste Monitoring Dashboard")
st.caption("1 Live IoT Smart Bin + 11 Simulated Neighbor Bins")

# Auto refresh interval (seconds)
REFRESH_INTERVAL = 3


# ================= HELPER FUNCTIONS =================
def extract_value(node, default=None):
    """
    Extracts value from Firebase node structure.
    Handles both raw values and { "Value": x } format.
    """
    if isinstance(node, dict):
        return node.get("Value", default)
    return node if node is not None else default


@st.cache_data(ttl=REFRESH_INTERVAL, show_spinner=False)
def get_data():
    """
    Fetch smart bin data from Firebase.
    Cached for performance.
    """
    return ref.get()


# ================= FETCH LIVE BIN DATA =================
data = get_data()
if not data:
    st.error("❌ No data found at /smartbin")
    st.stop()

# Extract live bin location
loc = data.get("location", {})
live_lat = round(float(extract_value(loc.get("lat"), 5.35888)), 5)
live_lng = round(float(extract_value(loc.get("lng"), 100.30099)), 5)

# Fallback coordinates if GPS fails
if live_lat == 0.0: live_lat = 5.35888
if live_lng == 0.0: live_lng = 100.30099

# Extract bin state and fill level
live_state = str(extract_value(data.get("state"), "UNKNOWN"))
live_fill = float(extract_value(data.get("fillPercentage"), 0.0))


# ================= ADDRESS RESOLUTION =================
def get_address_once(lat, lon):
    """
    Converts latitude and longitude into a readable address.
    Called only once to avoid API rate limits.
    """
    geolocator = Nominatim(user_agent="smart_bin_dashboard")
    try:
        location = geolocator.reverse(f"{lat}, {lon}", exactly_one=True, timeout=10)
        return location.address if location else "Unknown location"
    except:
        return "Address unavailable"

# Store address in session state
if "geo_locked" not in st.session_state:
    st.session_state.geo_locked = False
    st.session_state.address = "Resolving address..."

if not st.session_state.geo_locked:
    st.session_state.address = get_address_once(live_lat, live_lng)
    st.session_state.geo_locked = True

live_address = st.session_state.address


# ================= STATUS MESSAGE LOGIC =================
def live_status_message(state):
    """
    Converts system state into user-friendly message.
    """
    if state == "IDLE":
        return "Operating normally"
    if state in ["EXTENDING", "RETRACTING"]:
        return "Compression in progress"
    if state == "JAMMED":
        return "🚨 Bin full — Please pick up trash"
    return "Status unknown"


# ================= LIVE BIN DATA STRUCTURE =================
# This is the REAL IoT bin
live_bin = {
    "name": "Bin A (LIVE)",
    "fill": live_fill,
    "state": live_state,
    "message": live_status_message(live_state),
    "lat": live_lat,
    "lon": live_lng,
    "live": True,
    "Address": live_address,
    "color": [0, 0, 255, 200]  # Blue marker
}


# ================= SIMULATED NEIGHBOR BINS =================
# Used to demonstrate scalability
if "neighbor_offsets" not in st.session_state:
    st.session_state.neighbor_offsets = [
        (np.random.normal(0, 0.002), np.random.normal(0, 0.002))
        for _ in range(11)
    ]

simulated_places = [
    "Central Park", "City Hall", "Library", "Museum", "Market",
    "Community Center", "Stadium", "University", "Hospital", "Mall", "Train Station"
]

dummy_bins = []
for i in range(11):
    fill = random.randint(10, 98)

    dummy_bins.append({
        "name": f"Bin {chr(66+i)}",
        "fill": fill,
        "state": "SIMULATED",
        "message": "🚨 Pickup required" if fill > 85 else "Operating normally",
        "lat": live_lat + st.session_state.neighbor_offsets[i][0],
        "lon": live_lng + st.session_state.neighbor_offsets[i][1],
        "live": False,
        "Address": simulated_places[i],
        "color": [255, 0, 0, 200]  # Red marker
    })

# Combine live and simulated bins
df_bins = pd.DataFrame([live_bin] + dummy_bins)
