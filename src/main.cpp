#include <AudioTools.h>
#include <AudioTools/Communication/AudioHttp.h>
#include "Credentials.h"
#include "Audio.h"

// PINS

Audio audio;

void setup() {
  Serial.begin(115200);
  Serial.println("Program Start");
  // initialize audio after Serial is ready
  if (!audio.beginMic()) Serial.println("beginMic failed");
  if (!audio.beginAmp()) Serial.println("beginAmp failed");
  audio.setVolume(0.5);
}

void loop() {
  audio.copy();
}
