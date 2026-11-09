# Changelog

All notable changes. Format: [Keep a Changelog](https://keepachangelog.com/); versions: SemVer.

## [Unreleased]

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
