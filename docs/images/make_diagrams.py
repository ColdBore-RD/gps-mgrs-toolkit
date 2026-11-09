"""Regenerate docs/images/mgrs-world-grid.svg and mgrs-breakdown.svg (stdlib only).

MGRS strings below are the library's own output (tests/host/host_test.cpp vectors).
Run: python docs/images/make_diagrams.py
"""
import pathlib

OUT = pathlib.Path(__file__).parent
BG, GRID, GREEN, ACCENT, TEXT, DIM = "#0A0A0A", "#1F2A1C", "#76FF4B", "#4BC92A", "#E8E8E8", "#8A9486"
FONT = "font-family='ui-monospace,SFMono-Regular,Consolas,monospace'"

POINTS = [  # name, lat, lon, MGRS (library output), label dx, dy
    ("White House", 38.8977, -77.0365, "18SUJ2339407395", 10, -14),
    ("San Francisco", 37.7749, -122.4194, "10SEG5113080998", -150, 26),
    ("Denver", 39.7392, -104.9903, "13SED0083198811", -40, -34),
    ("Anchorage", 61.2181, -149.9003, "06VUN4424790536", 10, -12),
    ("London", 51.5074, -0.1278, "30UXC9931610163", -150, -6),
    ("Oslo (32V)", 59.9139, 10.7522, "32VNM9797943118", 12, 4),
    ("Svalbard (33X)", 78.2232, 15.6267, "33XWG1427883355", 12, 4),
    ("Tokyo", 35.6762, 139.6503, "54SUE7785548874", -150, -14),
    ("Sydney", -33.8688, 151.2093, "56HLH3436850948", -150, 10),
]
BANDS = "CDEFGHJKLMNPQRSTUVWX"


def world():
    w, h, ml, mt = 1200, 640, 40, 50
    pw, ph = w - ml - 20, h - mt - 40
    x = lambda lon: ml + (lon + 180) / 360 * pw
    y = lambda lat: mt + (84 - lat) / 164 * ph
    s = [f"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 {w} {h}' width='{w}' height='{h}'>",
         f"<rect width='{w}' height='{h}' fill='{BG}'/>",
         f"<text x='{ml}' y='30' fill='{GREEN}' font-size='20' font-weight='700' {FONT}>MGRS grid zones: 60 UTM zones x 20 latitude bands</text>"]
    for z in range(61):
        s.append(f"<line x1='{x(-180 + 6 * z):.1f}' y1='{y(84):.1f}' x2='{x(-180 + 6 * z):.1f}' y2='{y(-80):.1f}' stroke='{GRID}'/>")
    for i in range(21):
        lat = -80 + 8 * i if i < 20 else 84
        s.append(f"<line x1='{x(-180):.1f}' y1='{y(lat):.1f}' x2='{x(180):.1f}' y2='{y(lat):.1f}' stroke='{GRID}'/>")
    for i, b in enumerate(BANDS):
        top = 84 if b == "X" else -72 + 8 * i
        s.append(f"<text x='{ml - 14}' y='{(y(top) + y(-80 + 8 * i)) / 2 + 4:.1f}' fill='{DIM}' font-size='11' {FONT}>{b}</text>")
    for z in range(1, 61, 3):
        s.append(f"<text x='{x(-180 + 6 * (z - 1) + 3):.1f}' y='{y(-80) + 16:.1f}' fill='{DIM}' font-size='10' text-anchor='middle' {FONT}>{z}</text>")
    # Irregular zones: 32V widened (Norway), 31X/33X/35X/37X (Svalbard)
    for lon0, lon1, lat0, lat1, lab in [(3, 12, 56, 64, "32V"), (0, 9, 72, 84, "31X"), (9, 21, 72, 84, "33X"),
                                        (21, 33, 72, 84, "35X"), (33, 42, 72, 84, "37X")]:
        s.append(f"<rect x='{x(lon0):.1f}' y='{y(lat1):.1f}' width='{x(lon1) - x(lon0):.1f}' height='{y(lat0) - y(lat1):.1f}' fill='none' stroke='{ACCENT}' stroke-dasharray='3 2'/>")
    for name, lat, lon, m, dx, dy in POINTS:
        px, py = x(lon), y(lat)
        s.append(f"<circle cx='{px:.1f}' cy='{py:.1f}' r='4' fill='{GREEN}'/>")
        s.append(f"<text x='{px + dx:.1f}' y='{py + dy:.1f}' fill='{TEXT}' font-size='11' {FONT}>{name}</text>")
        s.append(f"<text x='{px + dx:.1f}' y='{py + dy + 13:.1f}' fill='{GREEN}' font-size='11' {FONT}>{m}</text>")
    s.append(f"<text x='{w - 20}' y='{h - 8}' fill='{DIM}' font-size='10' text-anchor='end' {FONT}>Equirectangular, grid only. Dashed: irregular zones (Norway, Svalbard). Points: library output.</text>")
    s.append("</svg>")
    (OUT / "mgrs-world-grid.svg").write_text("\n".join(s) + "\n", encoding="utf-8")


def breakdown():
    w, h = 1120, 300
    parts = [("18", "UTM zone", "6-degree longitude strip", GREEN), ("S", "Latitude band", "8-degree band, 32N-40N", ACCENT),
             ("UJ", "100 km square", "column U, row J", GREEN), ("23394", "Easting", "metres east within square", ACCENT),
             ("07395", "Northing", "metres north within square", GREEN)]
    s = [f"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 {w} {h}' width='{w}' height='{h}'>",
         f"<rect width='{w}' height='{h}' fill='{BG}'/>",
         f"<text x='40' y='40' fill='{TEXT}' font-size='16' {FONT}>latLonToMGRS(38.8977, -77.0365)</text>",
         f"<text x='40' y='66' fill='{DIM}' font-size='13' {FONT}>WGS84 -&gt; UTM zone 18N  E 323394.30  N 4307395.63  -&gt;  MGRS (1 m)</text>"]
    xx = 40
    for txt, title, sub, col in parts:
        bw = max(180, 60 + 34 * len(txt))
        s.append(f"<rect x='{xx}' y='96' width='{bw}' height='76' rx='6' fill='none' stroke='{col}' stroke-width='2'/>")
        s.append(f"<text x='{xx + bw / 2}' y='148' fill='{col}' font-size='40' font-weight='700' text-anchor='middle' {FONT}>{txt}</text>")
        s.append(f"<text x='{xx + bw / 2}' y='204' fill='{TEXT}' font-size='14' text-anchor='middle' {FONT}>{title}</text>")
        s.append(f"<text x='{xx + bw / 2}' y='224' fill='{DIM}' font-size='11' text-anchor='middle' {FONT}>{sub}</text>")
        xx += bw + 18
    s.append(f"<text x='40' y='270' fill='{DIM}' font-size='12' {FONT}>Easting/northing digits are truncated, not rounded: 323394.30 -&gt; 23394, 4307395.63 -&gt; 07395.</text>")
    s.append("</svg>")
    (OUT / "mgrs-breakdown.svg").write_text("\n".join(s) + "\n", encoding="utf-8")


if __name__ == "__main__":
    world()
    breakdown()
