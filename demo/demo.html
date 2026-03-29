<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>IRUTESAM — Dark Venom v1.0 | TFT Simulator</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{background:#0a0a0a;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;font-family:monospace;gap:16px;padding:20px}
  h1{color:#003040;font-size:11px;letter-spacing:.2em;text-transform:uppercase}
  .rig{display:flex;flex-direction:column;align-items:center;gap:0}
  .device{background:#111418;border-radius:22px;padding:18px 16px 16px;border:1.5px solid #1e2428;box-shadow:0 8px 40px #000a,0 0 0 1px #0c0f11;display:flex;flex-direction:column;align-items:center;gap:10px}
  .cam-row{display:flex;align-items:center;gap:8px;width:100%;padding:0 4px}
  .cam{width:6px;height:6px;border-radius:50%;background:#141a1e;border:1px solid #1e262c}
  .cam-line{flex:1;height:2px;background:#131a1e;border-radius:1px}
  .screen-frame{border:2.5px solid #0d1418;border-radius:6px;overflow:hidden;box-shadow:inset 0 0 8px #000}
  canvas{display:block;image-rendering:pixelated;image-rendering:crisp-edges}
  .ctrl{display:flex;gap:6px;align-items:center;margin-top:4px}
  .btn{width:32px;height:32px;border-radius:50%;background:#131a1e;border:1.5px solid #1e2a30;color:#445560;cursor:pointer;display:flex;align-items:center;justify-content:center;font-size:10px;font-family:monospace;font-weight:700;user-select:none;transition:background .08s,color .08s,transform .08s;outline:none}
  .btn:hover{background:#1a2830;color:#6699aa;border-color:#274050}
  .btn:active{transform:scale(.89);background:#0d1d28}
  #bOK{width:40px;height:40px;font-size:11px;background:#091c2c;border:2px solid #0f3a55;color:#1e6a8a;font-weight:700}
  #bOK:hover{color:#2a9bbf;border-color:#196080;background:#0c2438}
  .info{font-size:10px;color:#1a2d36;letter-spacing:.12em;text-align:center;margin-top:4px}
  .brand{font-size:9px;color:#0f1d24;letter-spacing:.3em;text-transform:uppercase;margin-top:2px}
</style>
</head>
<body>
<div class="rig">
  <div class="device">
    <div class="cam-row"><div class="cam-line"></div><div class="cam"></div><div class="cam-line"></div></div>
    <div class="screen-frame">
      <!-- Canvas exactly 240x320, displayed at 2.5x = 600x800 -->
      <canvas id="tft" width="240" height="320" style="width:240px;height:320px"></canvas>
    </div>
    <div class="ctrl">
      <button class="btn" id="bUP"  title="UP">▲</button>
      <button class="btn" id="bBK"  title="BACK" style="font-size:9px">◀</button>
      <button class="btn" id="bOK"  title="SELECT">OK</button>
      <button class="btn" id="bFW"  title="FWD" style="font-size:9px">▶</button>
      <button class="btn" id="bDN"  title="DOWN">▼</button>
    </div>
  </div>
  <div class="info" id="sinfo">BOOT SEQUENCE — tap screen to skip</div>
  <div class="brand">IRUTESAM · DARK VENOM · v1.0</div>
</div>

<script>
// ════════════════════════════════════════════════════════
// IRUTESAM Dark Venom — TFT Simulator (exact 240×320)
// Mirrors tft_ui.cpp pixel-for-pixel
// ════════════════════════════════════════════════════════

const C = document.getElementById('tft');
const ctx = C.getContext('2d');

// ── Polyfill roundRect ───────────────────────────────────
if (!CanvasRenderingContext2D.prototype.roundRect) {
  CanvasRenderingContext2D.prototype.roundRect = function(x,y,w,h,r) {
    this.beginPath();
    this.moveTo(x+r,y); this.lineTo(x+w-r,y);
    this.quadraticCurveTo(x+w,y,x+w,y+r); this.lineTo(x+w,y+h-r);
    this.quadraticCurveTo(x+w,y+h,x+w-r,y+h); this.lineTo(x+r,y+h);
    this.quadraticCurveTo(x,y+h,x,y+h-r); this.lineTo(x,y+r);
    this.quadraticCurveTo(x,y,x+r,y); this.closePath();
  };
}

// ── Dark Venom Palette ───────────────────────────────────
const P = {
  BG:'#060f18', B2:'#0b1c2a', CY:'#00dff0', CM:'#006878',
  CD:'#002a36', PU:'#8b2eee', PK:'#ee1f68', RD:'#ee2020',
  GN:'#00e876', AM:'#f0a000', WH:'#b8e0f8', HI:'#07202e',
  PN:'#071826', RF:'#1a0808', AF:'#1a1000', GF:'#001a10',
  DRD:'#3a0808'
};

// ── Draw Primitives ──────────────────────────────────────
const fil  = c         => { ctx.fillStyle=c; ctx.fillRect(0,0,240,320); };
const fr   = (x,y,w,h,c)  => { ctx.fillStyle=c; ctx.fillRect(x,y,w,h); };
const dr   = (x,y,w,h,c)  => { ctx.strokeStyle=c; ctx.lineWidth=1; ctx.strokeRect(x+.5,y+.5,w,h); };
const dl   = (x1,y1,x2,y2,c) => { ctx.strokeStyle=c; ctx.lineWidth=1; ctx.beginPath(); ctx.moveTo(x1+.5,y1+.5); ctx.lineTo(x2+.5,y2+.5); ctx.stroke(); };
const dhl  = (x,y,w,c)    => dl(x,y,x+w,y,c);
const dvl  = (x,y,h,c)    => dl(x,y,x,y+h,c);
const dc   = (cx,cy,r,c)  => { ctx.strokeStyle=c; ctx.lineWidth=1; ctx.beginPath(); ctx.arc(cx,cy,r,0,Math.PI*2); ctx.stroke(); };
const fc   = (cx,cy,r,c)  => { ctx.fillStyle=c; ctx.beginPath(); ctx.arc(cx,cy,r,0,Math.PI*2); ctx.fill(); };
const frr  = (x,y,w,h,r,c) => { ctx.fillStyle=c; ctx.beginPath(); ctx.roundRect(x,y,w,h,r); ctx.fill(); };
const drr  = (x,y,w,h,r,c) => { ctx.strokeStyle=c; ctx.lineWidth=1; ctx.beginPath(); ctx.roundRect(x,y,w,h,r); ctx.stroke(); };

// ── Text Renderer (bitmap monospace simulation) ──────────
let _ts=1, _tc=P.CY, _cx=0, _cy=0;
const CW = 6, CH = 8;
const ss = s => _ts = s;
const sc = c => _tc = c;
const cur = (x,y) => { _cx=x; _cy=y; };
function pr(t) {
  const fh = CH * _ts;
  ctx.font = `${fh}px 'Courier New',monospace`;
  ctx.textBaseline = 'top';
  ctx.fillStyle = _tc;
  ctx.fillText(t, _cx, _cy - (_ts > 1 ? _ts * 0 : 0));
  _cx += t.length * CW * _ts;
}
const gw = (t, s=_ts) => t.length * CW * s;
function ct(t, y, c, s) {
  ss(s); sc(c);
  const w = gw(t, s);
  cur(Math.round((240 - w) / 2), y);
  pr(t);
}

// ── Config constants matching config.h ───────────────────
const STATUS_H   = 16;
const FOOTER_Y   = 298;
const FOOTER_H   = 22;
const MENU_START_Y = 34;
const MENU_ROW_H   = 44;
const MENU_MAX_ROWS = 5;
const WIFI_LIST_ROW_H   = 30;
const WIFI_LIST_START_Y = 34;
const WIFI_LIST_MAX_VIS = 8;
const SEC_OPEN=0, SEC_WEP=1, SEC_WPA=2, SEC_WPA2=3, SEC_WPA3=4;

// ── Peripheral flags ─────────────────────────────────────
let wifiReady=true, sdAvailable=true, nfcAvailable=true, touchAvailable=true;

// ════════════════════════════════════════════════════════
// drawStatusBar() — mirrors tft_ui.cpp exactly
// ════════════════════════════════════════════════════════
function drawStatusBar(name, nameColor) {
  fr(0,0,240,STATUS_H,P.B2);
  dhl(0,STATUS_H,240,P.CM);
  ss(1); sc(nameColor); cur(6,4); pr(name);
  fr(196,6,4,4, wifiReady    ? P.CY : P.CD);
  fr(206,6,4,4, sdAvailable  ? P.CY : P.CD);
  fr(216,6,4,4, nfcAvailable ? P.PU : P.CD);
  fr(226,6,4,4, touchAvailable ? P.CY : P.AM);
}

// ════════════════════════════════════════════════════════
// drawFooter() — mirrors tft_ui.cpp exactly
// ════════════════════════════════════════════════════════
function drawFooter(h1, h2, h3) {
  fr(0,FOOTER_Y,240,FOOTER_H,P.B2);
  dhl(0,FOOTER_Y,240,P.CM);
  ss(1);
  sc(P.CY);  cur(4,308); pr(h1);
  sc(P.CD);  pr(' '); pr(h2);
  if (h3 && h3.length) { sc(P.CD); cur(170,308); pr(h3); }
  if (touchAvailable) fc(230,308,4,P.CD);
}

// ════════════════════════════════════════════════════════
// drawMenu() — mirrors tft_ui.cpp exactly
// ════════════════════════════════════════════════════════
function drawMenu(items, count, sel, offset, title) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// '); pr(title);
  const maxVis = MENU_MAX_ROWS;
  for (let v=0; v<maxVis; v++) {
    const idx = offset+v;
    if (idx >= count) break;
    const rowY = MENU_START_Y + v * MENU_ROW_H;
    const isSel = (idx === sel);
    if (isSel) {
      fr(0,rowY,234,MENU_ROW_H-2,P.HI);
      sc(P.CY); ss(2); cur(6,rowY+8); pr('>');
      cur(20,rowY+8); pr(items[idx]);
      fc(228,rowY+17,3,P.PK);
    } else {
      sc(P.CD); ss(2); cur(20,rowY+8); pr(items[idx]);
    }
  }
  if (count > maxVis) {
    fr(236,MENU_START_Y,2,MENU_ROW_H*maxVis,P.B2);
    const thumbH = Math.max(4,Math.floor(MENU_ROW_H*maxVis*maxVis/count));
    const thumbY = MENU_START_Y + Math.floor(offset*(MENU_ROW_H*maxVis-thumbH)/(count-maxVis));
    fr(236,thumbY,2,thumbH,P.CY);
  }
}

// ════════════════════════════════════════════════════════
// drawDialog() — mirrors tft_ui.cpp exactly
// ════════════════════════════════════════════════════════
function drawDialog(title, b1, b2, yesSelected) {
  frr(20,95,200,95,4,P.PN);
  drr(20,95,200,95,4,P.CM);
  ct(title,112,P.PK,1);
  dhl(30,122,180,P.CM);
  ct(b1,135,P.WH,1);
  if (b2 && b2.length) ct(b2,148,P.WH,1);
  // YES button
  frr(38,160,72,22,3, yesSelected ? P.HI : P.B2);
  drr(38,160,72,22,3, yesSelected ? P.CY : P.CM);
  ss(1); sc(yesSelected ? P.CY : P.CD); cur(64,167); pr('YES');
  // NO button
  frr(130,160,72,22,3, !yesSelected ? P.HI : P.B2);
  drr(130,160,72,22,3, !yesSelected ? P.CY : P.CM);
  sc(!yesSelected ? P.CY : P.CD); cur(156,167); pr('NO');
}

// ════════════════════════════════════════════════════════
// drawWifiList() — mirrors tft_ui.cpp exactly
// ════════════════════════════════════════════════════════
const WIFI_NETS = [
  {s:'Airtel_5G_Home',    r:-52, t:SEC_WPA3},
  {s:'JIOFIBER_2.4G',     r:-61, t:SEC_WPA2},
  {s:'AndroidAP_F4A2',   r:-70, t:SEC_WPA2},
  {s:'TP-LINK_8B3C',     r:-75, t:SEC_WPA},
  {s:'Neighbor_WiFi',    r:-80, t:SEC_WEP},
  {s:'FREE_WIFI_HERE',   r:-58, t:SEC_OPEN},
  {s:'BSNL_Broadband',   r:-85, t:SEC_WPA2},
  {s:'HP-Print-5G',      r:-88, t:SEC_WPA2},
];
let wSel=0, wOff=0;

function drawWifiList(nets, sel, offset) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// WiFi Networks');
  const maxVis = WIFI_LIST_MAX_VIS;
  for (let v=0; v<maxVis; v++) {
    const idx = offset+v;
    if (idx >= nets.length) break;
    const n = nets[idx];
    const rowY = WIFI_LIST_START_Y + v * WIFI_LIST_ROW_H;
    const isSel = (idx === sel);
    if (isSel) fr(0,rowY,210,WIFI_LIST_ROW_H-2,P.HI);
    ss(1); sc(isSel ? P.CY : P.CD);
    cur(4,rowY+4); pr(n.s.substring(0,18));
    sc(P.CD); cur(140,rowY+4); pr(''+n.r);
    let bt,bb,bf,bl;
    if      (n.t===SEC_OPEN)              { bt=P.RD; bb=P.RD; bf=P.RF; bl='OPEN'; }
    else if (n.t===SEC_WEP||n.t===SEC_WPA){ bt=P.AM; bb=P.AM; bf=P.AF; bl=(n.t===SEC_WEP?'WEP':'WPA'); }
    else                                   { bt=P.GN; bb=P.GN; bf=P.GF; bl=(n.t===SEC_WPA3?'WPA3':'WPA2'); }
    frr(155,rowY+2,52,14,2,bf);
    drr(155,rowY+2,52,14,2,bb);
    sc(bt);
    const bw = gw(bl,1);
    cur(155+Math.floor((52-bw)/2), rowY+5); pr(bl);
    const bars = n.r>-60 ? 3 : n.r>-75 ? 2 : 1;
    for (let b=0;b<3;b++) {
      const bh2=4+b*3, bx2=216+b*5, by2=rowY+14-bh2;
      fr(bx2,by2,4,bh2, b<bars ? P.CY : P.CM);
    }
  }
}

// ════════════════════════════════════════════════════════
// drawWifiDetail()
// ════════════════════════════════════════════════════════
function drawWifiDetail(net) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// Network Detail');
  dhl(0,30,240,P.CM);
  const rows = [
    ['SSID:',net.s],['BSSID:','4C:ED:DE:'+net.s.substring(0,2).toUpperCase()+':11:AA'],
    ['RSSI:',net.r+' dBm'],['CHANNEL:','6'],
    ['SECURITY:',['OPEN','WEP','WPA','WPA2','WPA3'][net.t]||'?']
  ];
  rows.forEach(([k,v],i)=>{
    ss(1); sc(P.CD); cur(6,40+i*16); pr(k);
    sc(P.CY); cur(70,40+i*16); pr(v);
  });
  dhl(0,124,240,P.CM);
  sc(P.PK); cur(6,136); pr('SELECT');
  sc(P.CD); pr(' > Launch Deauth Attack');
  sc(P.GN); cur(6,152); pr('BACK');
  sc(P.CD); pr(' > Return to list');
}

// ════════════════════════════════════════════════════════
// drawDeauthScreen() — mirrors tft_ui.cpp
// ════════════════════════════════════════════════════════
function drawDeauthScreen(ssid, channel, pktCount, pulse) {
  fil(P.BG);
  ct('DEAUTH ACTIVE',40,pulse?P.RD:P.CD,2);
  const rcx=120,rcy=134;
  if (pulse) { fc(rcx,rcy,28,P.RD); }
  else       { fc(rcx,rcy,28,P.BG); dc(rcx,rcy,28,P.RD); }
  for (let i=0;i<6;i++) {
    const a=i*60*Math.PI/180;
    dl(rcx+Math.round(35*Math.cos(a)),rcy+Math.round(35*Math.sin(a)),
       rcx+Math.round(50*Math.cos(a)),rcy+Math.round(50*Math.sin(a)),P.RD);
  }
  dc(rcx,rcy,52,P.CM);
  ss(1); sc(P.CD); cur(6,194); pr('TARGET:');
  sc(P.CY); cur(60,194); pr(ssid);
  sc(P.AM); cur(6,210); pr('PKT:');
  sc(P.RD); cur(40,210); pr(''+pktCount);
  ct('CH '+channel,224,P.PU,1);
  ct('TAP LOWER HALF TO STOP',272,P.CD,1);
}

// ════════════════════════════════════════════════════════
// drawDeauthProtect()
// ════════════════════════════════════════════════════════
function drawDeauthProtect(deauthCount) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// Deauth Protect');
  dhl(0,30,240,P.CM);
  sc(P.CD); cur(6,44); pr('Monitoring for deauth...');
  sc(P.AM); cur(6,70); pr('DEAUTH FRAMES DETECTED:');
  ct(''+deauthCount,96,deauthCount>0?P.RD:P.GN,3);
  ct('BACK to stop monitoring',262,P.CD,1);
}

// ════════════════════════════════════════════════════════
// drawSecurityAnalysis()
// ════════════════════════════════════════════════════════
function drawSecurityAnalysis(nets) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// Security Analysis');
  dhl(0,30,240,P.CM);
  let open=0,weak=0,strong=0;
  nets.forEach(n=>{
    if(n.t===SEC_OPEN)open++;
    else if(n.t===SEC_WEP||n.t===SEC_WPA)weak++;
    else strong++;
  });
  sc(P.CD); cur(6,40); pr('Total networks: ');
  sc(P.CY); pr(''+nets.length);
  sc(P.RD);  cur(6,58); pr('OPEN:   '+open);
  sc(P.AM);  cur(6,74); pr('WEAK:   '+weak);
  sc(P.GN);  cur(6,90); pr('SECURE: '+strong);
  dhl(0,106,240,P.CM);
  sc(P.CD); cur(6,116); pr('Risk score:');
  const risk = Math.min(100,open*25+weak*10);
  sc(risk>50?P.RD:risk>20?P.AM:P.GN); cur(90,116); pr(risk+'%');
  // bar
  fr(6,132,228,10,P.B2); drr(6,132,228,10,2,P.CM);
  fr(6,132,Math.floor(228*risk/100),10,risk>50?P.RD:risk>20?P.AM:P.GN);
}

// ════════════════════════════════════════════════════════
// drawNFCScan() — mirrors tft_ui.cpp
// ════════════════════════════════════════════════════════
let nfcStep=0, nfcUID=null;
function drawNFCScan() {
  const cx=120,cy=130;
  const cs=[[P.CY,P.PU,P.CM],[P.PU,P.CY,P.CM],[P.CM,P.CY,P.PU]];
  const [c0,c1,c2]=cs[nfcStep%3];
  fr(cx-58,cy-58,116,116,P.BG);
  dc(cx,cy,18,c0); dc(cx,cy,32,c1); dc(cx,cy,46,c2);
  fc(cx,cy,4,P.CY);
  for(let i=0;i<6;i++){
    const a=i*60*Math.PI/180;
    dl(cx+Math.round(49*Math.cos(a)),cy+Math.round(49*Math.sin(a)),
       cx+Math.round(54*Math.cos(a)),cy+Math.round(54*Math.sin(a)),P.CD);
  }
  frr(8,195,224,28,3,P.PN); drr(8,195,224,28,3,P.CM);
  if (nfcUID) {
    ss(2); sc(P.CY);
    const w=gw(nfcUID,2); cur(Math.round((240-w)/2),200); pr(nfcUID);
    ct('UID LEN: 4 bytes',232,P.PU,1);
  } else {
    ct('SCANNING...',204,P.CD,1);
  }
}

// ════════════════════════════════════════════════════════
// drawNFCEmulate()
// ════════════════════════════════════════════════════════
function drawNFCEmulate(uid, pulse) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  frr(8,60,224,28,3,P.PN); drr(8,60,224,28,3,P.CM);
  ss(2); sc(P.CY);
  const w=gw(uid,2); cur(Math.round((240-w)/2),66); pr(uid);
  ct(pulse?'BROADCASTING...':'STANDBY...',115,pulse?P.CY:P.CD,1);
  const ecx=120,ecy=178;
  fr(ecx-58,ecy-58,116,116,P.BG);
  dc(ecx,ecy,20,P.PU); dc(ecx,ecy,35,P.CY); dc(ecx,ecy,50,P.CM);
  fc(ecx,ecy,6,pulse?P.PU:P.CD);
}

// ════════════════════════════════════════════════════════
// drawIRCapture() — mirrors tft_ui.cpp
// ════════════════════════════════════════════════════════
const IR_RAW_DEMO=[350,900,450,850,400,900,450,1200,350,600,800,900,450,1800,400,950,350,700,800,1200,450,900];
function drawIRCapture(rawData) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PK); cur(6,20); pr('// IR Capture');
  fr(8,38,224,58,P.PN); dr(8,38,224,58,P.CM);
  if (!rawData || rawData.length===0) {
    ct('WAITING FOR SIGNAL...',64,P.CD,1); return;
  }
  const panelX=10,panelW=220,panelY=40,panelH=56;
  const midY=panelY+Math.floor(panelH/2);
  let maxVal=1; rawData.forEach(v=>{ if(v>maxVal)maxVal=v; });
  const pxPerSample=Math.max(1,Math.floor(panelW/rawData.length));
  let xPos=panelX;
  rawData.forEach((v,i)=>{
    if(xPos>=panelX+panelW)return;
    let w=Math.max(1,Math.floor(v*pxPerSample*4/maxVal));
    if(xPos+w>panelX+panelW)w=panelX+panelW-xPos;
    const col=i%2===0?P.CY:P.CM;
    const barH=Math.floor(panelH/2);
    const barY=i%2===0?panelY+4:midY+2;
    fr(xPos,barY,w,barH-4,col);
    xPos+=w;
  });
  ct('RAW SAMPLES: '+rawData.length,104,P.CD,1);
}

// ════════════════════════════════════════════════════════
// drawIRSignalInfo()
// ════════════════════════════════════════════════════════
function drawIRSignalInfo(proto, addr, cmd) {
  frr(8,116,224,72,3,P.PN); drr(8,116,224,72,3,P.CM);
  ss(1); sc(P.PU); cur(16,126); pr('PROTO');
  dvl(70,120,60,P.CM);
  sc(P.CY); cur(80,126); pr(proto);
  dhl(16,140,208,P.CM); sc(P.PU); cur(16,146); pr('ADDR');
  sc(P.PK); cur(80,146); pr('0x'+addr.toString(16).toUpperCase().padStart(4,'0'));
  dhl(16,160,208,P.CM); sc(P.PU); cur(16,166); pr('CMD');
  sc(P.PK); cur(80,166); pr('0x'+cmd.toString(16).toUpperCase().padStart(4,'0'));
}

// ════════════════════════════════════════════════════════
// drawIRSend()
// ════════════════════════════════════════════════════════
function drawIRSend(pulse) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PK); cur(6,20); pr('// IR Transmit');
  ss(2); ct('TRANSMITTING',88,pulse?P.CY:P.CD,2);
  const cx=120,cy=168;
  fr(cx-58,cy-58,116,116,P.BG);
  dc(cx,cy,20,P.PK); dc(cx,cy,35,P.PU); dc(cx,cy,50,P.CY);
  if(pulse) fc(cx,cy,6,P.PK);
}

// ════════════════════════════════════════════════════════
// drawAbout() — mirrors tft_ui.cpp
// ════════════════════════════════════════════════════════
function drawAbout() {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// About IRUTESAM');
  dhl(0,30,240,P.CM);
  ct('IRUTESAM',44,P.CY,2);
  ct('Dark Venom',68,P.PU,1);
  ct('v1.0',82,P.PU,1);
  dhl(20,96,200,P.CM);
  ss(1); sc(P.CD);
  cur(6,108); pr('Board: ESP32 DevKit V1');
  cur(6,122); pr('Display: ILI9341 2.8in 240x320');
  cur(6,136); pr('NFC: PN532 I2C');
  cur(6,150); pr('IR: VS1838B RX / 38kHz TX');
  sc(P.CY); cur(6,170); pr('WiFi: '); sc(wifiReady?P.GN:P.RD); pr(wifiReady?'READY':'OFFLINE');
  sc(P.CY); cur(6,184); pr('SD:   '); sc(sdAvailable?P.GN:P.RD); pr(sdAvailable?'MOUNTED':'MISSING');
  sc(P.CY); cur(6,198); pr('NFC:  '); sc(nfcAvailable?P.GN:P.RD); pr(nfcAvailable?'ONLINE':'OFFLINE');
  sc(P.CY); cur(6,212); pr('Touch:'); sc(touchAvailable?P.GN:P.AM); pr(touchAvailable?' READY':' OFFLINE');
}

// ════════════════════════════════════════════════════════
// drawBrightness() — mirrors tft_ui.cpp
// ════════════════════════════════════════════════════════
let brite=180;
function drawBrightness() {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// Brightness');
  dhl(0,30,240,P.CM);
  ct(''+brite,76,P.CY,3);
  const barW=Math.floor(brite*200/255);
  fr(20,128,200,16,P.B2); dr(20,128,200,16,P.CM);
  fr(20,128,barW,16,P.CY);
  ct('UP/DOWN to adjust',158,P.CD,1);
  ct('OK to confirm',172,P.CD,1);
}

// ════════════════════════════════════════════════════════
// drawTouchCalCross()
// ════════════════════════════════════════════════════════
function drawTouchCalCross(step) {
  fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
  ss(1); sc(P.PU); cur(6,20); pr('// Touch Calibration');
  const pts=[[20,280],[220,280],[220,60],[20,60]];
  const [px,py]=pts[step]||[120,160];
  ct('Tap corner '+(step+1)+' of 4',152,P.CD,1);
  dhl(px-12,py,24,P.CY); dvl(px,py-12,24,P.CY);
  dc(px,py,6,P.PK);
}

// ════════════════════════════════════════════════════════
// Saved lists
// ════════════════════════════════════════════════════════
const SAVED_NFC=['Card-A04F2E','Card-8C11D4','Tag-EB3344','Mifare-7A2F','Card-0012FF'];
const SAVED_IR=['TV_Power','AC_Cool22','Fan_Speed3','LED_Toggle','Amp_VolUp'];
const FILES=['wifi_scan_0001.json','nfc_uid_list.txt','ir_nec_set1.bin','log_20240101.txt','config.json'];

// ════════════════════════════════════════════════════════
// Main menu definition
// ════════════════════════════════════════════════════════
const MENU_ITEMS=[
  'WiFi Scanner','Deauth Attack','Deauth Protect','Security Analysis',
  'NFC Read','NFC Emulate','Saved NFC',
  'IR Capture','IR Transmit','Saved IR',
  'Files','Brightness','About'
];
let mSel=0, mOff=0;

// ════════════════════════════════════════════════════════
// Screen router
// ════════════════════════════════════════════════════════
let screen='boot';
let animHandle=null;

const stop = () => { if(animHandle){clearTimeout(animHandle);animHandle=null;} };

// Screen name display
const SCREEN_NAMES = {
  menu:'MAIN MENU',wifi:'WiFi Scanner',wdetail:'Network Detail',
  deauth:'Deauth Active',dprotect:'Deauth Protect',secanal:'Security Analysis',
  nfc:'NFC Scan',nfcemul:'NFC Emulate',savednfc:'Saved NFC',
  ir:'IR Capture',irsend:'IR Transmit',saveir:'Saved IR',
  files:'Files',bright:'Brightness',about:'About',tcal:'Touch Cal',boot:'BOOT SEQUENCE'
};

function setInfo(s){ document.getElementById('sinfo').textContent=SCREEN_NAMES[s]||s; }

// ════════════════════════════════════════════════════════
// RENDER — draws the current screen
// ════════════════════════════════════════════════════════
function render() {
  stop();
  setInfo(screen);

  if (screen==='menu') {
    drawStatusBar('IRUTESAM',P.CY);
    drawMenu(MENU_ITEMS,MENU_ITEMS.length,mSel,mOff,'MAIN MENU');
    drawFooter('OK','select','SCROLL');

  } else if (screen==='wifi') {
    drawStatusBar('WiFi Scanner',P.CY);
    drawWifiList(WIFI_NETS,wSel,wOff);
    drawFooter('OK','detail','SCROLL');

  } else if (screen==='wdetail') {
    drawStatusBar('Network Detail',P.CY);
    drawWifiDetail(WIFI_NETS[wSel]);
    drawFooter('BCK','back','OK=ATCK');

  } else if (screen==='deauth') {
    let pulse=false, pkt=0;
    drawStatusBar('DEAUTH ACTIVE',P.RD);
    drawDeauthScreen(WIFI_NETS[wSel].s,6,pkt,pulse);
    drawFooter('STOP','tap lower','');
    const tick=()=>{
      pulse=!pulse; if(pulse) pkt+=Math.floor(Math.random()*8+3);
      drawStatusBar('DEAUTH ACTIVE',pulse?P.RD:P.CD);
      drawDeauthScreen(WIFI_NETS[wSel].s,6,pkt,pulse);
      animHandle=setTimeout(tick,380);
    };
    animHandle=setTimeout(tick,380);

  } else if (screen==='dprotect') {
    drawStatusBar('Deauth Protect',P.GN);
    drawDeauthProtect(0);
    drawFooter('BCK','stop','');

  } else if (screen==='secanal') {
    drawStatusBar('Security Analysis',P.CY);
    drawSecurityAnalysis(WIFI_NETS);
    drawFooter('BCK','back','');

  } else if (screen==='nfc') {
    nfcStep=0; nfcUID=null;
    drawStatusBar('NFC Read',P.PU);
    fr(0,STATUS_H,240,FOOTER_Y-STATUS_H,P.BG);
    ss(1); sc(P.PU); cur(6,20); pr('// NFC Scan');
    drawNFCScan();
    drawFooter('BCK','cancel','AUTO');
    const tick=()=>{
      nfcStep++;
      drawStatusBar('NFC Read',P.PU);
      drawNFCScan();
      drawFooter('BCK','cancel','AUTO');
      if(nfcStep===12) nfcUID='A4 3F 88 C2';
      if(nfcStep<20) animHandle=setTimeout(tick,350);
    };
    animHandle=setTimeout(tick,350);

  } else if (screen==='nfcemul') {
    let pulse=false;
    drawStatusBar('NFC Emulate',P.PU);
    drawNFCEmulate('A4 3F 88 C2',pulse);
    drawFooter('BCK','stop','');
    const tick=()=>{
      pulse=!pulse;
      drawStatusBar('NFC Emulate',P.PU);
      drawNFCEmulate('A4 3F 88 C2',pulse);
      animHandle=setTimeout(tick,500);
    };
    animHandle=setTimeout(tick,500);

  } else if (screen==='savednfc') {
    drawStatusBar('Saved NFC',P.PU);
    let ss2=0,so=0;
    drawMenu(SAVED_NFC,SAVED_NFC.length,ss2,so,'Saved NFC Cards');
    drawFooter('BCK','back','OK=load');

  } else if (screen==='ir') {
    drawStatusBar('IR Capture',P.PK);
    drawIRCapture(IR_RAW_DEMO);
    drawIRSignalInfo('NEC',0x04FB,0x08F7);
    drawFooter('BCK','menu','OK=TX');

  } else if (screen==='irsend') {
    let pulse=false;
    drawStatusBar('IR Transmit',P.PK);
    drawIRSend(pulse);
    drawFooter('BCK','stop','');
    const tick=()=>{
      pulse=!pulse;
      drawStatusBar('IR Transmit',P.PK);
      drawIRSend(pulse);
      animHandle=setTimeout(tick,300);
    };
    animHandle=setTimeout(tick,300);

  } else if (screen==='saveir') {
    drawStatusBar('Saved IR',P.PK);
    drawMenu(SAVED_IR,SAVED_IR.length,0,0,'Saved IR Signals');
    drawFooter('BCK','back','OK=TX');

  } else if (screen==='files') {
    drawStatusBar('Files',P.CY);
    drawMenu(FILES,FILES.length,0,0,'SD Card Files');
    drawFooter('BCK','back','');

  } else if (screen==='bright') {
    drawStatusBar('Brightness',P.AM);
    drawBrightness();
    drawFooter('BCK','save','UP/DN');

  } else if (screen==='about') {
    drawStatusBar('IRUTESAM v1.0',P.CY);
    drawAbout();
    drawFooter('BCK','back','');

  } else if (screen==='tcal') {
    drawStatusBar('Touch Cal',P.CY);
    drawTouchCalCross(0);
    drawFooter('BCK','cancel','');
  }
}

// ════════════════════════════════════════════════════════
// Input handler
// ════════════════════════════════════════════════════════
function inp(a) {
  if (screen==='boot') { skipBoot=true; return; }

  if (screen==='menu') {
    if      (a==='up') { if(mSel>0){mSel--;if(mSel<mOff)mOff=mSel;} }
    else if (a==='dn') { if(mSel<MENU_ITEMS.length-1){mSel++;if(mSel>=mOff+MENU_MAX_ROWS)mOff=mSel-MENU_MAX_ROWS+1;} }
    else if (a==='ok') {
      const m=MENU_ITEMS[mSel];
      const map={'WiFi Scanner':'wifi','Deauth Attack':'deauth','Deauth Protect':'dprotect',
        'Security Analysis':'secanal','NFC Read':'nfc','NFC Emulate':'nfcemul',
        'Saved NFC':'savednfc','IR Capture':'ir','IR Transmit':'irsend',
        'Saved IR':'saveir','Files':'files','Brightness':'bright','About':'about'};
      if(map[m]){stop();screen=map[m];}
    }
    render();

  } else if (screen==='wifi') {
    if      (a==='up') { if(wSel>0){wSel--;if(wSel<wOff)wOff=wSel;} }
    else if (a==='dn') { if(wSel<WIFI_NETS.length-1){wSel++;if(wSel>=wOff+WIFI_LIST_MAX_VIS)wOff=wSel-WIFI_LIST_MAX_VIS+1;} }
    else if (a==='ok') { stop(); screen='wdetail'; }
    else if (a==='bk') { stop(); screen='menu'; }
    render();

  } else if (screen==='wdetail') {
    if (a==='bk') { stop(); screen='wifi'; render(); }
    else if (a==='ok') { stop(); screen='deauth'; render(); }

  } else if (screen==='bright') {
    if      (a==='up') { brite=Math.min(255,brite+10); }
    else if (a==='dn') { brite=Math.max(0,brite-10); }
    else if (a==='bk'||a==='ok') { stop(); screen='menu'; }
    render();

  } else if (a==='bk' || (screen==='deauth'&&a==='ok')) {
    stop(); screen='menu'; render();
  }
}

// ── Button bindings ──────────────────────────────────────
document.getElementById('bUP').onclick=()=>inp('up');
document.getElementById('bDN').onclick=()=>inp('dn');
document.getElementById('bOK').onclick=()=>inp('ok');
document.getElementById('bBK').onclick=()=>inp('bk');
document.getElementById('bFW').onclick=()=>inp('ok');

// ── Canvas touch / click ─────────────────────────────────
C.addEventListener('click', e => {
  if (screen==='boot') { skipBoot=true; return; }
  const rect=C.getBoundingClientRect();
  const scaleX=240/rect.width, scaleY=320/rect.height;
  const cx=(e.clientX-rect.left)*scaleX;
  const cy=(e.clientY-rect.top)*scaleY;

  if (screen==='menu') {
    for(let v=0;v<MENU_MAX_ROWS;v++){
      const ry=MENU_START_Y+v*MENU_ROW_H;
      if(cy>=ry&&cy<ry+MENU_ROW_H){
        const idx=mOff+v; if(idx>=MENU_ITEMS.length)return;
        if(idx===mSel) inp('ok');
        else { mSel=idx; render(); }
        return;
      }
    }
  } else if (screen==='wifi') {
    for(let v=0;v<WIFI_LIST_MAX_VIS;v++){
      const ry=WIFI_LIST_START_Y+v*WIFI_LIST_ROW_H;
      if(cy>=ry&&cy<ry+WIFI_LIST_ROW_H){
        const idx=wOff+v; if(idx>=WIFI_NETS.length)return;
        if(idx===wSel) inp('ok');
        else { wSel=idx; render(); }
        return;
      }
    }
  } else if (screen==='deauth' && cy>160) {
    stop(); screen='menu'; render();
  } else if (screen==='wdetail') {
    if(cy>130&&cy<170) inp('ok');
    else inp('bk');
  }
});

// ════════════════════════════════════════════════════════
// BOOT SEQUENCE — animated, matches drawBootScreen() exactly
// ════════════════════════════════════════════════════════
let skipBoot=false;
const slp=ms=>new Promise(r=>{ animHandle=setTimeout(()=>{ animHandle=null; r(); }, skipBoot?0:ms); });

// Hex point helper
function hexPoint(cx,cy,r,i) {
  const a=i*60*Math.PI/180 - Math.PI/2;
  return [cx+r*Math.cos(a), cy+r*Math.sin(a)];
}

async function bootSequence() {
  screen='boot';
  const cx=120, cy=92;

  // Phase 1
  fil(P.BG);
  ss(1); sc('#112030'); cur(4,312); pr('tap screen to skip');
  await slp(200);

  // Phase 2: Outer hex r=52 edge by edge
  for(let i=0;i<6;i++){
    const [x1,y1]=hexPoint(cx,cy,52,i);
    const [x2,y2]=hexPoint(cx,cy,52,i+1);
    dl(x1,y1,x2,y2,P.CY); await slp(80);
  }
  await slp(100);

  // Phase 3: Inner hex r=42 all at once
  for(let i=0;i<6;i++){
    const [x1,y1]=hexPoint(cx,cy,42,i);
    const [x2,y2]=hexPoint(cx,cy,42,i+1);
    dl(x1,y1,x2,y2,P.CM);
  }
  await slp(80);

  // Phase 4: Rings + ticks
  dc(cx,cy,58,P.CD); dc(cx,cy,64,P.CM);
  for(let i=0;i<12;i++){
    const a=i*30*Math.PI/180;
    dl(cx+Math.round(58*Math.cos(a)),cy+Math.round(58*Math.sin(a)),
       cx+Math.round(62*Math.cos(a)),cy+Math.round(62*Math.sin(a)),
       i%2===1?P.CD:P.CM);
  }
  await slp(120);

  // Phase 5: "IRUTESAM" letter by letter
  const irutesam='IRUTESAM';
  for(let i=0;i<8;i++){
    ss(2); sc(P.CY); cur(72+i*12,84); pr(irutesam[i]); await slp(55);
  }
  await slp(80);

  // "v1.0"
  const ver='v1.0';
  for(let i=0;i<4;i++){
    ss(2); sc(P.PU); cur(96+i*12,108); pr(ver[i]); await slp(60);
  }
  await slp(150);

  // Phase 6: subtitle animation
  ct('IRUTESAM',148,P.CM,1); await slp(80);
  ct('IRUTESAM',148,P.CD,1); await slp(80);
  ct('IRUTESAM',148,P.CY,1); await slp(60);
  ct('DARK VENOM',162,P.PU,1); await slp(60);

  // Phase 7: Divider wipe
  for(let x=35;x<=200;x+=8){ dhl(35,174,x-35,P.CM); await slp(18); }
  await slp(100);

  // Phase 8: Boot log
  const logSpacing=13;
  const logs=[
    [P.GN,'[OK] ',P.CD,'WiFi module ready'],
    [P.GN,'[OK] ',P.CD,'SD card mounted'],
    [P.GN,'[OK] ',P.CD,'PN532 NFC online'],
    [P.GN,'[OK] ',P.CD,'IR receiver ready'],
    [P.GN,'[OK] ',P.CD,'Touch screen ready'],
    [P.CY,'',P.CY,'> Booting IRUTESAM v1.0'],
  ];
  let logY=180;
  for(const [c1,t1,c2,t2] of logs){
    ss(1); cur(6,logY);
    if(t1){sc(c1);pr(t1);}
    sc(c2); pr(t2);
    logY+=logSpacing;
    await slp(360);
  }

  // Phase 9: cursor blink x4
  for(let i=0;i<4;i++){
    ss(1); sc(P.CY); cur(6,logY); pr('_'); await slp(200);
    sc(P.BG); cur(6,logY); pr('_'); await slp(200);
  }

  // Phase 10: final pause
  await slp(600);

  screen='menu'; render();
}

bootSequence();
</script>
</body>
</html>
