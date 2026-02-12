# ESP32 Communication Protocol

This document outlines the JSON protocol used for communication between the Road-E ESP32 firmware and the web interface via WebSocket.

## 1. ESP32 -> Website (Sensor Data)

The ESP32 should send a JSON object containing any combination of the following fields. Partial updates are allowed.

| Field       | Type   | Description                       | Example                            |
| :---------- | :----- | :-------------------------------- | :--------------------------------- |
| `lat`       | Number | GPS Latitude                      | `32.085300`                        |
| `lng`       | Number | GPS Longitude                     | `34.781800`                        |
| `temp`      | Number | Temperature in Celsius            | `25.4`                             |
| `hum`       | Number | Humidity in %                     | `60.5`                             |
| `streamUrl` | String | URL for the camera stream (MJPEG) | `"http://192.168.1.100:81/stream"` |

**Example JSON Payload:**

```json
{
  "lat": 32.0853,
  "lng": 34.7818,
  "temp": 25.4,
  "hum": 60.5
}
```

## 2. Website -> ESP32 (Control Commands)

When the user interacts with the joystick/buttons, the website sends the following JSON commands to the ESP32.

| Field     | Type   | Description                     |
| :-------- | :----- | :------------------------------ |
| `type`    | String | Message type. Always `"drive"`. |
| `command` | String | The specific action code.       |

### Command Codes

| Code | Action                            |
| :--- | :-------------------------------- |
| `F`  | **Forward**: Move robot forward   |
| `B`  | **Backward**: Move robot backward |
| `L`  | **Left**: Turn left               |
| `R`  | **Right**: Turn right             |
| `S`  | **Stop**: Stop all motors         |

**Example JSON Payload:**

```json
{
  "type": "drive",
  "command": "F"
}
```
