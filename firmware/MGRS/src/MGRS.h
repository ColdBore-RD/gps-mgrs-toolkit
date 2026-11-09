#ifndef MGRS_H
#define MGRS_H

#include <Arduino.h>


struct MGRSCoordinate
{
    // Complete MGRS string
    // Example: 18SUJ2348065432
    char mgrs[20];

    // UTM zone
    int zone;

    // Latitude band
    char band;

    // 100km grid square
    char square[3];

    // 5 digit easting
    char easting[6];

    // 5 digit northing
    char northing[6];

    // Raw UTM values
    double utmEasting;
    double utmNorthing;
};


bool latLonToMGRS(
    double latitude,
    double longitude,
    MGRSCoordinate &output
);


#endif
