#include "MGRS.h"
#include "UTM.h"

#include <math.h>
#include <string.h>
#include <stdio.h>


static const char latitudeBands[] =
    "CDEFGHJKLMNPQRSTUVWXX";


// MGRS column sets
static const char columnSet1[] =
    "ABCDEFGH";

static const char columnSet2[] =
    "JKLMNPQR";

static const char columnSet3[] =
    "STUVWXYZ";


// MGRS row letters
static const char rowLetters[] =
    "ABCDEFGHJKLMNPQRSTUV";


static char getLatitudeBand(
    double latitude
)
{
    // Written so NaN fails the test
    if(!(latitude >= -80 &&
         latitude <= 84))
    {
        return 'Z';
    }


    int index =
        (int)((latitude + 80) / 8);


    return latitudeBands[index];
}



static char getColumnLetter(
    int zone,
    int easting
)
{
    int column =
        easting / 100000;


    if(column < 1 ||
       column > 8)
    {
        return '?';
    }


    const char *set;


    switch(zone % 3)
    {
        case 1:
            set = columnSet1;
            break;

        case 2:
            set = columnSet2;
            break;

        default:
            set = columnSet3;
            break;
    }


    return set[column - 1];
}



static char getRowLetter(
    int zone,
    int northing,
    char hemisphere
)
{
    int row =
        northing / 100000;


    row %= 20;


    /*
        Even zones start the row cycle at 'F' (offset 5).

        No extra southern-hemisphere offset: the 10,000,000 m false
        northing is already in `northing`, and 100 rows is a whole
        number of 20-letter cycles.
    */

    (void)hemisphere;

    if(zone % 2 == 0)
    {
        row += 5;
    }


    row %= 20;


    return rowLetters[row];
}



bool latLonToMGRS(
    double latitude,
    double longitude,
    MGRSCoordinate &output
)
{

    // Range checks written so NaN fails them
    if(!(latitude >= -80 &&
         latitude <= 84))
    {
        return false;
    }


    if(!(longitude >= -180 &&
         longitude <= 180))
    {
        return false;
    }



    UTMCoordinate utm;


    if(!latLonToUTM(
        latitude,
        longitude,
        utm))
    {
        return false;
    }



    // Zone (with Norway/Svalbard exceptions) the projection used
    int zone =
        utm.zone;



    char band =
        getLatitudeBand(latitude);


    if(band == 'Z')
    {
        return false;
    }



    int easting =
        (int)floor(
            utm.easting
        );


    int northing =
        (int)floor(
            utm.northing
        );



    char column =
        getColumnLetter(
            zone,
            easting
        );


    char row =
        getRowLetter(
            zone,
            northing,
            utm.hemisphere
        );



    if(column == '?' ||
       row == '?')
    {
        return false;
    }



    int eastRemainder =
        easting % 100000;


    int northRemainder =
        northing % 100000;



    sprintf(
        output.easting,
        "%05d",
        eastRemainder
    );


    sprintf(
        output.northing,
        "%05d",
        northRemainder
    );



    output.zone = zone;

    output.band = band;


    output.square[0] = column;
    output.square[1] = row;
    output.square[2] = '\0';



    sprintf(
        output.mgrs,
        "%02d%c%c%c%s%s",
        zone,
        band,
        column,
        row,
        output.easting,
        output.northing
    );


    output.utmEasting =
        utm.easting;

    output.utmNorthing =
        utm.northing;


    return true;
}
