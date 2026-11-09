# GPS_MQTT_Display_ESP32S3

Waveshare ESP32-S3-Zero + 2.4" ILI9341 SPI TFT — a read-only MQTT subscriber that renders the
JSON diagnostic payload published by
[`GPS_MQTT_Diagnostic_ESP32C3`](../GPS_MQTT_Diagnostic_ESP32C3/README.md) on-screen. No GPS
module attached to this board — it's purely a physical dashboard for the data the other sketch
puts on the broker.

## Purpose

`GPS_MQTT_Diagnostic_ESP32C3` proved GPS + MQTT works, viewable via Node-RED. This sketch is a
second, physical way to view the same feed — useful in the field where a laptop/Node-RED isn't
handy, and as a second independent consumer to sanity-check the broker/topic setup.

## Hardware

- Waveshare ESP32-S3-Zero
- 2.4" SPI TFT, ILI9341 controller, 240x320 (320x240 landscape)
- XPT2046 resistive touch controller is wired per the reference doc below but **not used** by
  this sketch — it's a passive display, not an input device. See "Future expansion" below.

Full wiring, pin table, and hardware notes:
[`ESP32-S3-Zero_ILI9341_XPT2046_Reference.md`](ESP32-S3-Zero_ILI9341_XPT2046_Reference.md).

| TFT pin | ESP32-S3-Zero |
|---|---:|
| CS | GPIO10 |
| RESET/RST | GPIO8 |
| DC/D-C | GPIO9 |
| SDI/MOSI | GPIO11 |
| SCK/CLK | GPIO12 |
| SDO/MISO | GPIO13 |
| VCC | 5V |
| LED/BL | 3V3 |
| GND | GND |

## MQTT

| | |
|---|---|
| Subscribes | `sensors/gps-tracker/diagnostic` (data), `sensors/gps-tracker/status` (LWT online/offline) |
| Broker | Same broker as the publisher (authenticated) |
| Credentials | **Not** the publisher's `gps-tracker` account — that's scoped for publish. This sketch reads, so it needs its own account (`gps-display` in `secrets.h`, scoped read-only to `sensors/gps-tracker/#`). Provision it on the broker before flashing. |

## Installation

1. Required libraries (install with `arduino-cli lib install`): `Adafruit GFX
   Library`, `Adafruit ILI9341`, `PubSubClient`, `ArduinoJson` (v7).
2. Copy `secrets.h.example` to `secrets.h` and fill in Wi-Fi SSID/password and the `gps-display` MQTT credentials.
   `secrets.h` is covered by the project `.gitignore` — never committed.

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc GPS_MQTT_Display_ESP32S3
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc GPS_MQTT_Display_ESP32S3
arduino-cli monitor -p COM3 -c baudrate=115200
```

## Operation

On boot, the screen shows a header, four connectivity chips, and "Waiting for MQTT data..."
until the first message arrives:

```
┌──────────────────────────────────────────┐
│  GPS MQTT DIAGNOSTIC                      │
├────────┬────────┬────────┬────────────────┤
│  WIFI  │  MQTT  │  DEV   │  DATA          │  <- green/red chips
├────────┴────────┴────────┴────────────────┤
│ FIX:         YES                          │
│ LAT:         38.897700                    │
│ LON:         -77.036500                   │
│ MGRS:        18SUJ2339407395              │
│ ALT (m):     165.2                        │
│ SPEED (mph): 0.37                         │
│ COURSE (deg): 0.00                        │
│ SATS:        4                            │
│ HDOP:        2.42                         │
│ DATE (UTC):  2026-07-19                   │
│ TIME (UTC):  21:55:25                     │
│ NMEA CHARS:  20391 / 32 bad               │
│ FIX SENT:    6                            │
│ GPS UPTIME:  283s                         │
│ LAST UPDATE: 1s ago                       │
└──────────────────────────────────────────┘
```

The four status chips are independent signals:

- **WIFI** — this display's own Wi-Fi connection
- **MQTT** — this display's own broker connection
- **DEV** — whether the GPS publisher's LWT says it's `online`
- **DATA** — whether a diagnostic message has arrived in the last 10 seconds (turns red if the
  feed goes stale even though WIFI/MQTT/DEV all look fine)

The whole dashboard redraws once per second driven by a local timer, independent of message
arrival, so `LAST UPDATE` counts up accurately even between publishes.

## Design Notes

- No LWT is registered for this device — unlike the publisher, nothing downstream depends on
  knowing whether the display itself is alive, so that complexity was left out.
- Touch (XPT2046) is wired but intentionally unused in this first version — the ask was a
  read-only diagnostic view, not an interactive UI. The reference doc's "Future expansion"
  section covers what touch could add later (paging between screens, etc.) if needed.

## Build Verification

Compiled clean (arduino-cli, `esp32:esp32:esp32s3:CDCOnBoot=cdc`):

```
Sketch uses 940293 bytes (71%) of program storage space. Maximum is 1310720 bytes.
Global variables use 47444 bytes (14%) of dynamic memory, leaving 280236 bytes for local variables. Maximum is 327680 bytes.
```

Flashed to a Waveshare ESP32-S3-Zero (COM9) and confirmed working on hardware (2026-07-19):
connects to Wi-Fi, connects to the broker as `gps-display`, subscribes to both topics, and
renders live GPS diagnostic data on the ILI9341 TFT — confirming the full pipeline end-to-end
against the `GPS_MQTT_Diagnostic_ESP32C3` publisher.
