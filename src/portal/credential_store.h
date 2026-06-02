#ifndef CREDENTIAL_STORE_H
#define CREDENTIAL_STORE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define CREDENTIALS_FILE "/credentials.csv"

struct Credential {
    String template_name;
    String field1;
    String field2;
    String timestamp;
    String client_mac;
};

class CredentialStore {
public:
    CredentialStore(int max_entries = 50);
    bool addCredential(const String& template_name, const String& field1, const String& field2, const String& client_mac = "");
    String getCredentialsJSON();
    int getCount();
    void clear();
    String getHTMLTable();

    // Persistencia en LittleFS
    void loadFromDisk();

private:
    Credential* credentials;
    int max_entries;
    int count;
    String formatTimestamp();
    void appendToDisk(const Credential& c);
};

#endif
