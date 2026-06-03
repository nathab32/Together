#include <AudioTools.h>
#include "Credentials.h"
#include "Audio.h"

#include <WiFi.h>
#include "ArduinoMqttClient.h"
// #include <PubSubClient.h>

WiFiClient espClient;
MqttClient client(espClient);
// PubSubClient client(espClient);

// PINS

Audio audio;
AudioInfo info(8000, 1, 16);
SineGenerator<int16_t> sine(440);
GeneratedSoundStream<int16_t> in(sine);
EncodedAudioStream out(&client, new WAVEncoder());
StreamCopy copier(out, in, BUFFER_SIZE);

void reconnect() {
  while(!client.connected()){
    Serial.println("Reconnecting to MQTT");

    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX);

    // pubsubclient code
    // if(client.connect(clientID.c_str(), MQTT_USER, MQTT_PASS)){
    //   Serial.println("Connected");
    // } else {
    //   Serial.print("Failed, code=");
    //   Serial.println(client.state());
    //   delay(5000);
    // }

    //arduinomqttclient code
    client.setId(clientID);
    client.setUsernamePassword(MQTT_USER, MQTT_PASS);
    if(!client.connect(SERVER, 1883)){
      Serial.println("Failed");
      delay(5000);
    } else {
      Serial.println("Connected");
      
    }
  }
}

void setup() {
  delay(100);
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Program Start");

  // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
  // sine.begin(info, 440);
  // in.begin(info);
  // out.begin(info);

  Serial.print("Connecting to: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASS);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected: ");
  Serial.println(WiFi.localIP());

  // client.setServer(mqtt_serv, 1883);

  // initialize audio after Serial is ready
  audio.beginLogger();
  if (!audio.beginEncoder(client)) Serial.println("beginEncoder failed");
  if (!audio.beginMic()) Serial.println("beginMic failed");
  // if (!audio.beginAmp()) Serial.println("beginAmp failed");
  // audio.setVolume(0.5);
  
  if(!client.connected()){
    reconnect();
  }

  client.beginMessage(MQTT_USER, BUFFER_SIZE * AUDIO_BUFFER_COUNT, true);

  // copier.copyN(AUDIO_BUFFER_COUNT);
  audio.copyMic(AUDIO_BUFFER_COUNT);

  if(client.endMessage()){
    Serial.println("Publish success");
  } else {
    Serial.println("Publish failed");
  }
}


void loop() {
  client.poll();  // Keep MQTT connection alive
  
  // if(!client.connected()){
  //   reconnect();
  // }
  delay(1000);
}