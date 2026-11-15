# DECISIONS

Newest first.

## 2026-11-15 - v1.1.2 scope: patch 0001 only
Shipped the fixes of the external report's patch 0001 (F1 exception-zone re-projection, F4/N1 NaN, F2 lat 84, F3 lon 180), re-implemented because the patch files were not in the bundle. Patch 0002 (N2, northing 1e7 for lat in [-8.4e-15, 0)) left out: low severity, unreachable from a GPS fix; listed as a limitation. lon 180 maps to zone 60 (the report's choice; wrapping to 01 was the alternative). `latLonToUTM` now returns the exception zone so its raw E/N match the MGRS string.

## 2026-11-08 - completed
Went public as v1.1.1 via a fresh repo with one squashed commit; the old history (LAN IP, a real GPS fix) stays private in gps-mgrs-toolkit-archive. Southern-hemisphere MGRS row-letter bug fixed, and tests/host added, before release.

## 2026-10-22 — Library stays beside the sketches
`MGRS` lives at `firmware\MGRS` instead of a separate `libs\` folder, so the many `..\MGRS\` cross-links in the sketch READMEs keep working. Build with `--library firmware\MGRS`, or copy it into the Arduino libraries folder.

## 2026-07 — From-scratch MGRS instead of a third-party library
The conversion is implemented directly (WGS84 -> UTM -> MGRS) with no dependencies, and validated in `MGRS_Validation` against known-correct strings.
