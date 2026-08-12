#include <AudioTools.h>
#include "Audio.h"
#include "HTTP.h"
#include "UI.h"

#include <WiFi.h>
#include "time.h"
#include <WiFiManager.h>

#include <Preferences.h>

#include <ArduinoJson.h>

Audio audio;
HTTP http;

bool recording = false;
bool toggleRecordingRequested = false;

bool toneOn = false;
bool toggleTone = false;

bool receiving = false;
bool toggleReceiveRequested = false;
bool isPlaying = false;

UI ui;
struct Credentials {
  String user;
  String pass;
  String server;
  String invite;
};

Credentials creds;

void startRecording() {
  if (audio.beginUpload(creds.server.c_str(), 8000, "/upload_audio", creds.user, creds.pass)){
    recording = true;
    Serial.println("Uploading audio via HTTP POST");
  }
}

void stopRecording() {
  if (!recording) return;

  audio.endUpload();
  recording = false;
  Serial.println("Upload finished");
}


/////////// WiFiManager  ///////////
WiFiManagerParameter togetherUser("Username", "Username", "", 20);
WiFiManagerParameter togetherPass("Password", "Password", "", 20, "type=\"password\"");
WiFiManagerParameter togetherServer("Server", "Server", "", 20);
WiFiManagerParameter togetherInvite("Invite_Code", "Invite Code", "", 8, "type=\"password\"");
WiFiManagerParameter promptParameter("Prompt", "Prompt", "", 64);

WiFiManager wm;

bool portalRunning = false;
bool stopPortal = false;
void L_interrupt()
{
  stopPortal = true;
}

bool shouldSaveConfig = false;

void saveConfigCallback(){
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool shouldSaveParams = false;

void saveParamsCallback(){
  Serial.println("Should save params");
  shouldSaveParams = true;
}

void saveParams(){
  Preferences pref;

  pref.begin("TogetherCreds", false);

  String user = togetherUser.getValue();
  if(!user.equals("")){ 
    pref.putString("username", user);
    creds.user = user;
  }

  String pass = togetherPass.getValue();
  if(!pass.equals("")){
    pref.putString("password", pass);
    creds.pass = pass;
  }

  String server = togetherServer.getValue();
  if(!server.equals("")){
    pref.putString("server", server);
    creds.server = server;
  }
  
  String invite = togetherInvite.getValue();
  if(!invite.equals("")){
    pref.putString("invite", invite);
    creds.invite = invite;
  }

  pref.end();

  shouldSaveParams = false;
  Serial.println("New login info saved");

  http.updateCreds();
}

/////////////////////// HELPER FUNCTIONS ///////////////////
void initializeAudio() {
  audio.beginLogger();
  if (!audio.beginSineGenerator()) Serial.println("beginSine failed");
  if (!audio.beginMic()) Serial.println("beginMic failed");
  if (!audio.beginAmp()) Serial.println("beginAmp failed");
  audio.ampOn();
  audio.setSpeakerVolume(0.3);
}

void initializeCredentials() {
  Preferences preferences;
  preferences.begin("TogetherCreds", true);
  creds.user = preferences.getString("username");
  creds.pass = preferences.getString("password");
  creds.server = preferences.getString("server");
  creds.invite = preferences.getString("invite");
  preferences.end();
}

//runs autoconnect at end
void initializeWiFiManager() {
  // wm.resetSettings(); //remove saved wifis for testing

  wm.setAPStaticIPConfig(IPAddress(142, 250, 186, 131), IPAddress(142, 250, 186, 0), IPAddress(255, 255, 255, 0));
  wm.setSaveConfigCallback(saveConfigCallback);

  wm.addParameter(&togetherUser);
  wm.addParameter(&togetherPass);
  wm.addParameter(&togetherServer);
  wm.addParameter(&togetherInvite);
  wm.addParameter(&promptParameter);

  wm.setSaveParamsCallback(saveParamsCallback);
  wm.setParamsPage(true);
  wm.setConfigPortalTimeout(180);
  wm.autoConnect("Together");
}

void testCreds(bool startup) {
  if (http.testConnection()) {
    if (startup) ui.infoText("Connected to server");
    else ui.centerText("Connected to server", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);

    if (!http.testLogin()) {
      if (!http.registerUser()) {
        ui.centerText("Credentials incorrect", u8g2_font_ciircle13_tr, EMPTY, 1, 2000);
        ui.centerText("Please reconfigure", u8g2_font_ciircle13_tr, EMPTY);
      } else {
        ui.centerText("User registered successfully", u8g2_font_ciircle13_tr, EMPTY, 1, 2000);
        ui.centerText("Welcome " + creds.user + "!", u8g2_font_ciircle13_tr, EMPTY, 1, 2000);
        String prompt = promptParameter.getValue();
        if (!prompt.isEmpty()) {
          if (http.submitPrompt(prompt)) {
            ui.centerText("Prompt submitted!", u8g2_font_ciircle13_tr, EMPTY, 1, 2000);
          }
        }
      }
    } else {
      ui.centerText("Welcome " + creds.user + "!", u8g2_font_ciircle13_tr, EMPTY, 1, 2000);
      String prompt = promptParameter.getValue();
      if (!prompt.isEmpty()) {
        if (http.submitPrompt(prompt)) {
          ui.centerText("Prompt submitted!", u8g2_font_ciircle13_tr, EMPTY, 1, 2000);
        }
      }
    }
  } else {
    ui.centerText("DC from server");
  }
}

////////////////////////// SETUP //////////////////////////
void setup() {
  // delay(100);
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Program Start");

  
  ui.begin();
  ui.info();
  ui.mainMenuItems = {
    {"Together", [&]()
      {
        JsonDocument doc = http.fetchTodayPrompt();
        ui.together(doc["date"], doc["prompt"]);

      }},
    {"Volume", [&]() {

      }},
    {"Tuner", [&]() {

      }},
    {"Lights", [&]() {

      }},
    {"Message", [&]() {

      }},
    {"Configure", [&]() {
        if (portalRunning) {
          if (wm.getConfigPortalActive()) {
            wm.process();
          } else {
            portalRunning = false;
            detachInterrupt(L_PIN);
            if (shouldSaveParams) {
              saveParams();
              testCreds(false);
            }
            ui.mainMenu();
          }
        } else {
          ui.configure();
          attachInterrupt(L_PIN, L_interrupt, FALLING);
          wm.setConfigPortalBlocking(false);
          wm.startConfigPortal("Together");
          portalRunning = true;
        }

        if (stopPortal) {
          stopPortal = false;
          portalRunning = false;
          detachInterrupt(L_PIN);
          wm.stopConfigPortal();
          if (shouldSaveParams) {
            saveParams();
            testCreds(false);
          }
          ui.mainMenu();
        }
      }
    }
  };

  ui.togetherItems = {
    {"Get Recordings", [&]() {
      http.fetchFileList();
    }}
  };

  ui.togetherMenuItems = {

  };

  // initialize audio after Serial is ready
  initializeAudio();

  initializeCredentials();

  initializeWiFiManager();

  if(shouldSaveParams){
    saveParams();
  }
  http.updateCreds(); //load creds into http

  testCreds(true); //tests creds and submits prompt if successful
  

  Serial.println("WiFi connected: ");
  Serial.println(WiFi.localIP());

  ui.info();
  ui.infoText("C to continue...");
  ui.waitForInput();
  ui.mainMenu();

}

void loop() {
  
  ui.update();

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
    std::vector<String> fileList = http.fetchFileList();

    if(fileList.size() > 0){
      Serial.println("The following files are available:");
      for(String fileName : fileList){
        Serial.println(fileName);
      }
    }

    // if(audio.beginURL_Stream(("http://" + creds.server + ":8000/recording_1781924740.wav").c_str(), creds.user, creds.pass)){
    //   Serial.println("Reading from URL now");
    //   isPlaying = true;
    // }
  }

  if (isPlaying) {
    if(audio.copyURLStream(20) == 0 && !audio.URL_Available()){
      Serial.println("Playback finished");
      audio.endURL();
      isPlaying = false;
    }
  }

}