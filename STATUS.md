# STATUS

**Status:** complete (v1.0); public-release prep in progress
**Category:** electronics
**Next step:** publish as a fresh repo with one squashed commit, then flip to public
**Updated:** 2026-11-08

## Handoff notes
- 2026-11-08 (issue #1): fixed the southern-hemisphere 100 km row letter in `MGRS.cpp` (library 1.1.1), added `tests/host` (PC regression test, 10 vectors cross-checked against an independent implementation), scrubbed broker IP / internal account names / a real GPS fix from the READMEs, fixed old-repo-name links.
- Before going public: fresh squashed repo (decided 2026-11-08), regional test point swapped for Baltimore, website link removed until there is a public page.
- Working `secrets.h` files and `MQTT_Dashboard_Handoff.md` are local only (gitignored); copy them in before flashing the MQTT sketches.
- Two sketch folders still carry versions in their names (`..._v2`, `TFT_Touch_Test_ESP32S3_v1.0`). D1 covers repo names only, so they were left alone.
