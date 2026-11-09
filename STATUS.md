# STATUS

**Status:** done (v1.1.1, public)
**Category:** electronics
**Next step:** none
**Updated:** 2026-11-08

## Handoff notes
- 2026-11-08 (issue #1): fixed the southern-hemisphere 100 km row letter in `MGRS.cpp` (library 1.1.1), added `tests/host` (PC regression test, 10 vectors cross-checked against an independent implementation), scrubbed broker IP / internal account names / a real GPS fix from the READMEs, fixed old-repo-name links.
- Public since 2026-11-08 as a fresh repo with one squashed commit (tag `v1.1.1`). The old private history is in `ColdBore-RD/gps-mgrs-toolkit-archive` (archived). Never push the old commits or the local `v1.0.0` tag here. License badge is static (the GitHub proxy cached "repo not found").
- Working `secrets.h` files and `MQTT_Dashboard_Handoff.md` are local only (gitignored); copy them in before flashing the MQTT sketches.
- Two sketch folders still carry versions in their names (`..._v2`, `TFT_Touch_Test_ESP32S3_v1.0`). D1 covers repo names only, so they were left alone.
