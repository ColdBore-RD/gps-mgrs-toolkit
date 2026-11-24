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

static const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>GPS / MGRS bench test</title>
<style>
:root{--bg:#0f1318;--card:#181e26;--fg:#e8edf2;--mut:#8a97a6;--ok:#2ecc71;--bad:#e5484d;--acc:#4da3ff}
@media(prefers-color-scheme:light){:root{--bg:#f2f4f7;--card:#fff;--fg:#14202b;--mut:#5b6b7b;--ok:#128a4a;--bad:#c62828;--acc:#1565c0}}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:16px system-ui,sans-serif}
main{max-width:720px;margin:0 auto;padding:16px}
h1{font-size:18px;margin:0 0 12px;color:var(--mut);font-weight:600}
.card{background:var(--card);border-radius:12px;padding:16px;margin-bottom:12px}
.badge{display:inline-block;padding:4px 12px;border-radius:99px;font-weight:700;color:#fff;background:var(--bad)}
.badge.ok{background:var(--ok)}
.mgrs{font:700 clamp(24px,7vw,40px) ui-monospace,Consolas,monospace;letter-spacing:1px;word-break:break-all;margin:8px 0 4px}
.sub{color:var(--mut);font-size:14px}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:10px 16px}
.k{color:var(--mut);font-size:12px;text-transform:uppercase;letter-spacing:.5px}
.v{font:600 18px ui-monospace,Consolas,monospace}
pre{margin:0;max-height:300px;overflow:auto;font:12px/1.4 ui-monospace,Consolas,monospace;white-space:pre-wrap;word-break:break-all}
.top{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}
label{color:var(--mut);font-size:13px}
</style></head><body><main>
<h1>GPS / MGRS bench test</h1>
<div class="card">
 <div class="top"><span id="badge" class="badge">NO FIX</span><span class="sub" id="age">connecting...</span></div>
 <div class="k">MGRS</div><div class="mgrs" id="mgrs">-</div>
 <div class="sub" id="parts">zone - band - square -</div>
</div>
<div class="card"><div class="grid">
 <div><div class="k">Latitude</div><div class="v" id="lat">-</div></div>
 <div><div class="k">Longitude</div><div class="v" id="lon">-</div></div>
 <div><div class="k">Satellites</div><div class="v" id="sats">-</div></div>
 <div><div class="k">HDOP</div><div class="v" id="hdop">-</div></div>
 <div><div class="k">Altitude</div><div class="v" id="alt">-</div></div>
 <div><div class="k">UTC</div><div class="v" id="utc">-</div></div>
 <div><div class="k">NMEA ok / bad</div><div class="v" id="chk">-</div></div>
 <div><div class="k">Uptime</div><div class="v" id="up">-</div></div>
</div></div>
<div class="card">
 <div class="top"><div class="k">Raw NMEA</div><label><input type="checkbox" id="pause"> pause</label></div>
 <pre id="raw">waiting for data...</pre>
</div>
</main>
<script>
const $=id=>document.getElementById(id);
async function tick(){
 try{
  const d=await (await fetch('/data',{cache:'no-store'})).json();
  const b=$('badge');b.textContent=d.fix?'FIX':'NO FIX';b.className='badge'+(d.fix?' ok':'');
  $('mgrs').textContent=d.mgrs||'-';
  $('parts').textContent=d.mgrs?('zone '+d.zone+' - band '+d.band+' - square '+d.square+' - E '+d.e+' N '+d.n):'no MGRS until a fix';
  $('lat').textContent=d.fix?d.lat.toFixed(6):'-';
  $('lon').textContent=d.fix?d.lon.toFixed(6):'-';
  $('sats').textContent=d.sats;$('hdop').textContent=d.hdop>0?d.hdop.toFixed(1):'-';
  $('alt').textContent=d.alt!==null?d.alt.toFixed(1)+' m':'-';
  $('utc').textContent=d.utc||'-';
  $('chk').textContent=d.ok+' / '+d.bad;
  $('up').textContent=d.up+' s';
  $('age').textContent=d.fix?('fix age '+(d.age/1000).toFixed(1)+' s'):(d.chars>0?'receiving NMEA, searching':'no NMEA - check wiring');
  if(!$('pause').checked){const r=$('raw');const bot=r.scrollTop+r.clientHeight>=r.scrollHeight-8;
   r.textContent=d.nmea.join('\n');if(bot)r.scrollTop=r.scrollHeight;}
 }catch(e){$('age').textContent='connection lost';}
}
tick();setInterval(tick,1000);
</script></body></html>)HTML";

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
