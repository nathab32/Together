#include <AudioTools.h>
#include "Credentials.h"
#include "Audio.h"

#include <OneButton.h>

#include <WiFi.h>
#include "ArduinoMqttClient.h"
#include <WiFiManager.h>

WiFiClient espClient;
MqttClient client(espClient);

Audio audio;
// CsvOutput<int16_t> csvOutput(Serial);

OneButton L(0);
OneButton C(2);
OneButton R(4);

bool recording = false;
bool toggleRecordingRequested = false;

bool toneOn = false;
bool toggleTone = false;

bool receiving = false;
bool toggleReceiveRequested = false;
bool isPlaying = false;

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

void LPfunction(void *OneButton) {
    Serial.println("L pressed");
    toggleRecordingRequested = true;
}

void startRecording() {
  // if (!client.connected()) reconnect();
  // if (client.beginMessage("esp32/audio/control", false)) {
  //   client.print("START_RECORDING");
  //   client.endMessage();
  //   recording = true;
  //   Serial.println("Recording started");
  // } else {
  //   Serial.println("Failed to start recording: beginMessage failed");
  // }
  if (audio.beginUpload("10.0.0.78", 8000, "/upload_audio")){
    recording = true;
    Serial.println("Uploading audio via HTTP POST");
  }
}

void stopRecording() {
  if (!recording) return;
  // if (client.beginMessage("esp32/audio/control", false)) {
  //   client.print("STOP_RECORDING");
  //   client.endMessage();
  //   Serial.println("Recording stopped and published");
  // } else {
  //   Serial.println("Recording stop failed");
  // }
  audio.endUpload();
  recording = false;
  Serial.println("Upload finished");
}

void onMessageCallback(int messageSize) {
  String topic = client.messageTopic();
  // Serial.print("Received a message with topic '");
  // Serial.print(topic);
  // Serial.print("', length ");
  // Serial.print(messageSize);
  // Serial.println(" bytes:");


  // if (topic == "esp32/audio/out")
  // {
  //   int bytesRead = 0;
  //   uint8_t buffer[256];

  //   while(bytesRead < messageSize){
  //     if (client.available()){
  //       int remaining = messageSize - bytesRead;
  //       int toRead = std::min((int)sizeof(buffer), remaining);

  //       int currentRead = client.read(buffer, toRead);

  //       if(currentRead > 0) {
  //         bytesRead += currentRead;
  //         audio.writeDecoder(buffer, currentRead);
  //       }
  //     } else {
  //       delay(1);
  //     }

      
      
  //   }
  // }
  
  if(topic == "esp32/audio/control") {
    String message = "";

    while(client.available()){
      message += (char)client.read();
    }

    if(message == "STOP_SEND") {
      receiving = false;
      client.unsubscribe("esp32/audio/out");
      Serial.println("unsubscribed from audio/out");
    }
  }

  // Serial.println();
}

void callback_L() {
  toggleRecordingRequested = true;
}

void callback_C() {
  toggleTone = true;
}

void callback_R() {
  toggleReceiveRequested = true;
}

void setup() {
  delay(100);
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Program Start");

  L.attachClick(callback_L);
  C.attachClick(callback_C);
  R.attachClick(callback_R);

  Serial.print("Connecting to: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASS);

  // WiFiManager wm;
  // wm.resetSettings();
  // wm.autoConnect("Together");


  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected: ");
  Serial.println(WiFi.localIP());


  // initialize audio after Serial is ready
  audio.beginLogger();
  if (!audio.beginSineGenerator()) Serial.println("beginSine failed");
  if (!audio.beginMic()) Serial.println("beginMic failed");
  if (!audio.beginAmp()) Serial.println("beginAmp failed");
  audio.ampOn();
  audio.setSpeakerVolume(0.3);

  
  if(!client.connected()){
    reconnect();
  }
  client.onMessage(onMessageCallback);
  client.subscribe("esp32/audio/control");

}

void loop() {
  L.tick();
  C.tick();
  R.tick();
  client.poll();

  if (toggleRecordingRequested) {
    toggleRecordingRequested = false;
    if (recording) {
      stopRecording();
    } else {
      startRecording();
    }
  }

  if (recording) {
    // if(audio.uploadMic()){
    //   Serial.println("Audio uploaded to HTTP");
    // } else {
    //   Serial.println("No audio written to HTTP");
    // }
    Serial.println(audio.uploadMic());
  }

  if (toggleTone) {
    toggleTone = false;
    toneOn = !toneOn;
  }
  
  if(toneOn){
      audio.copySpeaker();
  }
  
  if (toggleReceiveRequested) {
    toggleReceiveRequested = false;
    // // receiving = !receiving;
    // client.beginMessage("esp32/audio/control");
    // client.print("SEND");
    // client.endMessage();

    // Serial.println("listening to audio/out");
    // client.subscribe("esp32/audio/out");
    if(audio.beginURL_Stream("http://10.0.0.78:8000/recording_1781924740.wav")){
      Serial.println("Reading from URL now");
      isPlaying = true;
    }
  }

  if (isPlaying) {
    if(audio.copyURLStream(20) == 0 && !audio.URL_Available()){
      Serial.println("Playback finished");
      isPlaying = false;
    }
  }

  if (!client.connected()) {
    reconnect();
  }
  client.poll();
  // delay(10);
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

// #include "AudioTools.h"
// #include "AudioTools/AudioCodecs/CodecFLAC.h"
// #include "AudioTools/Communication/AudioHttp.h"
// #include "Audio.h"

// const char* ssid = SSID;
// const char* pwd = PASS;
// URLStream url(ssid, pwd);
// FLACDecoder dec;
// I2SStream i2s;


// WORKING AUDIO PLAYBACK (INCREASE BUFFER SIZE)
// void setup() {
//   Serial.begin(115200);
//   AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);  

//   auto config_amp = i2s.defaultConfig(TX_MODE);
//   config_amp.sample_rate = 44100;
//   config_amp.bits_per_sample = 16;
//   config_amp.i2s_format = I2S_STD_FORMAT;
//   config_amp.buffer_size = 1024;
//   config_amp.buffer_count = 8;
//   config_amp.channel_format = I2S_CHANNEL_FMT_ALL_LEFT; // For mono, use left channel
//   config_amp.port_no = 1;
//   config_amp.pin_ws = MAX_LRC;
//   config_amp.pin_bck = MAX_BCLK;
//   config_amp.pin_data = MAX_DIN;
//   i2s.begin(config_amp);

//   url.begin("https://github.com/ietf-wg-cellar/flac-test-files/raw/refs/heads/main/subset/01%20-%20blocksize%204096.flac");
//   dec.setInput(url);
//   dec.setOutput(i2s);
//   dec.begin();
// }

// void loop() {
//   dec.copy();
// }