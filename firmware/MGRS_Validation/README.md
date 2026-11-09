# MGRS_Validation

Reference-string validation test for the [`MGRS`](../MGRS/README.md) library: 8 global points,
each checked against a known-correct MGRS output string. The same vectors run on a PC, without a
board, in [`tests/host`](../../tests/host/README.md).

## Purpose

Where [`MGRS_Test`](../MGRS_Test/README.md) checks structural correctness across a spread of
points, `MGRS_Validation` is the precision check — it prints only the final MGRS string for each
point, meant to be diffed directly against externally-verified reference values. This is the
sketch to run after any change to the projection or grid-lettering math in
[`MGRS.cpp`](../MGRS/src/MGRS.cpp) or [`UTM.cpp`](../MGRS/src/UTM.cpp).

## Hardware

No GPS module required — this is an algorithm-only test. Any ESP32-C3 (or other supported board)
with a USB serial connection is sufficient to run the sketch and read the output.

## Operation

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 MGRS_Validation
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 MGRS_Validation
arduino-cli monitor -p COM3 -c baudrate=115200
```

## Validated Output

| Location | Latitude | Longitude | Expected MGRS |
|---|---|---|---|
| White House | 38.8977 | -77.0365 | `18SUJ2339407395` |
| Baltimore, MD | 39.2904 | -76.6122 | `18SUJ6096550242` |
| London, UK | 51.5074 | -0.1278 | `30UXC9931610163` |
| Tokyo, Japan | 35.6762 | 139.6503 | `54SUE7785548874` |
| Sydney, Australia | -33.8688 | 151.2093 | `56HLH3436850948` |
| Denver, CO | 39.7392 | -104.9903 | `13SED0083198811` |
| San Francisco, CA | 37.7749 | -122.4194 | `10SEG5113080998` |
| Anchorage, AK | 61.2181 | -149.9003 | `06VUN4424790536` |

All eight strings were cross-checked against an independent implementation (Krüger series);
matching every one exactly is the pass condition for a library change. Sydney is the only
southern-hemisphere point and guards the row-letter fix in 1.1.1. A `FAILED` line for any point indicates the
coordinate fell outside the library's valid latitude/longitude range, which should not occur for
any of these fixed test points.
