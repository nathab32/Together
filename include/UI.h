#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include "Models.h"
#include "Audio.h"
#include <functional>
#include <U8g2lib.h>
#include <RotaryEncoder.h>
#include <OneButton.h>
#include <vector>

#define ENCODER_A 21
#define ENCODER_B 22

#define L_PIN 0
#define C_PIN 16
#define R_PIN 4

#define I2C_SCL 14
#define I2C_SDA 27

enum Screen
{
    INFO,
    MAIN_MENU,
    TOGETHER,
    TOGETHER_MENU,
    RECORDING,
    PLAYBACK,
    VOLUME,
    TUNER,
    LIGHTS,
    CONFIGURE,
    EMPTY
};

typedef std::function<void()> MenuCallback;
struct MenuItem {
    String name;
    MenuCallback onSelect;
};

struct TogetherMenuItem {
    String username;
    MenuCallback onSelect;
    String time;
    unsigned long length;
};

const unsigned long MAX_RECORDING_LENGTH = 60000;
class UI {

private:
    RotaryEncoder encoder;

    OneButton L, C, R;

    bool lPressed, cPressed, rPressed = false;

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

    Audio *_audio;

    Screen currentScreen = MAIN_MENU;
    Screen oldScreen = EMPTY;

    String date;
    String prompt;

    void drawHollowRBox(int x, int y, int w, int h, int r);
    
    void drawGeneralMenu(const std::vector<MenuItem>& list, int selectedIndex);
    void drawTogetherMenuItem(bool selected, int yPosition, const char* username, const char* time, int length);
    void drawTogetherMenu(int selectedIndex);
    int currentIndex = 0;

    void drawTimerBar(long totalSeconds, int maxSeconds, bool isPaused);

    void updateRecording();
    bool timerPaused = true;
    unsigned long startTime;
    unsigned long lastTimer = 0;
    unsigned long currentTimer = 0;

    void updatePlayback();
    unsigned long playbackLength;

    void handleInfoInput();
    void handleMainMenuInput();
    void handleTogetherInput();
    void handleTogetherMenuInput();
    void handleRecordingInput();
    void handlePlaybackInput();
    void handleVolumeInput();
    void handleTunerInput();
    void handleLightsInput();
    void handleConfigureInput();

public:
    UI();
    bool begin();
    void info();
    void infoText(const char *text);
    void waitForInput();
    bool centerText(String text, const uint8_t *font = u8g2_font_ciircle13_tr, Screen newScreen = EMPTY, uint8_t lineSpacing = 2, unsigned long time = 0);
    bool mainMenu();

    void together(String date, String prompt);
    void togetherMenu();

    void recording();
    void playback(const char* user, unsigned long length);

    void configure();
    void update();

    int getCurrentIndex();

    std::vector<MenuItem> mainMenuItems;
    std::vector<MenuItem> togetherItems;
    std::vector<TogetherMenuItem> togetherMenuItems;
    MenuItem recordingsItems[2];
    MenuItem playbackItems[2];
};

#endif //UI_H