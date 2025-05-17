#include <Arduino.h>
#
// Correct GPIO pins for ESP8266 (NodeMCU)
const int led1 = D1;  // GPIO5
const int led2 = D2;  // GPIO4
const int led3 = D5;  // GPIO14
const int led4 = D6;  // GPIO12

void setup() {
  Serial.begin(115200);

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
  digitalWrite(led2, HIGH);
  delay(500);
  digitalWrite(led2, LOW);
  
  digitalWrite(led3, HIGH);
  delay(500);
  digitalWrite(led3, LOW);
  
  digitalWrite(led4, HIGH);
  delay(500);
  digitalWrite(led4, LOW);
  
  digitalWrite(led1, HIGH);  // this bulb is turned on always this program keep power supply on
}