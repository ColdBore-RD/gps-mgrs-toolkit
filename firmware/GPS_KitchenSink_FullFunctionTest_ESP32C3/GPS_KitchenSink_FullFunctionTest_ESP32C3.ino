#include <TinyGPSPlus.h>
#include <math.h>

// ============================================================
// XIAO ESP32-C3 + GPS + MGRS Full Function Test
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

bool latLonToMGRS(
  double latitude,
  double longitude,
  char *output,
  size_t outputSize
);

char latitudeBandLetter(double latitude);
int calculateUTMZone(double latitude, double longitude);

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
  Serial.println(F("XIAO ESP32-C3 GPS + MGRS Test"));
  Serial.println(F("=========================================="));

  Serial.print(F("TinyGPSPlus version: "));
  Serial.println(TinyGPSPlus::libraryVersion());

  Serial.println(F("GPS RX: GPIO20 / XIAO D7"));
  Serial.println(F("GPS TX: GPIO21 / XIAO D6"));
  Serial.println(F("GPS baud: 9600"));
  Serial.println(F("MGRS precision: 1 meter"));
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

  char mgrsCoordinate[32];

  bool conversionSuccessful = latLonToMGRS(
    gps.location.lat(),
    gps.location.lng(),
    mgrsCoordinate,
    sizeof(mgrsCoordinate)
  );

  if (conversionSuccessful)
  {
    Serial.println(mgrsCoordinate);
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

// ============================================================
// LATITUDE/LONGITUDE TO MGRS CONVERSION
// ============================================================

bool latLonToMGRS(
  double latitude,
  double longitude,
  char *output,
  size_t outputSize
)
{
  // Standard UTM/MGRS latitude coverage.
  if (
    latitude < -80.0 ||
    latitude > 84.0 ||
    longitude < -180.0 ||
    longitude > 180.0
  )
  {
    return false;
  }

  const double WGS84_A = 6378137.0;
  const double WGS84_ECC_SQUARED = 0.00669437999014;
  const double UTM_SCALE_FACTOR = 0.9996;

  const double kDegToRad = PI / 180.0;

  int zoneNumber = calculateUTMZone(latitude, longitude);

  // Longitude at center of the UTM zone.
  double zoneCentralMeridian =
    (zoneNumber - 1) * 6.0 - 180.0 + 3.0;

  double latitudeRadians = latitude * kDegToRad;
  double longitudeRadians = longitude * kDegToRad;
  double centralMeridianRadians =
    zoneCentralMeridian * kDegToRad;

  double eccentricityPrimeSquared =
    WGS84_ECC_SQUARED / (1.0 - WGS84_ECC_SQUARED);

  double sinLatitude = sin(latitudeRadians);
  double cosLatitude = cos(latitudeRadians);
  double tanLatitude = tan(latitudeRadians);

  double N =
    WGS84_A /
    sqrt(
      1.0 -
      WGS84_ECC_SQUARED *
      sinLatitude *
      sinLatitude
    );

  double T = tanLatitude * tanLatitude;

  double C =
    eccentricityPrimeSquared *
    cosLatitude *
    cosLatitude;

  double A =
    cosLatitude *
    (longitudeRadians - centralMeridianRadians);

  double eccentricity2 = WGS84_ECC_SQUARED;
  double eccentricity4 = eccentricity2 * eccentricity2;
  double eccentricity6 = eccentricity4 * eccentricity2;

  double meridionalArc =
    WGS84_A *
    (
      (
        1.0 -
        eccentricity2 / 4.0 -
        3.0 * eccentricity4 / 64.0 -
        5.0 * eccentricity6 / 256.0
      ) *
      latitudeRadians

      -

      (
        3.0 * eccentricity2 / 8.0 +
        3.0 * eccentricity4 / 32.0 +
        45.0 * eccentricity6 / 1024.0
      ) *
      sin(2.0 * latitudeRadians)

      +

      (
        15.0 * eccentricity4 / 256.0 +
        45.0 * eccentricity6 / 1024.0
      ) *
      sin(4.0 * latitudeRadians)

      -

      (
        35.0 * eccentricity6 / 3072.0
      ) *
      sin(6.0 * latitudeRadians)
    );

  double A2 = A * A;
  double A3 = A2 * A;
  double A4 = A3 * A;
  double A5 = A4 * A;
  double A6 = A5 * A;

  double easting =
    UTM_SCALE_FACTOR *
    N *
    (
      A +
      (1.0 - T + C) * A3 / 6.0 +
      (
        5.0 -
        18.0 * T +
        T * T +
        72.0 * C -
        58.0 * eccentricityPrimeSquared
      ) *
      A5 / 120.0
    ) +
    500000.0;

  double northing =
    UTM_SCALE_FACTOR *
    (
      meridionalArc +
      N *
      tanLatitude *
      (
        A2 / 2.0 +
        (
          5.0 -
          T +
          9.0 * C +
          4.0 * C * C
        ) *
        A4 / 24.0 +
        (
          61.0 -
          58.0 * T +
          T * T +
          600.0 * C -
          330.0 * eccentricityPrimeSquared
        ) *
        A6 / 720.0
      )
    );

  // Southern hemisphere uses a 10,000,000-meter false northing.
  if (latitude < 0.0)
  {
    northing += 10000000.0;
  }

  char latitudeBand = latitudeBandLetter(latitude);

  // MGRS 100 km easting letter sets repeat every three zones.
  static const char eastingLetterSets[3][9] =
  {
    "ABCDEFGH",
    "JKLMNPQR",
    "STUVWXYZ"
  };

  // MGRS northing letters repeat every 2,000 km.
  static const char northingLetters[] =
    "ABCDEFGHJKLMNPQRSTUV";

  int eastingColumn = (int)floor(easting / 100000.0);

  if (eastingColumn < 1)
  {
    eastingColumn = 1;
  }

  if (eastingColumn > 8)
  {
    eastingColumn = 8;
  }

  int eastingSetIndex = (zoneNumber - 1) % 3;

  char eastingLetter =
    eastingLetterSets[eastingSetIndex][eastingColumn - 1];

  int northingRow =
    ((int)floor(northing / 100000.0)) % 20;

  // Even-numbered zones begin their northing sequence at F.
  if ((zoneNumber % 2) == 0)
  {
    northingRow = (northingRow + 5) % 20;
  }

  char northingLetter = northingLetters[northingRow];

  // Remove the 100 km grid portion.
  long eastingRemainder =
    (long)floor(fmod(easting, 100000.0));

  long northingRemainder =
    (long)floor(fmod(northing, 100000.0));

  if (eastingRemainder < 0)
  {
    eastingRemainder += 100000;
  }

  if (northingRemainder < 0)
  {
    northingRemainder += 100000;
  }

  // Output example:
  // 18S UJ 12345 67890
  snprintf(
    output,
    outputSize,
    "%02d%c %c%c %05ld %05ld",
    zoneNumber,
    latitudeBand,
    eastingLetter,
    northingLetter,
    eastingRemainder,
    northingRemainder
  );

  return true;
}

// ============================================================
// UTM ZONE CALCULATION
// ============================================================

int calculateUTMZone(double latitude, double longitude)
{
  int zoneNumber =
    (int)floor((longitude + 180.0) / 6.0) + 1;

  // Handle longitude exactly at 180 degrees.
  if (zoneNumber > 60)
  {
    zoneNumber = 60;
  }

  // Norway special zone.
  if (
    latitude >= 56.0 &&
    latitude < 64.0 &&
    longitude >= 3.0 &&
    longitude < 12.0
  )
  {
    zoneNumber = 32;
  }

  // Svalbard special zones.
  if (latitude >= 72.0 && latitude < 84.0)
  {
    if (longitude >= 0.0 && longitude < 9.0)
    {
      zoneNumber = 31;
    }
    else if (longitude >= 9.0 && longitude < 21.0)
    {
      zoneNumber = 33;
    }
    else if (longitude >= 21.0 && longitude < 33.0)
    {
      zoneNumber = 35;
    }
    else if (longitude >= 33.0 && longitude < 42.0)
    {
      zoneNumber = 37;
    }
  }

  return zoneNumber;
}

// ============================================================
// MGRS LATITUDE BAND
// ============================================================

char latitudeBandLetter(double latitude)
{
  static const char latitudeBands[] =
    "CDEFGHJKLMNPQRSTUVWXX";

  if (latitude < -80.0 || latitude > 84.0)
  {
    return 'Z';
  }

  int bandIndex =
    (int)floor((latitude + 80.0) / 8.0);

  if (bandIndex < 0)
  {
    bandIndex = 0;
  }

  if (bandIndex > 20)
  {
    bandIndex = 20;
  }

  return latitudeBands[bandIndex];
}
