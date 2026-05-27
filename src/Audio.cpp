#include "Audio.h"

Audio::Audio()
    : volume(nullptr), copier(nullptr)
{
    info = AudioInfo(SAMPLE_RATE, 1, BIT_DEPTH);
}

Audio::~Audio(){
    if (copier) { delete copier; copier = nullptr; }
    if (volume) { delete volume; volume = nullptr; }
}

bool Audio::beginMic(){
    // Ensure logger is started after Serial.begin() in setup()
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
    auto config_mic = mic.defaultConfig(RX_MODE);
    config_mic.copyFrom(info);
    config_mic.i2s_format = I2S_STD_FORMAT;
    config_mic.channel_format = I2S_CHANNEL_FMT_ALL_LEFT;  // For mono, use left channel
    config_mic.port_no = 0;
    config_mic.pin_ws = INMP_WS;
    config_mic.pin_bck = INMP_BCLK;
    config_mic.pin_data = INMP_SD;

    return mic.begin(config_mic);
}

bool Audio::beginAmp(){
    auto config_amp = amp.defaultConfig(TX_MODE);
    config_amp.copyFrom(info);
    config_amp.i2s_format = I2S_STD_FORMAT;
    config_amp.channel_format = I2S_CHANNEL_FMT_ALL_LEFT;  // For mono, use left channel
    config_amp.port_no = 1;
    config_amp.pin_ws = MAX_LRC;
    config_amp.pin_bck = MAX_BCLK;
    config_amp.pin_data = MAX_DIN;
    if (!amp.begin(config_amp)) return false;

    // Create volume and copier at runtime (after amp and mic exist)
    if (!volume) volume = new VolumeStream(amp);
    if (!copier) copier = new StreamCopy(*volume, mic);

    auto vcfg = volume->defaultConfig();
    vcfg.copyFrom(info);

    return volume->begin(vcfg);
}