#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include "portal/credential_store.h"
#include "portal/captive_portal.h"
#include "wifi/ap_manager.h"
#include "wifi/deauth.h"
#include "wifi/beacon_flood.h"
#include "wifi/probe_sniffer.h"
#include "wifi/evil_twin.h"
#include "wifi/auto_portal.h"
#include "wifi/pmkid_capture.h"

class PhantomWebServer {
public:
    PhantomWebServer(CredentialStore& store, CaptivePortal& portal, APManager& ap,
                     DeauthModule& deauth, BeaconFloodModule& beacon,
                     ProbeSnifferModule& probe, EvilTwinModule& evilTwin,
                     AutoPortalModule& autoPortal, PMKIDCaptureModule& pmkid);
    void begin();
    void handleClient();
    ESP8266WebServer& getServer() { return server; }

private:
    ESP8266WebServer server;
    CredentialStore& store;
    CaptivePortal& portal;
    APManager& ap;
    DeauthModule& deauth;
    BeaconFloodModule& beacon;
    ProbeSnifferModule& probe;
    EvilTwinModule& evilTwin;
    AutoPortalModule& autoPortal;
    PMKIDCaptureModule& pmkid;

    // Authentication
    String sessionToken;
    uint8_t failedLoginAttempts;
    unsigned long loginLockedUntil;
    void generateSessionToken();
    bool isAuthenticated();
    bool defaultCredentialsActive();
    bool loginLocked();
    void requireAuth();
    String jsonEscape(const String& value);

    void handleLogin();
    void handleLoginPost();
    void handleLogout();

    void handleDashboard();
    void handleAPIStatus();
    void handleAPICredentials();
    void handleAPILog();
    void handleAPIControl();
    void handleAPITemplate();
    void handleAPISSID();
    void handleAPIClear();
    void handleAPIDeauth();
    void handleAPIBeacon();
    void handleAPIProbe();
    void handleAPIEvilTwin();
    void handleAPIAutoPortal();
    void handleAPIExportCSV();
    void handleAPIExportReport();
    void handleAPIStealth();
    void handleAPIAutoAttack();
    void handleAPIKarma();
    void handleAPIPMKID();
    void handleAPIPanic();

    String getDashboardHTML();
    String getLoginHTML();
    String getCSS();
    String getJS();
};

#endif

