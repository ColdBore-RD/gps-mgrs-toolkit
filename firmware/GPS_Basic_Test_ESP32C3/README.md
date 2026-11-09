# GPS_Basic_Test_ESP32C3

Minimal raw-NMEA passthrough and UART wiring sanity check for a GPS module connected to a
Seeed XIAO ESP32-C3.

## Purpose

This is the first sketch to run against a new GPS module: it does no MGRS conversion and
barely touches `TinyGPSPlus` — it just proves that the GPS module is powered, wired correctly,
and actually producing NMEA sentences on the expected UART pins before moving on to the fuller
test sketches ([`GPS_KitchenSink_FullFunctionTest_ESP32C3`](../GPS_KitchenSink_FullFunctionTest_ESP32C3/README.md)).

## Hardware

- Seeed XIAO ESP32-C3
- Any NMEA-0183 GPS module (9600 baud UART output)

| GPS Module Pin | XIAO ESP32-C3 Pin |
|---|---|
| TX | GPIO20 / D7 |
| RX | GPIO21 / D6 |
| GND | GND |
| VCC | Per GPS module's supply voltage requirement |

The sketch opens a second hardware UART (`HardwareSerial(1)`) at 9600 baud on GPIO20/21, separate
from the USB-CDC `Serial` console used for debug output at 115200 baud.

## Operation

1. Flash the sketch and open a serial monitor at 115200 baud.
2. Every incoming character from the GPS module is echoed straight to the serial monitor as raw
   NMEA text (e.g. `$GNGGA`, `$GNRMC` sentences), and simultaneously fed into a `TinyGPSPlus`
   instance via `gps.encode()`.
3. Every 5 seconds, the sketch prints the running total of characters processed by
   `TinyGPSPlus`.

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 GPS_Basic_Test_ESP32C3
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 GPS_Basic_Test_ESP32C3
arduino-cli monitor -p COM3 -c baudrate=115200
```

## Validation Results

| Condition | Meaning |
|---|---|
| Raw `$GN...` sentences visible, character count increasing | Wiring and baud rate are correct |
| `Characters received: 0` (or a very low, static count) after 5+ seconds | No GPS data arriving — check power, ground, and TX/RX wiring (remember TX&rarr;RX is crossed) |

This sketch does not validate position accuracy or MGRS conversion — it only confirms the UART
link. Position/MGRS correctness is validated separately in
[`MGRS_Test`](../MGRS_Test/README.md) and [`MGRS_Validation`](../MGRS_Validation/README.md).
