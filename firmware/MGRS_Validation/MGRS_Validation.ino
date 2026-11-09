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
    },

    {
    "Denver CO",
    39.7392,
    -104.9903
    },

    {
    "San Francisco CA",
    37.7749,
    -122.4194
    },

    {
    "Anchorage AK",
    61.2181,
    -149.9003
    }
};


void setup()
{
    Serial.begin(115200);

    delay(3000);

    Serial.println();
    Serial.println("MGRS String Validation Test");
    Serial.println("==========================");


    int count = sizeof(tests) / sizeof(tests[0]);


    for(int i = 0; i < count; i++)
    {
        Serial.println();
        Serial.print(tests[i].name);
        Serial.println(":");


        bool success = latLonToMGRS(
            tests[i].lat,
            tests[i].lon,
            position
        );


        if(success)
        {
            Serial.print("MGRS = ");
            Serial.println(position.mgrs);
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
