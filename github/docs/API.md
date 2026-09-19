# TrustSeal API

## GET `/api/status`

Used by the ESP32 to determine whether monitoring is active.

Query:

```text
/api/status?device_id=TS-ESP32-001
```

Response:

```json
{
  "device_id": "TS-ESP32-001",
  "active": true,
  "tampered": false
}
```

## POST `/api/sensor-data`

Used by the ESP32 to upload sensor state.

Example:

```json
{
  "device_id": "TS-ESP32-001",
  "current_time": "15:03:19",
  "is_active": true,
  "tamper_score": 65,
  "is_tampered": true,
  "initial_tamper_time": "15:03:19",
  "sensor": {
    "ldr": 2450,
    "motion": 14.2,
    "tamper_switch": true,
    "second_switch": false
  }
}
```

## POST `/api/device/<device_id>/activate`

Demo endpoint to activate a device.

## POST `/api/device/<device_id>/deactivate`

Demo endpoint to deactivate a device.

## GET `/api/package/<trustseal_id>`

Returns the customer-facing package information.

Example:

```text
/api/package/TS-1048
```

## POST `/api/verify`

Demo receiver confirmation.

Example:

```json
{
  "trustseal_id": "TS-1048"
}
```

Production should add:
- authentication
- authorization
- HTTPS
- database persistence
- OTP or equivalent receiver verification
- replay protection
- rate limiting
- device credentials/certificates
- audit logging
