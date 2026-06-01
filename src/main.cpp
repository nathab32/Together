#include <AudioTools.h>
#include <AudioTools/Communication/AudioHttp.h>
#include "Credentials.h"
#include "Audio.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

const char *mqtt_serv = SERVER;

WiFiClient espClient;
PubSubClient client(espClient);

// PINS

// Audio audio;

void setup() {
  delay(100);
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Program Start");

  Serial.print("Connecting to: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASS);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_serv, 1883);

  // initialize audio after Serial is ready
  // if (!audio.beginMic()) Serial.println("beginMic failed");
  // if (!audio.beginAmp()) Serial.println("beginAmp failed");
  // audio.setVolume(0.5);
}

void reconnect() {
  while(!client.connected()){
    Serial.println("Reconnecting to MQTT");

    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX);

    if(client.connect(clientID.c_str(), MQTT_USER, MQTT_PASS)){
      Serial.println("Connected");
    } else {
      Serial.print("Failed, code=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

unsigned long lastMsg = 0;
int count = 0;
void loop() {
  if(!client.connected()){
    reconnect();
  }

  JsonDocument doc;
  char output[80];

  long now = millis();
  if (now - lastMsg > 10000){
    lastMsg = now;

    doc["Message"] = count;

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("hello", output);

    count++;
  }
}