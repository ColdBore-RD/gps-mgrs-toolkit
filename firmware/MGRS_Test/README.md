# MGRS_Test

Multi-point desk test for the [`MGRS`](../MGRS/README.md) library, exercising 5 globally
distributed reference points and printing the full MGRS breakdown (string, zone, band, grid
square) for each.

## Purpose

`MGRS_Test` sweeps points across both hemispheres and multiple UTM zones — Washington D.C. area,
London, Tokyo, and Sydney — to confirm the library produces structurally correct output
(a valid zone number, a valid latitude band letter, a two-letter grid square) across a broad
geographic spread, not just the single control point used in [`UTM_Test`](../UTM_Test/README.md).

## Hardware

No GPS module required — this is an algorithm-only test. Any ESP32-C3 (or other supported board)
with a USB serial connection is sufficient to run the sketch and read the output.

## Test Points

| Location | Latitude | Longitude |
|---|---|---|
| White House | 38.8977 | -77.0365 |
| Baltimore, MD | 39.2904 | -76.6122 |
| London, UK | 51.5074 | -0.1278 |
| Tokyo, Japan | 35.6762 | 139.6503 |
| Sydney, Australia | -33.8688 | 151.2093 |

## Operation

```bash
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3 MGRS_Test
arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 MGRS_Test
arduino-cli monitor -p COM3 -c baudrate=115200
```

For each point, the sketch prints:

```
White House
MGRS: 18SUJ2339407395
UTM Zone: 18
Band: S
Square: UJ
```

## Validation Results

The two North American points overlap with the fixed-precision values documented in
[`MGRS_Validation`](../MGRS_Validation/README.md) and can be checked directly:

| Location | Expected MGRS |
|---|---|
| White House | `18SUJ2339407395` |
| Baltimore, MD | `18SUJ6096550242` |

For London, Tokyo, and Sydney, this sketch validates structural correctness (a valid zone, a
non-`Z` latitude band, and a resolvable grid square letter pair) rather than an exact
character-for-character reference string — a `FAILED` result on any point indicates the
coordinate fell outside the library's valid range or that grid-square lettering resolved to `?`,
both of which point to a regression rather than expected behavior for these inputs.
