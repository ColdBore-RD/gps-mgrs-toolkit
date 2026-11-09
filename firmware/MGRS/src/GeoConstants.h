#ifndef GEO_CONSTANTS_H
#define GEO_CONSTANTS_H


/*
    WGS84 Ellipsoid Constants

    Used for UTM Transverse Mercator projection.
*/


namespace GeoConstants
{

    // WGS84 semi-major axis (meters)
    constexpr double WGS84_A = 6378137.0;


    // WGS84 flattening
    constexpr double WGS84_F = 1.0 / 298.257223563;


    // UTM scale factor
    constexpr double UTM_K0 = 0.9996;


    // UTM false easting (meters)
    constexpr double UTM_FALSE_EASTING = 500000.0;


    // UTM false northing southern hemisphere (meters)
    constexpr double UTM_FALSE_NORTHING = 10000000.0;


    // Use unique names to avoid Arduino macro collisions

    constexpr double DEG_TO_RAD_CB =
        0.017453292519943295769236907684886;


    constexpr double RAD_TO_DEG_CB =
        57.295779513082320876798154814105;

}


#endif
