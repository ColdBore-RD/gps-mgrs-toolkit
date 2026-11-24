# STATUS

**Status:** active (library v1.1.2 released; Wi-Fi bench test sketch v2 merged)
**Category:** electronics
**Next step:** test the v2 map tiles on a phone with mobile data; then pick up the open threads below
**Updated:** 2026-11-24

## Handoff notes
- 2026-11-24: new sketch `GPS_WiFi_Bench_Test` (XIAO ESP32-C3/S3): the board is its own Wi-Fi AP (`gps-bench`) and serves a phone page at `192.168.4.1`. Readings, MGRS, raw NMEA, map centred on the fix (OSM tiles fetched by the phone), client-side MGRS grid overlay, CBRD logo, repo link. PRs #10 (v1 + bench record `tests/bench-xiao-esp32s3`) and #11 (v2) merged.
- Verified on a XIAO ESP32-S3 + ATGM336H: GPS_Basic_Test, MGRS_Test and MGRS_Validation pass; v1 showed a fix on 11 satellites outdoors. v2 was flashed to COM9 and the phone UI checked outdoors (works). The map was not tested: the phone had no cellular, and tiles need the phone's internet.
- S3 pins: GPS TX to GPIO44, RX to GPIO43 (the C3 numbers read nothing). Build: `arduino-cli compile --profile s3` in the sketch folder (new `sketch.yaml`, esp32 core 3.3.10).
- Hardware: the bench XIAO ESP32-S3 (USB serial `A4CB8FD1D680`, COM9 on 2026-11-24, changes per plug-in) is flashed with v2 and wired to the ATGM336H.
- Open threads: v2 map tiles untested; C3 build untested; MQTT and display sketches not run; GPS_KitchenSink_v2 never got a fix on this board (indoors only); the page is open to anyone with the AP password (default is in the sketch); left open in v1.1.2: N2 near-equator strip, v1 KitchenSink embedded math, integration findings I1-I6.
- Photos in `sandbox\2026-11-bench-photos` (prune about 2026-12-24); screenshots in `docs/images` are demo data, never real positions.
