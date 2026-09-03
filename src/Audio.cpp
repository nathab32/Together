#include "Audio.h"

Audio::Audio()
    : info(SAMPLE_RATE, 1, BIT_DEPTH), micVolume(nullptr), micCopier(nullptr), 
    speakerVolume(nullptr), speakerCopier(nullptr), 
    encoder(nullptr),
    adpcmEncoder(nullptr), wavEncoder(nullptr), uploadCopier(nullptr),
    decoder(nullptr), adpcmDecoder(nullptr), wavDecoder(nullptr), urlCopier(nullptr),
    _playbackTaskHandle(nullptr), _isPlayActive(false), _isPlaybackPaused(true),
    _targetDuration(0),
    sineGenerator(nullptr), sineStream(nullptr)
{
}

Audio::~Audio(){
    if (micCopier) { delete micCopier; micCopier = nullptr; }
    if (micVolume) { delete micVolume; micVolume = nullptr; }

    if (speakerCopier) { delete speakerCopier; speakerCopier = nullptr; }
    if (speakerVolume) { delete speakerVolume; speakerVolume = nullptr; }

    if (encoder) { delete encoder; encoder = nullptr; }

    if (decoder) { delete decoder; decoder = nullptr; }
    if (wavDecoder) { delete wavDecoder; wavDecoder = nullptr; }
    if (adpcmDecoder) { delete adpcmDecoder; adpcmDecoder = nullptr; }

    if (sineGenerator) { delete sineGenerator; sineGenerator = nullptr; }
    if (sineStream) { delete sineStream; sineStream = nullptr; }
    if (http) { delete http; http = nullptr; }
}

void Audio::beginLogger(){
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
}

bool Audio::beginMic(){
    // Ensure logger is started after Serial.begin() in setup()
    
    auto config_mic = mic.defaultConfig(RX_MODE);
    config_mic.copyFrom(info);
    config_mic.signal_type = PDM;
    config_mic.use_apll = true;
    config_mic.port_no = 0;
    config_mic.pin_ws = MIC_WS;
    config_mic.pin_bck = -1;
    config_mic.pin_data = MIC_DATA;
    config_mic.buffer_size = 1024;
    config_mic.buffer_count = 20;


    if(!mic.begin(config_mic)) return false;

    // if(!micVolume) {
    //     micVolume = new VolumeStream(mic);
    //     auto vcfg = micVolume->defaultConfig();
    //     vcfg.copyFrom(info);
    //     // vcfg.allow_boost = true;
    //     micVolume->begin(vcfg);

    //     micVolume->setVolume(1);
    // }
    Serial.println("Mic started");
    // delay(200);
    return true;
}

bool Audio::endMic() {
    mic.end();
    micVolume->end();
    delete micVolume;
    micVolume = nullptr;
    Serial.println("Mic ended");
    return true;
}

bool Audio::beginAmp(){
    pinMode(MAX_MODE, OUTPUT);
    ampOn();
    auto config_amp = amp.defaultConfig(TX_MODE);
    config_amp.copyFrom(info);
    config_amp.i2s_format = I2S_STD_FORMAT;
    config_amp.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;  // For mono, use left channel
    config_amp.buffer_size = 1024;
    config_amp.buffer_count = 12;
    config_amp.port_no = 1;
    config_amp.pin_ws = MAX_LRC;
    config_amp.pin_bck = MAX_BCLK;
    config_amp.pin_data = MAX_DIN;
    if (!amp.begin(config_amp)) return false;
    // amp.clearNotifyAudioChange();

    if (!speakerVolume) speakerVolume = new VolumeStream(amp);
    // speakerVolume->clearNotifyAudioChange();
    
    

    auto vcfg = speakerVolume->defaultConfig();
    vcfg.copyFrom(info);

    Serial.println("Amp started");
    // delay(200);
    return speakerVolume->begin(vcfg);
}

bool Audio::endAmp() {
    amp.end();
    amp.begin();
    ampOff();
    speakerVolume->end();
    delete speakerVolume;
    speakerVolume = nullptr;
    Serial.println("Amp ended");
    return true;
}

//url_str:include http://, port, and route
bool Audio::beginUpload(const char *url_str, String user, String pass) {
    // beginMic();
    Url url(url_str);

    //test with and without
    uploadClient.setNoDelay(true);
    httpRequest = new HttpRequest(uploadClient);
    httpRequest->header().put(TRANSFER_ENCODING, CHUNKED);

    String encoded = base64::encode(user + ":" + pass);
    httpRequest->addRequestHeader("Authorization", ("Basic " + encoded).c_str());
    if (!httpRequest->processBegin(POST, url, "application/octet-stream")) {
        delete httpRequest;
        httpRequest = nullptr;
        Serial.println("post failed");
        return false;
    }

    if (adpcmEncoder) {
        delete adpcmEncoder;
        adpcmEncoder = nullptr;
    }

    if (wavEncoder) {
        delete wavEncoder;
        wavEncoder = nullptr;
    }

    if (encoder) {
        delete encoder;
        encoder = nullptr;
    }

    adpcmEncoder = new ADPCMEncoder(id);
    wavEncoder = new WAVEncoder(*adpcmEncoder, AudioFormat::DVI_ADPCM);
    encoder = new EncodedAudioStream(httpRequest, wavEncoder);
    encoder->begin(AudioInfo(16000, 1, 4));

    if (uploadCopier) {
        delete uploadCopier;
        uploadCopier = nullptr;
    }
    uploadCopier = new StreamCopy(*encoder, mic);
    Serial.println("beginUpload succeeded");
    return true;
}

size_t Audio::uploadMic(){
    if (uploadCopier) return uploadCopier->copyN(1);
    return 0;
}

bool Audio::endUpload(){

    int code;
    if (httpRequest) {
        code = httpRequest->processEnd();
        delete httpRequest; 
        httpRequest = nullptr;
    }

    if (adpcmEncoder) {
        delete adpcmEncoder;
        adpcmEncoder = nullptr;
    }

    if (wavEncoder) {
        delete wavEncoder;
        wavEncoder = nullptr;
    }

    if (encoder) {
        delete encoder;
        encoder = nullptr;
    }

    if (uploadCopier) {
        delete uploadCopier; 
        uploadCopier = nullptr;
    }

    return true;
    // endMic();
}

//private
void Audio::setupDecoder(){
    if (!adpcmDecoder) {
        adpcmDecoder = new ADPCMDecoder(id);
        Serial.println("new adpcmDecoder created");
    }
    if (!wavDecoder) {
        wavDecoder = new WAVDecoder(*adpcmDecoder, AudioFormat::DVI_ADPCM);
        Serial.println("new wavDecoder created");
    }
    if (!decoder) {
        decoder = new EncodedAudioStream(speakerVolume, wavDecoder);
        Serial.println("new decoder created");
    }
    decoder->begin(info);
    Serial.println("decoder setup");
}


bool Audio::initializeURL(String audio_url, String user, String pass) {
    if (!http) http = new URLStream();
    String encoded = base64::encode(user + ":" + pass);
    http->addRequestHeader("Authorization", ("Basic " + encoded).c_str());
    _playUrl = audio_url;
    return true;
}

bool Audio::beginURL(unsigned long duration){
    // beginAmp();
    setupDecoder();
    if (!http) http = new URLStream();
    if(!http->begin(_playUrl.c_str(), "audio/wav")) return false;
    
    
    
    if (urlCopier) {
        delete urlCopier;
        urlCopier = nullptr;
    }
    urlCopier = new StreamCopy(*decoder, *http);
    
    startPlaybackTask(duration);
    return true;
}

int Audio::copyURLStream(int pages){
    return urlCopier->copyN(pages);
}

bool Audio::URL_Available(){
    return http && http->available();
}

void Audio::startPlaybackTask(unsigned long duration) {
    if (_isPlayActive) return;
    
    // Stop any existing stream
    stopPlayback();
    _targetDuration = duration;
    _isPlayActive = true;
    _isPlaybackPaused = false;

    // Create playback task pinned to Core 0 (or Core 1 depending on where UI runs)
    xTaskCreatePinnedToCore(
        [](void* param) {
            Audio* audioInstance = static_cast<Audio*>(param);
            audioInstance->runPlaybackLoop();
            vTaskDelete(NULL);
        },
        "AudioPlayTask",
        4096,
        this,
        3,
        &_playbackTaskHandle,
        0 // Core 0
    );
}

void Audio::runPlaybackLoop() {
    Serial.println("Starting playback task...");

    unsigned long startTime = millis();

    while (_isPlayActive) {
        if (_isPlaybackPaused) {
            startTime += 50;
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        if (_targetDuration > 0 && (millis() - startTime >= _targetDuration)) {
            Serial.println("Target duration reached");
            break;
        }

        if (urlCopier && urlCopier->copy() == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            if (urlCopier->copy() == 0 && (!http || !http->available())) {
                Serial.println("Playback connection lost");
            //     // break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    if (http) http->end();
    
    if (urlCopier) {
        delete urlCopier;
        urlCopier = nullptr;
    }

    if (adpcmDecoder) {
        delete adpcmDecoder;
        adpcmDecoder = nullptr;
        Serial.println("adpcmDecoder deleted");
    }

    if (wavDecoder) {
        delete wavDecoder;
        wavDecoder = nullptr;
        Serial.println("wavdecoder deleted");
    }
    
    if (decoder) {
        delete decoder;
        decoder = nullptr;
        Serial.println("decoder deleted");
    }
    _isPlayActive = false;
    _playbackTaskHandle = nullptr;
    Serial.println("Playback task finished.");
}

void Audio::stopPlayback() {
    if (!_playbackTaskHandle) return;    

    _isPlayActive = false;
    while (_playbackTaskHandle) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool Audio::beginSineGenerator(float frequency){
    if(!sineGenerator) sineGenerator = new SineGenerator<int16_t>();
    if(!sineGenerator->begin(info, frequency)) return false;

    if(!sineStream) sineStream = new GeneratedSoundStream<int16_t>(*sineGenerator);
    // if (!speakerCopier) speakerCopier = new StreamCopy(*speakerVolume, *sineStream);
    return sineStream->begin(info);
}

bool Audio::endSineGenerator() {
    sineGenerator->end();
    sineStream->end();
    return true;
}