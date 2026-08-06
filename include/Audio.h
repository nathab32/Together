#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <AudioTools.h>
#include "AudioTools/Communication/AudioHttp.h"
#include <WiFi.h>
#include <base64.h>

#define MAX_DIN 32 
#define MAX_LRC 33
#define MAX_BCLK 25
#define MAX_MODE 23

#define MIC_DATA 18
#define MIC_WS 19

#define SAMPLE_RATE 16000
#define BIT_DEPTH 16

#define BUFFER_SIZE 1024


class Audio
{
private:
    AudioInfo info;  // 1 = mono
    I2SStream mic;
    VolumeStream *micVolume;
    StreamCopy *micCopier;
    
    I2SStream amp;
    VolumeStream *speakerVolume;
    StreamCopy *speakerCopier;

    WiFiClient uploadClient;

    EncodedAudioStream *decoder;
    URLStream http;
    BufferedStream *bufferedStream;
    StreamCopy *urlCopier;

    SineGenerator<int16_t> *sineGenerator;
    GeneratedSoundStream<int16_t> *sineStream;

    void setupDecoder();

public:
    Audio();
    ~Audio();
    void beginLogger();

    bool beginMic();

    bool beginAmp();
    void ampOn() { digitalWrite(MAX_MODE, HIGH); }
    void ampOff() { digitalWrite(MAX_MODE, LOW); }

    bool beginUpload(const char *host, int port, const char *path, String user, String pass);
    size_t uploadMic();
    void endUpload();

    void addCredentialsToURL(String user, String pass);
    bool beginURL_Stream(const char* audio_url, String user, String pass);
    int copyURLStream(int pages);
    bool URL_Available();
    void endURL() { http.end(); }

    bool beginSineGenerator();

    // void setMicVolume(double vol) {if (micVolume) micVolume->setVolume(vol); }
    size_t copyMic(int N) { return micCopier ? micCopier->copyN(N) : 0; }
    bool copyMic() { return micCopier ? (micCopier->copy() > 0) : false; }

    void setSpeakerVolume(double vol) {if (speakerVolume) speakerVolume->setVolume(vol); }
    bool copySpeaker() { return speakerCopier ? (speakerCopier->copy() > 0) : false; }

    void setSineFrequency(float f) { sineGenerator->setFrequency(f); }
};


#endif //AUDIO_H