# STATUS

**Status:** active (library v1.1.2 released; Wi-Fi bench test sketch in review)
**Category:** electronics
**Next step:** Anand tries the v2 page on his phone outdoors, then push `agent/s3-bench-map-v2` and open its PR against `agent/s3-bench-test`
**Updated:** 2026-11-24

## Handoff notes
- 2026-11-24: new sketch `GPS_WiFi_Bench_Test` (XIAO ESP32-C3/S3): the board is its own Wi-Fi AP (`gps-bench`) and serves a phone page at `192.168.4.1`. v1 (PR #10, open): readings, MGRS, raw NMEA, plus bench record `tests/bench-xiao-esp32s3`. Verified on a XIAO ESP32-S3 + ATGM336H: GPS_Basic_Test, MGRS_Test and MGRS_Validation pass; the page showed a fix on 11 satellites outdoors on an iPhone. S3 pins are GPS TX to GPIO44, RX to GPIO43 (the C3 numbers read nothing).
- v2 is on the local branch `agent/s3-bench-map-v2` (not pushed, Anand asked to try it first): map centred on the fix (OSM tiles fetched by the phone, so the basemap needs internet on the phone), client-side MGRS grid overlay, CBRD logo, repo link. Checked in a browser with demo data only; the grid math matches the library for 8 vectors. Never run on a phone against the board.
- Hardware: the bench XIAO ESP32-S3 (USB serial `A4CB8FD1D680`, COM9 on 2026-11-24, it changes per plug-in) is flashed with the v2 build and wired to the ATGM336H.
- Open threads: C3 build untested; MQTT and display sketches not run; GPS_KitchenSink_v2 never got a fix on this board (indoors only); the page is open to anyone with the AP password (default is in the sketch).
- Photos in `sandbox\2026-11-bench-photos` (Codex-cleaned backgrounds, prune about 2026-12-24); the repo uses photo-c. Screenshots in `docs/images` are demo data at a public landmark, never real positions.
- Earlier (2026-11-15, v1.1.2 and before): see CHANGELOG.md. Left open there: N2 near-equator strip, v1 KitchenSink embedded math, integration findings I1-I6.
