/**
 * @file web_portal.cpp
 * @brief WiFi configuration web portal implementation (ESP32 only)
 *
 * Creates a captive portal access point ("PakanIkan-Config") with a modern
 * web form for configuring WiFi credentials. Uses ESPAsyncWebServer for HTTP
 * handling and DNSServer for DNS hijacking (captive portal).
 *
 * Design: Dark aquatic theme with ocean blue / teal accent colors.
 *   Primary:   #0066CC (ocean blue)
 *   Accent:    #00D4AA (teal / mint)
 *   BG:        #0A1628 (dark navy)
 *   Card:      #112240 (slate)
 *   Text:      #E8F4F8 (soft white)
 *   Muted:     #8899AA (gray-blue)
 *
 * Layout: Centered card, responsive, minimal. No external dependencies.
 *
 * On non-ESP32 platforms, this entire file is compiled as empty (guarded by
 * #ifdef ARDUINO_ARCH_ESP32).
 */

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
    bool shouldRestart = false;   ///< Set true after saving WiFi config; main loop calls ESP.restart()
    const char* apSSID = "PakanIkan-Config";
    const char* apPass = "12345678";

    // ── HTML head template (dark theme, responsive) ────────────────────
    static const char PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>PakanIkan — WiFi Setup</title>
  <style>
    *,::before,::after{box-sizing:border-box;margin:0;padding:0}
    :root{
      --bg:#0A1628;--card:#112240;--border:#1E3A5F;
      --primary:#0066CC;--primary-hover:#0077EE;
      --accent:#00D4AA;--text:#E8F4F8;--muted:#8899AA;
      --input-bg:#0D1F3C;--success:#00C853;--danger:#FF5252;
    }
    body{
      font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
      background:var(--bg);color:var(--text);min-height:100vh;
      display:flex;align-items:center;justify-content:center;padding:24px;
    }
    .card{
      background:var(--card);border:1px solid var(--border);border-radius:16px;
      padding:40px 36px;width:100%;max-width:420px;
      box-shadow:0 20px 60px rgba(0,0,0,.4);
    }
    .logo{text-align:center;font-size:48px;line-height:1;margin-bottom:8px}
    h1{text-align:center;font-size:22px;font-weight:700;color:var(--text);margin-bottom:4px}
    .subtitle{text-align:center;color:var(--muted);font-size:14px;margin-bottom:32px}
    label{display:block;font-size:13px;font-weight:600;color:var(--muted);text-transform:uppercase;letter-spacing:.5px;margin-bottom:8px}
    input[type=text],input[type=password]{
      width:100%;padding:14px 16px;background:var(--input-bg);
      border:1px solid var(--border);border-radius:10px;color:var(--text);
      font-size:15px;outline:none;transition:border-color .2s,box-shadow .2s;margin-bottom:20px;
    }
    input[type=text]:focus,input[type=password]:focus{
      border-color:var(--primary);box-shadow:0 0 0 3px rgba(0,102,204,.25);
    }
    input::placeholder{color:#3D5A7A}
    .btn{
      display:block;width:100%;padding:15px;background:var(--primary);color:#fff;
      border:none;border-radius:10px;font-size:16px;font-weight:600;cursor:pointer;
      transition:background .2s,transform .1s;margin-top:8px;
    }
    .btn:hover{background:var(--primary-hover)}
    .btn:active{transform:scale(.98)}
    .btn:disabled{opacity:.6;cursor:not-allowed}
    .status{text-align:center;margin-top:20px;padding:14px;border-radius:10px;font-size:14px;display:none}
    .status.show{display:block}
    .status.ok{background:rgba(0,200,83,.12);border:1px solid rgba(0,200,83,.3);color:var(--success)}
    .status.err{background:rgba(255,82,82,.12);border:1px solid rgba(255,82,82,.3);color:var(--danger)}
    .divider{height:1px;background:var(--border);margin:28px 0}
    .info{font-size:12px;color:var(--muted);text-align:center;line-height:1.6}
    .info strong{color:var(--accent)}
  </style>
</head>
<body>
)rawliteral";

    static const char PAGE_FOOT[] PROGMEM = R"rawliteral(
</body>
</html>
)rawliteral";

    // ── Page handlers ───────────────────────────────────────────────────

    void initWebPortal() {
        isConfigMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPass);
        dnsServer.start(53, "*", WiFi.softAPIP());

        // ── Main page: WiFi configuration form ──
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            String html = FPSTR(PAGE_HEAD);
            html += R"rawliteral(
<div class="card">
  <div class="logo">&#x1F41F;</div>
  <h1>PakanIkan</h1>
  <p class="subtitle">WiFi Configuration</p>
  <form id="wifiForm">
    <label for="ssid">Network Name (SSID)</label>
    <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi name" required autocomplete="off">
    <label for="pass">Password</label>
    <input type="password" id="pass" name="pass" placeholder="Enter password (optional)">
    <button type="submit" class="btn" id="submitBtn">Save &amp; Connect</button>
  </form>
  <div id="statusMsg" class="status"></div>
  <div class="divider"></div>
  <p class="info">Device will restart and connect to <strong>your network</strong>.<br>If connection fails, the AP will reappear.</p>
</div>
<script>
document.getElementById('wifiForm').addEventListener('submit',function(e){
  e.preventDefault();
  var btn=document.getElementById('submitBtn');
  var st=document.getElementById('statusMsg');
  var ssid=document.getElementById('ssid').value.trim();
  if(!ssid){st.className='status err show';st.textContent='Please enter a network name.';return;}
  btn.textContent='Connecting...';btn.disabled=true;
  var x=new XMLHttpRequest();
  x.open('POST','/save',true);
  x.setRequestHeader('Content-Type','application/x-www-form-urlencoded');
  x.onload=function(){st.className='status ok show';st.innerHTML='Saved! Device is connecting to <strong>'+ssid+'</strong>...';btn.textContent='Done';};
  x.onerror=function(){st.className='status err show';st.textContent='Connection lost. Device is restarting.';};
  x.send('ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(document.getElementById('pass').value));
});
</script>
)rawliteral";
            html += FPSTR(PAGE_FOOT);
            request->send(200, "text/html", html);
        });

        // ── Save handler: connect to WiFi and restart ──
        server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
            String ssid = request->getParam("ssid", true)->value();
            String pass = request->getParam("pass", true)->value();
            Serial.printf("Connecting to %s...\n", ssid.c_str());
            WiFi.begin(ssid.c_str(), pass.c_str());

            String html = FPSTR(PAGE_HEAD);
            html += R"rawliteral(
<div class="card">
  <div class="logo">&#x2705;</div>
  <h1>Saved!</h1>
  <p class="subtitle">Device is restarting</p>
  <div class="status ok show">Connecting to <strong>)rawliteral";
            html += ssid;
            html += R"rawliteral(</strong>...</div>
  <p class="info" style="margin-top:20px">The device will restart and connect to your network.<br>If the connection fails, the configuration AP will reappear.</p>
</div>
)rawliteral";
            html += FPSTR(PAGE_FOOT);
            request->send(200, "text/html", html);
            shouldRestart = true;  // Deferred restart — handled in handleWebRequests()
        });

        // ── Captive portal redirect: all unknown paths → root ──
        server.onNotFound([](AsyncWebServerRequest *request){
            request->redirect("http://" + WiFi.softAPIP().toString());
        });

        server.begin();
        Serial.println(F("Web Portal Started. Connect to PakanIkan-Config"));
    }

    void handleWebRequests() {
        dnsServer.processNextRequest();
        if (shouldRestart) {
            delay(1000);  // Give time for HTTP response to be sent
            ESP.restart();
        }
    }

#endif // ARDUINO_ARCH_ESP32
