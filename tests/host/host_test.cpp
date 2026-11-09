// Host-side regression test for the MGRS library (no board needed).
// Reference strings were cross-checked against an independent Krueger-series
// implementation. Build: see tests/host/README.md
#include <cstdio>
#include <cstring>
#include "MGRS.h"

struct P { const char* name; double lat, lon; const char* expected; };

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
    };
    int fail = 0;
    for (const P& p : t) {
        MGRSCoordinate c{};
        bool ok = latLonToMGRS(p.lat, p.lon, c) && strcmp(c.mgrs, p.expected) == 0;
        if (!ok) fail++;
        printf("%s  %-18s %-16s expected %s\n", ok ? "PASS" : "FAIL", p.name, c.mgrs, p.expected);
    }
    // Out-of-range latitudes must be rejected.
    MGRSCoordinate c{};
    if (latLonToMGRS(84.5, 0, c) || latLonToMGRS(-80.5, 0, c)) { fail++; puts("FAIL  out-of-range accepted"); }
    printf("%s\n", fail ? "FAILED" : "ALL PASS");
    return fail;
}
