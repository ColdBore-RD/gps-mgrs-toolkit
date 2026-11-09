# gps-mgrs-toolkit

XIAO ESP32-C3 GPS platform with a from-scratch, dependency-free WGS84 -> UTM -> MGRS library (`MGRS`), validation sketches, an MQTT diagnostic publisher and an ESP32-S3-Zero TFT dashboard. **Public repo** (MIT).

## Layout
- `firmware\MGRS\` the Arduino library (`src\MGRS.*`, `src\UTM.*`, `src\GeoConstants.h`). It sits beside the sketches so the READMEs' `..\MGRS\` links work.
- `firmware\<sketch>\` one folder per sketch (folder name = `.ino` name): GPS_Basic_Test, GPS_KitchenSink (embedded MGRS) and _v2 (uses the library), MGRS_Test, MGRS_Validation, UTM_Test, GPS_MQTT_Diagnostic_ESP32C3, GPS_MQTT_Display_ESP32S3, TFT_Touch_Test_ESP32S3_v1.0.
- `docs\pinouts\` XIAO ESP32-C3 pinout.
- `GPS_MQTT_Display_ESP32S3\ESP32-S3-Zero_ILI9341_XPT2046_Reference.md` is the canonical TFT/touch wiring reference; other repos link here.

## Build / flash
```powershell
arduino-cli compile --library firmware\MGRS --fqbn esp32:esp32:XIAO_ESP32C3 firmware\MGRS_Validation
arduino-cli upload -p COMx --fqbn esp32:esp32:XIAO_ESP32C3 firmware\MGRS_Validation
# display sketches: --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc
```
Libraries: TinyGPSPlus, PubSubClient (MQTT sketches), Adafruit GFX + ILI9341 + XPT2046 (display sketches).

## Rules
- `MGRS_Validation` is the regression test for the library. Run it after any change to `MGRS.cpp` / `UTM.cpp`; every reference string must still match.
- The two MQTT sketches need `secrets.h` (gitignored, never read). Broker/topic notes are in `MQTT_Dashboard_Handoff.md` (gitignored, local only).
- This repo is public. Nothing personal, no broker hostnames, no credentials.

## Hardware
GPS TX -> GPIO20/D7, GPS RX -> GPIO21/D6, 9600 baud NMEA, shared GND.
