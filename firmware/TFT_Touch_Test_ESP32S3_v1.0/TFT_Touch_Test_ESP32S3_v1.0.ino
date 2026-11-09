#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// ============================================================
// Waveshare ESP32-S3-Zero + ILI9341 TFT + XPT2046 Touch — Wiring
// and capability test.
//
// Wiring: ESP32-S3-Zero_ILI9341_XPT2046_Reference.md in
// ../GPS_MQTT_Display_ESP32S3 (this board's TFT is unused by that
// project's sketch — this test exercises the touch controller for
// the first time on this hardware).
//
// Prints raw XPT2046 X/Y/Z to Serial, shows the same values plus a
// touch trail on the TFT itself, and tracks the min/max raw values
// seen so far for later calibration (see the reference doc's
// "Calibration note"). Also prints a GPIO availability summary at
// boot.
//
// Arduino Serial Monitor: 115200 baud
// ============================================================

// ── TFT pins (ESP32-S3-Zero_ILI9341_XPT2046_Reference.md) ──────────────────
#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_MISO  13
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8
#define TOUCH_CS   7
#define TOUCH_IRQ  6

// ── Raw touch calibration ───────────────────────────────────────────────
// No hardcoded raw min/max here — the panel's raw range, axis orientation,
// and any physical rotation between the touch overlay and the display vary
// per unit and can't be guessed safely. Instead, a 3-point wizard runs once
// at boot (see runCalibrationStep()) and derives a full 2D affine transform
// (calA..calF, see computeCalibration()) from three touches you provide.

static const int SCREEN_W = 320;
static const int SCREEN_H = 240;

static const int HEADER_H   = 24;
static const int STATUS_TOP = HEADER_H;
static const int STATUS_H   = 92;
static const int TRAIL_TOP  = STATUS_TOP + STATUS_H;
static const int LINE_HEIGHT = 12;
static const int LEFT_X      = 6;

static const int CLR_BTN_X = 256, CLR_BTN_Y = 2, CLR_BTN_W = 60, CLR_BTN_H = 20;

// ── Calibration wizard targets ───────────────────────────────────────────
static const int CAL_MARGIN = 24;
static const int CAL_TL_X = CAL_MARGIN,           CAL_TL_Y = CAL_MARGIN;
static const int CAL_TR_X = SCREEN_W - 1 - CAL_MARGIN, CAL_TR_Y = CAL_MARGIN;
static const int CAL_BL_X = CAL_MARGIN,           CAL_BL_Y = SCREEN_H - 1 - CAL_MARGIN;

Adafruit_ILI9341 tft(&SPI, TFT_DC, TFT_CS, TFT_RST);

// Constructed WITHOUT TOUCH_IRQ (T_IRQ wiring on this module reads noisy/
// floating and isn't needed): reads a real pressure value over SPI instead
// of trusting the IRQ line.
XPT2046_Touchscreen touchscreen(TOUCH_CS);

// The library's own touched() uses a hardcoded Z threshold (400) that's too
// low for this panel — idle/no-touch Z sits around 1150-1300 here, so
// touched() was permanently true even with nothing touching the screen.
// Confirmed via raw telemetry: idle z tops out ~1300. Finger touches read
// 1900-3400+, but a stylus (small contact area) reads much lower for the
// same physical press — as low as ~1600 near the panel's true edges, where
// resistive panels are also naturally less sensitive. 1400 trades some
// noise margin (idle ceiling ~1300) for better stylus/edge reach.
static const int Z_PRESS_THRESHOLD = 1400;

bool touchOk = false;

// Reads one point and reports whether it's a real press by our own
// threshold, replacing touchscreen.touched()/getPoint() pairs everywhere.
bool readTouch(TS_Point &p)
{
  p = touchscreen.getPoint();
  return p.z > Z_PRESS_THRESHOLD;
}

// As a finger presses down or lifts, Z ramps through Z_PRESS_THRESHOLD and
// can noisily dip back across it for a few ms mid-transition. Without
// debouncing, the calibration wizard saw that dip as a genuine release and
// advanced to the next corner while the finger was still on the previous
// one (confirmed: TL and TR raw values came back nearly identical). This
// requires the pressed/released state to hold stable for DEBOUNCE_MS before
// anything acts on it.
static const unsigned long DEBOUNCE_MS = 150;
bool debouncedPressed = false;
bool rawPressedPrev = false;
unsigned long pressChangeTime = 0;
TS_Point currentPoint;

bool updateTouchState()
{
  TS_Point p;
  bool rawPressedNow = touchOk && readTouch(p);
  if (rawPressedNow) currentPoint = p;

  if (rawPressedNow != rawPressedPrev)
  {
    pressChangeTime = millis();
    rawPressedPrev = rawPressedNow;
  }
  if (millis() - pressChangeTime >= DEBOUNCE_MS)
  {
    debouncedPressed = rawPressedNow;
  }
  return debouncedPressed;
}

bool haveSample = false;
int  lastRawX = 0, lastRawY = 0, lastRawZ = 0;
int  lastMappedX = -1, lastMappedY = -1;
int  rawXSeenMin = 4095, rawXSeenMax = 0;
int  rawYSeenMin = 4095, rawYSeenMax = 0;
unsigned long lastStatusDraw = 0;
unsigned long lastTouchPrint = 0;

// Calibration state
enum CalState { CAL_TL, CAL_TL_WAIT_RELEASE, CAL_TR, CAL_TR_WAIT_RELEASE, CAL_BL, CAL_BL_WAIT_RELEASE, CAL_DONE };
CalState calState = CAL_TL;
bool calPromptDrawn = false;

// A single sample right as a corner touch is confirmed can still be
// mid-settle (finger still rolling into full contact) — confirmed by a
// TL->TR calibration delta that moved heavily on BOTH raw axes when it
// should only move on one. Corner capture now requires holding the touch
// for CAL_SETTLE_MS and averages every sample seen during that hold.
static const unsigned long CAL_SETTLE_MS = 300;
bool calCapturing = false;
unsigned long calCaptureArmTime = 0;
long calSumX = 0, calSumY = 0;
int calSampleCount = 0;

int rawTLx, rawTLy, rawTRx, rawTRy, rawBLx, rawBLy;

// Full 2D affine fit (screenX = calA*rawX + calB*rawY + calC, screenY =
// calD*rawX + calE*rawY + calF) instead of independent per-axis linear
// maps. A photographed touch trail showed straight left-right/up-down
// finger sweeps coming out as diagonal crossed lines on screen — proof the
// touch overlay sits at a slight physical rotation relative to the
// display, which a swap+per-axis-scale model can't correct but a full
// affine fit (all 3 calibration points solved together) can. Standard
// "3-point calibration" technique (Vidales, Embedded Systems Programming,
// 2002 / TI SLYT277).
float calA = 1, calB = 0, calC = 0;
float calD = 0, calE = 1, calF = 0;

void printGpioAudit();
void drawHeader();
void drawClearButton();
void drawStatus();
void clearTrailArea();
bool hitClearButton(int x, int y);
void drawRow(int y, const char* label, const String& value);
void runCalibrationStep(bool pressed);
void drawCalTarget(const char* label, int x, int y);
void computeCalibration();

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("=== TFT + XPT2046 Touch Test — Waveshare ESP32-S3-Zero ==="));
  printGpioAudit();

  // Constructor no longer manages TOUCH_IRQ, so read it manually purely as
  // a diagnostic signal (a real XPT2046 pulls it low on contact).
  pinMode(TOUCH_IRQ, INPUT);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  drawHeader();

  touchOk = touchscreen.begin();
  if (touchOk)
  {
    Serial.println(F("[Touch] XPT2046 initialized — starting 3-point calibration wizard"));
    Serial.println(F("[Cal] Touch the crosshair at each prompt: TOP-LEFT, TOP-RIGHT, BOTTOM-LEFT"));
  }
  else
  {
    Serial.println(F("[Touch] XPT2046 NOT detected (begin() returned false) — check T_CS/T_CLK/T_DIN/T_DO wiring"));
    calState = CAL_DONE; // nothing to calibrate; go straight to dashboard
    drawStatus();
  }
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
  bool pressed = updateTouchState();

  if (calState != CAL_DONE)
  {
    runCalibrationStep(pressed);
    return;
  }

  if (touchOk && pressed)
  {
    const TS_Point &p = currentPoint;
    lastRawX = p.x;
    lastRawY = p.y;
    lastRawZ = p.z;
    haveSample = true;

    if (p.x < rawXSeenMin) rawXSeenMin = p.x;
    if (p.x > rawXSeenMax) rawXSeenMax = p.x;
    if (p.y < rawYSeenMin) rawYSeenMin = p.y;
    if (p.y > rawYSeenMax) rawYSeenMax = p.y;

    int mappedX = constrain((int)lroundf(calA * p.x + calB * p.y + calC), 0, SCREEN_W - 1);
    int mappedY = constrain((int)lroundf(calD * p.x + calE * p.y + calF), 0, SCREEN_H - 1);
    lastMappedX = mappedX;
    lastMappedY = mappedY;

    if (millis() - lastTouchPrint >= 300)
    {
      lastTouchPrint = millis();
      Serial.printf("[Touch] raw(%4d,%4d) z=%4d -> screen(%3d,%3d)\n", p.x, p.y, p.z, mappedX, mappedY);
    }

    if (hitClearButton(mappedX, mappedY))
    {
      clearTrailArea();
    }
    else if (mappedY >= TRAIL_TOP)
    {
      tft.fillCircle(mappedX, mappedY, 2, ILI9341_YELLOW);
    }
  }

  if (millis() - lastStatusDraw >= 150)
  {
    lastStatusDraw = millis();
    drawStatus();
  }
}

// ============================================================
// CALIBRATION WIZARD
// ============================================================

void drawCalTarget(const char* label, int x, int y)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(40, 100);
  tft.print("CALIBRATION");
  tft.setCursor(40, 130);
  tft.print(label);
  tft.setTextSize(1);
  tft.setCursor(40, 155);
  tft.print("(hold still ~half a second)");

  tft.drawFastHLine(x - 8, y, 17, ILI9341_CYAN);
  tft.drawFastVLine(x, y - 8, 17, ILI9341_CYAN);
}

// Averages every sample seen while held, only "capturing" once the touch
// has stayed pressed continuously for CAL_SETTLE_MS. Returns true (with
// outX/outY filled) the moment that hold completes; releasing early resets
// the average so a bounce or a stray tap can't produce a half-formed point.
bool captureCornerSample(bool pressed, int &outX, int &outY)
{
  if (!pressed)
  {
    calCapturing = false;
    return false;
  }
  if (!calCapturing)
  {
    calCapturing = true;
    calCaptureArmTime = millis();
    calSumX = 0; calSumY = 0; calSampleCount = 0;
  }
  calSumX += currentPoint.x;
  calSumY += currentPoint.y;
  calSampleCount++;
  if (millis() - calCaptureArmTime >= CAL_SETTLE_MS)
  {
    outX = calSumX / calSampleCount;
    outY = calSumY / calSampleCount;
    calCapturing = false;
    return true;
  }
  return false;
}

void runCalibrationStep(bool pressed)
{
  switch (calState)
  {
    case CAL_TL:
      if (!calPromptDrawn) { drawCalTarget("Touch TOP-LEFT", CAL_TL_X, CAL_TL_Y); calPromptDrawn = true; calCapturing = false; }
      if (captureCornerSample(pressed, rawTLx, rawTLy))
      {
        Serial.printf("[Cal] TL touch: raw(%d,%d) avg of %d samples\n", rawTLx, rawTLy, calSampleCount);
        calState = CAL_TL_WAIT_RELEASE;
      }
      break;

    case CAL_TL_WAIT_RELEASE:
      if (!pressed) { calState = CAL_TR; calPromptDrawn = false; }
      break;

    case CAL_TR:
      if (!calPromptDrawn) { drawCalTarget("Touch TOP-RIGHT", CAL_TR_X, CAL_TR_Y); calPromptDrawn = true; calCapturing = false; }
      if (captureCornerSample(pressed, rawTRx, rawTRy))
      {
        Serial.printf("[Cal] TR touch: raw(%d,%d) avg of %d samples\n", rawTRx, rawTRy, calSampleCount);
        calState = CAL_TR_WAIT_RELEASE;
      }
      break;

    case CAL_TR_WAIT_RELEASE:
      if (!pressed) { calState = CAL_BL; calPromptDrawn = false; }
      break;

    case CAL_BL:
      if (!calPromptDrawn) { drawCalTarget("Touch BOTTOM-LEFT", CAL_BL_X, CAL_BL_Y); calPromptDrawn = true; calCapturing = false; }
      if (captureCornerSample(pressed, rawBLx, rawBLy))
      {
        Serial.printf("[Cal] BL touch: raw(%d,%d) avg of %d samples\n", rawBLx, rawBLy, calSampleCount);
        calState = CAL_BL_WAIT_RELEASE;
      }
      break;

    case CAL_BL_WAIT_RELEASE:
      if (!pressed)
      {
        computeCalibration();
        calState = CAL_DONE;
        tft.fillScreen(ILI9341_BLACK);
        drawHeader();
        drawStatus();
      }
      break;

    default:
      break;
  }
}

// Standard 3-point affine touchscreen calibration (Vidales 2002 / TI
// SLYT277): solves screenX = A*rawX + B*rawY + C and
// screenY = D*rawX + E*rawY + F directly from the three (raw -> screen)
// point pairs, so swap, scale, offset, and any small rotation/shear between
// the touch overlay and the display are all corrected together in one fit.
void computeCalibration()
{
  float x0 = rawTLx, y0 = rawTLy, X0 = CAL_TL_X, Y0 = CAL_TL_Y;
  float x1 = rawTRx, y1 = rawTRy, X1 = CAL_TR_X, Y1 = CAL_TR_Y;
  float x2 = rawBLx, y2 = rawBLy, X2 = CAL_BL_X, Y2 = CAL_BL_Y;

  float delta = (x0 - x2) * (y1 - y2) - (x1 - x2) * (y0 - y2);

  calA = ((X0 - X2) * (y1 - y2) - (X1 - X2) * (y0 - y2)) / delta;
  calB = ((x0 - x2) * (X1 - X2) - (x1 - x2) * (X0 - X2)) / delta;
  calC = X2 - calA * x2 - calB * y2;

  calD = ((Y0 - Y2) * (y1 - y2) - (Y1 - Y2) * (y0 - y2)) / delta;
  calE = ((x0 - x2) * (Y1 - Y2) - (x1 - x2) * (Y0 - Y2)) / delta;
  calF = Y2 - calD * x2 - calE * y2;

  Serial.println(F("[Cal] Calibration complete:"));
  Serial.printf("[Cal]   screenX = %.5f*rawX + %.5f*rawY + %.2f\n", calA, calB, calC);
  Serial.printf("[Cal]   screenY = %.5f*rawX + %.5f*rawY + %.2f\n", calD, calE, calF);
}

// ============================================================
// GPIO AUDIT (printed once at boot)
// ============================================================

void printGpioAudit()
{
  Serial.println(F("--- Waveshare ESP32-S3-Zero GPIO audit ---"));
  Serial.println(F("Header-broken-out pins (15 total): GP1 GP2 GP3 GP4 GP5 GP6 GP7"));
  Serial.println(F("  GP8 GP9 GP10 GP11 GP12 GP13 TX(43) RX(44)"));
  Serial.println(F("Used by this TFT+touch wiring (8): GP6 GP7 GP8 GP9 GP10 GP11 GP12 GP13"));
  Serial.println(F("Free at header (7): GP1 GP2 GP3 GP4 GP5 TX(43) RX(44)"));
  Serial.println(F("  Caution: GP3 is a strapping pin — avoid driving it externally at boot."));
  Serial.println(F("Not header-exposed: GPIO0 (BOOT button, strapping),"));
  Serial.println(F("  GPIO21 (onboard WS2812 RGB LED) — internal only, not available for wiring."));
  Serial.println(F("------------------------------------------"));
}

// ============================================================
// DISPLAY
// ============================================================

void drawHeader()
{
  tft.fillRect(0, 0, SCREEN_W, HEADER_H, ILI9341_BLUE);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(6, 4);
  tft.print("TFT+TOUCH TEST");
  drawClearButton();
}

void drawClearButton()
{
  tft.fillRect(CLR_BTN_X, CLR_BTN_Y, CLR_BTN_W, CLR_BTN_H, ILI9341_RED);
  tft.drawRect(CLR_BTN_X, CLR_BTN_Y, CLR_BTN_W, CLR_BTN_H, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(CLR_BTN_X + 12, CLR_BTN_Y + 6);
  tft.print("CLR");
}

bool hitClearButton(int x, int y)
{
  return x >= CLR_BTN_X && x <= (CLR_BTN_X + CLR_BTN_W) &&
         y >= CLR_BTN_Y && y <= (CLR_BTN_Y + CLR_BTN_H);
}

void clearTrailArea()
{
  tft.fillRect(0, TRAIL_TOP, SCREEN_W, SCREEN_H - TRAIL_TOP, ILI9341_BLACK);
}

void drawRow(int y, const char* label, const String& value)
{
  char line[40];
  snprintf(line, sizeof(line), "%-16s%s", label, value.c_str());
  tft.setCursor(LEFT_X, y);
  tft.print(line);
}

void drawStatus()
{
  tft.fillRect(0, STATUS_TOP, SCREEN_W, STATUS_H, ILI9341_BLACK);
  tft.setTextSize(1);

  int y = STATUS_TOP + 4;

  tft.setTextColor(touchOk ? ILI9341_GREEN : ILI9341_RED);
  drawRow(y, "TOUCH CTRL:", touchOk ? "OK" : "NOT DETECTED");
  y += LINE_HEIGHT;

  tft.setTextColor(ILI9341_WHITE);
  drawRow(y, "RAW X:", haveSample ? String(lastRawX) : "--");
  y += LINE_HEIGHT;
  drawRow(y, "RAW Y:", haveSample ? String(lastRawY) : "--");
  y += LINE_HEIGHT;
  drawRow(y, "PRESSURE:", haveSample ? String(lastRawZ) : "--");
  y += LINE_HEIGHT;
  drawRow(y, "MAPPED X,Y:", haveSample ? (String(lastMappedX) + "," + String(lastMappedY)) : "--");
  y += LINE_HEIGHT;
  drawRow(y, "RAW X MIN/MAX:", haveSample ? (String(rawXSeenMin) + " / " + String(rawXSeenMax)) : "--");
  y += LINE_HEIGHT;
  drawRow(y, "RAW Y MIN/MAX:", haveSample ? (String(rawYSeenMin) + " / " + String(rawYSeenMax)) : "--");
}
