from datetime import datetime, timezone
from flask import Flask, jsonify, request
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

DEVICES = {
    "TS-ESP32-001": {
        "active": True,
        "tampered": False,
        "last_seen": None,
        "last_sensor_data": {},
    }
}

PACKAGES = {
    "TS-1048": {
        "trustseal_id": "TS-1048",
        "device_id": "TS-ESP32-001",
        "order_id": "ORD-78142",
        "customer": {"name": "Alex Johnson"},
        "package": "Food Delivery",
        "restaurant": {
            "name": "Olive & Basil",
            "branch": "Kitchen 04",
            "address": "24 Market Street, Chennai",
        },
        "sealed_at": "14:02:11",
        "status": "IN TRANSIT",
        "verified": False,
        "tampered": False,
        "events": [
            {"time": "14:02:11", "title": "RESTAURANT", "detail": "Package sealed and device activated"},
            {"time": "14:08:42", "title": "PICKUP", "detail": "Movement detected, consistent with handover"},
            {"time": "14:14:26", "title": "TRANSIT", "detail": "Normal movement pattern detected"},
            {"time": "14:42:07", "title": "MONITORING", "detail": "No suspicious combination of sensor events detected"},
            {"time": "15:03:19", "title": "ARRIVAL", "detail": "Package reached the receiver and is awaiting authorized handover"},
        ],
    }
}

def utc_now():
    return datetime.now(timezone.utc).isoformat()

def find_package_by_device(device_id):
    for package in PACKAGES.values():
        if package["device_id"] == device_id:
            return package
    return None

@app.get("/")
def home():
    return jsonify({"service": "TrustSeal API", "status": "running"})

@app.get("/api/health")
def health():
    return jsonify({"ok": True})

@app.get("/api/status")
def status():
    device_id = request.args.get("device_id", "").strip()
    if not device_id:
        return jsonify({"error": "device_id is required"}), 400

    device = DEVICES.setdefault(device_id, {
        "active": False, "tampered": False, "last_seen": None, "last_sensor_data": {}
    })

    return jsonify({
        "device_id": device_id,
        "active": device["active"],
        "tampered": device["tampered"]
    })

@app.post("/api/sensor-data")
def sensor_data():
    data = request.get_json(silent=True)
    if not isinstance(data, dict):
        return jsonify({"error": "JSON body required"}), 400

    device_id = data.get("device_id")
    if not device_id:
        return jsonify({"error": "device_id is required"}), 400

    device = DEVICES.setdefault(device_id, {
        "active": bool(data.get("is_active", False)),
        "tampered": False,
        "last_seen": None,
        "last_sensor_data": {}
    })

    device["last_seen"] = utc_now()
    device["last_sensor_data"] = data

    package = find_package_by_device(device_id)

    if data.get("is_tampered") is True:
        device["tampered"] = True
        if package and not package["tampered"]:
            package["tampered"] = True
            package["status"] = "TAMPER DETECTED"
            package["events"].append({
                "time": data.get("initial_tamper_time") or data.get("current_time", "N/A"),
                "title": "TAMPER",
                "detail": f"Sensor fusion score: {data.get('tamper_score', 'N/A')}"
            })

    return jsonify({"ok": True, "device_id": device_id, "received_at": utc_now()})

@app.post("/api/device/<device_id>/activate")
def activate(device_id):
    device = DEVICES.setdefault(device_id, {
        "active": False, "tampered": False, "last_seen": None, "last_sensor_data": {}
    })
    device["active"] = True
    device["tampered"] = False
    return jsonify({"ok": True, "device_id": device_id, "active": True})

@app.post("/api/device/<device_id>/deactivate")
def deactivate(device_id):
    device = DEVICES.get(device_id)
    if not device:
        return jsonify({"error": "Unknown device"}), 404
    device["active"] = False
    device["tampered"] = False
    package = find_package_by_device(device_id)
    if package:
        package["status"] = "READY FOR RETURN"
    return jsonify({"ok": True, "device_id": device_id, "active": False})

@app.get("/api/package/<trustseal_id>")
def package(trustseal_id):
    data = PACKAGES.get(trustseal_id)
    if not data:
        return jsonify({"error": "TrustSeal package not found"}), 404
    return jsonify(data)

@app.post("/api/verify")
def verify():
    data = request.get_json(silent=True) or {}
    trustseal_id = data.get("trustseal_id")
    if not trustseal_id:
        return jsonify({"error": "trustseal_id is required"}), 400

    package = PACKAGES.get(trustseal_id)
    if not package:
        return jsonify({"error": "TrustSeal package not found"}), 404

    package["verified"] = True
    package["status"] = "RECEIPT CONFIRMED"
    package["events"].append({
        "time": datetime.now().strftime("%H:%M:%S"),
        "title": "VERIFIED",
        "detail": "Receiver confirmed the TrustSeal handover"
    })

    return jsonify({
        "ok": True,
        "trustseal_id": trustseal_id,
        "status": package["status"]
    })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
