# DECISIONS

Newest first.

## 2026-10-22 — Library stays beside the sketches
`MGRS` lives at `firmware\MGRS` instead of a separate `libs\` folder, so the many `..\MGRS\` cross-links in the sketch READMEs keep working. Build with `--library firmware\MGRS`, or copy it into the Arduino libraries folder.

## 2026-07 — From-scratch MGRS instead of a third-party library
The conversion is implemented directly (WGS84 -> UTM -> MGRS) with no dependencies, and validated in `MGRS_Validation` against known-correct strings.
