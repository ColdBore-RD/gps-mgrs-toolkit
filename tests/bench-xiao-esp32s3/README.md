# Bench test: XIAO ESP32-S3 + ATGM336H (2026-11-24)

The toolkit targets the XIAO ESP32-C3. These sketches were built unchanged except for the GPS pins and run on a XIAO ESP32-S3 to check they work there.

## Setup
- Board: XIAO ESP32-S3 (USB-JTAG/serial). Core `esp32:esp32` 3.3.10, FQBN `esp32:esp32:XIAO_ESP32S3`, TinyGPSPlus 1.0.3, local `MGRS` 1.1.2.
- GPS: ATGM336H, 9600 baud, VCC 3V3. **GPS TX to D7 = GPIO44 (UART0 RX), GPS RX to D6 = GPIO43.** The C3 numbers in the sketches (GPIO20/21) read nothing on the S3.
- Change on a build copy only: `RXPin`/`GPS_RX_PIN` 20 -> 44, `TXPin`/`GPS_TX_PIN` 21 -> 43. Nothing in `firmware\` was edited.
- Build: a `sketch.yaml` profile with the FQBN, `TinyGPSPlus (1.0.3)` and `dir:` pointing at `firmware\MGRS`. Serial monitor: DTR high, RTS low (RTS toggled puts the S3 into download mode).

## Results
| Sketch | Result |
|---|---|
| GPS_Basic_Test | Pass. NMEA at ~270 chars/s, `$GN` talker, `ANTENNA OK`. |
| MGRS_Test | Pass. Runs standalone, MGRS/zone/band/square printed. |
| MGRS_Validation | Pass. All 10 printed MGRS strings equal the references in `tests\host\host_test.cpp`. |
| GPS_KitchenSink_v2 | Parses NMEA (hundreds passed, 3 failed at boot); no fix indoors (0 satellites tracked, no sky view). Not re-run outdoors; the same library path was exercised by GPS_WiFi_Bench_Test below. |
| GPS_WiFi_Bench_Test | Pass. Board served the page to a phone over its own AP; outdoors it showed FIX with 11 satellites and the MGRS conversion, raw NMEA, lat/lon. |
| MQTT / display sketches | Not run (need secrets and other hardware). |

## Notes
- `CLAUDE.md` "Hardware" lists C3 pins only; the S3 pins above are the difference.
- No coordinates recorded.
