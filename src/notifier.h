#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define NOTIFY_CONFIG_FILE "/notify.cfg"
#define NOTIFY_TIMEOUT_MS  3000

class NotifierModule {
public:
    NotifierModule();
    void configure(const String& url, const String& topic);
    void saveConfig();
    void loadConfig();
    bool isConfigured() const;
    String getURL() const;
    String getTopic() const;
    void notify(const String& tpl, const String& field1,
                const String& field2, const String& timestamp);
    bool test();

private:
    String _url;
    String _topic;
    bool _configured;
    bool sendHTTP(const String& host, const String& path,
                  const String& body, const String& contentType);
};

#endif
