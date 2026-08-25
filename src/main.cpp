#include <Arduino.h>
#include "Models.h"
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
bool recordingPaused = true;
// bool toggleRecordingRequested = false;

bool toneOn = false;
bool toggleTone = false;

// bool receiving = false;
// bool toggleReceiveRequested = false;
// bool isPlaying = false;
bool playingPaused = true;

UI ui;

Credentials creds;

//time variables
const char *ntpServer = "time.nist.gov";
long gmtOffset_sec = -28800;
int daylightOffset_sec = 3600;



/////////// WiFiManager  ///////////
WiFiManagerParameter togetherUser("Username", "Username", "", 20);
WiFiManagerParameter togetherPass("Password", "Password", "", 20, "type=\"password\"");
WiFiManagerParameter togetherServer("Server", "Server", "", 20);
WiFiManagerParameter togetherInvite("Invite_Code", "Invite Code", "", 8, "type=\"password\"");
WiFiManagerParameter promptParameter("Prompt", "Prompt", "", 64);
WiFiManagerParameter gmtOffsetParameter("gmtOffset", "GMT offset (seconds)", "", 6, "type=\"number\""); //-28800 for pacific
WiFiManagerParameter daylightOffsetParameter("daylightOffset", "Daylight Savings Time offset (seconds)", "", 4, "type=\"number\""); //3600 for DST

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

  pref.begin("time", false);

  const char* gmtOffset = gmtOffsetParameter.getValue();
  if(gmtOffset != ""){
    pref.putLong("gmtOffset", strtol(gmtOffset, NULL, 10));
    gmtOffset_sec = strtol(gmtOffset, NULL, 10);
  }

  const char* dst = daylightOffsetParameter.getValue();
  if(dst != ""){
    pref.putInt("daylight", strtol(dst, NULL, 10));
    daylightOffset_sec = strtol(dst, NULL, 10);
  }
  pref.end();

  shouldSaveParams = false;
  Serial.println("New parameters saved");

  http.updateCreds();
}

/////////////////////// HELPER FUNCTIONS ///////////////////
void initializeAudio() {
  audio.beginLogger();
  // if (!audio.beginSineGenerator()) Serial.println("beginSine failed");
  if (!audio.beginMic()) Serial.println("beginMic failed");
  if (!audio.beginAmp()) Serial.println("beginAmp failed");
  audio.ampOn();
  audio.setSpeakerVolume(0.75);
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
  wm.addParameter(&gmtOffsetParameter);
  wm.addParameter(&daylightOffsetParameter);

  wm.setSaveParamsCallback(saveParamsCallback);
  wm.setParamsPage(true);
  wm.setConfigPortalTimeout(180);
  wm.autoConnect("Together");
}

String getFormattedDay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  char timeStr[15];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d", &timeinfo);
  return String(timeStr);
}

void initializeTime() {
  Preferences pref;

  pref.begin("time", true);

  gmtOffset_sec = pref.getLong("gmtOffset");
  daylightOffset_sec = pref.getInt("daylight");

  pref.end();
  Serial.println(gmtOffset_sec);
  Serial.println(daylightOffset_sec);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Serial.println(getFormattedDay());

}

void testCreds(bool startup) {
  if (http.testConnection()) {
    if (startup) ui.infoText("Connected to server");
    else ui.centerText("Connected to server", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);

    if (!http.testLogin()) {
      if (!http.registerUser()) {
        ui.centerText("Credentials incorrect", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);
        ui.centerText("Please reconfigure", u8g2_font_ciircle13_tr, EMPTY);
      } else {
        ui.centerText("User registered successfully", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);
        ui.centerText("Welcome " + creds.user + "!", u8g2_font_ciircle13_tr, EMPTY, 1, 1);
        String prompt = promptParameter.getValue();
        if (!prompt.isEmpty()) {
          if (http.submitPrompt(prompt)) {
            ui.centerText("Prompt submitted!", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);
          }
        }
      }
    } else {
      ui.centerText("Welcome " + creds.user + "!", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);
      String prompt = promptParameter.getValue();
      if (!prompt.isEmpty()) {
        if (http.submitPrompt(prompt)) {
          ui.centerText("Prompt submitted!", u8g2_font_ciircle13_tr, EMPTY, 1, 1000);
        }
      }
    }
  } else {
    ui.centerText("DC from server");
  }
}

void epochToLocalTime(time_t epoch, char* buffer, size_t size) {
  struct tm timeinfo;
  localtime_r(&epoch, &timeinfo);
  strftime(buffer, size, "%H:%M", &timeinfo);
}

void fillTogetherMenuItems() {
  std::vector<Recording> recordings = http.fetchTodayRecordings();

  ui.togetherMenuItems.resize(1);
  int i = 1;
  for (const Recording& rec : recordings) {
    String fileName = "audio_" + rec.username + "_" + String(rec.timestamp) + ".wav";
    TogetherMenuItem item;
    item.username = rec.username;
    
    char timeBuffer[6];
    epochToLocalTime(rec.timestamp, timeBuffer, sizeof(timeBuffer));
    item.time = timeBuffer;

    item.length = rec.length;
    
    item.onSelect = [&, item, fileName]() {
      audio.endURL();
      // if (audio.beginURL_Stream(("http://" + creds.server + ":8000/" + fileName).c_str(), creds.user, creds.pass)) {
      //   Serial.println("Download began");
      //   isPlaying = true;
      //   playingPaused = true;
      // }
      audio.initializeURL(("http://" + creds.server + ":8000/" + fileName), creds.user, creds.pass);
      // isPlaying = true;
      playingPaused = true;
      ui.playback(item.username.c_str(), item.length);
    };
    ui.togetherMenuItems.push_back(item);
    ++i;
  }
}

TaskHandle_t AudioTaskHandle = NULL;
void audioTask(void *pvParameters) {
  while (true) {
    if (recording && !recordingPaused) {
      // Serial.println(audio.uploadMic());
      audio.uploadMic();
     
    } else {
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

TaskHandle_t UiTaskHandle = NULL;
void uiTask(void *pvParameters) {
  const TickType_t xFrequency = pdMS_TO_TICKS(30);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (true) {
    ui.update();

    if (!playingPaused && !audio.isPlaying()) {
    Serial.println("Playback completed/stopped, resetting UI.");
    // isPlaying = false;
    playingPaused = true;
    audio.endURL();
    ui.togetherMenuItems[ui.getCurrentIndex()].onSelect();

    xTaskDelayUntil(&xLastWakeTime, xFrequency);
  }


  }
}

void startPauseRecording() {
  if (recording) {
    recordingPaused = !recordingPaused;
  } else {
    if (audio.beginUpload(("http://" + creds.server + ":8000/upload_audio").c_str(), creds.user, creds.pass)){
      recording = true;
      recordingPaused = false;
      Serial.println("Uploading audio via HTTP POST");
    }
    xTaskCreatePinnedToCore(
      audioTask,
      "AudioTask",
      4096,
      NULL,
      3,
      &AudioTaskHandle,
      0);
  }
  
}

void stopRecording() {
  if (!recording) return;

  audio.endUpload();
  vTaskDelete(AudioTaskHandle);
  recording = false;
  recordingPaused = true;
  Serial.println("Upload finished");
  ui.centerText("Audio uploaded!", u8g2_font_ciircle13_tr, EMPTY, 0);
  ui.recording();
}
////////////////////////// SETUP //////////////////////////
void setup() {

  Serial.begin(115200);
  while(!Serial);
  Serial.println("Program Start");

  WiFi.setSleep(WIFI_PS_NONE);
  // esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

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
              configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
          }
          ui.mainMenu();
        }
      }
    }
  };

  ui.togetherItems = {
    {"togetherMenu", [&]() {
      fillTogetherMenuItems();
      ui.togetherMenu();
    }}
  };

  ui.togetherMenuItems = {
    {"Record", [&]() {
      ui.recording();
      recordingPaused = true;
    }, "", 0}
  };

  ui.recordingsItems[0] = {
    "StartPauseRecording", [&]()
    {
      startPauseRecording();
    }
  };

  ui.recordingsItems[1] = {
    "FinishRecording", [&]()
    {
      stopRecording();
    }
  };
  
  ui.playbackItems[0] = {
    "Exit", [&]() {
      audio.endURL();
      audio.stopPlayback();
      playingPaused = true;
    }
  };

  ui.playbackItems[1] = {
    "StartPausePlayback", [&]() {
      if (!audio.isPlaying()) audio.beginURL(ui.togetherMenuItems[ui.getCurrentIndex()].length);
      playingPaused = !playingPaused;
      audio.setPlaybackPaused(playingPaused);
    }
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

  initializeTime();

  // xTaskCreatePinnedToCore(
  //     audioTask,
  //     "AudioTask",
  //     4096,
  //     NULL,
  //     2,
  //     &AudioTaskHandle,
  //     0);

  // xTaskCreatePinnedToCore(
  //     downloadTask,
  //     "DownloadTask",
  //     4096,
  //     NULL,
  //     2,
  //     &DownloadTaskHandle,
  //     0);

  ui.info();
  ui.infoText("C to continue...");
  ui.waitForInput();
  ui.mainMenu();

  xTaskCreatePinnedToCore(
      uiTask,
      "UiTask",
      4096,         // Stack size
      NULL,
      1,            // Priority (lower than AudioTask so audio never gets starved)
      &UiTaskHandle,
      1             // Core 1 (UI & Display)
  );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
  // ui.update();

  // if (!playingPaused && !audio.isPlaying()) {
  //   Serial.println("Playback completed/stopped, resetting UI.");
  //   // isPlaying = false;
  //   playingPaused = true;
  //   audio.endURL();
  //   ui.togetherMenuItems[ui.getCurrentIndex()].onSelect();
  // }
  // if (toggleTone) {
  //   toggleTone = false;
  //   toneOn = !toneOn;
  // }
  
  // if(toneOn){
  //     audio.copySpeaker();
  // }

  // if (recording && !recordingPaused) {
  //     // Serial.println(audio.uploadMic());
  //     audio.uploadMic();
     
  //   }

  // if (isPlaying && !playingPaused) {
  //     if(audio.copyURLStream(4) == 0 && !audio.URL_Available()){
  //       Serial.println("Playback finished");
  //       isPlaying = false;
  //       ui.togetherMenuItems[ui.getCurrentIndex()].onSelect();
  //     }
  //   }
}