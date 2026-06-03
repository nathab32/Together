#include "Audio.h"

Audio::Audio()
    : micVolume(nullptr), micCopier(nullptr), speakerVolume(nullptr), speakerCopier(nullptr), out_stream(nullptr)
{
    info = AudioInfo(SAMPLE_RATE, 1, BIT_DEPTH);
    
}

Audio::~Audio(){
    if (speakerCopier) { delete speakerCopier; speakerCopier = nullptr; }
    if (speakerVolume) { delete speakerVolume; speakerVolume = nullptr; }
    if (out_stream) { delete out_stream; out_stream = nullptr; }
}

void Audio::beginLogger(){
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
}

bool Audio::beginMic(){
    // Ensure logger is started after Serial.begin() in setup()
    
    auto config_mic = mic.defaultConfig(RX_MODE);
    config_mic.copyFrom(info);
    config_mic.i2s_format = I2S_STD_FORMAT;
    config_mic.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;  // For mono I2S mic input, select the actual channel side
    config_mic.bits_per_sample = 32;
    config_mic.port_no = 0;
    config_mic.use_apll = true;
    config_mic.pin_ws = INMP_WS;
    config_mic.pin_bck = INMP_BCLK;
    config_mic.pin_data = INMP_SD;

    if(!mic.begin(config_mic)) return false;

    if (!out_stream) {
        Serial.println("Audio::beginMic: out_stream is null; call beginEncoder() first");
        return false;
    }

    if (!micCopier) micCopier = new StreamCopy(*out_stream, mic, BUFFER_SIZE);

    return true;
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

    // Create volume and speakerCopier at runtime (after amp and mic exist)
    if (!speakerVolume) speakerVolume = new VolumeStream(amp);
    // if (!speakerCopier) speakerCopier = new StreamCopy(*speakerVolume, );
    

    auto vcfg = speakerVolume->defaultConfig();
    vcfg.copyFrom(info);

    return speakerVolume->begin(vcfg);
}

bool Audio::beginEncoder(MqttClient &client){
    if (!out_stream) out_stream = new EncodedAudioStream(&client, new WAVEncoder());
    return out_stream->begin(info);
}