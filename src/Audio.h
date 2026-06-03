#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <AudioTools.h>
#include <ArduinoMqttClient.h>

#define MAX_DIN 32 
#define MAX_LRC 33
#define MAX_BCLK 25

#define INMP_SD 23
#define INMP_WS 22
#define INMP_BCLK 21

#define SAMPLE_RATE 8000
#define BIT_DEPTH 32

#define BUFFER_SIZE 1024
static constexpr int AUDIO_BUFFER_COUNT = 100;


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

    EncodedAudioStream *out_stream;

public:
    Audio();
    ~Audio();
    void beginLogger();
    bool beginMic();
    bool beginAmp();
    bool beginEncoder(MqttClient &client);

    // void setMicVolume(double vol) {if (micVolume) micVolume->setVolume(vol); }
    void copyMic(int N) { if (micCopier) micCopier->copyN(N); }

    void setSpeakerVolume(double vol) {if (speakerVolume) speakerVolume->setVolume(vol); }
    void copySpeaker() { if (speakerCopier) speakerCopier->copy(); }

};


#endif //AUDIO_H