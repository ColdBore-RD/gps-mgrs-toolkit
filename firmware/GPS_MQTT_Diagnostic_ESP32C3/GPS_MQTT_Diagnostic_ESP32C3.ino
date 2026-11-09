#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <MGRS.h>
#include "secrets.h"

// ============================================================================
// Cold Bore R&D — XIAO ESP32-C3 GPS MQTT Diagnostic Publisher
//
// Reads a UART GPS, converts valid WGS84 coordinates to MGRS, and publishes a
// JSON diagnostic message once per second.
//
// Board: Seeed Studio XIAO ESP32-C3
// GPS RX: GPIO20 / D7 <- GPS TX
// GPS TX: GPIO21 / D6 -> GPS RX
// GPS baud: 9600
// Serial Monitor: 115200
//
// MQTT topics:
//   sensors/gps-tracker/diagnostic  JSON, QoS 0, not retained
//   sensors/gps-tracker/status      online/offline, retained
//
// The broker account must be "gps-tracker" (or an equivalent account) with:
//   topic write sensors/gps-tracker/#
// ============================================================================

#ifndef WIFI_SSID
  #error "WIFI_SSID is missing from secrets.h"
#endif
#ifndef WIFI_PASSWORD
  #error "WIFI_PASSWORD is missing from secrets.h"
#endif
#ifndef MQTT_HOST
  #error "MQTT_HOST is missing from secrets.h"
#endif
#ifndef MQTT_PORT
  #error "MQTT_PORT is missing from secrets.h"
#endif
#ifndef MQTT_USER
  #error "MQTT_USER is missing from secrets.h"
#endif
#ifndef MQTT_PASSWORD
  #error "MQTT_PASSWORD is missing from secrets.h"
#endif

// ── Hardware ────────────────────────────────────────────────────────────────
constexpr int GPS_RX_PIN = 20;
constexpr int GPS_TX_PIN = 21;
constexpr uint32_t GPS_BAUD = 9600;

// ── MQTT ────────────────────────────────────────────────────────────────────
constexpr char MQTT_TOPIC_DATA[]   = "sensors/gps-tracker/diagnostic";
constexpr char MQTT_TOPIC_STATUS[] = "sensors/gps-tracker/status";
constexpr size_t MQTT_BUFFER_SIZE  = 1024;

// ── Timing ──────────────────────────────────────────────────────────────────
constexpr unsigned long PUBLISH_INTERVAL_MS      = 1000;
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS  = 30000;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS   = 5000;
constexpr unsigned long MQTT_RETRY_INTERVAL_MS   = 5000;
constexpr unsigned long WARNING_INTERVAL_MS      = 5000;
constexpr unsigned long GPS_FIELD_MAX_AGE_MS     = 5000;

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastPublishMs = 0;
unsigned long lastWiFiAttemptMs = 0;
unsigned long lastMqttAttemptMs = 0;
unsigned long lastNoDataWarningMs = 0;
unsigned long successfulPublishes = 0;
unsigned long failedPublishes = 0;
bool wifiConnectionReported = false;
bool wifiAttemptInProgress = false;

// ── Forward declarations ────────────────────────────────────────────────────
void readGps();
void startWiFiConnection();
void serviceConnectivity();
bool connectMQTT();
bool buildDiagnosticPayload(char* buffer, size_t bufferSize, size_t& payloadLength);
void publishDiagnostic();

// ============================================================================
// SETUP / LOOP
// ============================================================================

void setup()
{
  Serial.begin(115200);

  const unsigned long serialStartMs = millis();
  while (!Serial && millis() - serialStartMs < 5000)
  {
    delay(10);
  }

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  Serial.println();
  Serial.println(F("=========================================="));
  Serial.println(F("CBRD XIAO ESP32-C3 GPS MQTT Publisher"));
  Serial.println(F("=========================================="));
  Serial.print(F("TinyGPSPlus version: "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("GPS RX: GPIO20 / XIAO D7"));
  Serial.println(F("GPS TX: GPIO21 / XIAO D6"));
  Serial.println(F("GPS baud: 9600"));
  Serial.printf("MQTT broker: %s:%d\n", MQTT_HOST, MQTT_PORT);
  Serial.printf("MQTT data topic: %s\n", MQTT_TOPIC_DATA);
  Serial.println();

  WiFi.mode(WIFI_STA);
  // Reconnection is managed below. Disabling the core's automatic reconnect
  // prevents it from racing our explicit WiFi.begin() attempts.
  WiFi.setAutoReconnect(false);
  startWiFiConnection();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(MQTT_BUFFER_SIZE);
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(5);
}

void loop()
{
  readGps();
  serviceConnectivity();

  if (mqtt.connected())
  {
    mqtt.loop();

    const unsigned long now = millis();
    if (now - lastPublishMs >= PUBLISH_INTERVAL_MS)
    {
      lastPublishMs = now;
      publishDiagnostic();
    }
  }

  const unsigned long now = millis();
  if (now > 5000 &&
      gps.charsProcessed() < 10 &&
      now - lastNoDataWarningMs >= WARNING_INTERVAL_MS)
  {
    lastNoDataWarningMs = now;
    Serial.println(F("[GPS] WARNING: No serial data detected."));
    Serial.println(F("[GPS] Check power, ground, TX/RX wiring, and 9600 baud."));
  }
}

// ============================================================================
// GPS
// ============================================================================

void readGps()
{
  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }
}

// ============================================================================
// WIFI / MQTT
// ============================================================================

void startWiFiConnection()
{
  // Cancel any previous association before applying the station config again.
  // Calling WiFi.begin() on an active attempt produces:
  //   wifi:sta is connecting, cannot set config
  WiFi.disconnect(false, false);
  delay(100);

  lastWiFiAttemptMs = millis();
  wifiAttemptInProgress = true;
  Serial.printf("[WiFi] Connecting to %s ...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void serviceConnectivity()
{
  const unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED)
  {
    if (wifiConnectionReported)
    {
      wifiConnectionReported = false;
      Serial.println(F("[WiFi] Connection lost."));
    }

    if (wifiAttemptInProgress)
    {
      if (now - lastWiFiAttemptMs < WIFI_CONNECT_TIMEOUT_MS)
      {
        return;
      }

      Serial.printf("[WiFi] Attempt timed out (status=%d).\n", WiFi.status());
      WiFi.disconnect(false, false);
      wifiAttemptInProgress = false;
      lastWiFiAttemptMs = now;
      return;
    }

    if (now - lastWiFiAttemptMs >= WIFI_RETRY_INTERVAL_MS)
    {
      startWiFiConnection();
    }
    return;
  }

  if (!wifiConnectionReported)
  {
    wifiAttemptInProgress = false;
    wifiConnectionReported = true;
    Serial.printf("[WiFi] Connected — IP: %s, RSSI: %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    // Permit an immediate MQTT attempt after a new Wi-Fi connection.
    lastMqttAttemptMs = 0;
  }

  if (!mqtt.connected() &&
      (lastMqttAttemptMs == 0 || now - lastMqttAttemptMs >= MQTT_RETRY_INTERVAL_MS))
  {
    lastMqttAttemptMs = now;
    connectMQTT();
  }
}

bool connectMQTT()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return false;
  }

  const uint64_t chipId = ESP.getEfuseMac();
  char clientId[40];
  snprintf(clientId, sizeof(clientId), "xiao-gps-%04X%08X",
           static_cast<unsigned int>((chipId >> 32) & 0xFFFF),
           static_cast<unsigned int>(chipId & 0xFFFFFFFF));

  Serial.printf("[MQTT] Connecting to %s:%d as %s ... ",
                MQTT_HOST, MQTT_PORT, clientId);

  const bool connected = mqtt.connect(
    clientId,
    MQTT_USER,
    MQTT_PASSWORD,
    MQTT_TOPIC_STATUS,
    1,       // Will QoS
    true,    // Retain Will
    "offline"
  );

  if (!connected)
  {
    Serial.printf("failed (rc=%d).\n", mqtt.state());
    return false;
  }

  Serial.println(F("connected."));

  if (!mqtt.publish(MQTT_TOPIC_STATUS, "online", true))
  {
    Serial.println(F("[MQTT] WARNING: Failed to publish retained online status."));
  }
  else
  {
    Serial.println(F("[MQTT] Retained status published: online"));
  }

  return true;
}

// ============================================================================
// DIAGNOSTIC PAYLOAD
// ============================================================================

bool buildDiagnosticPayload(char* buffer, size_t bufferSize, size_t& payloadLength)
{
  const bool hasFix = gps.location.isValid() &&
                      gps.location.age() <= GPS_FIELD_MAX_AGE_MS;

  const double latitude = hasFix ? gps.location.lat() : 0.0;
  const double longitude = hasFix ? gps.location.lng() : 0.0;

  char mgrsString[24] = "N/A";
  if (hasFix)
  {
    MGRSCoordinate position;
    if (latLonToMGRS(latitude, longitude, position))
    {
      strlcpy(mgrsString, position.mgrs, sizeof(mgrsString));
    }
    else
    {
      strlcpy(mgrsString, "OUT_OF_RANGE", sizeof(mgrsString));
    }
  }

  const bool altitudeFresh = gps.altitude.isValid() &&
                             gps.altitude.age() <= GPS_FIELD_MAX_AGE_MS;
  const bool speedFresh = gps.speed.isValid() &&
                          gps.speed.age() <= GPS_FIELD_MAX_AGE_MS;
  const bool courseFresh = gps.course.isValid() &&
                           gps.course.age() <= GPS_FIELD_MAX_AGE_MS;
  const bool satellitesFresh = gps.satellites.isValid() &&
                               gps.satellites.age() <= GPS_FIELD_MAX_AGE_MS;
  const bool hdopFresh = gps.hdop.isValid() &&
                         gps.hdop.age() <= GPS_FIELD_MAX_AGE_MS;

  const float altitudeM = altitudeFresh ? gps.altitude.meters() : 0.0f;
  const float speedMph = speedFresh ? gps.speed.mph() : 0.0f;
  const float courseDeg = courseFresh ? gps.course.deg() : 0.0f;
  const int satellites = satellitesFresh ? gps.satellites.value() : 0;
  const float hdop = hdopFresh && gps.hdop.hdop() < 99.0
                   ? gps.hdop.hdop()
                   : 99.9f;

  char dateString[11] = "N/A";
  if (gps.date.isValid() && gps.date.age() <= GPS_FIELD_MAX_AGE_MS)
  {
    snprintf(dateString, sizeof(dateString), "%04u-%02u-%02u",
             static_cast<unsigned int>(gps.date.year()),
             static_cast<unsigned int>(gps.date.month()),
             static_cast<unsigned int>(gps.date.day()));
  }

  char timeString[9] = "N/A";
  if (gps.time.isValid() && gps.time.age() <= GPS_FIELD_MAX_AGE_MS)
  {
    snprintf(timeString, sizeof(timeString), "%02u:%02u:%02u",
             static_cast<unsigned int>(gps.time.hour()),
             static_cast<unsigned int>(gps.time.minute()),
             static_cast<unsigned int>(gps.time.second()));
  }

  const int written = snprintf(
    buffer,
    bufferSize,
    "{"
      "\"fix\":%s,"
      "\"lat\":%.6f,"
      "\"lon\":%.6f,"
      "\"mgrs\":\"%s\","
      "\"altitude_m\":%.1f,"
      "\"speed_mph\":%.2f,"
      "\"course_deg\":%.2f,"
      "\"satellites\":%d,"
      "\"hdop\":%.2f,"
      "\"date_utc\":\"%s\","
      "\"time_utc\":\"%s\","
      "\"chars_processed\":%lu,"
      "\"chars_failed_checksum\":%lu,"
      "\"fix_sentences\":%lu,"
      "\"uptime_s\":%lu"
    "}",
    hasFix ? "true" : "false",
    latitude,
    longitude,
    mgrsString,
    altitudeM,
    speedMph,
    courseDeg,
    satellites,
    hdop,
    dateString,
    timeString,
    gps.charsProcessed(),
    gps.failedChecksum(),
    gps.sentencesWithFix(),
    millis() / 1000UL
  );

  if (written < 0 || static_cast<size_t>(written) >= bufferSize)
  {
    payloadLength = 0;
    return false;
  }

  payloadLength = static_cast<size_t>(written);
  return true;
}

void publishDiagnostic()
{
  if (!mqtt.connected())
  {
    return;
  }

  char payload[512];
  size_t payloadLength = 0;
  if (!buildDiagnosticPayload(payload, sizeof(payload), payloadLength))
  {
    failedPublishes++;
    Serial.println(F("[MQTT] ERROR: GPS JSON payload exceeded its buffer."));
    return;
  }

  const bool published = mqtt.publish(
    MQTT_TOPIC_DATA,
    reinterpret_cast<const uint8_t*>(payload),
    static_cast<unsigned int>(payloadLength),
    false
  );

  if (published)
  {
    successfulPublishes++;
    Serial.printf("[MQTT] Published #%lu (%u bytes): %s\n",
                  successfulPublishes,
                  static_cast<unsigned int>(payloadLength),
                  payload);
  }
  else
  {
    failedPublishes++;
    Serial.printf("[MQTT] PUBLISH FAILED #%lu (state=%d, %u bytes)\n",
                  failedPublishes,
                  mqtt.state(),
                  static_cast<unsigned int>(payloadLength));
  }
}
