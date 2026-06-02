#include "notifier.h"
#include <LittleFS.h>
#include <WiFiClient.h>

extern void addLog(const String& msg);

NotifierModule::NotifierModule() : _url(""), _topic(""), _configured(false) {}

void NotifierModule::configure(const String& url, const String& topic) {
    _url        = url;
    _topic      = topic;
    _configured = (url.length() > 0);
}

void NotifierModule::saveConfig() {
    File f = LittleFS.open(NOTIFY_CONFIG_FILE, "w");
    if (!f) return;
    f.println(_url);
    f.println(_topic);
    f.close();
    addLog("[NOTIFY] Configuracion guardada");
}

void NotifierModule::loadConfig() {
    if (!LittleFS.exists(NOTIFY_CONFIG_FILE)) return;
    File f = LittleFS.open(NOTIFY_CONFIG_FILE, "r");
    if (!f) return;
    _url   = f.readStringUntil('\n'); _url.trim();
    _topic = f.readStringUntil('\n'); _topic.trim();
    f.close();
    _configured = (_url.length() > 0);
    if (_configured) {
        addLog("[NOTIFY] Webhook cargado: " + _url);
    }
}

bool NotifierModule::isConfigured() const {
    return _configured && (WiFi.status() == WL_CONNECTED);
}

String NotifierModule::getURL()   const { return _url; }
String NotifierModule::getTopic() const { return _topic; }

// ---------------------------------------------------------------------------
// HTTP POST helper (plain HTTP, port 80)
// ---------------------------------------------------------------------------

bool NotifierModule::sendHTTP(const String& host, const String& path,
                               const String& body, const String& contentType) {
    WiFiClient client;
    client.setTimeout(NOTIFY_TIMEOUT_MS);

    if (!client.connect(host.c_str(), 80)) {
        addLog("[NOTIFY] Error de conexion a " + host);
        return false;
    }

    String req = "POST " + path + " HTTP/1.1\r\n";
    req += "Host: " + host + "\r\n";
    req += "Content-Type: " + contentType + "\r\n";
    req += "Content-Length: " + String(body.length()) + "\r\n";
    req += "Connection: close\r\n\r\n";
    req += body;

    client.print(req);

    // Read response status (first line only)
    unsigned long t0 = millis();
    while (!client.available() && millis() - t0 < NOTIFY_TIMEOUT_MS) {
        delay(10);
    }

    String statusLine = client.readStringUntil('\n');
    client.stop();

    return statusLine.indexOf("200") != -1 || statusLine.indexOf("201") != -1;
}

// ---------------------------------------------------------------------------

void NotifierModule::notify(const String& tpl, const String& field1,
                             const String& field2, const String& timestamp) {
    if (!isConfigured()) return;

    String host = _url;
    String path = "/";

    // Parse host and path from URL
    // Remove protocol prefix if present
    if (host.startsWith("http://"))  host = host.substring(7);
    if (host.startsWith("https://")) host = host.substring(8);

    int slashPos = host.indexOf('/');
    if (slashPos != -1) {
        path = host.substring(slashPos);
        host = host.substring(0, slashPos);
    }

    // ntfy.sh mode: if topic is set, append it to path
    if (_topic.length() > 0 && path == "/") {
        path = "/" + _topic;
    }

    // Build message body (plain text for ntfy.sh, JSON for generic webhooks)
    String body;
    String ct;
    if (_topic.length() > 0) {
        // ntfy.sh: plain text body
        body  = "PhantomKit capture\n";
        body += "Template: " + tpl + "\n";
        body += "Field1:   " + field1 + "\n";
        body += "Field2:   " + field2 + "\n";
        body += "Time:     " + timestamp;
        ct = "text/plain";
    } else {
        // Generic JSON webhook
        body  = "{";
        body += "\"source\":\"PhantomKit\",";
        body += "\"template\":\"" + tpl + "\",";
        body += "\"field1\":\"" + field1 + "\",";
        body += "\"field2\":\"" + field2 + "\",";
        body += "\"timestamp\":\"" + timestamp + "\"";
        body += "}";
        ct = "application/json";
    }

    bool ok = sendHTTP(host, path, body, ct);
    addLog(String("[NOTIFY] ") + (ok ? "Enviado OK" : "Error de envio"));
}

bool NotifierModule::test() {
    if (!_configured) return false;

    String host = _url;
    String path = "/";

    if (host.startsWith("http://"))  host = host.substring(7);
    if (host.startsWith("https://")) host = host.substring(8);

    int slashPos = host.indexOf('/');
    if (slashPos != -1) {
        path = host.substring(slashPos);
        host = host.substring(0, slashPos);
    }

    if (_topic.length() > 0 && path == "/") {
        path = "/" + _topic;
    }

    String body = "PhantomKit - prueba de notificacion";
    return sendHTTP(host, path, body, "text/plain");
}
