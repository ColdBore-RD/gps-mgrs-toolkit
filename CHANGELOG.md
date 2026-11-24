# Changelog

All notable changes. Format: [Keep a Changelog](https://keepachangelog.com/); versions: SemVer.

## [Unreleased]
### Added
- `GPS_WiFi_Bench_Test`: field test sketch for XIAO ESP32-C3 and ESP32-S3. The board runs its own Wi-Fi access point and serves a phone page with fix state, lat/lon, MGRS, satellites, HDOP, raw NMEA, a map centered on the position with an MGRS grid overlay (basemap needs internet on the phone), the CBRD logo and a repo link, so no computer is needed outdoors.
- `tests/bench-xiao-esp32s3`: bench record for the sketches on the S3 (pins 44/43).

## [1.1.2] — 2026-11-15
Fixes the library defects found by external testing of 1.1.1 (30 M points against GeographicLib 2.5).
### Fixed
- `MGRS` 1.1.2, critical: in the Norway (32V) and Svalbard (31X/33X/35X/37X) exception strips the zone was relabelled but the point was still projected on the plain 6° zone's meridian, so the string was 70–374 km off (0.138 % of the UTM domain: western Norway 3–6E incl. Bergen and Stavanger, and Svalbard). Points are now projected in the exception zone. E.g. 60N 3E: `32VNM0000051411` -> `32VJM6564066593`.
- `MGRS` 1.1.2, high: a NaN latitude passed the range check and crashed in the latitude-band lookup. NaN and infinite latitude or longitude now return `false` from `latLonToMGRS()` and `latLonToUTM()` (a NaN longitude used to return `true` from `latLonToUTM()` with a garbage zone).
- `MGRS` 1.1.2: latitude 84.0 exactly now gets the Svalbard zones (was e.g. `32X…`, a zone that does not exist).
- `MGRS` 1.1.2: longitude +180 now gives zone 60 (was zone 61).
### Changed
- `latLonToUTM()` now returns the exception zone and projects on its meridian, so its raw easting/northing change in the Norway/Svalbard strips. New public `utmZoneForLatLon()` in `UTM.h`.
### Added
- `tests/host`: 4 GeographicLib vectors for the fixes, zone checks for both functions, and NaN/±inf rejection. `MGRS_Validation`: Norway and Svalbard re-projection points.
- MGRS README: known limitations (precision, 84N / 180E conventions, near-equator northing strip, AVR unsuitable, no input plausibility check).

## [1.1.1] — 2026-11-08
First public release (fresh repository; earlier history is not published).
### Fixed
- `MGRS` 1.1.1: southern-hemisphere points got the wrong 100 km row letter (an extra +5 offset; e.g. Sydney `56HLN…` -> `56HLH…`).
- `GPS_KitchenSink_FullFunctionTest_ESP32C3` failed to compile on esp32 core 3.x (local `DEG_TO_RAD` clashed with the Arduino macro).
### Added
- `tests/host`: PC regression test for the `MGRS` library (10 reference vectors).
- README: CBRD logo, refreshed badges, MGRS breakdown and grid-zone diagrams (`docs/images/`), testing section.
### Security
- Removed a LAN broker address, internal account names and a real GPS fix from sketch READMEs.
### Changed
- Regional test point is now Baltimore, MD (`18SUJ6096550242`); website link removed from READMEs; license badge is static.
- Sketches and the `MGRS` library moved under `firmware\`; pinout under `docs\pinouts\`. Repo renamed from `GPS_MGRS_Toolkit_ESP32C3`.
- Editor configs (`.theia\`, `.vscode\`) untracked and ignored.
- Template scaffolding: pre-commit, `.gitattributes`, `CLAUDE.md`, `STATUS.md`, `DECISIONS.md`.

## [1.0.0] — 2026-07-19
- `MGRS` library (WGS84 -> UTM -> 10-digit MGRS), validated against reference strings.
- GPS test sketches, MQTT diagnostic publisher, TFT dashboard, touch-calibration test.
