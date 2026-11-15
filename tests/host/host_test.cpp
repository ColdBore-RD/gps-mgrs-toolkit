// Host-side regression test for the MGRS library (no board needed).
// Reference strings were cross-checked against an independent Krueger-series
// implementation; the zone-exception and edge vectors are GeographicLib 2.5
// values from the v1.1.1 external test report. Build: see tests/host/README.md
#include <cstdio>
#include <cstring>
#include <limits>
#include "MGRS.h"
#include "UTM.h"

struct P { const char* name; double lat, lon; const char* expected; };

static int fail = 0;

static void check(bool ok, const char* what)
{
    if (!ok) fail++;
    printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
}

int main()
{
    const P t[] = {
        {"White House",       38.8977,  -77.0365, "18SUJ2339407395"},
        {"Baltimore MD",      39.2904,  -76.6122, "18SUJ6096550242"},
        {"London UK",         51.5074,   -0.1278, "30UXC9931610163"},
        {"Tokyo Japan",       35.6762,  139.6503, "54SUE7785548874"},
        {"Sydney Australia", -33.8688,  151.2093, "56HLH3436850948"},
        {"Denver CO",         39.7392, -104.9903, "13SED0083198811"},
        {"San Francisco CA",  37.7749, -122.4194, "10SEG5113080998"},
        {"Anchorage AK",      61.2181, -149.9003, "06VUN4424790536"},
        {"Oslo (32V)",        59.9139,   10.7522, "32VNM9797943118"},
        {"Svalbard (33X)",    78.2232,   15.6267, "33XWG1427883355"},
        // Re-projected exception strips: native zone differs from MGRS zone (F1)
        {"Norway 3E (31->32V)",    60.0,       3.0,       "32VJM6564066593"},
        {"Svalbard (36->35X)",     78.184959,  32.234474, "35XPG1949384358"},
        // Band X at lat 84 exactly is still in a Svalbard zone (F2)
        {"lat 84 (32->31X)",       84.0,       6.0,       "31XEP3499429005"},
        // lon +180 clamps to zone 60 (F3)
        {"lon 180 (zone 60)",     -80.0,     180.0,       "60CWS5813216915"},
    };
    for (const P& p : t) {
        MGRSCoordinate c{};
        bool ok = latLonToMGRS(p.lat, p.lon, c) && strcmp(c.mgrs, p.expected) == 0;
        if (!ok) fail++;
        printf("%s  %-22s %-16s expected %s\n", ok ? "PASS" : "FAIL", p.name, c.mgrs, p.expected);
    }

    // latLonToUTM itself must report the exception zone it projected in.
    struct Z { const char* name; double lat, lon; int zone; };
    const Z z[] = {
        {"UTM zone 32 at 60N 3E",      60.0,       3.0,       32},
        {"UTM zone 32 at 56N 11.9E",   56.0,      11.9,       32},
        {"UTM zone 31 at 63.9N 2.9E",  63.9,       2.9,       31},
        {"UTM zone 31 at 78N 8.9E",    78.0,       8.9,       31},
        {"UTM zone 33 at 78N 9E",      78.0,       9.0,       33},
        {"UTM zone 35 at 78.18N 32.2E",78.184959, 32.234474,  35},
        {"UTM zone 37 at 80N 41.9E",   80.0,      41.9,       37},
        {"UTM zone 38 at 80N 42E",     80.0,      42.0,       38},
        {"UTM zone 60 at lon 180",      0.0,     180.0,       60},
    };
    for (const Z& p : z) {
        UTMCoordinate u{};
        char what[64];
        bool ok = latLonToUTM(p.lat, p.lon, u) && u.zone == p.zone;
        snprintf(what, sizeof what, "%s (got %d)", p.name, u.zone);
        check(ok, what);
        // MGRS must agree with the projection's zone
        MGRSCoordinate c{};
        check(latLonToMGRS(p.lat, p.lon, c) && c.zone == u.zone, "  MGRS zone matches UTM zone");
    }

    // Out-of-range, NaN and infinite inputs must be rejected (F4, N1).
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    struct B { const char* name; double lat, lon; };
    const B bad[] = {
        {"lat 84.5",      84.5,   0.0},
        {"lat -80.5",    -80.5,   0.0},
        {"lon 180.5",      0.0, 180.5},
        {"lon -180.5",     0.0, -180.5},
        {"lat NaN",        nan,   0.0},
        {"lon NaN",       45.0,   nan},
        {"lat+lon NaN",    nan,   nan},
        {"lat +inf",       inf,   0.0},
        {"lat -inf",      -inf,   0.0},
        {"lon +inf",       0.0,   inf},
        {"lon -inf",       0.0,  -inf},
    };
    for (const B& p : bad) {
        MGRSCoordinate c{};
        UTMCoordinate u{};
        char what[64];
        snprintf(what, sizeof what, "rejects %s (MGRS)", p.name);
        check(!latLonToMGRS(p.lat, p.lon, c), what);
        snprintf(what, sizeof what, "rejects %s (UTM)", p.name);
        check(!latLonToUTM(p.lat, p.lon, u), what);
    }

    printf("%s\n", fail ? "FAILED" : "ALL PASS");
    return fail;
}
