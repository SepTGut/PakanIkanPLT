#include "web_portal.h"
#include "config.h"
#include "feeding.h"
#include "rtc_manager.h"

#ifdef ARDUINO_ARCH_ESP32
    #include <WiFi.h>
    #include <ESPAsyncWebServer.h>
    #include <ESPmDNS.h>
    #include <DNSServer.h>

    AsyncWebServer server(80);
    DNSServer dnsServer;
    bool isConfigMode = false;
    const char* apSSID = "PakanIkan-Config";
    const char* apPass = "12345678";

    void initWebPortal() {
        isConfigMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPass);
        dnsServer.start(53, "*", WiFi.softAPIP());

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            html += "<style>body{font-family:Arial; background:#f4f4f9; text-align:center; padding:20px;}";
            html += ".card{background:white; padding:20px; border-radius:15px; box-shadow:0 4px 8px rgba(0,0,0,0.1); max-width:400px; margin:auto;}";
            html += "input{width:80%; padding:10px; margin:10px 0; border-radius:5px; border:1px solid #ccc;}";
            html += "button{background:#4CAF50; color:white; padding:10px 20px; border:none; border-radius:5px; cursor:pointer; font-size:16px;}</style>";
            html += "</head><body><div class='card'><h2 >?? Pakan Ikan Config</h2>";
            html += "<form action='/save' method='POST'>";
            html += "SSID:<br><input type='text' name='ssid' placeholder='WiFi Name'><br>";
            html += "Password:<br><input type='password' name='pass' placeholder='Password'><br>";
            html += "<button type='submit'>Save & Connect</button></form></div></body></html>";
            request->send(200, "text/html", html);
        });

        server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
            String ssid = request.getParam("ssid", true)->value();
            String pass = request.getParam("pass", true)->value();
            Serial.printf("Connecting to %s...\\n", ssid.c_str());
            WiFi.begin(ssid.c_str(), pass.c_str());
            request->send(200, "text/html", "<html><body><h1 >Saved!</h1><p>Rebooting and connecting to WiFi...</p></body></html>");
            delay(2000);
            ESP.restart();
        });

        server.begin();
        Serial.println(F("Web Portal Started. Connect to PakanIkan-Config"));
    }

    void handleWebRequests() {
        dnsServer.processNextRequest();
    }
#endif
