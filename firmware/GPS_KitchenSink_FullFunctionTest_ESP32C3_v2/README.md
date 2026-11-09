# GPS_KitchenSink_FullFunctionTest_ESP32C3_v2

Same full-featured GPS status dump as
[`GPS_KitchenSink_FullFunctionTest_ESP32C3`](../GPS_KitchenSink_FullFunctionTest_ESP32C3/README.md),
with the MGRS conversion switched over to the [`MGRS`](../MGRS/README.md) Arduino library instead
of an embedded implementation.

## Purpose

v1 of this sketch carried its own self-contained WGS84 &rarr; UTM &rarr; MGRS conversion so it
had zero dependencies beyond `TinyGPSPlus`. v2 replaces that embedded math with
`#include <MGRS.h>` and a call to `latLonToMGRS()`, so the live GPS field test now exercises the
same library code validated in [`MGRS_Test`](../MGRS_Test/README.md),
[`MGRS_Validation`](../MGRS_Validation/README.md), and [`UTM_Test`](../UTM_Test/README.md) —
one conversion implementation used everywhere instead of two that have to be kept in sync.
Everything else (satellites, position, altitude, speed, course, UTC date/time, HDOP, link
diagnostics) is unchanged from v1.

## Hardware

- Seeed XIAO ESP32-C3
- Any NMEA-0183 GPS module (9600 baud UART output)

| GPS Module Pin | XIAO ESP32-C3 Pin |
|---|---|
| TX | GPIO20 / D7 |
| RX | GPIO21 / D6 |
| GND | GND |
| VCC | Per GPS module's supply voltage requirement |

## Installation

This sketch requires the [`MGRS`](../MGRS/README.md) library to be installed (see the library's
README), or compiled with `arduino-cli`'s `--library` flag pointed at the `MGRS/` folder:

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 --library "../MGRS" GPS_KitchenSink_FullFunctionTest_ESP32C3_v2
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 --library "../MGRS" GPS_KitchenSink_FullFunctionTest_ESP32C3_v2
arduino-cli monitor -p COM3 -c baudrate=115200
```

## Operation

Identical status block to v1, once per second, except the `MGRS` line now prints the library's
compact format instead of the spaced format:

```
------------------------------------------
GPS Status : FIXED
Satellites : 8
Latitude   : 38.897700
Longitude  : -77.036500
Fix age    : 120 ms
MGRS       : 18SUJ2339407395
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

## Build Verification

Compiled clean against the `MGRS` library (arduino-cli 1.4.1, `esp32:esp32:XIAO_ESP32C3`):

```
Sketch uses 310250 bytes (23%) of program storage space. Maximum is 1310720 bytes.
Global variables use 13660 bytes (4%) of dynamic memory, leaving 314020 bytes for local variables. Maximum is 327680 bytes.
```

## Validation Results

As with v1, this sketch depends on a live GPS fix, so its MGRS output can't be checked against a
fixed expected value the way the desk-test sketches can. Correctness of the conversion math
itself is validated independently, with known-good control points, in
[`MGRS_Test`](../MGRS_Test/README.md) and [`MGRS_Validation`](../MGRS_Validation/README.md) —
since v2 calls the same library function those sketches validate, a passing result there is a
direct guarantee for this sketch's `MGRS` output as well. In the field, confirm this sketch's
`MGRS` line is consistent with the printed `Latitude`/`Longitude` by cross-checking against a
known reference.
