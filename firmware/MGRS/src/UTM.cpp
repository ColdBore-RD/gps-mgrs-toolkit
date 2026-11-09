#include "UTM.h"
#include "GeoConstants.h"

#include <math.h>


bool latLonToUTM(
    double latitude,
    double longitude,
    UTMCoordinate &output
)
{

    // UTM valid latitude range
    if(latitude < -80.0 || latitude > 84.0)
    {
        return false;
    }


    if(longitude < -180.0 || longitude > 180.0)
    {
        return false;
    }


    using namespace GeoConstants;


    double latRad = latitude * GeoConstants::DEG_TO_RAD_CB;
    double lonRad = longitude * GeoConstants::DEG_TO_RAD_CB;


    // Determine UTM zone
    int zone = (int)((longitude + 180.0) / 6.0) + 1;


    double lonOrigin =
        ((zone - 1) * 6 - 180 + 3) * GeoConstants::DEG_TO_RAD_CB;


    double eccSquared =
        WGS84_F * (2.0 - WGS84_F);


    double eccPrimeSquared =
        eccSquared /
        (1.0 - eccSquared);



    double N =
        WGS84_A /
        sqrt(
            1.0 -
            eccSquared *
            sin(latRad) *
            sin(latRad)
        );


    double T =
        tan(latRad) *
        tan(latRad);


    double C =
        eccPrimeSquared *
        cos(latRad) *
        cos(latRad);


    double A =
        cos(latRad) *
        (lonRad - lonOrigin);



    double M =
        WGS84_A *
        (
            (1
            - eccSquared / 4.0
            - 3.0 * eccSquared * eccSquared / 64.0
            - 5.0 * eccSquared * eccSquared * eccSquared / 256.0)
            * latRad

            -

            (3.0 * eccSquared / 8.0
            + 3.0 * eccSquared * eccSquared / 32.0
            + 45.0 * eccSquared * eccSquared * eccSquared / 1024.0)
            * sin(2.0 * latRad)

            +

            (15.0 * eccSquared * eccSquared / 256.0
            + 45.0 * eccSquared * eccSquared * eccSquared / 1024.0)
            * sin(4.0 * latRad)

            -

            (35.0 * eccSquared * eccSquared * eccSquared / 3072.0)
            * sin(6.0 * latRad)
        );



    double easting =
        UTM_K0 *
        N *
        (
            A
            +
            (1 - T + C)
            * pow(A,3)
            / 6.0

            +

            (5
            - 18*T
            + T*T
            + 72*C
            - 58*eccPrimeSquared)
            * pow(A,5)
            / 120.0
        )

        +

        UTM_FALSE_EASTING;



    double northing =
        UTM_K0 *
        (
            M
            +

            N *
            tan(latRad)
            *
            (
                A*A/2.0

                +

                (5
                - T
                + 9*C
                + 4*C*C)
                * pow(A,4)
                / 24.0

                +

                (61
                - 58*T
                + T*T
                + 600*C
                - 330*eccPrimeSquared)
                * pow(A,6)
                / 720.0
            )
        );



    char hemisphere = 'N';


    if(latitude < 0)
    {
        northing += UTM_FALSE_NORTHING;
        hemisphere = 'S';
    }



    output.zone = zone;
    output.hemisphere = hemisphere;
    output.easting = easting;
    output.northing = northing;


    return true;
}
