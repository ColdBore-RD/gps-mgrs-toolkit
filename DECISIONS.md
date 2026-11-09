# DECISIONS

Newest first.

## 2026-11-08 - completed
Went public as v1.1.1 via a fresh repo with one squashed commit; the old history (LAN IP, a real GPS fix) stays private in gps-mgrs-toolkit-archive. Southern-hemisphere MGRS row-letter bug fixed, and tests/host added, before release.

## 2026-10-22 — Library stays beside the sketches
`MGRS` lives at `firmware\MGRS` instead of a separate `libs\` folder, so the many `..\MGRS\` cross-links in the sketch READMEs keep working. Build with `--library firmware\MGRS`, or copy it into the Arduino libraries folder.

## 2026-07 — From-scratch MGRS instead of a third-party library
The conversion is implemented directly (WGS84 -> UTM -> MGRS) with no dependencies, and validated in `MGRS_Validation` against known-correct strings.
