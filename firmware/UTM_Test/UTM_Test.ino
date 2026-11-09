#include <MGRS.h>


MGRSCoordinate position;


void setup()
{
    Serial.begin(115200);

    delay(3000);

    Serial.println();
    Serial.println("MGRS Conversion Test");
    Serial.println("--------------------");


    // White House
    double latitude = 38.8977;
    double longitude = -77.0365;


    bool success = latLonToMGRS(
        latitude,
        longitude,
        position
    );


    if(success)
    {
        Serial.println("SUCCESS");

        Serial.print("MGRS: ");
        Serial.println(position.mgrs);


        Serial.print("Zone: ");
        Serial.println(position.zone);


        Serial.print("Band: ");
        Serial.println(position.band);


        Serial.print("Square: ");
        Serial.println(position.square);


        Serial.print("Easting: ");
        Serial.println(position.easting);


        Serial.print("Northing: ");
        Serial.println(position.northing);


        Serial.print("Raw Easting: ");
        Serial.println(position.utmEasting, 3);


        Serial.print("Raw Northing: ");
        Serial.println(position.utmNorthing, 3);
    }
    else
    {
        Serial.println("FAILED");
    }
}


void loop()
{

}
