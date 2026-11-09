<div align="center">

  <h1>MGRS</h1>

  <p>
    WGS84 latitude/longitude &rarr; UTM &rarr; 10-digit MGRS conversion for Arduino / ESP32.
  </p>

<p>
  <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/ColdBore-RD/gps-mgrs-toolkit" alt="license" />
  </a>
  <img src="https://img.shields.io/badge/version-1.1.1-blue" alt="version" />
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
coverage range (see [Limitations](#limitations)).

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

`MGRS.cpp` calls `latLonToUTM()` internally, then independently derives the UTM zone and
latitude band from the input lat/lon (rather than reading them back off `UTMCoordinate`) before
computing the 100 km grid square letters and formatting the final string.

## Algorithm Notes

- **Projection:** standard WGS84 ellipsoid Transverse Mercator, scale factor `k0 = 0.9996`,
  false easting `500,000 m`, false northing `10,000,000 m` in the southern hemisphere.
- **UTM zone exceptions:** the library applies the standard irregular zone boundaries around
  Norway (zone 32V) and Svalbard (zones 31X/33X/35X/37X).
- **Latitude bands:** `C` through `X`, skipping `I` and `O` to avoid confusion with `1` and `0`.
- **100 km grid square letters:** the easting letter set cycles through 3 alternating
  8-letter sequences based on `zone % 3`; the northing letter set cycles through a 20-letter
  sequence, offset for even-numbered zones and for the southern hemisphere.
- **Precision:** 10-digit MGRS (5-digit easting + 5-digit northing) — 1 meter precision.

## Limitations

- Valid latitude range is **-80.0 to 84.0 degrees**. Outside this range the grid system switches
  to Universal Polar Stereographic (UPS), which is not implemented.
- No MGRS &rarr; lat/lon reverse conversion.
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
