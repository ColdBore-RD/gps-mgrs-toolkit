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
        false = invalid input (out of range, NaN or infinite)

    output.zone is the MGRS zone, including the Norway (32V) and
    Svalbard (31X/33X/35X/37X) exceptions, and easting/northing are
    projected on that zone's central meridian.

*/

bool latLonToUTM(
    double latitude,
    double longitude,
    UTMCoordinate &output
);


/*
    UTM zone (1-60) for a WGS84 position, applying the Norway and
    Svalbard exceptions. lon = +180 is clamped to zone 60.
    Inputs are assumed to be in range (checked by latLonToUTM).
*/

int utmZoneForLatLon(
    double latitude,
    double longitude
);


#endif
