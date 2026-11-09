#include <TinyGPSPlus.h>

static const int RXPin = 20;  // XIAO D7 / GPIO20, connected to GPS TX
static const int TXPin = 21;  // XIAO D6 / GPIO21, connected to GPS RX
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

unsigned long lastWarning = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000);

  GPSSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);

  Serial.println();
  Serial.println("XIAO ESP32-C3 GPS Test");
  Serial.println("Waiting for raw NMEA data...");
}

void loop()
{
  while (GPSSerial.available())
  {
    char c = GPSSerial.read();

    // Show raw GPS sentences such as $GNGGA and $GNRMC
    Serial.write(c);

    gps.encode(c);
  }

  if (millis() - lastWarning >= 5000)
  {
    lastWarning = millis();

    Serial.println();
    Serial.print("Characters received: ");
    Serial.println(gps.charsProcessed());

    if (gps.charsProcessed() < 10)
    {
      Serial.println("No GPS serial data detected.");
      Serial.println("Check power, ground, TX/RX wiring, and baud rate.");
    }
  }
}
