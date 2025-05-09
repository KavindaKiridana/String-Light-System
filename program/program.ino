#include <Arduino.h>
#include <ESP8266WiFi.h>  // This is the WiFi library for ESP8266
#include <NTPClient.h>      // For NTP time functionality
#include <WiFiUdp.h>        // For UDP communication
#include <LittleFS.h>

#include "credentials.h"  // Changed from direct credentials

// --------------------- Global Objects ---------------------------
WiFiUDP ntpUDP;
// GMT+5:30 for Sri Lanka (19800 seconds offset)
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

// Correct GPIO pins for ESP8266 (NodeMCU)
const int led1 = D1;  // GPIO5
const int led2 = D2;  // GPIO4
const int led3 = D5;  // GPIO14
const int led4 = D6;  // GPIO12

String turningPoint="2025-05-09 21:20";

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

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n Successfully connected to WiFi!");

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
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  
  // Turn all LEDs off initially
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void loop() {
  String currentTime = getFormattedTime();
  if(turningPoint==currentTime){
    Serial.println("Turning point reached! Program stopping.");
    
    // Turn off all LEDs before stopping
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    
    delay(1000); 
    return;  // Exit the loop function immediately
  }
  Serial.println(currentTime);

  // Sequence with delays between each LED
  digitalWrite(led1, HIGH);
  delay(500);
  digitalWrite(led1, LOW);
  
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