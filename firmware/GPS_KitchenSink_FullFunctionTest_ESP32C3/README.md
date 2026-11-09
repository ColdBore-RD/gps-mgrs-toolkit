# GPS_KitchenSink_FullFunctionTest_ESP32C3

Full-featured GPS status dump on the Seeed XIAO ESP32-C3: satellites, position, altitude, speed,
course, UTC date/time, HDOP, link diagnostics, and a live MGRS conversion of the current fix.

> **v2 available:** [`GPS_KitchenSink_FullFunctionTest_ESP32C3_v2`](../GPS_KitchenSink_FullFunctionTest_ESP32C3_v2/README.md)
> swaps the embedded MGRS conversion described below for the [`MGRS`](../MGRS/README.md) library.
> Everything else is identical. v2 is the recommended version going forward; this v1 sketch is
> kept as the original zero-dependency reference implementation.

## Purpose

This is the "everything at once" field test sketch — once
[`GPS_Basic_Test_ESP32C3`](../GPS_Basic_Test_ESP32C3/README.md) confirms the UART link is good,
this sketch exercises the full `TinyGPSPlus` API and converts each fix to an MGRS coordinate in
real time, printing a complete status block once per second.

## Hardware

- Seeed XIAO ESP32-C3
- Any NMEA-0183 GPS module (9600 baud UART output)

| GPS Module Pin | XIAO ESP32-C3 Pin |
|---|---|
| TX | GPIO20 / D7 |
| RX | GPIO21 / D6 |
| GND | GND |
| VCC | Per GPS module's supply voltage requirement |

## Operation

1. Flash the sketch and open a serial monitor at 115200 baud (the sketch waits up to 5 seconds
   for the USB CDC monitor to attach before starting).
2. Once per second, a status block is printed:

```
------------------------------------------
GPS Status : FIXED
Satellites : 8
Latitude   : 38.897700
Longitude  : -77.036500
Fix age    : 120 ms
MGRS       : 18S UJ 23394 07395
Altitude   : 45.2 m / 148.3 ft
Speed      : 0.10 mph / 0.16 km/h
Course     : 182.30 degrees S
UTC Date   : 07/18/2026
UTC Time   : 14:32:07.00
HDOP       : 0.90
Characters : 48213
Passed NMEA: 812
Failed NMEA: 0
Fix sentences: 203
------------------------------------------
```

Until a fix is acquired, each field prints `Waiting for fix` (or `SEARCHING` for GPS status), and
a `WARNING: No GPS serial data detected.` message appears every 5 seconds if no NMEA data is
arriving at all.

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 GPS_KitchenSink_FullFunctionTest_ESP32C3
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 GPS_KitchenSink_FullFunctionTest_ESP32C3
arduino-cli monitor -p COM3 -c baudrate=115200
```

## Note on MGRS Conversion

This sketch does **not** depend on the [`MGRS`](../MGRS/README.md) library — it embeds its own
self-contained `latLonToMGRS()` implementation so the `.ino` file has zero external
dependencies beyond `TinyGPSPlus`. The embedded implementation uses the same WGS84 Transverse
Mercator math and MGRS grid-square lettering as the library, but formats its output with spaces
(`18S UJ 23394 07395`) instead of the library's compact form (`18SUJ2339407395`). The two
implementations are algorithmically equivalent; the library's implementation is the one that
should be treated as the source of truth going forward (see [`MGRS/README.md`](../MGRS/README.md)).
See [`GPS_KitchenSink_FullFunctionTest_ESP32C3_v2`](../GPS_KitchenSink_FullFunctionTest_ESP32C3_v2/README.md)
for the library-based version of this sketch.

## Validation Results

Because this sketch depends on a live GPS fix, its MGRS output can't be checked against a fixed
expected value the way the desk-test sketches can. Correctness of the conversion math itself is
validated independently, with known-good control points, in
[`MGRS_Test`](../MGRS_Test/README.md) and [`MGRS_Validation`](../MGRS_Validation/README.md). In
the field, confirm this sketch's `MGRS` line is consistent with the printed `Latitude`/`Longitude`
by cross-checking against a known reference (e.g. a second GPS device, or a mapping tool that
displays MGRS).
