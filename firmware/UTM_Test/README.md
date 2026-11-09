# UTM_Test

Single-point desk test for the [`MGRS`](../MGRS/README.md) library, printing every field of the
conversion — MGRS string, zone, band, grid square, and raw UTM easting/northing — for one known
control point.

## Purpose

Where [`MGRS_Test`](../MGRS_Test/README.md) and [`MGRS_Validation`](../MGRS_Validation/README.md)
sweep multiple global points, `UTM_Test` focuses on a single fixed coordinate (the White House)
and dumps the complete `MGRSCoordinate` struct, including the raw UTM easting/northing values
that the other sketches don't print. It's the fastest way to sanity-check the library after a
code change, without needing GPS hardware attached.

## Hardware

No GPS module required — this is an algorithm-only test. Any ESP32-C3 (or other supported board)
with a USB serial connection is sufficient to run the sketch and read the output.

## Operation

The sketch calls `latLonToMGRS()` once in `setup()` for the fixed coordinate `38.8977, -77.0365`
(White House) and prints every field:

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 UTM_Test
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 UTM_Test
arduino-cli monitor -p COM3 -c baudrate=115200
```

Expected serial output:

```
MGRS Conversion Test
--------------------
SUCCESS
MGRS: 18SUJ2339407395
Zone: 18
Band: S
Square: UJ
Easting: 39407
Northing: 07395
Raw Easting: 323940.xxx
Raw Northing: 4307395.xxx
```

(Raw UTM easting/northing are printed to 3 decimal places; exact fractional digits depend on
floating-point rounding and are not asserted here — only the MGRS string and integer grid fields
are treated as validated output.)

## Validation Results

| Field | Expected |
|---|---|
| MGRS | `18SUJ2339407395` |
| Zone | `18` |
| Band | `S` |
| Square | `UJ` |

A `FAILED` result would indicate the input coordinate fell outside the library's valid range
(latitude -80 to 84, longitude -180 to 180) — which should never happen for this fixed control
point, so a failure here means a regression in [`MGRS.cpp`](../MGRS/src/MGRS.cpp) or
[`UTM.cpp`](../MGRS/src/UTM.cpp).
