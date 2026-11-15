<div align="center">

  <h1>MGRS</h1>

  <p>
    WGS84 latitude/longitude &rarr; UTM &rarr; 10-digit MGRS conversion for Arduino / ESP32.
  </p>

<p>
  <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-MIT-blue" alt="license: MIT" />
  </a>
  <img src="https://img.shields.io/badge/version-1.1.2-blue" alt="version" />
</p>

</div>

<br />

# Table of Contents

- [About](#about)
- [Installation](#installation)
- [API Reference](#api-reference)
  * [latLonToMGRS()](#latlontomgrs)
  * [MGRSCoordinate](#mgrscoordinate)
  * [latLonToUTM()](#latlontoutm)
  * [UTMCoordinate](#utmcoordinate)
- [Architecture](#architecture)
- [Algorithm Notes](#algorithm-notes)
- [Limitations](#limitations)
- [Example](#example)
- [Validated Output](#validated-output)
- [License](#license)
- [Contact](#contact)

## About

`MGRS` is a lightweight, dependency-free Arduino library that converts a WGS84
latitude/longitude pair into a
[Military Grid Reference System](https://en.wikipedia.org/wiki/Military_Grid_Reference_System)
(MGRS) coordinate string, via an internal WGS84 &rarr; UTM Transverse Mercator projection. It
targets embedded platforms (developed and tested on the Seeed XIAO ESP32-C3) and has no
dependencies beyond `Arduino.h`, `<math.h>`, `<string.h>`, and `<stdio.h>`.

Output is a standard 10-digit MGRS string at 1-meter precision, e.g. `18SUJ2339407395`.

## Installation

Copy (or symlink) this folder into your Arduino libraries directory as `MGRS`:

```bash
cp -r MGRS "$HOME/Documents/Arduino/libraries/MGRS"
```

Then, in a sketch:

```cpp
#include <MGRS.h>
```

`arduino-cli` picks up the library automatically once it's in the libraries path; no
`library.properties` changes are required.

## API Reference

### `latLonToMGRS()`

```cpp
bool latLonToMGRS(
    double latitude,
    double longitude,
    MGRSCoordinate &output
);
```

Converts a WGS84 latitude/longitude pair to MGRS, writing the result into `output`.

| Parameter | Type | Description |
|---|---|---|
| `latitude` | `double` | WGS84 latitude, valid range **-80.0 to 84.0** degrees |
| `longitude` | `double` | WGS84 longitude, valid range **-180.0 to 180.0** degrees |
| `output` | `MGRSCoordinate&` | Populated on success |

**Returns:** `true` on success, `false` if `latitude`/`longitude` are outside the valid MGRS/UTM
coverage range or are NaN/infinite (see [Limitations](#limitations)).

### `MGRSCoordinate`

```cpp
struct MGRSCoordinate
{
    char mgrs[20];        // Complete MGRS string, e.g. "18SUJ2339407395"
    int zone;              // UTM zone number
    char band;              // Latitude band letter (C-X, excluding I and O)
    char square[3];        // 100 km grid square ID (2 letters + null terminator)
    char easting[6];        // 5-digit easting string, zero-padded
    char northing[6];       // 5-digit northing string, zero-padded
    double utmEasting;      // Raw UTM easting in meters
    double utmNorthing;     // Raw UTM northing in meters
};
```

### `latLonToUTM()`

```cpp
bool latLonToUTM(
    double latitude,
    double longitude,
    UTMCoordinate &output
);
```

Converts a WGS84 latitude/longitude pair directly to UTM, without the MGRS grid-square
lettering step. Used internally by `latLonToMGRS()`, and available directly if only the raw
projection is needed. Same valid range and return semantics as `latLonToMGRS()`.

Since 1.1.2, `zone` is the MGRS zone, including the Norway and Svalbard exceptions, and
`easting`/`northing` are projected on that zone's central meridian. (In 1.1.1 `latLonToUTM()`
always used the plain 6° zone; callers that read the raw UTM values in those strips will see
different numbers after upgrading.)

### `utmZoneForLatLon()`

```cpp
int utmZoneForLatLon(double latitude, double longitude);
```

Returns the UTM zone (1–60) with the Norway/Svalbard exceptions applied; longitude +180 maps to
zone 60. Does no range checking: call it with values `latLonToUTM()` would accept.

### `UTMCoordinate`

```cpp
struct UTMCoordinate
{
    int zone;
    char hemisphere;   // 'N' or 'S'
    double easting;
    double northing;
};
```

## Architecture

| File | Responsibility |
|---|---|
| `src/GeoConstants.h` | WGS84 ellipsoid constants (semi-major axis, flattening, UTM scale factor, false easting/northing) |
| `src/UTM.h` / `src/UTM.cpp` | `latLonToUTM()` — WGS84 &rarr; Transverse Mercator projection |
| `src/MGRS.h` / `src/MGRS.cpp` | `latLonToMGRS()` — UTM zone/band/grid-square lettering on top of `UTM.cpp`'s projection, and final MGRS string assembly |

`MGRS.cpp` calls `latLonToUTM()` internally and takes the zone from `UTMCoordinate`, so the zone
in the string is always the zone the easting/northing were projected in. It derives the latitude
band from the input latitude, then computes the 100 km grid square letters and formats the final
string.

## Algorithm Notes

- **Projection:** standard WGS84 ellipsoid Transverse Mercator, scale factor `k0 = 0.9996`,
  false easting `500,000 m`, false northing `10,000,000 m` in the southern hemisphere.
- **UTM zone exceptions:** the library applies the standard irregular zone boundaries around
  Norway (zone 32V: 56–64N, 3–12E) and Svalbard (zones 31X/33X/35X/37X: 72–84N, 0–42E) and
  projects on the exception zone's central meridian, so points there can sit up to 6° from it
  (raw UTM error rises from under 0.96 mm to about 1.07 mm).
- **Latitude bands:** `C` through `X`, skipping `I` and `O` to avoid confusion with `1` and `0`.
- **100 km grid square letters:** the easting letter set cycles through 3 alternating
  8-letter sequences based on `zone % 3`; the northing letter set cycles through a 20-letter
  sequence, offset by 5 for even-numbered zones (no extra southern-hemisphere offset: the
  10,000,000 m false northing is a whole number of cycles).
- **Precision:** 10-digit MGRS (5-digit easting + 5-digit northing) — 1 meter precision.

## Limitations

- Valid latitude range is **-80.0 to 84.0 degrees**. Outside this range the grid system switches
  to Universal Polar Stereographic (UPS), which is not implemented.
- No MGRS &rarr; lat/lon reverse conversion.
- 1 m (10-digit) precision only; there is no precision parameter. Truncate the digit strings for
  coarser grids.
- Latitude 84.0 exactly is accepted and lettered as band X (UTM-extended convention); some
  decoders treat 84N as UPS. Longitude +180 is mapped to zone 60 (some tools wrap it to zone 01).
- Latitudes in roughly [-8.4e-15, 0) get hemisphere `S` with northing exactly 10,000,000 m and the
  wrong 100 km row letter. The strip is about a nanometre wide and unreachable from a GPS fix
  (TinyGPSPlus resolves 1e-9°); not fixed in 1.1.2.
- Needs 64-bit `double` and at least 32-bit `int`. ESP32, RP2040, SAMD and desktop builds are fine;
  classic AVR (Uno, Nano, Mega: 32-bit `double`, 16-bit `int`) gives wrong results even though
  `library.properties` says `architectures=*`.
- The library trusts its input. It has no plausibility check on what the NMEA parser hands it,
  so a malformed but checksum-valid sentence becomes a confident, wrong MGRS string; gate on fix
  status and fix age in the sketch.
- `MGRSCoordinate.mgrs` is a fixed 20-byte buffer; this comfortably fits the standard
  `ZZBLL EEEEE NNNNN` format with no truncation risk under normal input.

## Example

```cpp
#include <MGRS.h>

MGRSCoordinate position;

void setup()
{
  Serial.begin(115200);
  delay(3000);

  bool success = latLonToMGRS(38.8977, -77.0365, position);

  if (success)
  {
    Serial.print("MGRS: ");
    Serial.println(position.mgrs);      // 18SUJ2339407395

    Serial.print("Zone: ");
    Serial.println(position.zone);      // 18

    Serial.print("Band: ");
    Serial.println(position.band);      // S

    Serial.print("Square: ");
    Serial.println(position.square);    // UJ
  }
}

void loop() {}
```

See also the [`MGRS_Test`](../MGRS_Test/README.md) and
[`MGRS_Validation`](../MGRS_Validation/README.md) sketches for full working examples,
[`UTM_Test`](../UTM_Test/README.md) for a single-point field dump including raw UTM values, and
[`GPS_KitchenSink_FullFunctionTest_ESP32C3_v2`](../GPS_KitchenSink_FullFunctionTest_ESP32C3_v2/README.md)
for a live GPS integration example.

## Validated Output

These control points are used to confirm `latLonToMGRS()` output against known-correct MGRS
strings (see [`MGRS_Validation/`](../MGRS_Validation/README.md)):

| Location | Latitude | Longitude | Expected MGRS |
|---|---|---|---|
| White House | 38.8977 | -77.0365 | `18SUJ2339407395` |
| Baltimore, MD | 39.2904 | -76.6122 | `18SUJ6096550242` |
| Denver, CO | 39.7392 | -104.9903 | `13SED0083198811` |
| San Francisco, CA | 37.7749 | -122.4194 | `10SEG5113080998` |
| Anchorage, AK | 61.2181 | -149.9003 | `06VUN4424790536` |

## License

Distributed under the MIT License. See [`LICENSE`](../../LICENSE) for more information.

## Contact

Cold Bore Research & Development

- GitHub: [github.com/ColdBore-RD](https://github.com/ColdBore-RD/)
- Instagram: [@coldbore_rd](https://www.instagram.com/coldbore_rd/)
- Email: [ColdBoreRD@proton.me](mailto:ColdBoreRD@proton.me)
