#include "../cfg/DFConfig.h"

#if USE_WEBSERVER == 1

#include "DFWebServer.h"
#include "DFWifi.h"
#include "../Utility/DebugPort.h"
#include "../Utility/helpers.h"
#include "../cfg/pins.h"
#include "../Utility/DF_JSON.h"
#include "../Utility/NVStorage.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <Update.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ── Globals ──────────────────────────────────────────────────
static AsyncWebServer g_server(80);
static AsyncWebSocket g_ws("/");
static AsyncWebSocket g_wsUpdate("/update");
static bool g_clientChange = false;
static int  g_lastClientCount = 0;
static TaskHandle_t handleWebServerTask = NULL;

// ── Helpers ──────────────────────────────────────────────────
static const char* contentType(const char* path) {
  const char* ext = strrchr(path, '.');
  if (!ext) return "text/html";
  if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
  if (strcmp(ext, ".css") == 0)  return "text/css";
  if (strcmp(ext, ".js") == 0)   return "application/javascript";
  if (strcmp(ext, ".json") == 0) return "application/json";
  if (strcmp(ext, ".png") == 0)  return "image/png";
  if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
  if (strcmp(ext, ".svg") == 0)  return "image/svg+xml";
  if (strcmp(ext, ".ico") == 0)  return "image/x-icon";
  if (strcmp(ext, ".gz") == 0)   return "application/gzip";
  return "text/plain";
}

// ── WebSocket event handler (JSON telemetry) ──────────────────
static void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                      AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DebugPort.printf("WS client #%u connected from %s\r\n",
      client->id(), client->remoteIP().toString().c_str());
    g_clientChange = true;
    resetAllJSONmoderators();
  } else if (type == WS_EVT_DISCONNECT) {
    DebugPort.printf("WS client #%u disconnected\r\n", client->id());
    g_clientChange = true;
  }
}

// ── WebSocket event handler (update) ─────────────────────────
static void onWsUpdateEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                            AwsEventType type, void* arg, uint8_t* data, size_t len) {
  // Handle incoming text commands from the update page (erase, rename)
  if (type == WS_EVT_DATA && len > 0) {
    char cmd[64];
    size_t clen = len < sizeof(cmd)-1 ? len : sizeof(cmd)-1;
    memcpy(cmd, data, clen);
    cmd[clen] = 0;

    if (strncmp(cmd, "erase ", 6) == 0) {
      const char* fname = cmd + 6;
      if (SPIFFS.exists(fname)) {
        SPIFFS.remove(fname);
        client->printf("{\"msg\":\"Erased %s\"}", fname);
      }
    } else if (strncmp(cmd, "rename ", 7) == 0) {
      char from[64], to[64];
      if (sscanf(cmd + 7, "%63s %63s", from, to) == 2) {
        if (SPIFFS.exists(from)) {
          SPIFFS.rename(from, to);
          client->printf("{\"msg\":\"Renamed %s to %s\"}", from, to);
        }
      }
    }
  }
}

// ── Route handlers ───────────────────────────────────────────
static void handleRoot(AsyncWebServerRequest* request) {
  String path = request->url();
  if (path == "/") path = "/index.htm";
  if (!SPIFFS.exists(path)) {
    request->send(404, "text/plain", "Not found");
    return;
  }
  File f = SPIFFS.open(path, "r");
  if (!f) {
    request->send(404, "text/plain", "Not found");
    return;
  }
  request->send(SPIFFS, path, contentType(path.c_str()));
  f.close();
}

static void handleUpdatePage(AsyncWebServerRequest* request) {
  String html = R"(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>DieselFire Update</title>
<style>body{font-family:sans-serif;background:#222;color:#fff;margin:20px}
h1{color:#FF8C00}form{margin:20px 0}input{margin:10px 0}
#msg{white-space:pre-wrap;font-family:monospace;background:#111;padding:10px;border-radius:4px;min-height:100px}
table{width:100%;border-collapse:collapse}td,th{border:1px solid #444;padding:6px;text-align:left}
th{background:#333}.del{color:#f44;cursor:pointer}
</style></head><body>
<h1>DieselFire Update</h1>
<form id="upf" method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="fw"><br>
<input type="submit" value="Upload">
</form>
<h2>SPIFFS Files</h2>
<table><tr><th>File</th><th>Size</th><th></th></tr>
)";
  listSPIFFS("/", 3, html, 0);
  html += R"(
</table>
<div id="msg"></div>
<script>
var ws = new WebSocket((location.protocol=='https'?'wss:':'ws:')+'//'+location.host+'/update');
ws.onmessage=function(e){document.getElementById('msg').textContent+=e.data+'\n'};
ws.onopen=function(){ws.send('ready')};
document.getElementById('upf').onsubmit=function(e){
e.preventDefault();var f=new FormData(this);
fetch('/update',{method:'POST',body:f}).then(r=>r.text()).then(t=>document.getElementById('msg').textContent+=t+'\n');
};
function erase(n){ws.send('erase '+n)}
function rename(f,t){var n=prompt('New name:',f);if(n)ws.send('rename '+f+' '+n)}
</script></body></html>
)";
  request->send(200, "text/html", html);
}

static void handleUpload(AsyncWebServerRequest* request, String filename,
                         size_t index, uint8_t* data, size_t len, bool final) {
  if (!index) {
    bool isFirmware = filename.endsWith(".bin");
    DebugPort.printf("Upload start: %s (%s)\r\n",
      filename.c_str(), isFirmware ? "firmware" : "SPIFFS");
    if (isFirmware) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
        Update.printError(DebugPort);
      }
    } else {
      if (!Update.begin(SPIFFS.totalBytes() - SPIFFS.usedBytes(), U_SPIFFS)) {
        Update.printError(DebugPort);
      }
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(DebugPort);
  }

  if (final) {
    if (Update.end(true)) {
      DebugPort.printf("Upload success: %u bytes\r\n", index + len);
      g_wsUpdate.textAll("{\"updateProgress\":100}");
    } else {
      Update.printError(DebugPort);
      g_wsUpdate.textAll("{\"updateProgress\":-1}");
    }
  } else {
    int pct = (index + len) * 100 / (request->contentLength() + 1);
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"updateProgress\":%d}", pct);
    g_wsUpdate.textAll(buf);
  }
}

static void handleUploadEnd(AsyncWebServerRequest* request) {
  if (Update.hasError()) {
    request->send(500, "text/plain", "Update failed");
  } else {
    request->send(200, "text/plain", "Update OK, rebooting...");
    delay(100);
    ESP.restart();
  }
}

static void handleReboot(AsyncWebServerRequest* request) {
  if (request->method() == HTTP_GET) {
    String html = "<!DOCTYPE html><html><body><h1>Reboot</h1>"
      "<form method='POST'><input type='submit' value='Reboot Now'></form></body></html>";
    request->send(200, "text/html", html);
  } else {
    request->send(200, "text/plain", "Rebooting...");
    delay(100);
    ESP.restart();
  }
}

static void handleFormatSPIFFS(AsyncWebServerRequest* request) {
  if (request->method() == HTTP_GET) {
    String html = "<!DOCTYPE html><html><body><h1>Format SPIFFS</h1>"
      "<form method='POST'><input type='submit' value='Format Now'></form></body></html>";
    request->send(200, "text/html", html);
  } else {
    SPIFFS.format();
    request->send(200, "text/plain", "SPIFFS formatted");
  }
}

static void handleWMConfig(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "Launching WiFi config portal...");
  wifiEnterConfigPortal(true);
}

static void handleResetWifi(AsyncWebServerRequest* request) {
  request->send(200, "text/plain", "Resetting WiFi credentials...");
  wifiFactoryDefault();
}

static void handleNotFound(AsyncWebServerRequest* request) {
  String html = "<!DOCTYPE html><html><body><h1>404 Not Found</h1>"
    "<p>Path: " + request->url() + "</p></body></html>";
  listSPIFFS("/", 2, html, 1);
  request->send(404, "text/html", html);
}

// ── Public API ───────────────────────────────────────────────
void initWebServer() {
  // mDNS
  if (!MDNS.begin("DieselFire")) {
    DebugPort.println("mDNS: failed");
  }
  MDNS.addService("http", "tcp", 80);

  // WebSocket JSON telemetry
  g_ws.onEvent(onWsEvent);
  g_server.addHandler(&g_ws);

  // WebSocket update progress
  g_wsUpdate.onEvent(onWsUpdateEvent);
  g_server.addHandler(&g_wsUpdate);

  // Routes
  g_server.on("/", HTTP_GET, handleRoot);
  g_server.on("/update", HTTP_GET, handleUpdatePage);
  g_server.on("/update", HTTP_POST, handleUploadEnd, handleUpload);
  g_server.on("/reboot", HTTP_GET, handleReboot);
  g_server.on("/reboot", HTTP_POST, handleReboot);
  g_server.on("/formatspiffs", HTTP_GET, handleFormatSPIFFS);
  g_server.on("/formatspiffs", HTTP_POST, handleFormatSPIFFS);
  g_server.on("/wmconfig", HTTP_GET, handleWMConfig);
  g_server.on("/resetwifi", HTTP_GET, handleResetWifi);
  g_server.onNotFound(handleNotFound);

  g_server.begin();
  DebugPort.println("Web server started on port 80");
}

bool doWebServer() {
  g_ws.cleanupClients();
  g_wsUpdate.cleanupClients();
  return true;
}

void stopWebServer() {
  g_ws.closeAll();
  g_wsUpdate.closeAll();
  g_server.end();
}

bool sendWebSocketString(const char* str) {
  if (g_ws.count() == 0) return false;
  g_ws.textAll(str);
  return true;
}

bool isWebSocketClientChange() {
  bool changed = g_clientChange;
  g_clientChange = false;
  return changed;
}

void listSPIFFS(const char* dirname, uint8_t levels, String& html, int withHTMLanchors) {
  // Ring the watchdog during potentially slow SPIFFS traversal
  feedWatchdog();
  File root = SPIFFS.open(dirname);
  if (!root || !root.isDirectory()) return;
  File file = root.openNextFile();
  while (file) {
    String name = String(file.name());
    // Strip leading /
    if (name.startsWith("/")) name = name.substring(1);
    if (file.isDirectory()) {
      if (levels > 0) {
        listSPIFFS(file.name(), levels - 1, html, withHTMLanchors);
      }
    } else {
      if (withHTMLanchors) {
        html += "<a href='/" + name + "'>" + name + "</a> (" + String(file.size()) + "B)<br>";
      } else {
        char buf[512];
        snprintf(buf, sizeof(buf),
          "<tr><td><a href='javascript:rename(\"%s\",\"%s\")'>%s</a></td><td>%u</td>"
          "<td class='del' onclick='erase(\"%s\")'>del</td></tr>",
          name.c_str(), name.c_str(), name.c_str(), file.size(), name.c_str());
        html += buf;
      }
    }
    file = root.openNextFile();
  }
}

const char* getWebContent(bool start) { return ""; }
void getWebContent(const char* filename) {}
bool checkWebSocketSend() { return g_ws.count() > 0; }

#endif
