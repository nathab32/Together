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
// AudioInfo info(8000, 1, 16);
// SineGenerator<int16_t> sine(440);
// GeneratedSoundStream<int16_t> in(sine);
// EncodedAudioStream out(&client, new WAVEncoder());
// StreamCopy copier(out, in, BUFFER_SIZE);

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


// #include "AudioTools.h"
// #include "Audio.h"

// AudioInfo info(44100, 1, 32);
// I2SStream i2sStream; // Access I2S as stream
// CsvOutput<int16_t> csvOutput(Serial);
// FormatConverterStream formatConverter(csvOutput);
// StreamCopy copier(formatConverter, i2sStream); // copy i2sStream to csvOutput

// // Arduino Setup
// void setup(void) {
//     Serial.begin(115200);
//     AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
    
//     auto cfg = i2sStream.defaultConfig(RX_MODE);
//     cfg.copyFrom(info);
//     // cfg.i2s_format = I2S_STD_FORMAT; // or try with I2S_LSB_FORMAT
//     cfg.signal_type = PDM;
//     // cfg.is_master = true;
//     // cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
//     // this module nees a master clock if the ESP32 is master
//     cfg.pin_bck = -1;
//     cfg.pin_ws = 19;
//     cfg.pin_data = 18;
//     // cfg.use_apll = false;  // try with yes
//     i2sStream.begin(cfg);

//     formatConverter.begin(info, AudioInfo(info.sample_rate, 1, 16));
//     // make sure that we have the correct channels set up
//     csvOutput.setDelimiter(">sample:");
//     csvOutput.begin(AudioInfo(info.sample_rate,1,16));

// }

// // Arduino loop - copy data
// void loop() {
//   copier.copy();
// }



// #include "AudioTools.h"
// #include "AudioTools/Communication/AudioHttp.h"

// //AudioEncodedServer server(new WAVEncoder(),"ssid","password");  
// AudioWAVServer server(SSID,PASS); // the same a above

// I2SStream i2sStream;    // Access I2S as stream
// VolumeStream vol(i2sStream);
// // ConverterFillLeftAndRight<int16_t> filler(LeftIsEmpty); // fill both channels - or change to RightIsEmpty

// void setup(){
//   Serial.begin(115200);
//   AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

//   // start i2s input with default configuration
//   Serial.println("starting I2S...");
//   auto config = i2sStream.defaultConfig(RX_MODE);
//   config.sample_rate = 22050;
//   config.channels = 1;
//   config.bits_per_sample = 16;
//   config.signal_type = PDM;
//   config.pin_bck = -1;
//   config.pin_data = 18;
//   config.pin_ws = 19;
//   i2sStream.begin(config);
//   Serial.println("PDM started");

//   auto vcfg = vol.defaultConfig();
//   vcfg.allow_boost = true;
//   vcfg.sample_rate = config.sample_rate;
//   vcfg.channels = config.channels;
//   vcfg.bits_per_sample = config.bits_per_sample;
//   vol.begin(vcfg);

//   vol.setVolume(10);
//   // start data sink
//   server.begin(vol, config);
// }

// // Arduino loop  
// void loop() {
//   // Handle new connections
//   server.copy();  
// }