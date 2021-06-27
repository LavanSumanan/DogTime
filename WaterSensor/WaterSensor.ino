#include "arduino_secrets.h"
// ArduinoJson - Version: 5.13.1
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#define FIREBASE_HOST SECRET_HOST
#define FIREBASE_AUTH SECRET_AUTH
#define WIFI_SSID SECRET_SSID
#define WIFI_PASSWORD SECRET_PASS

const int signalPin = A0;
int sensorValue = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(100); }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  pinMode(signalPin, INPUT);
}

void loop() {
  int sensorValue = analogRead(signalPin);
  Firebase.setInt("water level reading", sensorValue);
  if (sensorValue > 550) {
    Firebase.setInt("water/level","3");
  } else if (sensorValue <= 550 && sensorValue > 480){
    Firebase.setString("water/level","2");
  } else if (sensorValue <= 480 && sensorValue > 300) {
    Firebase.setString("water/level","1");
  } else if (sensorValue <= 380) {
    Firebase.setString("water/level","0");
  }
  delay(1000);
}
