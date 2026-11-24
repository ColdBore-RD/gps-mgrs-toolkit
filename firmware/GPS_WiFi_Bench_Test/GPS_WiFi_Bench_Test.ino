// GPS + MGRS Wi-Fi bench test (XIAO ESP32-C3 / ESP32-S3)
//
// The board starts its own Wi-Fi access point and serves a status page, so a
// GPS/MGRS check can be done outdoors with only a power bank and a phone:
//   1. Join the Wi-Fi network below.
//   2. Open http://192.168.4.1
// The page shows fix state, latitude/longitude, the MGRS conversion (from the
// MGRS library), satellites, HDOP, altitude and the raw NMEA stream.
//
// Needs no router, no credentials and no secrets.h. Change AP_PASSWORD below.
// Nothing is stored: positions only exist in RAM and on the phone's screen.

#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPSPlus.h>
#include <MGRS.h>
#include "page.h"

// ---- settings ---------------------------------------------------------
static const char* AP_SSID     = "gps-bench";
static const char* AP_PASSWORD = "gpsbench24";   // at least 8 characters
static const uint32_t GPS_BAUD = 9600;

// GPS TX -> GPS_RX_PIN, GPS RX -> GPS_TX_PIN (GPIO numbers)
#if CONFIG_IDF_TARGET_ESP32S3
static const int GPS_RX_PIN = 44;   // XIAO ESP32-S3 D7
static const int GPS_TX_PIN = 43;   // XIAO ESP32-S3 D6
#else
static const int GPS_RX_PIN = 20;   // XIAO ESP32-C3 D7
static const int GPS_TX_PIN = 21;   // XIAO ESP32-C3 D6
#endif

static const uint8_t  NMEA_LINES    = 24;
static const uint8_t  NMEA_LINE_LEN = 100;
static const uint32_t FIX_MAX_AGE_MS = 3000;
// -----------------------------------------------------------------------

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);
WebServer server(80);

char nmeaRing[NMEA_LINES][NMEA_LINE_LEN];
uint8_t nmeaHead = 0;          // next slot to write
uint32_t nmeaCount = 0;        // total lines seen
char nmeaCur[NMEA_LINE_LEN];
uint8_t nmeaCurLen = 0;


static void nmeaFeed(char c) {
  if (c == '\n') {
    if (nmeaCurLen > 0) {
      nmeaCur[nmeaCurLen] = 0;
      strcpy(nmeaRing[nmeaHead], nmeaCur);
      nmeaHead = (nmeaHead + 1) % NMEA_LINES;
      nmeaCount++;
    }
    nmeaCurLen = 0;
  } else if (c >= 32 && c < 127 && nmeaCurLen < NMEA_LINE_LEN - 1) {
    nmeaCur[nmeaCurLen++] = c;
  }
}

static void handleData() {
  bool fix = gps.location.isValid() && gps.location.age() < FIX_MAX_AGE_MS;
  MGRSCoordinate p;
  bool haveMgrs = fix && latLonToMGRS(gps.location.lat(), gps.location.lng(), p);

  String j;
  j.reserve(NMEA_LINES * NMEA_LINE_LEN + 400);
  char t[160];
  j += F("{\"fix\":");
  j += fix ? F("true") : F("false");
  snprintf(t, sizeof t, ",\"lat\":%.7f,\"lon\":%.7f,\"age\":%lu",
           fix ? gps.location.lat() : 0.0, fix ? gps.location.lng() : 0.0,
           fix ? (unsigned long)gps.location.age() : 0UL);
  j += t;
  if (haveMgrs) {
    snprintf(t, sizeof t, ",\"mgrs\":\"%s\",\"zone\":%d,\"band\":\"%c\",\"square\":\"%s\",\"e\":\"%s\",\"n\":\"%s\"",
             p.mgrs, (int)p.zone, p.band, p.square, p.easting, p.northing);
    j += t;
  }
  snprintf(t, sizeof t, ",\"sats\":%u,\"hdop\":%.2f",
           gps.satellites.isValid() ? (unsigned)gps.satellites.value() : 0u,
           gps.hdop.isValid() ? gps.hdop.hdop() : 0.0);
  j += t;
  if (gps.altitude.isValid()) { snprintf(t, sizeof t, ",\"alt\":%.1f", gps.altitude.meters()); j += t; }
  else j += F(",\"alt\":null");
  if (gps.time.isValid() && gps.date.isValid()) {
    snprintf(t, sizeof t, ",\"utc\":\"%04u-%02u-%02u %02u:%02u:%02u\"", gps.date.year(), gps.date.month(),
             gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    j += t;
  }
  snprintf(t, sizeof t, ",\"chars\":%lu,\"ok\":%lu,\"bad\":%lu,\"up\":%lu,\"nmea\":[",
           (unsigned long)gps.charsProcessed(), (unsigned long)gps.passedChecksum(),
           (unsigned long)gps.failedChecksum(), (unsigned long)(millis() / 1000));
  j += t;
  uint8_t n = nmeaCount < NMEA_LINES ? (uint8_t)nmeaCount : NMEA_LINES;
  uint8_t start = (nmeaHead + NMEA_LINES - n) % NMEA_LINES;
  for (uint8_t i = 0; i < n; i++) {
    if (i) j += ',';
    j += '"';
    j += nmeaRing[(start + i) % NMEA_LINES];   // printable ASCII; NMEA has no quotes/backslashes
    j += '"';
  }
  j += F("]}");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", j);
}

void setup() {
  Serial.begin(115200);
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
  server.on("/", []() { server.send_P(200, "text/html", PAGE); });
  server.on("/data", handleData);
  server.begin();
  delay(500);
  Serial.printf("AP \"%s\" %s, open http://%s\n", AP_SSID, ok ? "up" : "FAILED",
                WiFi.softAPIP().toString().c_str());
}

void loop() {
  while (GPSSerial.available()) {
    char c = GPSSerial.read();
    gps.encode(c);
    nmeaFeed(c);
  }
  server.handleClient();
}
