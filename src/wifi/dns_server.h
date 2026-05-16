#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <DNSServer.h>
#include <Arduino.h>

class PhantomDNSServer {
public:
    PhantomDNSServer();
    void begin(IPAddress localIP);
    void processNextRequest();
    void stop();
    bool isRunning();

private:
    DNSServer dnsServer;
    bool running;
};

#endif
