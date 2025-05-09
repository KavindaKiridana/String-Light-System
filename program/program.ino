#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "credentials.h"  // Contains WIFI_SSID and WIFI_PASSWORD

// Global Objects
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // GMT+5:30
ESP8266WebServer server(80);

// GPIO Pins
const int led1 = D1;
const int led2 = D2;
const int led3 = D5;
const int led4 = D6;

// Variables
String turningPoint="2025-05-09 21:20";//
bool webSwitch = true;
int timerHours = 0;
unsigned long timerEndEpoch = 0;

// Get formatted current time
String getFormattedTime() {
  timeClient.update();
  time_t rawTime = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&rawTime);
  char buffer[25];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d",
          timeinfo->tm_year + 1900,
          timeinfo->tm_mon + 1,
          timeinfo->tm_mday,
          timeinfo->tm_hour,
          timeinfo->tm_min);
  return String(buffer);
}

// Root page handler
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>"
                "body{font-family:Arial,sans-serif;text-align:center;padding:20px;background:#f0f0f0;}"
                ".button{padding:15px 30px;border:none;border-radius:8px;font-size:18px;cursor:pointer;margin:20px auto;display:block;transition:0.3s;color:white;font-weight:bold;}"
                ".red{background:#e74c3c;}"
                ".green{background:#2ecc71;}"
                "form{margin-top:30px;}"
                "input[type='number']{padding:10px;font-size:16px;width:60px;border-radius:5px;border:1px solid #ccc;}"
                "input[type='submit']{margin-top:10px;background:#3498db;color:white;padding:10px 20px;border:none;border-radius:6px;cursor:pointer;}"
                ".yellow-box{background:#f1c40f;padding:10px;margin-top:20px;border-radius:6px;font-weight:bold;}"
                "</style></head><body><h1>ESP8266 Control Panel</h1>";

  // Power switch button
  html += webSwitch ? "<button id='switchBtn' class='button red' onclick='toggleSwitch()'>Turn Off</button>"
                    : "<button id='switchBtn' class='button green' onclick='toggleSwitch()'>Turn On</button>";

  // Timer form
  html += "<form onsubmit='return setTimer(event)'><h2>Set Timer</h2>"
          "<input type='number' id='hourInput' name='hours' min='1' max='24' required>"
          "<br><input type='submit' value='Set Timer'></form>";

  // Conditional display of timer info
  if (timerHours > 0) {
    html += "<div class='yellow-box'>Light would automatically turn off at " + String(timerHours) + " hour(s) from now.</div>";
  }

  // JavaScript
  html += "<script>"
          "function toggleSwitch(){"
          "fetch('/toggle',{method:'POST'})"
          ".then(res=>res.json())"
          ".then(data=>{"
          "const btn=document.getElementById('switchBtn');"
          "btn.className='button '+(data.state?'red':'green');"
          "btn.innerText=data.state?'Turn Off':'Turn On';});"
          "}"
          "function setTimer(e){"
          "e.preventDefault();"
          "const hours=parseInt(document.getElementById('hourInput').value);"
          "if(hours>0&&hours<25){"
          "fetch('/setTimer',{method:'POST',headers:{'Content-Type':'application/json'},"
          "body:JSON.stringify({hours:hours})})"
          ".then(()=>location.reload());"
          "}"
          "return false;}"
          "</script></body></html>";

  server.send(200, "text/html", html);
}

// Toggle button handler
void handleToggle() {
  webSwitch = !webSwitch;
  DynamicJsonDocument doc(64);
  doc["state"] = webSwitch;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  Serial.print("WebSwitch state changed to: ");
  Serial.println(webSwitch ? "ON" : "OFF");
}

// Timer submission handler
void handleSetTimer() {
  DynamicJsonDocument doc(128);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int hours = doc["hours"];
  if (hours >= 1 && hours <= 24) {
    timerHours = hours;
    timerEndEpoch = timeClient.getEpochTime() + (hours * 3600);
    Serial.print("Timer set for ");
    Serial.print(timerHours);
    Serial.println(" hour(s).");
    server.send(200, "application/json", "{\"status\":\"Timer Set\"}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid hour value\"}");
  }
}

// 404 handler
void handleNotFound() {
  String message = "Page Not Found\n\nURI: " + server.uri() + "\nMethod: " +
                   (server.method() == HTTP_GET ? "GET" : "POST") + "\nArguments: " +
                   server.args() + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin()) Serial.println("LittleFS failed!");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected");
  Serial.print("Visit: http://");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    delay(100);
  }

  // Web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/setTimer", HTTP_POST, handleSetTimer);
  server.onNotFound(handleNotFound);
  server.begin();

  // LED pin setup
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void loop() {
  server.handleClient();
  String currentTime = getFormattedTime();

  // Stop conditions
  if (turningPoint==currentTime || !webSwitch || (timerEndEpoch > 0 && timeClient.getEpochTime() >= timerEndEpoch)) {
    Serial.println(!webSwitch ? "User turned off the lights" :
                   (turningPoint == currentTime ? "Turning point reached!" : "Timer ended! Lights off."));
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    timerEndEpoch = 0;
    return;
  }

  Serial.println(currentTime);

  // LED sequence
  digitalWrite(led1, HIGH); delay(500); digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH); delay(500); digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH); delay(500); digitalWrite(led3, LOW);
  digitalWrite(led4, HIGH); delay(500); digitalWrite(led4, LOW);
}
