#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <AudioTools.h>
#include "AudioTools/Communication/AudioHttp.h"
#include "AudioTools/AudioCodecs/CodecWAV.h" 
#include "AudioTools/AudioCodecs/CodecADPCM.h"
#include <WiFi.h>
#include <base64.h>

#define MAX_DIN 32 
#define MAX_LRC 33
#define MAX_BCLK 25
#define MAX_MODE 26

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

    AVCodecID id = AV_CODEC_ID_ADPCM_IMA_WAV;

    WiFiClient uploadClient;
    HttpRequest *httpRequest = nullptr;
    EncodedAudioStream *encoder;
    ADPCMEncoder *adpcmEncoder;
    WAVEncoder *wavEncoder;
    StreamCopy *uploadCopier = nullptr;

    EncodedAudioStream *decoder;
    ADPCMDecoder *adpcmDecoder;
    WAVDecoder *wavDecoder;
    // FormatConverterStream *converter;
    URLStream *http = nullptr;
    StreamCopy *urlCopier;

    TaskHandle_t _playbackTaskHandle = nullptr;
    volatile bool _isPlayActive = false;
    volatile bool _isPlaybackPaused = true;
    String _playUrl = "";
    void runPlaybackLoop();
    unsigned long _targetDuration;

    SineGenerator<int16_t> *sineGenerator;
    GeneratedSoundStream<int16_t> *sineStream;

    void setupDecoder();

public:
    Audio();
    ~Audio();
    void beginLogger();

    bool beginMic();
    bool endMic();

    bool beginAmp();
    bool endAmp();
    void ampOn() { digitalWrite(MAX_MODE, HIGH); }
    void ampOff() { digitalWrite(MAX_MODE, LOW); }

    bool beginUpload(const char *url, String user, String pass);
    size_t uploadMic();
    bool endUpload();

    void addCredentialsToURL(String user, String pass);
    bool initializeURL(String audio_url, String user, String pass);
    bool beginURL(unsigned long duration);
    int copyURLStream(int pages);
    bool URL_Available();
    void endURL() { if (http) http->end(); }

    void startPlaybackTask(unsigned long duration);
    void stopPlayback();
    bool isPlaying() { return _isPlayActive; }
    void setPlaybackPaused(bool paused) { _isPlaybackPaused = paused; }

    bool beginSineGenerator(float frequency);
    bool endSineGenerator();

    size_t copyMic(int N) { return micCopier ? micCopier->copyN(N) : 0; }
    bool copyMic() { return micCopier ? (micCopier->copy() > 0) : false; }

    void setSpeakerVolume(float vol) {if (speakerVolume) speakerVolume->setVolume(vol); }
    bool copySpeaker() { return speakerCopier ? (speakerCopier->copy() > 0) : false; }

    void setSineFrequency(float f) { sineGenerator->setFrequency(f); }
};


#endif //AUDIO_H