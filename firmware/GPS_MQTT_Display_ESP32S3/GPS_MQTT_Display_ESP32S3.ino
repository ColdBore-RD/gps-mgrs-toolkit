#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"   // WIFI_SSID, WIFI_PASSWORD, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD

// ============================================================
// Waveshare ESP32-S3-Zero + ILI9341 TFT — GPS MQTT Diagnostic Display
//
// Subscribes to the JSON diagnostic payload published by
// GPS_MQTT_Diagnostic_ESP32C3 (../GPS_MQTT_Diagnostic_ESP32C3) and
// renders it on a 320x240 SPI TFT. Read-only consumer — this sketch
// does not talk to a GPS module itself.
//
// Wiring: see ESP32-S3-Zero_ILI9341_XPT2046_Reference.md in this
// folder. Touch (XPT2046) is wired per that reference but not used
// here — this is a passive status display, not an input device.
//
// MQTT:
// Data   topic (subscribe): sensors/gps-tracker/diagnostic
// Status topic (subscribe): sensors/gps-tracker/status ("online"/"offline")
// Broker credentials live in secrets.h ONLY — never commit that file.
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

// ── MQTT topics ──────────────────────────────────────────────────────────
#define MQTT_TOPIC_DATA    "sensors/gps-tracker/diagnostic"
#define MQTT_TOPIC_STATUS  "sensors/gps-tracker/status"

static const unsigned long REDRAW_INTERVAL_MS    = 1000;
static const unsigned long RECONNECT_INTERVAL_MS = 5000;
static const unsigned long STALE_TIMEOUT_MS      = 10000;

static const int LINE_HEIGHT = 12;
static const int BODY_TOP_Y  = 50;
static const int LEFT_X      = 6;

Adafruit_ILI9341 tft(&SPI, TFT_DC, TFT_CS, TFT_RST);
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

struct GpsDiagnostic
{
  bool   fix = false;
  double lat = 0.0;
  double lon = 0.0;
  char   mgrs[24] = "N/A";
  float  altitudeM = 0.0f;
  float  speedMph = 0.0f;
  float  courseDeg = 0.0f;
  int    satellites = 0;
  float  hdop = 99.9f;
  char   dateUtc[11] = "N/A";
  char   timeUtc[9] = "N/A";
  unsigned long charsProcessed = 0;
  unsigned long charsFailedChecksum = 0;
  unsigned long fixSentences = 0;
  unsigned long deviceUptimeS = 0;
};

GpsDiagnostic gpsData;
bool          deviceOnline = false;
bool          everReceivedData = false;
unsigned long lastMessageMs = 0;
unsigned long lastDraw = 0;
unsigned long lastReconnect = 0;

// Forward declarations
void connectWiFi();
bool connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void drawDashboard();
void drawStatusChip(int x, int w, const char* label, bool ok);
void drawRow(int y, const char* label, const String& value);

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("=== GPS MQTT Diagnostic Display ==="));

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextWrap(false);

  tft.setCursor(8, 8);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Starting...");

  connectWiFi();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(768);
  mqtt.setCallback(mqttCallback);
  connectMQTT();
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("[WiFi] Lost — reconnecting..."));
    connectWiFi();
  }

  if (!mqtt.connected())
  {
    unsigned long now = millis();
    if (now - lastReconnect > RECONNECT_INTERVAL_MS)
    {
      lastReconnect = now;
      connectMQTT();
    }
  }
  mqtt.loop();

  if (millis() - lastDraw >= REDRAW_INTERVAL_MS)
  {
    lastDraw = millis();
    drawDashboard();
  }
}

// ============================================================
// WIFI / MQTT
// ============================================================

void connectWiFi()
{
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\n[WiFi] Connected — IP: %s\n", WiFi.localIP().toString().c_str());
}

bool connectMQTT()
{
  Serial.printf("[MQTT] Connecting to %s:%d ... ", MQTT_HOST, MQTT_PORT);

  String clientId = "s3zero-gps-display-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  bool ok = mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);

  if (ok)
  {
    Serial.println("connected.");
    mqtt.subscribe(MQTT_TOPIC_DATA);
    mqtt.subscribe(MQTT_TOPIC_STATUS);
  }
  else
  {
    Serial.printf("failed (rc=%d)\n", mqtt.state());
  }

  return ok;
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  if (strcmp(topic, MQTT_TOPIC_STATUS) == 0)
  {
    deviceOnline = (length == 6 && strncmp((const char*)payload, "online", 6) == 0);
    return;
  }

  if (strcmp(topic, MQTT_TOPIC_DATA) != 0)
  {
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err)
  {
    Serial.printf("[MQTT] JSON parse failed: %s\n", err.c_str());
    return;
  }

  gpsData.fix        = doc["fix"] | false;
  gpsData.lat         = doc["lat"] | 0.0;
  gpsData.lon         = doc["lon"] | 0.0;
  strlcpy(gpsData.mgrs, doc["mgrs"] | "N/A", sizeof(gpsData.mgrs));
  gpsData.altitudeM   = doc["altitude_m"] | 0.0f;
  gpsData.speedMph    = doc["speed_mph"] | 0.0f;
  gpsData.courseDeg   = doc["course_deg"] | 0.0f;
  gpsData.satellites  = doc["satellites"] | 0;
  gpsData.hdop        = doc["hdop"] | 99.9f;
  strlcpy(gpsData.dateUtc, doc["date_utc"] | "N/A", sizeof(gpsData.dateUtc));
  strlcpy(gpsData.timeUtc, doc["time_utc"] | "N/A", sizeof(gpsData.timeUtc));
  gpsData.charsProcessed       = doc["chars_processed"] | 0UL;
  gpsData.charsFailedChecksum  = doc["chars_failed_checksum"] | 0UL;
  gpsData.fixSentences         = doc["fix_sentences"] | 0UL;
  gpsData.deviceUptimeS         = doc["uptime_s"] | 0UL;

  everReceivedData = true;
  lastMessageMs = millis();
}

// ============================================================
// DISPLAY
// ============================================================

void drawStatusChip(int x, int w, const char* label, bool ok)
{
  uint16_t bg = ok ? ILI9341_DARKGREEN : ILI9341_RED;
  tft.fillRect(x, 28, w, 20, bg);
  tft.drawRect(x, 28, w, 20, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(x + 4, 34);
  tft.print(label);
}

void drawRow(int y, const char* label, const String& value)
{
  char line[48];
  snprintf(line, sizeof(line), "%-14s%s", label, value.c_str());
  tft.setCursor(LEFT_X, y);
  tft.print(line);
}

void drawDashboard()
{
  bool wifiOk = (WiFi.status() == WL_CONNECTED);
  bool mqttOk = mqtt.connected();
  bool dataFresh = everReceivedData && (millis() - lastMessageMs < STALE_TIMEOUT_MS);

  // Header
  tft.fillRect(0, 0, 320, 26, ILI9341_BLUE);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(6, 5);
  tft.print("GPS MQTT DIAGNOSTIC");

  // Connectivity chips
  drawStatusChip(0,   80, "WIFI", wifiOk);
  drawStatusChip(80,  80, "MQTT", mqttOk);
  drawStatusChip(160, 80, "DEV",  deviceOnline);
  drawStatusChip(240, 80, "DATA", dataFresh);

  // Body
  tft.fillRect(0, BODY_TOP_Y - 2, 320, 240 - (BODY_TOP_Y - 2), ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);

  if (!everReceivedData)
  {
    tft.setCursor(LEFT_X, BODY_TOP_Y + 10);
    tft.print("Waiting for MQTT data on");
    tft.setCursor(LEFT_X, BODY_TOP_Y + 22);
    tft.print(MQTT_TOPIC_DATA);
    return;
  }

  int y = BODY_TOP_Y;

  tft.setTextColor(gpsData.fix ? ILI9341_GREEN : ILI9341_YELLOW);
  drawRow(y, "FIX:", gpsData.fix ? "YES" : "NO (searching)");
  y += LINE_HEIGHT;

  tft.setTextColor(ILI9341_WHITE);
  drawRow(y, "LAT:",   String(gpsData.lat, 6));            y += LINE_HEIGHT;
  drawRow(y, "LON:",   String(gpsData.lon, 6));            y += LINE_HEIGHT;
  drawRow(y, "MGRS:",  String(gpsData.mgrs));               y += LINE_HEIGHT;
  drawRow(y, "ALT (m):", String(gpsData.altitudeM, 1));    y += LINE_HEIGHT;
  drawRow(y, "SPEED (mph):", String(gpsData.speedMph, 2)); y += LINE_HEIGHT;
  drawRow(y, "COURSE (deg):", String(gpsData.courseDeg, 1)); y += LINE_HEIGHT;
  drawRow(y, "SATS:",  String(gpsData.satellites));         y += LINE_HEIGHT;
  drawRow(y, "HDOP:",  String(gpsData.hdop, 2));            y += LINE_HEIGHT;
  drawRow(y, "DATE (UTC):", String(gpsData.dateUtc));       y += LINE_HEIGHT;
  drawRow(y, "TIME (UTC):", String(gpsData.timeUtc));       y += LINE_HEIGHT;

  char chars[24];
  snprintf(chars, sizeof(chars), "%lu / %lu bad", gpsData.charsProcessed, gpsData.charsFailedChecksum);
  drawRow(y, "NMEA CHARS:", String(chars));                 y += LINE_HEIGHT;

  drawRow(y, "FIX SENT:", String(gpsData.fixSentences));    y += LINE_HEIGHT;
  drawRow(y, "GPS UPTIME:", String(gpsData.deviceUptimeS) + "s"); y += LINE_HEIGHT;

  unsigned long ageS = (millis() - lastMessageMs) / 1000;
  tft.setTextColor(dataFresh ? ILI9341_LIGHTGREY : ILI9341_RED);
  drawRow(y, "LAST UPDATE:", String(ageS) + "s ago");
}
