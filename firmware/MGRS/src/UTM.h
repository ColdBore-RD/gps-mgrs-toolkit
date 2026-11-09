#ifndef UTM_H
#define UTM_H

#include <Arduino.h>


struct UTMCoordinate
{
    int zone;

    char hemisphere;

    double easting;

    double northing;
};


/*
    Convert WGS84 latitude / longitude
    to UTM coordinates.

    latitude:
        -80 to +84 degrees

    longitude:
        -180 to +180 degrees


    Returns:
        true  = successful conversion
        false = invalid input

*/

bool latLonToUTM(
    double latitude,
    double longitude,
    UTMCoordinate &output
);


#endif
