// Status page served by GPS_WiFi_Bench_Test. Plain HTML/CSS/JS, no libraries.
// The page polls /data (JSON) once a second. The map is a small slippy-map
// written for this page: OpenStreetMap tiles are fetched by the PHONE, so the
// basemap only shows when the phone has internet while joined to this board
// (mobile data usually works). Without internet the MGRS grid, position
// marker and every reading still work on a plain background.
#pragma once

static const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>GPS / MGRS bench test</title>
<style>
:root{--bg:#0f1318;--card:#181e26;--fg:#e8edf2;--mut:#8a97a6;--ok:#2ecc71;--bad:#e5484d;--acc:#76ff4b;--line:#2a3340;--mapbg:#222a34}
@media(prefers-color-scheme:light){:root{--bg:#f2f4f7;--card:#fff;--fg:#14202b;--mut:#5b6b7b;--ok:#128a4a;--bad:#c62828;--acc:#2f9e12;--line:#d5dbe3;--mapbg:#e4e8ee}}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:16px system-ui,sans-serif}
main{max-width:720px;margin:0 auto;padding:16px}
header{display:flex;align-items:center;gap:12px;margin-bottom:14px}
header svg{width:44px;height:42px;flex:none}
h1{font-size:18px;margin:0;font-weight:700}
.org{color:var(--mut);font-size:12px;letter-spacing:.6px;text-transform:uppercase}
.card{background:var(--card);border-radius:12px;padding:16px;margin-bottom:12px}
.badge{display:inline-block;padding:4px 12px;border-radius:99px;font-weight:700;color:#fff;background:var(--bad)}
.badge.ok{background:var(--ok)}
.mgrs{font:700 clamp(24px,7vw,40px) ui-monospace,Consolas,monospace;letter-spacing:1px;word-break:break-all;margin:8px 0 4px}
.sub{color:var(--mut);font-size:14px}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:10px 16px}
.k{color:var(--mut);font-size:12px;text-transform:uppercase;letter-spacing:.5px}
.v{font:600 18px ui-monospace,Consolas,monospace}
pre{margin:0;max-height:300px;overflow:auto;font:12px/1.4 ui-monospace,Consolas,monospace;white-space:pre-wrap;word-break:break-all}
.top{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px;gap:8px}
label{color:var(--mut);font-size:13px}
#map{position:relative;height:min(62vh,440px);border-radius:10px;overflow:hidden;background:var(--mapbg);touch-action:none;user-select:none;-webkit-user-select:none}
#tiles,#gridc{position:absolute;left:0;top:0;width:100%;height:100%}
#tiles img{position:absolute;width:256px;height:256px;max-width:none}
#ctl{position:absolute;right:8px;top:8px;display:flex;flex-direction:column;gap:6px}
#ctl button{width:40px;height:40px;border-radius:8px;border:1px solid var(--line);background:var(--card);color:var(--fg);font:700 18px system-ui;cursor:pointer;opacity:.95}
#ctl button.off{opacity:.45}
#ctl button.sm{font-size:12px}
#ovl{position:absolute;inset:0;display:flex;align-items:center;justify-content:center;text-align:center;padding:24px;color:var(--mut);pointer-events:none}
#attr{position:absolute;left:6px;bottom:4px;font-size:11px;background:rgba(255,255,255,.75);color:#222;padding:1px 5px;border-radius:4px}
#attr a{color:#0645ad}
#readout{margin-top:8px;font-size:13px;color:var(--mut)}
footer{margin:18px 0 8px;text-align:center;color:var(--mut);font-size:13px}
footer a{color:var(--acc)}
</style></head><body><main>
<header>
 <svg viewBox="14 10 172 156" role="img" aria-label="Cold Bore R&amp;D logo"><g fill="none" stroke-linecap="square" stroke-linejoin="miter">
  <path d="M 21.25,156.25 L 178.75,156.25 L 100,16.25 Z" stroke="#76FF4B" stroke-width="2.1"/>
  <path d="M 21.25,156.25 L 59.75,86.25 L 79,116 L 100,16.25 L 178.75,156.25" stroke="#76FF4B" stroke-width="1.47"/>
  <path d="M 122.05,55.45 L 101.05,55.45 M 132.29,73.65 L 111.29,73.65 M 142.52,91.85 L 121.52,91.85 M 152.76,110.05 L 131.76,110.05" stroke="#4BC92A" stroke-width=".95" stroke-linecap="butt"/>
  <path d="M 100,16.25 L 100,101.12" stroke="#4BC92A" stroke-width="1.47"/>
  <circle cx="100" cy="116" r="14.88" stroke="#76FF4B" stroke-width="2.1"/>
  <path d="M 62.38,116 L 85.12,116 M 114.88,116 L 137.62,116 M 100,78.38 L 100,101.12 M 100,130.88 L 100,153.62" stroke="#4BC92A" stroke-width="1.47" stroke-linecap="butt"/>
 </g></svg>
 <div><div class="org">Cold Bore R&amp;D</div><h1>GPS / MGRS bench test</h1></div>
</header>

<div class="card">
 <div class="top"><span id="badge" class="badge">NO FIX</span><span class="sub" id="age">connecting...</span></div>
 <div class="k">MGRS</div><div class="mgrs" id="mgrs">-</div>
 <div class="sub" id="parts">zone - band - square -</div>
</div>

<div class="card">
 <div class="top"><div class="k">Map</div><span class="sub" id="mapnote"></span></div>
 <div id="map">
  <div id="tiles"></div><canvas id="gridc"></canvas>
  <div id="ctl">
   <button id="bin" title="Zoom in">+</button><button id="bout" title="Zoom out">&minus;</button>
   <button id="bme" title="Follow my position">&#9678;</button>
   <button id="bgrid" class="sm" title="MGRS grid on/off">GRID</button>
   <button id="bmap" class="sm" title="Basemap on/off">MAP</button>
  </div>
  <div id="ovl">Waiting for a fix to center the map</div>
  <div id="attr">&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noopener">OpenStreetMap</a></div>
 </div>
 <div id="readout"></div>
</div>

<div class="card"><div class="grid">
 <div><div class="k">Latitude</div><div class="v" id="lat">-</div></div>
 <div><div class="k">Longitude</div><div class="v" id="lon">-</div></div>
 <div><div class="k">Satellites</div><div class="v" id="sats">-</div></div>
 <div><div class="k">HDOP</div><div class="v" id="hdop">-</div></div>
 <div><div class="k">Altitude</div><div class="v" id="alt">-</div></div>
 <div><div class="k">UTC</div><div class="v" id="utc">-</div></div>
 <div><div class="k">NMEA ok / bad</div><div class="v" id="chk">-</div></div>
 <div><div class="k">Uptime</div><div class="v" id="up">-</div></div>
</div></div>

<div class="card">
 <div class="top"><div class="k">Raw NMEA</div><label><input type="checkbox" id="pause"> pause</label></div>
 <pre id="raw">waiting for data...</pre>
</div>

<footer>
 <a href="https://github.com/ColdBore-RD/gps-mgrs-toolkit" target="_blank" rel="noopener">gps-mgrs-toolkit on GitHub</a><br>
 Cold Bore Research &amp; Development
</footer>
</main>
<script>
const $=id=>document.getElementById(id), rad=Math.PI/180;

/* ---- WGS84 <-> UTM (Kruger series) and 100 km square letters ---- */
const A_=6378137,N_=1/298.257223563/(2-1/298.257223563),K0=.9996,AA=A_/(1+N_)*(1+N_*N_/4+N_**4/64);
const AL=[N_/2-2*N_*N_/3+5*N_**3/16,13*N_*N_/48-3*N_**3/5,61*N_**3/240];
const BE=[N_/2-2*N_*N_/3+37*N_**3/96,N_*N_/48+N_**3/15,17*N_**3/480];
const DE=[2*N_-2*N_*N_/3-2*N_**3,7*N_*N_/3-8*N_**3/5,56*N_**3/15];
const zoneOf=lon=>Math.min(60,Math.max(1,Math.floor((lon+180)/6)+1));
const bandOf=lat=>'CDEFGHJKLMNPQRSTUVWXX'[Math.min(20,Math.max(0,Math.floor((lat+80)/8)))];
function toUTM(lat,lon,zone){
 const l0=((zone-1)*6-177)*rad,p=lat*rad,dl=lon*rad-l0,c=2*Math.sqrt(N_)/(1+N_);
 const t=Math.sinh(Math.atanh(Math.sin(p))-c*Math.atanh(c*Math.sin(p)));
 const x0=Math.atan2(t,Math.cos(dl)),e0=Math.atanh(Math.sin(dl)/Math.sqrt(1+t*t));
 let x=x0,e=e0;
 for(let j=1;j<=3;j++){x+=AL[j-1]*Math.sin(2*j*x0)*Math.cosh(2*j*e0);e+=AL[j-1]*Math.cos(2*j*x0)*Math.sinh(2*j*e0);}
 return{e:500000+K0*AA*e,n:K0*AA*x+(lat<0?1e7:0)};
}
function fromUTM(E,Nn,zone,south){
 const x=(Nn-(south?1e7:0))/(K0*AA),e=(E-500000)/(K0*AA);
 let x0=x,e0=e;
 for(let j=1;j<=3;j++){x0-=BE[j-1]*Math.sin(2*j*x)*Math.cosh(2*j*e);e0-=BE[j-1]*Math.cos(2*j*x)*Math.sinh(2*j*e);}
 const chi=Math.asin(Math.sin(x0)/Math.cosh(e0));let p=chi;
 for(let j=1;j<=3;j++)p+=DE[j-1]*Math.sin(2*j*chi);
 return[p/rad,(((zone-1)*6-177)*rad+Math.atan2(Math.sinh(e0),Math.cos(x0)))/rad];
}
function sq100(E,Nn,zone){
 const cols=['ABCDEFGH','JKLMNPQR','STUVWXYZ'][(zone-1)%3],rows='ABCDEFGHJKLMNPQRSTUV';
 const c=cols[Math.floor(E/1e5)-1],r=rows[(Math.floor(Nn/1e5)+(zone%2?0:5))%20];
 return c&&r?c+r:'';
}

/* ---- slippy map ---- */
const map=$('map'),tilesEl=$('tiles'),cv=$('gridc'),ctx=cv.getContext('2d');
let view={lat:0,lon:0,z:17},follow=true,showGrid=true,showMap=true,me=null,W=300,H=300,tileOk=0,tileBad=0;
const tileCache=new Map();
const world=(lat,lon,z)=>{const s=256*2**z,sn=Math.sin(lat*rad);return[(lon+180)/360*s,(.5-Math.log((1+sn)/(1-sn))/(4*Math.PI))*s];};
const unworld=(x,y,z)=>{const s=256*2**z;return[Math.atan(Math.sinh(Math.PI-2*Math.PI*y/s))/rad,x/s*360-180];};
function scr(lat,lon){const[px,py]=world(lat,lon,view.z),[cx,cy]=world(view.lat,view.lon,view.z);return[px-cx+W/2,py-cy+H/2];}
function unscr(x,y){const[cx,cy]=world(view.lat,view.lon,view.z);return unworld(cx+x-W/2,cy+y-H/2,view.z);}

function drawTiles(){
 const z=view.z,n=2**z,[cx,cy]=world(view.lat,view.lon,z),keep=new Set();
 if(showMap&&me){
  for(let ty=Math.floor((cy-H/2)/256);ty<=Math.floor((cy+H/2)/256);ty++){
   if(ty<0||ty>=n)continue;
   for(let tx=Math.floor((cx-W/2)/256);tx<=Math.floor((cx+W/2)/256);tx++){
    const xw=((tx%n)+n)%n,key=z+'/'+xw+'/'+ty+'/'+tx;keep.add(key);
    let im=tileCache.get(key);
    if(!im){im=new Image();im.onload=()=>{tileOk++;note();};im.onerror=()=>{tileBad++;note();im.style.display='none';};
     im.src='https://tile.openstreetmap.org/'+z+'/'+xw+'/'+ty+'.png';tileCache.set(key,im);tilesEl.appendChild(im);}
    im.style.left=Math.round(tx*256-cx+W/2)+'px';im.style.top=Math.round(ty*256-cy+H/2)+'px';
   }}
 }
 for(const[k,im]of tileCache)if(!keep.has(k)){im.remove();tileCache.delete(k);}
}
function note(){
 $('mapnote').textContent=!showMap?'basemap off':tileOk?'':tileBad>2?'no internet on this phone: grid only':'';
 $('attr').style.display=(showMap&&tileOk)?'':'none';
}

function drawGrid(){
 const dpr=devicePixelRatio||1;ctx.setTransform(dpr,0,0,dpr,0,0);ctx.clearRect(0,0,W,H);
 let info='';
 if(me){
  const lat=view.lat,zone=zoneOf(view.lon),south=lat<0;
  if(showGrid&&lat<84&&lat>-80){
   const mpp=156543.03392*Math.cos(lat*rad)/2**view.z;
   const s=[1,10,100,1000,10000,100000].find(v=>v>=70*mpp)||100000;
   const cs=[[0,0],[W,0],[0,H],[W,H],[W/2,H/2]].map(p=>{const q=unscr(p[0],p[1]);return toUTM(q[0],q[1],zone);});
   const emin=Math.min(...cs.map(c=>c.e)),emax=Math.max(...cs.map(c=>c.e)),nmin=Math.min(...cs.map(c=>c.n)),nmax=Math.max(...cs.map(c=>c.n));
   const E0=Math.ceil(emin/s)*s,N0=Math.ceil(nmin/s)*s;
   if((emax-E0)/s<60&&(nmax-N0)/s<60){
    ctx.font='11px ui-monospace,Consolas,monospace';ctx.textBaseline='top';
    const pt=(e,n)=>scr(...fromUTM(e,n,zone,south));
    const digits=v=>{const len=Math.round(Math.log10(1e5/s));return len?String(Math.floor((v%1e5)/s)).padStart(len,'0'):'';};
    const stroke=v=>{ctx.lineWidth=v%1e5===0?3:v%1e4===0?2:1.3;ctx.strokeStyle=v%1e5===0?'rgba(255,100,0,1)':'rgba(255,110,0,.85)';};
    const label=(t,x,y)=>{if(!t)return;ctx.lineWidth=3;ctx.strokeStyle='rgba(0,0,0,.65)';ctx.strokeText(t,x,y);ctx.fillStyle='#ffd27a';ctx.fillText(t,x,y);};
    for(let e=E0;e<=emax;e+=s){
     stroke(e);ctx.beginPath();for(let i=0;i<=4;i++){const p=pt(e,nmin+(nmax-nmin)*i/4);i?ctx.lineTo(p[0],p[1]):ctx.moveTo(p[0],p[1]);}ctx.stroke();
     const p=pt(e,nmax);label(digits(e),Math.max(2,Math.min(W-30,p[0]+3)),4);
    }
    for(let n=N0;n<=nmax;n+=s){
     stroke(n);ctx.beginPath();for(let i=0;i<=4;i++){const p=pt(emin+(emax-emin)*i/4,n);i?ctx.lineTo(p[0],p[1]):ctx.moveTo(p[0],p[1]);}ctx.stroke();
     const p=pt(emin,n);label(digits(n),3,Math.max(16,Math.min(H-14,p[1]-14)));
    }
    if(s>=10000){ctx.font='700 38px ui-monospace,Consolas,monospace';ctx.textAlign='center';ctx.textBaseline='middle';
     for(let ce=Math.floor(emin/1e5);ce<=Math.floor(emax/1e5);ce++)for(let cn=Math.floor(nmin/1e5);cn<=Math.floor(nmax/1e5);cn++){
      const t=sq100(ce*1e5+5e4,cn*1e5+5e4,zone);if(!t)continue;const p=pt(ce*1e5+5e4,cn*1e5+5e4);
      ctx.fillStyle='rgba(255,170,0,.4)';ctx.fillText(t,p[0],p[1]);}
     ctx.textAlign='left';}
    const u=toUTM(view.lat,view.lon,zone);
    info='Grid '+(s>=1000?s/1000+' km':s+' m')+' &middot; zone '+zone+bandOf(lat)+' &middot; 100 km square '+sq100(u.e,u.n,zone);
   } else info='Zoom in to see the MGRS grid';
  }
  if(me){const[x,y]=scr(me[0],me[1]);
   ctx.beginPath();ctx.arc(x,y,13,0,7);ctx.fillStyle='rgba(118,255,75,.22)';ctx.fill();
   ctx.beginPath();ctx.arc(x,y,6,0,7);ctx.fillStyle='#76ff4b';ctx.fill();ctx.lineWidth=2.5;ctx.strokeStyle='#0a0a0a';ctx.stroke();}
 }
 $('readout').innerHTML=info;
}
function redraw(){drawTiles();drawGrid();}
function resize(){W=map.clientWidth;H=map.clientHeight;const d=devicePixelRatio||1;cv.width=W*d;cv.height=H*d;redraw();}
new ResizeObserver(resize).observe(map);

function setBtn(){$('bme').classList.toggle('off',!follow);$('bgrid').classList.toggle('off',!showGrid);$('bmap').classList.toggle('off',!showMap);}
$('bin').onclick=()=>{view.z=Math.min(19,view.z+1);redraw();};
$('bout').onclick=()=>{view.z=Math.max(3,view.z-1);redraw();};
$('bme').onclick=()=>{follow=true;if(me){view.lat=me[0];view.lon=me[1];}setBtn();redraw();};
$('bgrid').onclick=()=>{showGrid=!showGrid;setBtn();redraw();};
$('bmap').onclick=()=>{showMap=!showMap;setBtn();note();redraw();};
let drag=null;
map.addEventListener('pointerdown',e=>{if(e.target.closest('#ctl'))return;drag={x:e.clientX,y:e.clientY};map.setPointerCapture(e.pointerId);});
map.addEventListener('pointermove',e=>{if(!drag||!me)return;const dx=e.clientX-drag.x,dy=e.clientY-drag.y;drag={x:e.clientX,y:e.clientY};
 const[cx,cy]=world(view.lat,view.lon,view.z),p=unworld(cx-dx,cy-dy,view.z);view.lat=Math.max(-84,Math.min(84,p[0]));view.lon=p[1];follow=false;setBtn();redraw();});
map.addEventListener('pointerup',()=>{drag=null;});
setBtn();

/* ---- live data ---- */
async function tick(){
 try{
  const d=await(await fetch('/data',{cache:'no-store'})).json();
  const b=$('badge');b.textContent=d.fix?'FIX':'NO FIX';b.className='badge'+(d.fix?' ok':'');
  $('mgrs').textContent=d.mgrs||'-';
  $('parts').textContent=d.mgrs?('zone '+d.zone+' - band '+d.band+' - square '+d.square+' - E '+d.e+' N '+d.n):'no MGRS until a fix';
  $('lat').textContent=d.fix?d.lat.toFixed(6):'-';
  $('lon').textContent=d.fix?d.lon.toFixed(6):'-';
  $('sats').textContent=d.sats;$('hdop').textContent=d.hdop>0?d.hdop.toFixed(1):'-';
  $('alt').textContent=d.alt!==null?d.alt.toFixed(1)+' m':'-';
  $('utc').textContent=d.utc||'-';
  $('chk').textContent=d.ok+' / '+d.bad;
  $('up').textContent=d.up+' s';
  $('age').textContent=d.fix?('fix age '+(d.age/1000).toFixed(1)+' s'):(d.chars>0?'receiving NMEA, searching':'no NMEA - check wiring');
  if(d.fix){const first=!me;me=[d.lat,d.lon];$('ovl').style.display='none';if(first||follow){view.lat=d.lat;view.lon=d.lon;}}
  else{me=null;$('ovl').style.display='';}
  redraw();
  if(!$('pause').checked){const r=$('raw'),bot=r.scrollTop+r.clientHeight>=r.scrollHeight-8;
   r.textContent=d.nmea.join('\n');if(bot)r.scrollTop=r.scrollHeight;}
 }catch(e){$('age').textContent='connection lost';}
}
tick();setInterval(tick,1000);
</script></body></html>)HTML";
