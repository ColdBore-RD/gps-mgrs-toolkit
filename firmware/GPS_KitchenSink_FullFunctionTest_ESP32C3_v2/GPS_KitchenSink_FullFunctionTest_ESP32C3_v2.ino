#include <TinyGPSPlus.h>
#include <MGRS.h>

// ============================================================
// XIAO ESP32-C3 + GPS + MGRS Full Function Test (v2)
//
// Same as GPS_KitchenSink_FullFunctionTest_ESP32C3, but the
// embedded MGRS conversion has been replaced with the MGRS
// Arduino library (latLonToMGRS() / MGRSCoordinate).
//
// Wiring:
// GPS TX  -> XIAO D7 / GPIO20
// GPS RX  -> XIAO D6 / GPIO21
// GPS GND -> XIAO GND
// GPS VCC -> Appropriate supply for your GPS module
//
// Arduino Serial Monitor: 115200 baud
// GPS module baud:        9600 baud
// ============================================================

static const int GPS_RX_PIN = 20;  // XIAO D7, receives from GPS TX
static const int GPS_TX_PIN = 21;  // XIAO D6, sends to GPS RX
static const uint32_t GPS_BAUD = 9600;

static const unsigned long STATUS_INTERVAL_MS = 1000;
static const unsigned long WARNING_INTERVAL_MS = 5000;

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

unsigned long lastStatusPrint = 0;
unsigned long lastNoDataWarning = 0;

// Forward declarations
void printGPSStatus();
void printSatellites();
void printLocation();
void printMGRS();
void printAltitude();
void printSpeed();
void printCourse();
void printDate();
void printTime();
void printHDOP();
void printDiagnostics();
void printTwoDigits(int value);

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Wait up to five seconds for USB CDC Serial Monitor.
  unsigned long serialStart = millis();

  while (!Serial && millis() - serialStart < 5000)
  {
    delay(10);
  }

  // Hardware UART for GPS:
  // baud, configuration, RX pin, TX pin
  GPSSerial.begin(
    GPS_BAUD,
    SERIAL_8N1,
    GPS_RX_PIN,
    GPS_TX_PIN
  );

  Serial.println();
  Serial.println(F("=========================================="));
  Serial.println(F("XIAO ESP32-C3 GPS + MGRS Test (v2)"));
  Serial.println(F("=========================================="));

  Serial.print(F("TinyGPSPlus version: "));
  Serial.println(TinyGPSPlus::libraryVersion());

  Serial.println(F("GPS RX: GPIO20 / XIAO D7"));
  Serial.println(F("GPS TX: GPIO21 / XIAO D6"));
  Serial.println(F("GPS baud: 9600"));
  Serial.println(F("MGRS precision: 1 meter (MGRS library)"));
  Serial.println();
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
  // Continuously feed GPS characters into TinyGPSPlus.
  while (GPSSerial.available() > 0)
  {
    char incomingCharacter = GPSSerial.read();

    // Uncomment to show raw NMEA sentences.
    // Serial.write(incomingCharacter);

    gps.encode(incomingCharacter);
  }

  // Print complete GPS status once per second.
  if (millis() - lastStatusPrint >= STATUS_INTERVAL_MS)
  {
    lastStatusPrint = millis();
    printGPSStatus();
  }

  // Warn when no GPS serial data is arriving.
  if (
    millis() > 5000 &&
    gps.charsProcessed() < 10 &&
    millis() - lastNoDataWarning >= WARNING_INTERVAL_MS
  )
  {
    lastNoDataWarning = millis();

    Serial.println(F("WARNING: No GPS serial data detected."));
    Serial.println(
      F("Check power, ground, TX/RX wiring and GPS baud.")
    );
    Serial.println();
  }
}

// ============================================================
// STATUS DISPLAY
// ============================================================

void printGPSStatus()
{
  Serial.println(F("------------------------------------------"));

  Serial.print(F("GPS Status : "));

  if (gps.location.isValid())
  {
    Serial.println(F("FIXED"));
  }
  else
  {
    Serial.println(F("SEARCHING"));
  }

  printSatellites();
  printLocation();
  printMGRS();
  printAltitude();
  printSpeed();
  printCourse();
  printDate();
  printTime();
  printHDOP();
  printDiagnostics();

  Serial.println(F("------------------------------------------"));
  Serial.println();
}

void printSatellites()
{
  Serial.print(F("Satellites : "));

  if (gps.satellites.isValid())
  {
    Serial.println(gps.satellites.value());
  }
  else
  {
    Serial.println(F("Waiting"));
  }
}

void printLocation()
{
  Serial.print(F("Latitude   : "));

  if (gps.location.isValid())
  {
    Serial.println(gps.location.lat(), 6);
  }
  else
  {
    Serial.println(F("Waiting for fix"));
  }

  Serial.print(F("Longitude  : "));

  if (gps.location.isValid())
  {
    Serial.println(gps.location.lng(), 6);
  }
  else
  {
    Serial.println(F("Waiting for fix"));
  }

  Serial.print(F("Fix age    : "));

  if (gps.location.isValid())
  {
    Serial.print(gps.location.age());
    Serial.println(F(" ms"));
  }
  else
  {
    Serial.println(F("N/A"));
  }
}

void printMGRS()
{
  Serial.print(F("MGRS       : "));

  if (!gps.location.isValid())
  {
    Serial.println(F("Waiting for fix"));
    return;
  }

  MGRSCoordinate position;

  bool conversionSuccessful = latLonToMGRS(
    gps.location.lat(),
    gps.location.lng(),
    position
  );

  if (conversionSuccessful)
  {
    Serial.println(position.mgrs);
  }
  else
  {
    Serial.println(F("Outside MGRS/UTM range"));
  }
}

void printAltitude()
{
  Serial.print(F("Altitude   : "));

  if (gps.altitude.isValid())
  {
    Serial.print(gps.altitude.meters(), 1);
    Serial.print(F(" m / "));
    Serial.print(gps.altitude.feet(), 1);
    Serial.println(F(" ft"));
  }
  else
  {
    Serial.println(F("Waiting for fix"));
  }
}

void printSpeed()
{
  Serial.print(F("Speed      : "));

  if (gps.speed.isValid())
  {
    Serial.print(gps.speed.mph(), 2);
    Serial.print(F(" mph / "));
    Serial.print(gps.speed.kmph(), 2);
    Serial.println(F(" km/h"));
  }
  else
  {
    Serial.println(F("Waiting for fix"));
  }
}

void printCourse()
{
  Serial.print(F("Course     : "));

  if (gps.course.isValid())
  {
    Serial.print(gps.course.deg(), 2);
    Serial.print(F(" degrees "));
    Serial.println(TinyGPSPlus::cardinal(gps.course.deg()));
  }
  else
  {
    Serial.println(F("Waiting for fix"));
  }
}

void printDate()
{
  Serial.print(F("UTC Date   : "));

  if (gps.date.isValid())
  {
    printTwoDigits(gps.date.month());
    Serial.print('/');

    printTwoDigits(gps.date.day());
    Serial.print('/');

    Serial.println(gps.date.year());
  }
  else
  {
    Serial.println(F("Waiting for valid date"));
  }
}

void printTime()
{
  Serial.print(F("UTC Time   : "));

  if (gps.time.isValid())
  {
    printTwoDigits(gps.time.hour());
    Serial.print(':');

    printTwoDigits(gps.time.minute());
    Serial.print(':');

    printTwoDigits(gps.time.second());
    Serial.print('.');

    printTwoDigits(gps.time.centisecond());
    Serial.println();
  }
  else
  {
    Serial.println(F("Waiting for valid time"));
  }
}

void printHDOP()
{
  Serial.print(F("HDOP       : "));

  if (gps.hdop.isValid() && gps.hdop.hdop() < 99.0)
  {
    Serial.println(gps.hdop.hdop(), 2);
  }
  else
  {
    Serial.println(F("Waiting"));
  }
}

void printDiagnostics()
{
  Serial.print(F("Characters : "));
  Serial.println(gps.charsProcessed());

  Serial.print(F("Passed NMEA: "));
  Serial.println(gps.passedChecksum());

  Serial.print(F("Failed NMEA: "));
  Serial.println(gps.failedChecksum());

  Serial.print(F("Fix sentences: "));
  Serial.println(gps.sentencesWithFix());
}

void printTwoDigits(int value)
{
  if (value < 10)
  {
    Serial.print('0');
  }

  Serial.print(value);
}
