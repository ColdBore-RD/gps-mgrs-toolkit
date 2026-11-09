#include <MGRS.h>


MGRSCoordinate position;


struct TestPoint
{
    const char* name;
    double lat;
    double lon;
};


TestPoint tests[] =
{
    {
        "White House",
        38.8977,
        -77.0365
    },

    {
        "Baltimore MD",
        39.2904,
        -76.6122
    },

    {
        "London UK",
        51.5074,
        -0.1278
    },

    {
        "Tokyo Japan",
        35.6762,
        139.6503
    },

    {
        "Sydney Australia",
        -33.8688,
        151.2093
    }
};


void setup()
{
    Serial.begin(115200);

    delay(3000);

    Serial.println();
    Serial.println("MGRS Validation Test");
    Serial.println("===================");


    int count = sizeof(tests) / sizeof(tests[0]);


    for(int i = 0; i < count; i++)
    {
        Serial.println();
        Serial.println(tests[i].name);


        bool success = latLonToMGRS(
            tests[i].lat,
            tests[i].lon,
            position
        );


        if(success)
        {
            Serial.print("MGRS: ");
            Serial.println(position.mgrs);


            Serial.print("UTM Zone: ");
            Serial.println(position.zone);


            Serial.print("Band: ");
            Serial.println(position.band);


            Serial.print("Square: ");
            Serial.println(position.square);
        }
        else
        {
            Serial.println("FAILED");
        }
    }
}


void loop()
{

}
