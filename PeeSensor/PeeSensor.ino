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

const int trigPin = 2;  //D4
const int echoPin = 0;  //D3
int soundPin = A0;
int distance = 0;
int prevDistance = 0;
long duration = 0;
bool notify = false;
bool barked = false;
unsigned long StartTime;
unsigned long CurrentTime;
unsigned long ElapsedTime;
unsigned long StartCooldown = 0;
unsigned long CurrentCooldown;
unsigned long ElapsedCooldown;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial) { delay(100); }

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(soundPin, INPUT);

  // connect to wifi.
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
}

void loop() {
  // put your main code here, to run repeatedly:
  // clear trigger pin
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // send signal
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  // stop signal
  digitalWrite(trigPin, LOW);
  // get microsecond duration of sound wave travel
  duration = pulseIn(echoPin, HIGH);
  // calculate distance by taking time * speed of sound (divide by 2 because two-way trip)
  distance = duration * 0.01723;
//  Serial.print("Distance: ");
//  Serial.print(distance);
//  Serial.println(" cm");
  Firebase.setInt("distance", distance);
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }

  // microphone input
  int soundState = analogRead(soundPin);
//  Serial.print("Sound level: ");
//  Serial.println(soundState);
  Firebase.setInt("sound", soundState);
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }

  // case 1: dog has entered the proximity range
  if (prevDistance > 15 && distance <= 15) {
    // start timer to track how long dog is in range
    StartTime = millis();
    if (soundState > 300) {
      barked = true;
    }
  // case 2: dog has remained in prox. range
  } else if (prevDistance <= 15 && distance <= 15) {
    if (soundState > 300) {
      barked = true;
    }
    // calc how long dog has been in range
    CurrentTime = millis();
    ElapsedTime = CurrentTime - StartTime;
    // calc how long cooldown has been (if one has been going on)
    CurrentCooldown = millis();
    ElapsedCooldown = CurrentCooldown - StartCooldown;
    // if the dog has been in range for 2 seconds AND either the cooldown of 5 seconds is up or it was never on AND the dog has barked, send notification on app)
    if (ElapsedTime > 2000 && (ElapsedCooldown > 5000 || ElapsedCooldown == CurrentCooldown) && barked) {
      Serial.println("Send message because 5 seconds of waiting");
      Firebase.setBool("washroom/notify", true);
      delay(500);
      // start cooldown timer
      StartCooldown = millis();
      barked = false;
      Firebase.setBool("washroom/notify", false);
    }
  // case 3: dog has left range
  } else if (prevDistance <= 15 && distance > 15) {
    Serial.println("Set notify to false because dog left range");
    Firebase.setBool("washroom/notify", false);
  // case 4: dog was not in range before and did not enter range
  }
  prevDistance = distance;
  delay(100);
}
