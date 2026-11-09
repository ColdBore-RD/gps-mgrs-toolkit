<div align="center">

  <img src="docs/images/cbrd-logo.png" alt="Cold Bore Research &amp; Development" width="420" />

  <h1>GPS MGRS Toolkit</h1>

  <p>
    A from-scratch, dependency-free WGS84 &rarr; UTM &rarr; MGRS library for Arduino, with GPS test
    sketches for the Seeed XIAO ESP32-C3.
  </p>

<!-- Badges -->
<p>
  <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/ColdBore-RD/gps-mgrs-toolkit" alt="license" />
  </a>
  <img src="https://img.shields.io/badge/MGRS_library-1.1.1-76FF4B" alt="MGRS library version" />
  <img src="https://img.shields.io/badge/board-XIAO_ESP32--C3-4BC92A" alt="board" />
  <img src="https://img.shields.io/badge/framework-Arduino_(esp32_3.x)-00979D" alt="framework" />
  <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/commits/main">
    <img src="https://img.shields.io/github/last-commit/ColdBore-RD/gps-mgrs-toolkit" alt="last commit" />
  </a>
  <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/issues/">
    <img src="https://img.shields.io/github/issues/ColdBore-RD/gps-mgrs-toolkit" alt="open issues" />
  </a>
  <a href="https://www.instagram.com/coldbore_rd/">
    <img src="https://img.shields.io/badge/Instagram-@coldbore__rd-E4405F?logo=instagram&amp;logoColor=white" alt="Instagram @coldbore_rd" />
  </a>
</p>

<h4>
    <a href="firmware/MGRS/README.md">Library Docs</a>
  <span> · </span>
    <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/issues/">Report Bug</a>
  <span> · </span>
    <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit/issues/">Request Feature</a>
  </h4>
</div>

<p align="center">
  <img src="docs/images/mgrs-breakdown.svg" alt="How latLonToMGRS breaks 38.8977, -77.0365 into 18 S UJ 23394 07395" width="100%" />
</p>

<br />

<!-- Table of Contents -->
# Table of Contents

- [About the Project](#about-the-project)
  * [Repository Structure](#repository-structure)
  * [Hardware](#hardware)
  * [Tech Stack](#tech-stack)
  * [Features](#features)
  * [Validated MGRS Output](#validated-mgrs-output)
  * [Testing](#testing)
- [Getting Started](#getting-started)
  * [Prerequisites](#prerequisites)
  * [Installation](#installation)
  * [Wiring](#wiring)
  * [Compiling and Flashing](#compiling-and-flashing)
- [Usage](#usage)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [FAQ](#faq)
- [License](#license)
- [Contact](#contact)
- [Acknowledgements](#acknowledgements)

<!-- About the Project -->
## About the Project

This repository is a Seeed XIAO ESP32-C3 GPS development platform. It pairs a hardware UART GPS
feed (parsed with [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus)) with **MGRS**, a
custom, dependency-free Arduino library that converts WGS84 latitude/longitude into
[Military Grid Reference System](https://en.wikipedia.org/wiki/Military_Grid_Reference_System)
(MGRS) coordinates via an intermediate UTM projection.

The repo contains the library itself plus a set of standalone test sketches used to validate GPS
wiring, UART decoding, and the UTM/MGRS math independently of one another.

<div align="center">
  <img src="docs/images/mgrs-world-grid.svg" alt="MGRS grid zones with the library's output for nine reference points" width="100%" />
</div>

<!-- Repository Structure -->
### Repository Structure

| Path | Type | Description |
|---|---|---|
| [`firmware/MGRS/`](firmware/MGRS/README.md) | Arduino library | `latLonToMGRS()` — WGS84 lat/lon &rarr; UTM &rarr; 10-digit MGRS |
| [`firmware/GPS_Basic_Test_ESP32C3/`](firmware/GPS_Basic_Test_ESP32C3/README.md) | Test sketch | Raw NMEA passthrough / UART wiring sanity check |
| [`firmware/GPS_KitchenSink_FullFunctionTest_ESP32C3/`](firmware/GPS_KitchenSink_FullFunctionTest_ESP32C3/README.md) | Test sketch | Full `TinyGPSPlus` field dump + live MGRS conversion (embedded, zero-dependency MGRS math) |
| [`firmware/GPS_KitchenSink_FullFunctionTest_ESP32C3_v2/`](firmware/GPS_KitchenSink_FullFunctionTest_ESP32C3_v2/README.md) | Test sketch | Same as above, using the [`MGRS`](firmware/MGRS/README.md) library instead of embedded math — recommended version |
| [`firmware/UTM_Test/`](firmware/UTM_Test/README.md) | Test sketch | Single-point UTM/MGRS field dump against a known control point |
| [`firmware/MGRS_Test/`](firmware/MGRS_Test/README.md) | Test sketch | MGRS conversion across 5 global reference points |
| [`firmware/MGRS_Validation/`](firmware/MGRS_Validation/README.md) | Test sketch | MGRS string output checked against known-correct values |
| [`firmware/GPS_MQTT_Diagnostic_ESP32C3/`](firmware/GPS_MQTT_Diagnostic_ESP32C3/README.md) | Test sketch | Same full GPS field set as the kitchen-sink sketches, published as JSON over MQTT for viewing in Node-RED |
| [`firmware/GPS_MQTT_Display_ESP32S3/`](firmware/GPS_MQTT_Display_ESP32S3/README.md) | Test sketch | Waveshare ESP32-S3-Zero + ILI9341 TFT — subscribes to the MQTT diagnostic feed and renders it as a physical dashboard |
| [`tests/host/`](tests/host/README.md) | Host test | Runs the `MGRS` library on a PC against 10 reference strings (no board needed) |
| `docs/pinouts/` | Reference | XIAO ESP32-C3 pinout diagram |
| `docs/images/` | Reference | Logo and README diagrams (`make_diagrams.py` regenerates the SVGs) |

<!-- Hardware -->
### Hardware

- **MCU:** Seeed Studio XIAO ESP32-C3
- **GPS:** any NMEA-0183 GPS module wired to a hardware UART (9600 baud)
- **Wiring:** GPS TX &rarr; XIAO **GPIO20 / D7**, GPS RX &rarr; XIAO **GPIO21 / D6**, shared GND

<div align="center">
  <img src="docs/pinouts/XIAO_ESP32C3.png" alt="XIAO ESP32-C3 pinout" width="500" />
</div>

<!-- TechStack -->
### Tech Stack

<details>
  <summary>Firmware</summary>
  <ul>
    <li><a href="https://www.arduino.cc/">Arduino framework</a> via <a href="https://github.com/espressif/arduino-esp32">esp32 core</a></li>
    <li><a href="https://github.com/arduino/arduino-cli">arduino-cli</a></li>
    <li>Seeed Studio XIAO ESP32-C3 (RISC-V, single core)</li>
  </ul>
</details>

<details>
  <summary>Libraries</summary>
  <ul>
    <li><a href="https://github.com/mikalhart/TinyGPSPlus">TinyGPSPlus</a> — NMEA sentence parsing</li>
    <li><code>MGRS</code> — this repo's custom WGS84 &rarr; UTM &rarr; MGRS library (no external dependencies)</li>
  </ul>
</details>

<!-- Features -->
### Features

- Custom WGS84 &rarr; UTM &rarr; MGRS conversion library with no third-party geodesy dependencies
- Correct handling of the Norway (32V) and Svalbard (31X/33X/35X/37X) UTM zone exceptions
- Standard MGRS 100 km grid square lettering (column sets cycling per zone, row letters cycling every 2,000 km)
- Live GPS-to-MGRS conversion on real hardware (`GPS_KitchenSink_FullFunctionTest_ESP32C3` / `_v2`, the latter driven directly by the `MGRS` library)
- Layered test suite: raw UART &rarr; full GPS field dump &rarr; algorithm-only UTM/MGRS checks

<!-- Validated MGRS Output -->
### Validated MGRS Output

The following control points are used across the test suite (see
[`MGRS_Validation/`](firmware/MGRS_Validation/README.md) and [`tests/host/`](tests/host/README.md))
to confirm the library's output. Every string was cross-checked against an independent
implementation:

| Location | Latitude | Longitude | Expected MGRS |
|---|---|---|---|
| White House | 38.8977 | -77.0365 | `18SUJ2339407395` |
| Baltimore, MD | 39.2904 | -76.6122 | `18SUJ6096550242` |
| Denver, CO | 39.7392 | -104.9903 | `13SED0083198811` |
| San Francisco, CA | 37.7749 | -122.4194 | `10SEG5113080998` |
| Anchorage, AK | 61.2181 | -149.9003 | `06VUN4424790536` |
| London, UK | 51.5074 | -0.1278 | `30UXC9931610163` |
| Tokyo, Japan | 35.6762 | 139.6503 | `54SUE7785548874` |
| Sydney, Australia | -33.8688 | 151.2093 | `56HLH3436850948` |
| Oslo, Norway (zone 32V exception) | 59.9139 | 10.7522 | `32VNM9797943118` |
| Svalbard (zone 33X exception) | 78.2232 | 15.6267 | `33XWG1427883355` |

<!-- Testing -->
### Testing

`tests/host` compiles the library with a desktop C++ compiler and checks all ten strings, so math
changes can be verified without flashing a board. See [`tests/host/README.md`](tests/host/README.md).

<!-- Getting Started -->
## Getting Started

<!-- Prerequisites -->
### Prerequisites

- [arduino-cli](https://arduino.github.io/arduino-cli/) or the Arduino IDE
- ESP32 board package installed (`esp32:esp32`)
- `TinyGPSPlus` library installed (Library Manager, or clone alongside this repo)
- MQTT sketches only: `PubSubClient`; display sketches: `Adafruit GFX Library`, `Adafruit ILI9341`,
  `XPT2046_Touchscreen`, `ArduinoJson`
- A GPS module capable of NMEA-0183 output at 9600 baud (only needed for the GPS sketches — the
  `MGRS_Test`, `MGRS_Validation`, and `UTM_Test` sketches are algorithm-only and need no GPS)

<!-- Installation -->
### Installation

Clone the repository:

```bash
git clone https://github.com/ColdBore-RD/gps-mgrs-toolkit.git
```

Install the `MGRS` library by copying (or symlinking) the `firmware/MGRS/` folder into your Arduino
libraries directory:

```bash
cp -r firmware/MGRS "$HOME/Documents/Arduino/libraries/MGRS"
# or skip the copy and pass it per build:
# arduino-cli compile --library firmware/MGRS --fqbn esp32:esp32:XIAO_ESP32C3 firmware/MGRS_Test
```

Install `TinyGPSPlus` through the Arduino Library Manager, or:

```bash
arduino-cli lib install TinyGPSPlus
```

<!-- Wiring -->
### Wiring

| GPS Module Pin | XIAO ESP32-C3 Pin |
|---|---|
| TX | GPIO20 / D7 |
| RX | GPIO21 / D6 |
| GND | GND |
| VCC | Per GPS module's supply voltage requirement |

<!-- Compiling and Flashing -->
### Compiling and Flashing

The ESP32-C3 sketches target the XIAO ESP32-C3 (the `*_ESP32S3` sketches use
`--fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc`). Pass `--library firmware/MGRS` if the library isn't installed:

```bash
arduino-cli compile --library firmware/MGRS --fqbn esp32:esp32:XIAO_ESP32C3 firmware/<sketch_folder>

arduino-cli board list   # find the COM port

arduino-cli compile --upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C3 firmware/<sketch_folder>

arduino-cli monitor -p COM3 -c baudrate=115200
```

<!-- Usage -->
## Usage

The `MGRS` library exposes a single conversion function that takes a WGS84 latitude/longitude
pair and fills in an `MGRSCoordinate` struct:

```cpp
#include <MGRS.h>

MGRSCoordinate position;

void setup()
{
  Serial.begin(115200);

  bool success = latLonToMGRS(38.8977, -77.0365, position);

  if (success)
  {
    Serial.println(position.mgrs);   // 18SUJ2339407395
  }
}

void loop() {}
```

See [`MGRS/README.md`](firmware/MGRS/README.md) for the full API reference.

<!-- Roadmap -->
## Roadmap

* [x] WGS84 &rarr; UTM projection
* [x] UTM &rarr; MGRS (10-digit, 1 meter precision) conversion
* [x] Norway / Svalbard UTM zone exceptions
* [x] Live GPS-to-MGRS field test sketch (embedded implementation, `GPS_KitchenSink_FullFunctionTest_ESP32C3`)
* [x] Live GPS-to-MGRS field test sketch backed by the `MGRS` library (`GPS_KitchenSink_FullFunctionTest_ESP32C3_v2`)
* [x] Multi-point MGRS string validation against known-correct output
* [ ] MGRS &rarr; lat/lon reverse conversion
* [ ] UPS (polar region, above 84N / below 80S) support

<!-- Contributing -->
## Contributing

This is a Cold Bore Research & Development project. Issues and pull requests are welcome — see
[Report Bug](https://github.com/ColdBore-RD/gps-mgrs-toolkit/issues/) or
[Request Feature](https://github.com/ColdBore-RD/gps-mgrs-toolkit/issues/) to get started.

<!-- FAQ -->
## FAQ

- **Why GPIO20/GPIO21 for the GPS UART instead of the default pins?**

  + The XIAO ESP32-C3's default `Serial0` pins are shared with the USB-CDC console used for
    debug output, so the GPS feed uses a second hardware UART (`HardwareSerial(1)`) on GPIO20/21
    (D7/D6) to keep the two streams independent.

- **Why does the MGRS library stop at latitude 80S/84N?**

  + That is the standard UTM/MGRS coverage range. Outside it, the grid system switches to the
    Universal Polar Stereographic (UPS) system, which this library does not yet implement.

<!-- License -->
## License

Distributed under the MIT License. See [`LICENSE`](LICENSE) for more information.

<!-- Contact -->
## Contact

Cold Bore Research & Development

- GitHub: [github.com/ColdBore-RD](https://github.com/ColdBore-RD/)
- Instagram: [@coldbore_rd](https://www.instagram.com/coldbore_rd/)
- Email: [ColdBoreRD@proton.me](mailto:ColdBoreRD@proton.me)

Project Link: [https://github.com/ColdBore-RD/gps-mgrs-toolkit](https://github.com/ColdBore-RD/gps-mgrs-toolkit)

<!-- Acknowledgements -->
## Acknowledgements

- [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus) by Mikal Hart
- [NGA MGRS specification](https://earth-info.nga.mil/) — Military Grid Reference System standard
- [Shields.io](https://shields.io/)
- [Awesome README](https://github.com/matiassingers/awesome-readme)
- [Louis3797/awesome-readme-template](https://github.com/Louis3797/awesome-readme-template)
