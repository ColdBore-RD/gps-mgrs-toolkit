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

## 2026-11-24 - Wi-Fi bench test is a self-hosted AP page, not router Wi-Fi or BLE
Options considered: join the home router (needs credentials in secrets.h), BLE UART, board as its own access point serving a page. Chosen: own AP. Why: no credentials, works outdoors with a power bank and any phone; the page is the start of a test method offered in a later release. Consequence: the phone has no internet from the board, so the v2 map tiles come from the phone's mobile data; offline the grid and readings still work.

## 2026-11-24 - Client-side MGRS grid, library string stays authoritative
Options considered: a map library plus plugin, a grid drawn by the board, a small WGS84-UTM grid drawn in the page. Chosen: in-page grid (about 40 lines of UTM math, no dependencies). Why: no CDN scripts, matches the library vectors. Consequence: it ignores the Norway/Svalbard zone exceptions and stops beyond 84N/80S; the MGRS string from the board's library is the value to trust.
