#include <Arduino.h>
#include <ESP8266WiFi.h>  // This is the WiFi library for ESP8266
#include <NTPClient.h>      // For NTP time functionality
#include <WiFiUdp.h>        // For UDP communication
#include <LittleFS.h>          // File system for storing web files
#include <ESP8266WebServer.h>  // Web server library
#include <ArduinoJson.h>       // For JSON handling

#include "credentials.h"  // Changed from direct credentials

// --------------------- Global Objects ---------------------------
WiFiUDP ntpUDP;
// GMT+5:30 for Sri Lanka (19800 seconds offset)
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

ESP8266WebServer server(80);// Create web server on port 80

// Correct GPIO pins for ESP8266 (NodeMCU)
//const int led1 = D1;  // GPIO5
const int led2 = D2;  // GPIO4
const int led3 = D5;  // GPIO14
const int led4 = D6;  // GPIO12

String turningPoint="2025-05-09 21:20";
bool webSwitch = true;  // Switch state variable

// --------------------- Get Formatted Time -----------------------
String getFormattedTime()//later added
{
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

// --------------------- Web Server Handlers ---------------------

// Handler for the root page
void handleRoot() {
  // HTML content with inline CSS and JavaScript for memory efficiency
  String html = "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>"
                "body {"
                "  font-family: Arial, sans-serif;"
                "  text-align: center;"
                "  margin: 0;"
                "  padding: 20px;"
                "  background-color: #f0f0f0;"
                "}"
                ".button {"
                "  padding: 15px 30px;"
                "  border: none;"
                "  border-radius: 8px;"
                "  font-size: 18px;"
                "  cursor: pointer;"
                "  margin: 20px auto;"
                "  display: block;"
                "  transition: all 0.3s;"
                "  color: white;"
                "  font-weight: bold;"
                "}"
                ".red {"
                "  background-color: #e74c3c;"
                "}"
                ".green {"
                "  background-color: #2ecc71;"
                "}"
                "</style>"
                "</head>"
                "<body>"
                "<h1>ESP8266 Control Panel</h1>";
  
  // Add button with correct state based on webSwitch
  if (webSwitch) {
    html += "<button id='switchBtn' class='button red' onclick='toggleSwitch()'>Turn Off</button>";
  } else {
    html += "<button id='switchBtn' class='button green' onclick='toggleSwitch()'>Turn On</button>";
  }
  
  html += "<script>"
          "function toggleSwitch() {"
          "  fetch('/toggle', {method: 'POST'})"
          "    .then(response => response.json())"
          "    .then(data => {"
          "      const btn = document.getElementById('switchBtn');"
          "      if (data.state) {"
          "        btn.className = 'button red';"
          "        btn.innerText = 'Turn Off';"
          "      } else {"
          "        btn.className = 'button green';"
          "        btn.innerText = 'Turn On';"
          "      }"
          "    });"
          "}"
          "</script>"
          "</body>"
          "</html>";
  
  // Send the HTML response
  server.send(200, "text/html", html);
}

// Handler for toggle action
void handleToggle() {
  // Toggle the webSwitch state
  webSwitch = !webSwitch;
  
  // Create JSON response with current state
  DynamicJsonDocument doc(64);
  doc["state"] = webSwitch;
  
  String response;
  serializeJson(doc, response);
  
  // Send JSON response back to client
  server.send(200, "application/json", response);
  
  Serial.print("WebSwitch state changed to: ");
  Serial.println(webSwitch ? "ON" : "OFF");
}

// Handler for 404 - Page Not Found
void handleNotFound() {
  String message = "Page Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (LittleFS.begin()) {
    Serial.println("LittleFS initialized successfully");
  } else {
    Serial.println("An error occurred while mounting LittleFS");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n Successfully connected to WiFi!");

  // Print the local IP address for accessing the web server
  Serial.print("Web server URL: http://");
  Serial.println(WiFi.localIP());

  // Setup server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.onNotFound(handleNotFound);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");

  // Initialize NTP client
  timeClient.begin();
  Serial.println("Waiting for NTP time sync...");
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
    delay(100);
  }
  Serial.println("Time synchronized");

  // Initialize each LED pin as output
  // pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  
  // Turn all LEDs off initially
  // digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void loop() {
  // Handle incoming client requests
  server.handleClient();

  String currentTime = getFormattedTime();
  if(turningPoint==currentTime || !webSwitch){
    if(turningPoint==currentTime){
      Serial.println("Turning point reached! Program stopping.");
    }else{
      Serial.println("User turn off the lights");
    }
    
    // Turn off all LEDs before stopping
    // digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    
    delay(1000); 
    return;  // Exit the loop function immediately
  }
  Serial.println(currentTime);

  // Sequence with delays between each LED
  // digitalWrite(led1, HIGH);
  // delay(500);
  // digitalWrite(led1, LOW);
  
  digitalWrite(led2, HIGH);
  delay(500);
  digitalWrite(led2, LOW);
  
  digitalWrite(led3, HIGH);
  delay(500);
  digitalWrite(led3, LOW);
  
  digitalWrite(led4, HIGH);
  delay(500);
  digitalWrite(led4, LOW);
}