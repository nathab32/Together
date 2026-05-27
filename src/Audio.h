#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <AudioTools.h>

#define MAX_DIN 32 
#define MAX_LRC 33
#define MAX_BCLK 25

#define INMP_SD 23
#define INMP_WS 22
#define INMP_BCLK 21

#define SAMPLE_RATE 22050
#define BIT_DEPTH 16


class Audio
{
private:
    AudioInfo info;  // 1 = mono
    I2SStream mic;
    I2SStream amp;
    VolumeStream *volume;
    StreamCopy *copier;

public:
    Audio();
    ~Audio();
    bool beginMic();
    bool beginAmp();
    void setVolume(double vol) {if (volume)volume->setVolume(vol); }
    void copy() { if (copier) copier->copy(); }
};


#endif //AUDIO_H