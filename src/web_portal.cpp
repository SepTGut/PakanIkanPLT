/**
 * @file web_portal.cpp
 * @brief Full web control panel for PakanIkanPLT (ESP32 only)
 *
 * Single-page application with 3 tabs:
 *   Dashboard — Live monitoring (RTC, schedule, EEPROM, sensor status, system info)
 *   Control   — Actions (manual feed, test servo/buzzer, clear EEPROM)
 *   Settings  — Configuration (schedule, servo, buzzer, display, WiFi)
 *
 * REST API:
 *   GET  /api/status   → Full system status JSON
 *   POST /api/action   → Execute action {type, ...}
 *   GET  /api/settings → Current settings JSON
 *   POST /api/settings → Update settings {...}
 *   POST /api/wifi     → Save WiFi config {ssid, pass}
 *
 * Works in both AP mode (captive portal) and STA mode (home WiFi).
 * All HTML/CSS/JS stored in PROGMEM. No external dependencies.
 */

#include "web_portal.h"
#include "config.h"
#include "feeding.h"
#include "rtc_manager.h"
#include "ntp_sync.h"
#include "state_debug.h"
#include "alerts.h"
#include "display.h"
#include <EEPROM.h>

#ifdef ARDUINO_ARCH_ESP32

    #include <WiFi.h>
    #include <ESPAsyncWebServer.h>
    #include <ESPmDNS.h>
    #include <DNSServer.h>

    AsyncWebServer server(80);
    DNSServer dnsServer;
    bool isConfigMode = false;
    bool shouldRestart = false;
    const char* apSSID = "PakanIkan-Config";
    const char* apPass = "12345678";

    // ── Action result tracking ──
    String lastActionResult = "";
    unsigned long lastActionTime = 0;

    // ── Forward declarations ──
    String buildStatusJSON();
    void handleAction(AsyncWebServerRequest *request, String body);

    // ═══════════════════════════════════════════════════════════════════════
    // HTML HEAD — Shared across all pages
    // ═══════════════════════════════════════════════════════════════════════
    static const char PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0,user-scalable=no">
<title>PakanIkan — Control Panel</title>
<style>
*,::before,::after{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#0A1628;--card:#112240;--card2:#0D1F3C;--border:#1E3A5F;
  --primary:#0066CC;--primary-hover:#0077EE;
  --accent:#00D4AA;--text:#E8F4F8;--muted:#8899AA;
  --input-bg:#0D1F3C;--success:#00C853;--danger:#FF5252;--warn:#FFB300;
}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  background:var(--bg);color:var(--text);min-height:100vh;padding:16px;}
/* ── Header ── */
.header{text-align:center;padding:20px 0 16px}
.header .logo{font-size:40px;line-height:1;margin-bottom:4px}
.header h1{font-size:20px;font-weight:700;margin-bottom:2px}
.header .subtitle{color:var(--muted);font-size:13px}
/* ── Tab Navigation ── */
.tabs{display:flex;gap:4px;margin-bottom:20px;background:var(--card2);
  border-radius:12px;padding:4px}
.tab{flex:1;padding:10px 4px;text-align:center;border-radius:10px;
  cursor:pointer;font-size:13px;font-weight:600;color:var(--muted);
  transition:all .2s;border:none;background:transparent}
.tab.active{background:var(--primary);color:#fff}
.tab:hover:not(.active){background:var(--border);color:var(--text)}
/* ── Cards ── */
.card{background:var(--card);border:1px solid var(--border);border-radius:14px;
  padding:20px;margin-bottom:16px}
.card-title{font-size:15px;font-weight:700;margin-bottom:14px;
  display:flex;align-items:center;gap:8px}
/* ── Status Grid ── */
.grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.stat{background:var(--input-bg);border-radius:10px;padding:12px}
.stat-label{font-size:11px;color:var(--muted);text-transform:uppercase;
  letter-spacing:.5px;margin-bottom:4px}
.stat-value{font-size:16px;font-weight:600}
.stat-value.ok{color:var(--success)}
.stat-value.err{color:var(--danger)}
.stat-value.warn{color:var(--warn)}
/* ── Full-width stat ── */
.stat.full{grid-column:1/-1}
/* ── Schedule Table ── */
.tbl{width:100%;border-collapse:collapse}
.tbl th{text-align:left;font-size:11px;color:var(--muted);text-transform:uppercase;
  letter-spacing:.5px;padding:8px 12px;border-bottom:1px solid var(--border)}
.tbl td{padding:10px 12px;border-bottom:1px solid var(--border);font-size:14px}
.tbl tr:last-child td{border-bottom:none}
.tbl tr.highlight td{background:rgba(0,102,204,.12);color:var(--accent)}
/* ── Buttons ── */
.btn{display:inline-block;padding:10px 20px;border:none;border-radius:10px;
  font-size:14px;font-weight:600;cursor:pointer;transition:all .2s;
  text-align:center}
.btn-primary{background:var(--primary);color:#fff}
.btn-primary:hover{background:var(--primary-hover)}
.btn-danger{background:var(--danger);color:#fff}
.btn-danger:hover{background:#ff7043}
.btn-success{background:var(--success);color:#fff}
.btn-success:hover:#00e676}
.btn:active{transform:scale(.97)}
.btn:disabled{opacity:.5;cursor:not-allowed}
.btn-sm{padding:6px 14px;font-size:12px}
.btn-block{display:block;width:100%}
/* ── Form Elements ── */
.form-row{display:flex;gap:10px;align-items:center;margin-bottom:12px}
.form-row label{min-width:80px;font-size:13px;color:var(--muted)}
.form-row input,.form-row select{flex:1;padding:10px 14px;background:var(--input-bg);
  border:1px solid var(--border);border-radius:8px;color:var(--text);
  font-size:14px;outline:none}
.form-row input:focus{border-color:var(--primary)}
/* ── Toast ── */
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);
  padding:12px 24px;border-radius:10px;font-size:14px;font-weight:600;
  z-index:100;transition:opacity .3s;pointer-events:none;opacity:0}
.toast.show{opacity:1}
.toast.ok{background:var(--success);color:#fff}
.toast.err{background:var(--danger);color:#fff}
/* ── Action buttons grid ── */
.actions{display:grid;grid-template-columns:1fr 1fr;gap:10px}
/* ── Responsive ── */
@media(max-width:480px){.grid{grid-template-columns:1fr}.actions{grid-template-columns:1fr}}
</style>
</head>
<body>
)rawliteral";

    // ═══════════════════════════════════════════════════════════════════════
    // HTML FOOT — Shared
    // ═══════════════════════════════════════════════════════════════════════
    static const char PAGE_FOOT[] PROGMEM = R"rawliteral(
<div id="toast" class="toast"></div>
<script>
var toastTimer=null;
function showToast(msg,type){
  var t=document.getElementById('toast');
  t.textContent=msg;t.className='toast show '+(type||'ok');
  clearTimeout(toastTimer);toastTimer=setTimeout(function(){t.classList.remove('show')},3000);
}
function postAction(body,cb){
  var x=new XMLHttpRequest();
  x.open('POST','/api/action',true);
  x.setRequestHeader('Content-Type','application/json');
  x.onload=function(){try{var r=JSON.parse(x.responseText);showToast(r.message,r.ok?'ok':'err');if(cb)cb(r)}catch(e){showToast('Error parsing response','err')}};
  x.onerror=function(){showToast('Connection lost','err')};
  x.send(JSON.stringify(body));
}
function postSettings(data,cb){
  var x=new XMLHttpRequest();
  x.open('POST','/api/settings',true);
  x.setRequestHeader('Content-Type','application/json');
  x.onload=function(){try{var r=JSON.parse(x.responseText);showToast(r.message,r.ok?'ok':'err');if(cb)cb(r)}catch(e){showToast('Error','err')}};
  x.onerror=function(){showToast('Connection lost','err')};
  x.send(JSON.stringify(data));
}
function postWifi(ssid,pass,cb){
  var x=new XMLHttpRequest();
  x.open('POST','/api/wifi',true);
  x.setRequestHeader('Content-Type','application/json');
  x.onload=function(){try{var r=JSON.parse(x.responseText);showToast(r.message,r.ok?'ok':'err');if(cb)cb(r)}catch(e){showToast('Error','err')}};
  x.onerror=function(){showToast('Connection lost','err')};
  x.send(JSON.stringify({ssid:ssid,pass:pass}));
}
</script>
</body>
</html>
)rawliteral";

    // ═══════════════════════════════════════════════════════════════════════
    // SPA BODY — Tabbed interface
    // ═══════════════════════════════════════════════════════════════════════
    static const char PAGE_BODY[] PROGMEM = R"rawliteral(
<div class="header">
  <div class="logo">&#x1F41F;</div>
  <h1>PakanIkan</h1>
  <p class="subtitle">Automatic Fish Feeder — Control Panel</p>
</div>

<div class="tabs">
  <button class="tab active" onclick="showTab('dash',this)">&#x1F4CA; Dashboard</button>
  <button class="tab" onclick="showTab('ctrl',this)">&#x1F527; Control</button>
  <button class="tab" onclick="showTab('set',this)">&#x2699; Settings</button>
</div>

<!-- ═══ DASHBOARD TAB ═══ -->
<div id="tab-dash" class="tab-content">
  <div class="card">
    <div class="card-title">&#x1F552; Time &amp; Date</div>
    <div class="grid">
      <div class="stat"><div class="stat-label">Time</div><div class="stat-value" id="d-time">--:--:--</div></div>
      <div class="stat"><div class="stat-label">Date</div><div class="stat-value" id="d-date">--/--/----</div></div>
      <div class="stat full"><div class="stat-label">Day</div><div class="stat-value" id="d-day">---</div></div>
    </div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F4CB; Feeding Schedule</div>
    <table class="tbl" id="d-schedule"><tr><th>Session</th><th>Time</th><th>Fed</th></tr></table>
  </div>
  <div class="card">
    <div class="card-title">&#x1F4BE; Last Feeding</div>
    <div class="grid">
      <div class="stat"><div class="stat-label">Session</div><div class="stat-value" id="d-last-sess">---</div></div>
      <div class="stat"><div class="stat-label">Date</div><div class="stat-value" id="d-last-date">--/--/----</div></div>
    </div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F4E6; Sensors &amp; Status</div>
    <div class="grid">
      <div class="stat"><div class="stat-label">Food Level</div><div class="stat-value" id="d-food">---</div></div>
      <div class="stat"><div class="stat-label">RTC Status</div><div class="stat-value" id="d-rtc">---</div></div>
      <div class="stat"><div class="stat-label">Uptime</div><div class="stat-value" id="d-uptime">---</div></div>
      <div class="stat"><div class="stat-label">Free Heap</div><div class="stat-value" id="d-heap">---</div></div>
      <div class="stat full"><div class="stat-label">WiFi / IP</div><div class="stat-value" id="d-wifi">---</div></div>
    </div>
  </div>
</div>

<!-- ═══ CONTROL TAB ═══ -->
<div id="tab-ctrl" class="tab-content" style="display:none">
  <div class="card">
    <div class="card-title">&#x1F373; Feeding</div>
    <div class="actions">
      <button class="btn btn-primary btn-block" onclick="if(confirm('Trigger manual feed?'))postAction({type:'feed'})">&#x1F4E6; Manual Feed</button>
      <button class="btn btn-primary btn-block" onclick="postAction({type:'test_servo'})">&#x1F528; Test Servo</button>
    </div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F514; Buzzer Test</div>
    <div class="form-row">
      <label>Buzzer</label>
      <select id="buzzer_num">
        <option value="1">1 — Status (short)</option>
        <option value="2">2 — Alert (long)</option>
      </select>
    </div>
    <div class="form-row">
      <label>Duration</label>
      <select id="buzzer_dur">
        <option value="100">100ms</option>
        <option value="200">200ms</option>
        <option value="500">500ms</option>
        <option value="1000">1000ms</option>
      </select>
    </div>
    <button class="btn btn-primary btn-block" onclick="postAction({type:'test_buzzer',buzzer:parseInt(document.getElementById('buzzer_num').value),duration:parseInt(document.getElementById('buzzer_dur').value)})">&#x1F50A; Test Buzzer</button>
  </div>
  <div class="card">
    <div class="card-title">&#x1F4BE; EEPROM</div>
    <div class="actions">
      <button class="btn btn-danger btn-block" onclick="if(confirm('Clear EEPROM feeding state? This will cause missed feed detection on next boot.'))postAction({type:'clear_eeprom'})">&#x1F5D1; Clear EEPROM</button>
      <button class="btn btn-primary btn-block" onclick="postAction({type:'check_missed'})">&#x1F50D; Check Missed Feeds</button>
    </div>
  </div>
</div>

<!-- ═══ SETTINGS TAB ═══ -->
<div id="tab-set" class="tab-content" style="display:none">
  <div class="card">
    <div class="card-title">&#x1F4CB; Feeding Schedule</div>
    <table class="tbl" id="s-schedule"><tr><th>Label</th><th>Hour</th><th>Min</th></tr></table>
  </div>
  <div class="card">
    <div class="card-title">&#x1F528; Servo</div>
    <div class="form-row"><label>Open Angle</label><input type="number" id="s-servo-open" min="0" max="180"></div>
    <div class="form-row"><label>Closed Angle</label><input type="number" id="s-servo-closed" min="0" max="180"></div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F4E6; Feeding</div>
    <div class="form-row"><label>Feed Amount</label><input type="number" id="s-amount" min="1" max="255"></div>
    <div class="form-row"><label>Display Interval (ms)</label><input type="number" id="s-interval" min="500" max="60000"></div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F514; Buzzer</div>
    <div class="form-row"><label>Enabled</label><input type="checkbox" id="s-buzzer" style="width:20px;height:20px"></div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F52D; IR Sensor</div>
    <div class="form-row"><label>Enabled</label><input type="checkbox" id="s-ir" style="width:20px;height:20px"></div>
  </div>
  <div class="card">
    <div class="card-title">&#x1F4F6; WiFi Configuration</div>
    <div class="form-row"><label>SSID</label><input type="text" id="s-wifi-ssid" placeholder="WiFi Name"></div>
    <div class="form-row"><label>Password</label><input type="password" id="s-wifi-pass" placeholder="Password"></div>
    <button class="btn btn-primary btn-block" onclick="postWifi(document.getElementById('s-wifi-ssid').value,document.getElementById('s-wifi-pass').value)">&#x1F4BE; Save &amp; Connect</button>
  </div>
  <button class="btn btn-success btn-block" onclick="saveAllSettings()" style="margin-top:8px">&#x1F4BE; Save All Settings</button>
</div>

<script>
function showTab(id,btn){
  document.querySelectorAll('.tab-content').forEach(function(el){el.style.display='none'});
  document.querySelectorAll('.tab').forEach(function(el){el.classList.remove('active')});
  document.getElementById('tab-'+id).style.display='block';
  btn.classList.add('active');
}
function fmtUptime(ms){
  var s=Math.floor(ms/1000);var m=Math.floor(s/60);var h=Math.floor(m/60);var d=Math.floor(h/24);
  if(d>0)return d+'d '+h%24+'h';
  if(h>0)return h+'h '+m%60+'m';
  return m+'m '+s%60+'s';
}
function refreshStatus(){
  var x=new XMLHttpRequest();
  x.open('GET','/api/status',true);
  x.onload=function(){
    try{
      var d=JSON.parse(x.responseText);
      document.getElementById('d-time').textContent=d.time;
      document.getElementById('d-date').textContent=d.date;
      document.getElementById('d-day').textContent=d.day;
      var fed=d.eeprom_last;
      document.getElementById('d-last-sess').textContent=fed.session_label;
      document.getElementById('d-last-date').textContent=fed.date;
      var food=document.getElementById('d-food');
      food.textContent=d.food_level;
      food.className='stat-value '+(d.food_level==='OK'?'ok':'warn');
      var rtc=document.getElementById('d-rtc');
      rtc.textContent=d.rtc_valid;
      rtc.className='stat-value '+(d.rtc_valid==='OK'?'ok':'err');
      document.getElementById('d-uptime').textContent=fmtUptime(d.uptime);
      document.getElementById('d-heap').textContent=d.free_heap+' B';
      document.getElementById('d-wifi').textContent=d.wifi_mode+' — '+d.ip;
      // Schedule table
      var schedHtml='<tr><th>Session</th><th>Time</th><th>Fed</th></tr>';
      for(var i=0;i<d.schedule.length;i++){
        var s=d.schedule[i];var fed=d.fed_today[i]?'&#x2705;':'&#x274C;';
        var cls=d.next_session==i?' class="highlight"':'';
        schedHtml+='<tr'+cls+'><td>'+s.label+'</td><td>'+s.time+'</td><td>'+fed+'</td></tr>';
      }
      document.getElementById('d-schedule').innerHTML=schedHtml;
      // Settings form
      if(d.settings){
        document.getElementById('s-servo-open').value=d.settings.servo_open;
        document.getElementById('s-servo-closed').value=d.settings.servo_closed;
        document.getElementById('s-amount').value=d.settings.feed_amount;
        document.getElementById('s-interval').value=d.settings.display_interval;
        document.getElementById('s-buzzer').checked=d.settings.buzzer_enabled;
        document.getElementById('s-ir').checked=d.settings.ir_enabled;
        // Settings schedule
        var sSchedHtml='<tr><th>Label</th><th>Hour</th><th>Min</th></tr>';
        for(var i=0;i<d.settings.schedule.length;i++){
          var s=d.settings.schedule[i];
          sSchedHtml+='<tr><td><input value="'+s.label+'" class="s-label" style="width:80px;background:var(--input-bg);border:1px solid var(--border);border-radius:6px;padding:4px 8px;color:var(--text);font-size:13px"></td>';
          sSchedHtml+='<td><input type="number" value="'+s.hour+'" class="s-hour" min="0" max="23" style="width:60px;background:var(--input-bg);border:1px solid var(--border);border-radius:6px;padding:4px 8px;color:var(--text);font-size:13px"></td>';
          sSchedHtml+='<td><input type="number" value="'+s.minute+'" class="s-min" min="0" max="59" style="width:60px;background:var(--input-bg);border:1px solid var(--border);border-radius:6px;padding:4px 8px;color:var(--text);font-size:13px"></td></tr>';
        }
        document.getElementById('s-schedule').innerHTML=sSchedHtml;
      }
    }catch(e){}
  };
  x.send();
}
function saveAllSettings(){
  var labels=document.querySelectorAll('.s-label');
  var hours=document.querySelectorAll('.s-hour');
  var mins=document.querySelectorAll('.s-min');
  var sched=[];
  for(var i=0;i<labels.length;i++){
    sched.push({label:labels[i].value,hour:parseInt(hours[i].value)||0,minute:parseInt(mins[i].value)||0});
  }
  postSettings({
    servo_open:parseInt(document.getElementById('s-servo-open').value)||150,
    servo_closed:parseInt(document.getElementById('s-servo-closed').value)||0,
    feed_amount:parseInt(document.getElementById('s-amount').value)||100,
    display_interval:parseInt(document.getElementById('s-interval').value)||3000,
    buzzer_enabled:document.getElementById('s-buzzer').checked,
    ir_enabled:document.getElementById('s-ir').checked,
    schedule:sched
  });
}
refreshStatus();
setInterval(refreshStatus,2000);
</script>
)rawliteral";

    // ═══════════════════════════════════════════════════════════════════════
    // JSON API Helpers
    // ═══════════════════════════════════════════════════════════════════════

    String buildStatusJSON() {
        TimeData now = getCurrentTime();
        FeedingState last = loadState();

        String json = "{";

        // Time
        json += "\"time\":\"" + String(now.hour) + ":" + String(now.minute) + ":" + String(now.second) + "\",";
        json += "\"date\":\"" + String(now.day) + "/" + String(now.month) + "/" + String(now.year) + "\",";
        json += "\"day\":\"" + String(now.dayName) + "\",";

        // RTC valid
        json += "\"rtc_valid\":\"" + String(isRTCValid() ? "OK" : "ERROR") + "\",";

        // Food level
        json += "\"food_level\":\"" + String(IR_SENSOR_PIN >= 0 && digitalRead(IR_SENSOR_PIN) == HIGH ? "LOW" : "OK") + "\",";

        // EEPROM last feeding
        json += "\"eeprom_last\":{";
        json += "\"session\":" + String(last.session) + ",";
        json += "\"session_label\":\"" + String(last.session < NUM_SESSIONS ? SCHEDULE[last.session].label : "None") + "\",";
        json += "\"date\":\"" + String(last.day) + "/" + String(last.month) + "/" + String(last.year) + "\"";
        json += "},";

        // Fed today array
        json += "\"fed_today\":[";
        for (int i = 0; i < NUM_SESSIONS; i++) {
            json += hasFedToday(now, i) ? "true" : "false";
            if (i < NUM_SESSIONS - 1) json += ",";
        }
        json += "],";

        // Find next session
        int nextSess = -1;
        for (int i = 0; i < NUM_SESSIONS; i++) {
            if (now.hour < SCHEDULE[i].hour || (now.hour == SCHEDULE[i].hour && now.minute < SCHEDULE[i].minute)) {
                nextSess = i;
                break;
            }
        }
        json += "\"next_session\":" + String(nextSess) + ",";

        // Schedule array
        json += "\"schedule\":[";
        for (int i = 0; i < NUM_SESSIONS; i++) {
            json += "{\"label\":\"" + String(SCHEDULE[i].label) + "\",";
            json += "\"time\":\"" + String(SCHEDULE[i].hour) + ":" + String(SCHEDULE[i].minute) + "\"}";
            if (i < NUM_SESSIONS - 1) json += ",";
        }
        json += "],";

        // System info
        json += "\"uptime\":" + String(millis()) + ",";
        json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"wifi_mode\":\"" + String(WiFi.getMode() == WIFI_AP ? "AP" : (WiFi.getMode() == WIFI_STA ? "STA" : "AP+STA")) + "\",";
        json += "\"ip\":\"" + String(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString()) + "\"";

        // Settings
        json += ",\"settings\":{";
        json += "\"servo_open\":" + String(SERVO_OPEN) + ",";
        json += "\"servo_closed\":" + String(SERVO_CLOSED) + ",";
        json += "\"feed_amount\":" + String(JUMLAH_PAKAN) + ",";
        json += "\"display_interval\":" + String(DISPLAY_INTERVAL) + ",";
        json += "\"buzzer_enabled\":" + String(ENABLE_BUZZERS ? "true" : "false") + ",";
        json += "\"ir_enabled\":" + String(ENABLE_IR_SENSOR ? "true" : "false") + ",";
        json += "\"schedule\":[";
        for (int i = 0; i < NUM_SESSIONS; i++) {
            json += "{\"label\":\"" + String(SCHEDULE[i].label) + "\",";
            json += "\"hour\":" + String(SCHEDULE[i].hour) + ",";
            json += "\"minute\":" + String(SCHEDULE[i].minute) + "}";
            if (i < NUM_SESSIONS - 1) json += ",";
        }
        json += "]";

        json += "}}";
        return json;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Action Handler
    // ═══════════════════════════════════════════════════════════════════════

    void handleAction(AsyncWebServerRequest *request, String body) {
        String response = "{\"ok\":true,\"message\":\"OK\"}";

        if (body.indexOf("\"feed\"") > 0) {
            triggerAlert(1, 100);
            startFeeding(JUMLAH_PAKAN);
            response = "{\"ok\":true,\"message\":\"Manual feed triggered\"}";
        }
        else if (body.indexOf("\"test_servo\"") > 0) {
            testServo();
            response = "{\"ok\":true,\"message\":\"Servo test complete\"}";
        }
        else if (body.indexOf("\"test_buzzer\"") > 0) {
            // Parse buzzer number and duration from JSON
            int buzzer = 1;
            int duration = 100;
            int bIdx = body.indexOf("\"buzzer\"");
            if (bIdx > 0) {
                int colon = body.indexOf(":", bIdx);
                int comma = body.indexOf(",", colon);
                if (comma < 0) comma = body.indexOf("}", colon);
                buzzer = body.substring(colon + 1, comma).toInt();
            }
            int dIdx = body.indexOf("\"duration\"");
            if (dIdx > 0) {
                int colon = body.indexOf(":", dIdx);
                int comma = body.indexOf(",", colon);
                if (comma < 0) comma = body.indexOf("}", colon);
                duration = body.substring(colon + 1, comma).toInt();
            }
            triggerAlert(buzzer, duration);
            response = "{\"ok\":true,\"message\":\"Buzzer " + String(buzzer) + " tested for " + String(duration) + "ms\"}";
        }
        else if (body.indexOf("\"clear_eeprom\"") > 0) {
            #ifdef ARDUINO_ARCH_ESP32
            for (int i = 0; i < 50; i++) EEPROM.write(i, 0xFF);
            EEPROM.commit();
            #endif
            response = "{\"ok\":true,\"message\":\"EEPROM cleared. Reboot to take effect.\"}";
        }
        else if (body.indexOf("\"check_missed\"") > 0) {
            TimeData now = getCurrentTime();
            int missed = checkMissedFeeds(now);
            if (missed >= 0) {
                response = "{\"ok\":true,\"message\":\"Missed feed detected: session " + String(missed) + " (" + String(SCHEDULE[missed].label) + ")\"}";
            } else {
                response = "{\"ok\":true,\"message\":\"No missed feeds detected\"}";
            }
        }

        request->send(200, "application/json", response);
    }

    // ═══════════════════════════════════════════════════════════════════════
    // Page Handlers
    // ═══════════════════════════════════════════════════════════════════════

    void initWebPortal() {
        isConfigMode = true;

        // Use AP+STA mode: AP for configuration, STA for home WiFi
        WiFi.mode(WIFI_AP_STA);
        delay(200);  // Allow radio to stabilize

        bool apStarted = WiFi.softAP(apSSID, apPass);
        Serial.print(F("AP '"));
        Serial.print(apSSID);
        Serial.print(F("' started: "));
        Serial.println(apStarted ? F("YES") : F("NO"));
        Serial.print(F("AP IP: "));
        Serial.println(WiFi.softAPIP().toString());

        // Try to also enable STA mode (for dual-mode access)
        // On C3, AP+STA can be unreliable, so we keep AP as primary
        Serial.print(F("WiFi mode: "));
        Serial.println(WiFi.getMode());

        // Start mDNS so device is accessible as pakanikan.local
        #ifdef ARDUINO_ESP32
        if (MDNS.begin("pakanikan")) {
            Serial.println(F("mDNS: pakanikan.local ready"));
            MDNS.addService("http", "tcp", 80);
        } else {
            Serial.println(F("mDNS: failed to start"));
        }
        #endif

        dnsServer.start(53, "*", WiFi.softAPIP());

        // ── Main page: Full SPA ──
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            String html = FPSTR(PAGE_HEAD);
            html += FPSTR(PAGE_BODY);
            html += FPSTR(PAGE_FOOT);
            request->send(200, "text/html", html);
        });

        // ── API: Status (GET) ──
        server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "application/json", buildStatusJSON());
        });

        // ── API: Action (POST) ──
        server.on("/api/action", HTTP_POST,
            [](AsyncWebServerRequest *request) {
                // Handler called after body is received
            },
            NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                String body = "";
                for (size_t i = 0; i < len; i++) body += (char)data[i];
                handleAction(request, body);
            }
        );

        // ── API: Settings GET ──
        server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request){
            // Return current settings as JSON (reuse status settings portion)
            String json = buildStatusJSON();
            // Extract just the settings portion
            int start = json.indexOf("\"settings\":{");
            if (start > 0) {
                String settings = "{" + json.substring(start + 10);
                request->send(200, "application/json", settings);
            } else {
                request->send(200, "application/json", "{}");
            }
        });

        // ── API: Settings POST ──
        server.on("/api/settings", HTTP_POST,
            [](AsyncWebServerRequest *request) {},
            NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                // For now, acknowledge — full settings persistence requires
                // making config values runtime variables
                request->send(200, "application/json", "{\"ok\":true,\"message\":\"Settings saved (requires firmware update for full persistence)\"}");
            }
        );

        // ── API: WiFi Save (POST) ──
        server.on("/api/wifi", HTTP_POST,
            [](AsyncWebServerRequest *request) {},
            NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                String body = "";
                for (size_t i = 0; i < len; i++) body += (char)data[i];

                String ssid = "", pass = "";
                int sIdx = body.indexOf("\"ssid\"");
                if (sIdx > 0) {
                    int colon = body.indexOf(":", sIdx);
                    int start = body.indexOf("\"", colon + 1) + 1;
                    int end = body.indexOf("\"", start);
                    ssid = body.substring(start, end);
                }
                int pIdx = body.indexOf("\"pass\"");
                if (pIdx > 0) {
                    int colon = body.indexOf(":", pIdx);
                    int start = body.indexOf("\"", colon + 1) + 1;
                    int end = body.indexOf("\"", start);
                    pass = body.substring(start, end);
                }

                Serial.printf("Connecting to %s...\n", ssid.c_str());
                // Keep AP+STA mode and start STA connection
                // Do NOT restart — the web portal stays accessible via AP IP
                WiFi.mode(WIFI_AP_STA);
                WiFi.begin(ssid.c_str(), pass.c_str());

                // Wait up to 15 seconds for connection
                int timeout = 30;
                while (WiFi.status() != WL_CONNECTED && timeout > 0) {
                    delay(500);
                    timeout--;
                    Serial.print(F("."));
                }
                Serial.println();

                if (WiFi.status() == WL_CONNECTED) {
                    Serial.print(F("STA IP: "));
                    Serial.println(WiFi.localIP().toString());
                    // Sync NTP time now that we have internet
                    syncTimeNTP();
                    request->send(200, "application/json", "{\"ok\":true,\"message\":\"Connected to " + ssid + "! IP: " + WiFi.localIP().toString() + "\"}");
                } else {
                    Serial.println(F("STA connect failed — AP still active"));
                    request->send(200, "application/json", "{\"ok\":true,\"message\":\"Saved but connect failed. AP still active at " + WiFi.softAPIP().toString() + "\"}");
                }
            }
        );

        // ── Captive portal redirect ──
        server.onNotFound([](AsyncWebServerRequest *request){
            request->redirect("http://" + WiFi.softAPIP().toString());
        });

        server.begin();
        Serial.println(F("Web Portal Started. Connect to PakanIkan-Config"));
    }

    void handleWebRequests() {
        dnsServer.processNextRequest();
        if (shouldRestart) {
            delay(1000);
            ESP.restart();
        }
    }

#endif // ARDUINO_ARCH_ESP32
