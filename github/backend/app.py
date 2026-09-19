from datetime import datetime, timezone
from flask import Flask, jsonify, request
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

# -------------------------------------------------------------------
# Demo in-memory state
# Replace this with a database for a real deployment.
# -------------------------------------------------------------------

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
        "customer": {
            "name": "Alex Johnson",
        },
        "package": "Food Delivery",
        "restaurant": {
            "name": "Olive & Basil",
            "branch": "Kitchen 04",
            "address": "24 Market Street, Chennai",
        },
        "sealed_at": "14:02:11",
        "status": "IN TRANSIT",
        "verified": False,
        "events": [
            {"time": "14:02:11", "title": "Package sealed", "detail": "TrustSeal attached"},
            {"time": "14:08:42", "title": "Delivery started", "detail": "Seal activated"},
            {"time": "14:14:26", "title": "Movement detected", "detail": "Normal transport movement"},
            {"time": "14:42:07", "title": "Package in transit", "detail": "TrustSeal monitoring active"},
            {"time": "15:03:19", "title": "Arrived", "detail": "Awaiting receiver verification"},
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
    return jsonify({
        "service": "TrustSeal API",
        "status": "running"
    })


@app.get("/api/health")
def health():
    return jsonify({
        "ok": True,
        "service": "TrustSeal backend"
    })


# -------------------------------------------------------------------
# ESP32 activation status
# -------------------------------------------------------------------

@app.get("/api/status")
def device_status():
    device_id = request.args.get("device_id", "").strip()

    if not device_id:
        return jsonify({"error": "device_id is required"}), 400

    device = DEVICES.get(device_id)

    if not device:
        # New devices are created as inactive by default.
        DEVICES[device_id] = {
            "active": False,
            "tampered": False,
            "last_seen": None,
            "last_sensor_data": {},
        }
        device = DEVICES[device_id]

    return jsonify({
        "device_id": device_id,
        "active": device["active"],
        "tampered": device["tampered"],
    })


# -------------------------------------------------------------------
# ESP32 sensor data
# -------------------------------------------------------------------

@app.post("/api/sensor-data")
def sensor_data():
    data = request.get_json(silent=True)

    if not isinstance(data, dict):
        return jsonify({"error": "JSON body required"}), 400

    device_id = data.get("device_id")

    if not device_id:
        return jsonify({"error": "device_id is required"}), 400

    if device_id not in DEVICES:
        DEVICES[device_id] = {
            "active": bool(data.get("is_active", False)),
            "tampered": False,
            "last_seen": None,
            "last_sensor_data": {},
        }

    device = DEVICES[device_id]

    device["last_seen"] = utc_now()
    device["last_sensor_data"] = data

    # Once a tamper event is detected, keep it latched on the backend
    # until an authorized reset/deactivation is implemented.
    if data.get("is_tampered") is True:
        device["tampered"] = True

    package = find_package_by_device(device_id)

    if package:
        if data.get("is_tampered") is True:
            package["status"] = "TAMPER DETECTED"
            tamper_time = data.get("initial_tamper_time") or data.get("current_time", "N/A")

            package["events"].append({
                "time": tamper_time,
                "title": "Tamper detected",
                "detail": f"Sensor fusion score: {data.get('tamper_score', 'N/A')}",
            })

        elif data.get("is_active") is True and package["status"] == "SEALED":
            package["status"] = "IN TRANSIT"

    return jsonify({
        "ok": True,
        "device_id": device_id,
        "received_at": utc_now(),
        "active": device["active"],
    })


# -------------------------------------------------------------------
# Demo device activation/deactivation
# In production, protect these endpoints with authentication.
# -------------------------------------------------------------------

@app.post("/api/device/<device_id>/activate")
def activate_device(device_id):
    device = DEVICES.setdefault(device_id, {
        "active": False,
        "tampered": False,
        "last_seen": None,
        "last_sensor_data": {},
    })

    device["active"] = True
    device["tampered"] = False

    return jsonify({
        "ok": True,
        "device_id": device_id,
        "active": True,
    })


@app.post("/api/device/<device_id>/deactivate")
def deactivate_device(device_id):
    device = DEVICES.get(device_id)

    if not device:
        return jsonify({"error": "Unknown device"}), 404

    device["active"] = False
    device["tampered"] = False

    package = find_package_by_device(device_id)

    if package:
        package["status"] = "READY FOR RETURN"

    return jsonify({
        "ok": True,
        "device_id": device_id,
        "active": False,
    })


# -------------------------------------------------------------------
# Customer verification page
# -------------------------------------------------------------------

@app.get("/api/package/<trustseal_id>")
def get_package(trustseal_id):
    package = PACKAGES.get(trustseal_id)

    if not package:
        return jsonify({"error": "TrustSeal package not found"}), 404

    return jsonify(package)


@app.post("/api/verify")
def verify_package():
    data = request.get_json(silent=True) or {}
    trustseal_id = data.get("trustseal_id")

    if not trustseal_id:
        return jsonify({"error": "trustseal_id is required"}), 400

    package = PACKAGES.get(trustseal_id)

    if not package:
        return jsonify({"error": "TrustSeal package not found"}), 404

    # Prototype-only confirmation.
    # Production should require OTP/authentication here.
    package["verified"] = True
    package["status"] = "RECEIPT CONFIRMED"

    package["events"].append({
        "time": datetime.now().strftime("%H:%M:%S"),
        "title": "Receipt confirmed",
        "detail": "Receiver verified the TrustSeal package",
    })

    return jsonify({
        "ok": True,
        "trustseal_id": trustseal_id,
        "status": package["status"],
        "message": "Receipt confirmed"
    })


if __name__ == "__main__":
    # host=0.0.0.0 allows an ESP32 on the same LAN to reach Flask.
    app.run(host="0.0.0.0", port=5000, debug=True)
