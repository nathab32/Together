#include "Audio.h"

Audio::Audio()
    : micVolume(nullptr), micCopier(nullptr), 
    speakerVolume(nullptr), speakerCopier(nullptr), 
    decoder(nullptr),
    sineGenerator(nullptr), sineStream(nullptr)
{
    info = AudioInfo(SAMPLE_RATE, 1, BIT_DEPTH);
}

Audio::~Audio(){
    if (micCopier) { delete micCopier; micCopier = nullptr; }
    if (micVolume) { delete micVolume; micVolume = nullptr; }

    if (speakerCopier) { delete speakerCopier; speakerCopier = nullptr; }
    if (speakerVolume) { delete speakerVolume; speakerVolume = nullptr; }

    if (decoder) { delete decoder; decoder = nullptr; }

    if (sineGenerator) { delete sineGenerator; sineGenerator = nullptr; }
    if (sineStream) { delete sineStream; sineStream = nullptr; }
}

void Audio::beginLogger(){
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
}

bool Audio::beginMic(){
    // Ensure logger is started after Serial.begin() in setup()
    
    auto config_mic = mic.defaultConfig(RX_MODE);
    config_mic.copyFrom(info);
    // config_mic.i2s_format = I2S_STD_FORMAT;
    // config_mic.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;  // For mono I2S mic input, select the actual channel side
    config_mic.signal_type = PDM;
    // config_mic.bits_per_sample = 32;
    config_mic.port_no = 0;
    config_mic.use_apll = false;
    config_mic.pin_ws = MIC_WS;
    config_mic.pin_bck = -1;
    config_mic.pin_data = MIC_DATA;

    if(!mic.begin(config_mic)) return false;

    if(!micVolume) {
        micVolume = new VolumeStream(mic);
        auto vcfg = micVolume->defaultConfig();
        vcfg.copyFrom(info);
        vcfg.allow_boost = true;
        micVolume->begin(vcfg);

        micVolume->setVolume(200);
    }

    return true;
}

bool Audio::beginAmp(){
    auto config_amp = amp.defaultConfig(TX_MODE);
    config_amp.copyFrom(info);
    config_amp.i2s_format = I2S_STD_FORMAT;
    config_amp.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;  // For mono, use left channel
    config_amp.buffer_size = 1024;
    config_amp.buffer_count = 8;
    config_amp.port_no = 1;
    config_amp.pin_ws = MAX_LRC;
    config_amp.pin_bck = MAX_BCLK;
    config_amp.pin_data = MAX_DIN;
    if (!amp.begin(config_amp)) return false;

    pinMode(MAX_MODE, OUTPUT);

    if (!speakerVolume) speakerVolume = new VolumeStream(amp);
    if (!speakerCopier) speakerCopier = new StreamCopy(*speakerVolume, *sineStream);
    

    auto vcfg = speakerVolume->defaultConfig();
    vcfg.copyFrom(info);

    return speakerVolume->begin(vcfg);
}

bool Audio::beginUpload(const char *host, int port, const char *path){
    if (!uploadClient.connect(host, port)){
        Serial.println("Couldn't connect to server.");
        return false;
    }

    // Send HTTP POST headers
    uploadClient.println("POST " + String(path) + " HTTP/1.1");
    uploadClient.println("Host: " + String(host));
    uploadClient.println("Content-Type: application/octet-stream");
    uploadClient.println("Transfer-Encoding: chunked");
    uploadClient.println("Connection: keep-alive");
    uploadClient.println(); 
    return true;
}

size_t Audio::uploadMic(){
    uint8_t buf[BUFFER_SIZE];
    size_t len = micVolume->readBytes(buf, BUFFER_SIZE);
    if (len > 0) {
        uploadClient.print(len, HEX);
        uploadClient.print("\r\n");

        size_t written = uploadClient.write(buf, len);
        uploadClient.print("\r\n");

        uploadClient.flush();
        return written;
    }
    return false;
}

void Audio::endUpload(){
    uploadClient.print("0\r\n\r\n");
    uploadClient.flush();
    uploadClient.stop();
}

//private
void Audio::setupDecoder(){
    if (decoder) delete decoder;
    decoder = new EncodedAudioStream(speakerVolume, new WAVDecoder());
    decoder->begin();
}


bool Audio::beginURL_Stream(const char* audio_url){
    if(!http.begin(audio_url, "audio/wav")) return false;

    if (bufferedStream) delete bufferedStream;
    bufferedStream = new BufferedStream(http, 16384);

    setupDecoder();
    if (urlCopier) delete urlCopier;
    urlCopier = new StreamCopy(*decoder, *bufferedStream, 1024);
    return true;
}

int Audio::copyURLStream(int pages){
    return urlCopier->copyN(pages);
}

bool Audio::URL_Available(){
    return http.available();
}

bool Audio::beginSineGenerator(){
    if(!sineGenerator) sineGenerator = new SineGenerator<int16_t>();
    if(!sineGenerator->begin(info, N_A4)) return false;

    if(!sineStream) sineStream = new GeneratedSoundStream<int16_t>(*sineGenerator);
    return sineStream->begin(info);
}
