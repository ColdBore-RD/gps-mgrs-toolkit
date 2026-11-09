# Waveshare ESP32-S3-Zero with 2.4-inch SPI TFT and XPT2046 Touchscreen

## Project status

- **TFT display:** Wired and confirmed working.
- **Touch controller:** Identified as XPT2046.
- **Touchscreen wiring:** Defined below but not yet tested.
- **Display resolution:** 240 × 320 pixels; 320 × 240 in landscape orientation.
- **Logic voltage:** 3.3 V.

This reference documents a Waveshare ESP32-S3-Zero connected to a common 2.4-inch TFT SPI V1.3 module using an ILI9341 display controller and XPT2046 resistive-touch controller.

## Safety and electrical notes

1. Disconnect USB power before changing wiring.
2. The ESP32-S3 GPIO pins use 3.3 V logic. Never apply 5 V to a GPIO pin.
3. The tested TFT module was powered through its `VCC` input from the board's `5V` pin. This is appropriate only for a module with an onboard regulator and a `VCC` input rated for 5 V.
4. The TFT backlight `LED`/`BL` connection uses `3V3`.
5. All devices must share a common ground.

## Confirmed TFT wiring

| TFT pin | ESP32-S3-Zero | Function |
|---|---:|---|
| `VCC` | `5V` | Display-module power |
| `GND` | `GND` | Common ground |
| `CS` | GPIO10 | TFT chip select |
| `RESET` / `RST` | GPIO8 | TFT reset |
| `DC` / `D/C` | GPIO9 | Data/command selection |
| `SDI` / `MOSI` | GPIO11 | SPI data from ESP32 |
| `SCK` / `CLK` | GPIO12 | SPI clock |
| `LED` / `BL` | `3V3` | Backlight power |
| `SDO` / `MISO` | GPIO13 | SPI data to ESP32 |

## XPT2046 touchscreen wiring

The XPT2046 shares the TFT's SPI clock, MOSI, and MISO lines. It has a separate chip-select line and an optional interrupt line.

| Touch pin | ESP32-S3-Zero | Function |
|---|---:|---|
| `T_CLK` | GPIO12 | Shared SPI clock |
| `T_CS` | GPIO7 | Touch-controller chip select |
| `T_DIN` | GPIO11 | Shared SPI MOSI |
| `T_DO` | GPIO13 | Shared SPI MISO |
| `T_IRQ` | GPIO6 | Touch interrupt |

### Shared SPI connections

- GPIO11 connects to both TFT `SDI/MOSI` and touch `T_DIN`.
- GPIO12 connects to both TFT `SCK/CLK` and touch `T_CLK`.
- GPIO13 connects to both TFT `SDO/MISO` and touch `T_DO`.
- TFT `CS` remains on GPIO10.
- Touch `T_CS` uses GPIO7.
- Touch `T_IRQ` uses GPIO6.

## Complete GPIO assignment

```cpp
#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_MISO  13
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8
#define TOUCH_CS   7
#define TOUCH_IRQ  6
```

## Required Arduino libraries

Install these through the Arduino IDE Library Manager:

- Adafruit GFX Library
- Adafruit ILI9341
- XPT2046_Touchscreen by Paul Stoffregen

## Confirmed TFT test sketch

The following basic display configuration worked with the documented wiring:

```cpp
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_MISO  13
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8

Adafruit_ILI9341 tft(&SPI, TFT_DC, TFT_CS, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.fillRect(0, 0, 320, 50, ILI9341_BLUE);
  tft.setCursor(12, 14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.println("Cold Bore");

  tft.setCursor(18, 85);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.println("TFT TEST");

  tft.setCursor(18, 135);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println("ESP32-S3-Zero");

  tft.drawRect(10, 185, 300, 40, ILI9341_RED);
  Serial.println("ILI9341 display initialized");
}

void loop() {
}
```

## Combined display and touchscreen test sketch

The following sketch is ready for the first XPT2046 test. The touchscreen portion has not yet been verified on this hardware.

```cpp
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_MISO  13
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8
#define TOUCH_CS   7
#define TOUCH_IRQ  6

Adafruit_ILI9341 tft(&SPI, TFT_DC, TFT_CS, TFT_RST);
XPT2046_Touchscreen touchscreen(TOUCH_CS, TOUCH_IRQ);

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(20, 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Touch the screen");

  if (!touchscreen.begin()) {
    tft.setCursor(20, 60);
    tft.setTextColor(ILI9341_RED);
    tft.println("Touch failed");
    Serial.println("Touch controller not detected");
    return;
  }

  touchscreen.setRotation(1);

  tft.setCursor(20, 60);
  tft.setTextColor(ILI9341_GREEN);
  tft.println("Touch ready");
  Serial.println("Touchscreen initialized");
}

void loop() {
  if (touchscreen.touched()) {
    TS_Point point = touchscreen.getPoint();

    Serial.print("Raw X: ");
    Serial.print(point.x);
    Serial.print("  Raw Y: ");
    Serial.print(point.y);
    Serial.print("  Pressure: ");
    Serial.println(point.z);

    delay(50);
  }
}
```

## First touchscreen test procedure

1. Complete the five XPT2046 connections with USB power disconnected.
2. Recheck the shared SPI lines and ensure the TFT and touch chip-select wires are not reversed.
3. Install the `XPT2046_Touchscreen` library.
4. Upload the combined test sketch.
5. Open Serial Monitor at 115200 baud.
6. Touch the center and each corner of the display.
7. Verify that raw X, raw Y, and pressure values change with each touch.
8. Record the approximate minimum and maximum X/Y values for later calibration.

## Calibration note

XPT2046 readings are raw touch-controller values, not screen pixel coordinates. After testing, determine the raw minimum and maximum values and map them to the display dimensions. Landscape rotation 1 normally uses a 320 × 240 coordinate space, but the axes may need to be swapped or inverted depending on the panel orientation.

Example mapping structure:

```cpp
int screenX = map(rawX, RAW_X_MIN, RAW_X_MAX, 0, 319);
int screenY = map(rawY, RAW_Y_MIN, RAW_Y_MAX, 0, 239);

screenX = constrain(screenX, 0, 319);
screenY = constrain(screenY, 0, 239);
```

Do not finalize these constants until actual corner readings have been collected.

## Troubleshooting

### TFT works but no touch is detected

- Confirm the XPT2046 IC is physically populated on the module.
- Confirm `T_CS` is connected to GPIO7.
- Confirm `T_CLK`, `T_DIN`, and `T_DO` share the correct display SPI lines.
- Confirm the library is using the same `SPI` bus initialized by `SPI.begin(...)`.
- Temporarily omit `TOUCH_IRQ` and construct the controller as `XPT2046_Touchscreen touchscreen(TOUCH_CS);` to test polling without the interrupt line.

### Touch values appear but coordinates are wrong

- This is expected before calibration.
- Test all four corners and record raw readings.
- Swap raw X and Y if movement is on the wrong axis.
- Reverse the mapping endpoints if an axis moves backward.
- Ensure the touchscreen and TFT use the same rotation setting.

### Display becomes unstable after connecting touch

- Check for swapped `T_DIN` and `T_DO` connections.
- Verify that TFT `CS` and touch `T_CS` are on separate GPIOs.
- Check for loose ground connections.
- Use short jumper wires where practical.
- If necessary, lower the TFT SPI initialization speed, for example: `tft.begin(10000000);`.

## Future expansion

Once touch input is confirmed and calibrated, this hardware can support buttons, menus, page navigation, sensor dashboards, data logging controls, and other interactive interfaces. If the display module also includes a microSD slot, it can share the same SPI bus with its own separate chip-select pin.
