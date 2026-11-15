# STATUS

**Status:** done (v1.1.2, public)
**Category:** electronics
**Next step:** none
**Updated:** 2026-11-15

## Handoff notes
- 2026-11-15: v1.1.2 released (PR #5, tag `v1.1.2`, GitHub release). It fixes the defects external testing found in 1.1.1 (patch 0001 of that report, re-implemented): Norway/Svalbard points are re-projected in the exception zone (was 70-374 km off), NaN/inf inputs return false (NaN latitude crashed), lat 84 gets the Svalbard zones, lon 180 gives zone 60. `tests/host` has the GeographicLib vectors and fails/segfaults on 1.1.1. Left open: N2 (patch 0002, near-equator nanometre strip), the v1 KitchenSink sketch's embedded math (NaN-unsafe checks and lat-84 edge; it already re-projects), integration findings I1-I6 in the sketches.
- 2026-11-08 (issue #1): fixed the southern-hemisphere 100 km row letter in `MGRS.cpp` (library 1.1.1), added `tests/host` (PC regression test, 10 vectors cross-checked against an independent implementation), scrubbed broker IP / internal account names / a real GPS fix from the READMEs, fixed old-repo-name links.
- Public since 2026-11-08 as a fresh repo with one squashed commit (tag `v1.1.1`). The old private history is in `ColdBore-RD/gps-mgrs-toolkit-archive` (archived). Never push the old commits or the local `v1.0.0` tag here. License badge is static (the GitHub proxy cached "repo not found").
- Working `secrets.h` files and `MQTT_Dashboard_Handoff.md` are local only (gitignored); copy them in before flashing the MQTT sketches.
- Two sketch folders still carry versions in their names (`..._v2`, `TFT_Touch_Test_ESP32S3_v1.0`). D1 covers repo names only, so they were left alone.
