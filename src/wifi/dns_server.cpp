#include "dns_server.h"

PhantomDNSServer::PhantomDNSServer() {
    running = false;
}

void PhantomDNSServer::begin(IPAddress localIP) {
    dnsServer.start(53, "*", localIP);
    running = true;
    Serial.println("[DNS] Servidor DNS iniciado (spoofing activo)");
}

void PhantomDNSServer::processNextRequest() {
    if (running) {
        dnsServer.processNextRequest();
    }
}

void PhantomDNSServer::stop() {
    dnsServer.stop();
    running = false;
    Serial.println("[DNS] Servidor DNS detenido");
}

bool PhantomDNSServer::isRunning() {
    return running;
}
