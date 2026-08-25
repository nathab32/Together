#include "UI.h"

UI::UI()
    : encoder(ENCODER_A, ENCODER_B, RotaryEncoder::LatchMode::FOUR3),
    L(L_PIN), C(C_PIN), R(R_PIN),
    u8g2(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA)
{
    L.attachClick([](void *ctx){ static_cast<UI *>(ctx)->lPressed = true; }, this);
    C.attachClick([](void *ctx){ static_cast<UI *>(ctx)->cPressed = true; }, this);
    R.attachClick([](void *ctx){ static_cast<UI *>(ctx)->rPressed = true; }, this);
}

void UI::drawHollowRBox(int x, int y, int w, int h, int r){
    u8g2.setDrawColor(1);
    u8g2.drawRBox(x, y, w, h, r);
    u8g2.setDrawColor(0); 
    u8g2.drawRBox(x + 1, y + 1, w - 2, h - 2, r - 1);
}

//can write helper function to simplify
void UI::drawGeneralMenu(const std::vector<MenuItem>& list, int selectedIndex){
    if(selectedIndex > list.size() - 1 || selectedIndex < 0) {
        Serial.println("drawGeneralMenu: Index not in list");
        return;
    }
    
    int yPositions[] = {0, 22, 42};

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ciircle13_tr);

    if(list.size() < 3){
        for (int i = 0; i < list.size(); ++i){
            if(i == selectedIndex){
                u8g2.setDrawColor(1);
                u8g2.drawRBox(0, yPositions[i], 128, 20, 4);
                u8g2.setDrawColor(0);
                u8g2.drawUTF8(4, yPositions[i] + 14, list[i].name.c_str());
            } else {
                drawHollowRBox(0, yPositions[i], 128, 20, 4);
                u8g2.setDrawColor(1);
                u8g2.drawUTF8(4, yPositions[i] + 14, list[i].name.c_str());
            }
        }
    } else {
        int filledIndex = selectedIndex % 3;
        int beginningIndex = max(selectedIndex - filledIndex, 0);
        // Serial.println(filledIndex);
        // Serial.println(beginningIndex);
        for (int i = 0; i < min(3, int(list.size() - beginningIndex)); ++i){
            if(i == filledIndex){
                u8g2.setDrawColor(1);
                u8g2.drawRBox(0, yPositions[i], 128, 20, 4);
                u8g2.setDrawColor(0);
                u8g2.drawUTF8(4, yPositions[i] + 14, list[i + beginningIndex].name.c_str());
            } else {
                drawHollowRBox(0, yPositions[i], 128, 20, 4);
                u8g2.setDrawColor(1);
                u8g2.drawUTF8(4, yPositions[i] + 14, list[i + beginningIndex].name.c_str());
            }
        }
    }
    // Serial.println("FLAG");
    u8g2.sendBuffer();
}

void UI::drawTogetherMenuItem(bool selected, int yPosition, const char* username, const char* time, int length) {
    if (selected) {
        u8g2.setDrawColor(1);
        u8g2.drawRBox(0, yPosition, 128, 20, 4);
        u8g2.setDrawColor(0);
    } else {
        drawHollowRBox(0, yPosition, 128, 20, 4);
        u8g2.setDrawColor(1);
    }
    // u8g2.setFont(u8g2_font_ciircle13_tr);
    u8g2.setFont(u8g2_font_6x13_tf);
    u8g2.drawUTF8(4, yPosition + 14, username);
    
    u8g2.drawUTF8(70, yPosition + (20 + u8g2.getAscent()) / 2, time);
    u8g2.drawUTF8(108, yPosition + (20 + u8g2.getAscent()) / 2, (length != 0) ? (String(length) + "s").c_str() : (""));
}

void UI::drawTogetherMenu(int selectedIndex) {
    u8g2.clearBuffer();
    if (selectedIndex > togetherMenuItems.size() - 1 || selectedIndex < 0) {
        Serial.println("drawTogethermenu: Index not in list");
        return;
    }
    
    int yPositions[] = {0, 22, 42};

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ciircle13_tr);

    if(togetherMenuItems.size() < 3){
        for (int i = 0; i < togetherMenuItems.size(); ++i){
            if(i == selectedIndex){
                drawTogetherMenuItem(true, yPositions[i], togetherMenuItems[i].username.c_str(), togetherMenuItems[i].time.c_str(), round(togetherMenuItems[i].length / 1000.0));
            } else {
                drawTogetherMenuItem(false, yPositions[i], togetherMenuItems[i].username.c_str(), togetherMenuItems[i].time.c_str(), round(togetherMenuItems[i].length / 1000.0));
            }
        }
    } else {
        int filledIndex = selectedIndex % 3;
        int beginningIndex = max(selectedIndex - filledIndex, 0);
        // Serial.println(filledIndex);
        // Serial.println(beginningIndex);
        for (int i = 0; i < min(3, int(togetherMenuItems.size() - beginningIndex)); ++i){
            if(i == filledIndex){
                drawTogetherMenuItem(true, yPositions[i], togetherMenuItems[i + beginningIndex].username.c_str(), togetherMenuItems[i + beginningIndex].time.c_str(), round(togetherMenuItems[i + beginningIndex].length / 1000.0));
            } else {
                drawTogetherMenuItem(false, yPositions[i], togetherMenuItems[i + beginningIndex].username.c_str(), togetherMenuItems[i + beginningIndex].time.c_str(), round(togetherMenuItems[i + beginningIndex].length / 1000.0));
            }
        }
    }
    // Serial.println("FLAG");
    u8g2.sendBuffer();
}

void formatTimer(long totalSeconds, char* buffer, size_t size, int maxSeconds) {
    if (totalSeconds > maxSeconds) totalSeconds = maxSeconds;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    snprintf(buffer, size, "%d:%02d/%d:%02d", minutes, seconds, maxSeconds / 60, maxSeconds % 60);
}

void UI::drawTimerBar(long totalSeconds, int maxSeconds, bool isPaused) {
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    int width = u8g2.getUTF8Width("0:00/1:00");
    int iconX = (u8g2.getWidth() - (width + 14)) / 2;

    u8g2.setDrawColor(0);
    u8g2.drawBox(iconX - 2, u8g2.getHeight() - 14, width + 16, 14);

    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_twelvedings_t_all);
    
    if (isPaused) {
        u8g2.drawGlyph(iconX, 64, 68); // Play icon
    } else {
        u8g2.drawGlyph(iconX, 64, 69); // Pause icon
    }

    char timeBuffer[16];
    formatTimer(totalSeconds, timeBuffer, sizeof(timeBuffer), maxSeconds);

    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.drawUTF8(iconX + 14, 63, timeBuffer);
}

void UI::updateRecording() {
    // Serial.println("UpdateRecording called");

    if (!timerPaused) {
        currentTimer = millis() - startTime;
    }
    
    long totalTime = (lastTimer + currentTimer);
    if (totalTime > MAX_RECORDING_LENGTH) {
        totalTime = MAX_RECORDING_LENGTH;
        timerPaused = true;
        currentTimer = MAX_RECORDING_LENGTH;
        lastTimer = 0;
        recordingsItems[0].onSelect();
    }
    drawTimerBar(totalTime / 1000, MAX_RECORDING_LENGTH / 1000, timerPaused);

    u8g2.sendBuffer();
}

void UI::updatePlayback() {
    if (!timerPaused) {
        currentTimer = millis() - startTime;
    }
    
    unsigned long totalTime = (lastTimer + currentTimer);
    
    if (totalTime > playbackLength + 1000) {
        totalTime = playbackLength;
        timerPaused = true;
        currentTimer = playbackLength;
        lastTimer = 0;
        playbackItems[0].onSelect();
    }
    drawTimerBar(totalTime / 1000, int(round(playbackLength / 1000.0)), timerPaused);

    u8g2.sendBuffer();
}

void UI::handleInfoInput()
{
    if(lPressed)
    {
        lPressed = false;
        mainMenu();
    }

    if(cPressed)
    {
        cPressed = false;
        mainMenu();
    }

    if(rPressed)
    {
        rPressed = false;
        mainMenu();
    }
}

void UI::handleMainMenuInput(){
    int newIndex = encoder.getPosition();

    if(currentIndex != newIndex)
    {
        if(newIndex >= (int)mainMenuItems.size())
        {
            currentIndex = mainMenuItems.size() - 1;
            encoder.setPosition(mainMenuItems.size() - 1);
            // delay(15);
            return;
        }
        else if (newIndex < 0)
        {
            currentIndex = 0;
            encoder.setPosition(0);
            // delay(15);
            return;
        }
        else
        {
            currentIndex = newIndex;
            drawGeneralMenu(mainMenuItems, currentIndex);
        }
    }

    if(lPressed)
    {
        lPressed = false;
        currentIndex = 0;
        info();
        infoText("Press to continue...");
    }

    if(cPressed)
    {
        cPressed = false;
        // int temp = currentIndex;
        // currentIndex = 0;
        mainMenuItems[currentIndex].onSelect();
    }

    if(rPressed)
    {
        rPressed = false;
    }

}

void UI::handleTogetherInput(){

    if(lPressed)
    {
        lPressed = false;
        mainMenu();
    }

    if(cPressed)
    {
        cPressed = false;
        togetherItems[0].onSelect();
    }

    if(rPressed)
    {
        rPressed = false;
        togetherItems[0].onSelect();
    }
}

void UI::handleTogetherMenuInput() {
    int newIndex = encoder.getPosition();

    if(currentIndex != newIndex)
    {
        if(newIndex >= (int)togetherMenuItems.size())
        {
            currentIndex = togetherMenuItems.size() - 1;
            encoder.setPosition(togetherMenuItems.size() - 1);
            // delay(15);
            return;
        }
        else if (newIndex < 0)
        {
            currentIndex = 0;
            encoder.setPosition(0);
            // delay(15);
            return;
        }
        else
        {
            currentIndex = newIndex;
            drawTogetherMenu(currentIndex);
        }
    }

    if(lPressed)
    {
        lPressed = false;
        mainMenu();
    }

    if(cPressed)
    {
        cPressed = false;
        togetherMenuItems[currentIndex].onSelect();
    }

    if(rPressed)
    {
        rPressed = false;
        togetherMenuItems[currentIndex].onSelect();
    }
}

void UI::handleRecordingInput() {
    if(lPressed)
    {
        lPressed = false;
        togetherItems[0].onSelect();
        // Serial.println("Back to togetherMenu");
    }

    if(cPressed)
    {
        cPressed = false;
        if (currentTimer == MAX_RECORDING_LENGTH) {
            // Serial.println("Max length reached");
            return;
        }
        timerPaused = !timerPaused;
        if (!timerPaused) { //if switch from paused to playing
            startTime = millis();
            lastTimer += currentTimer;
            currentTimer = 0;
        }
        recordingsItems[0].onSelect();
    }

    if(rPressed)
    {
        rPressed = false;
        timerPaused = true;
        recordingsItems[1].onSelect();
    }
}

void UI::handlePlaybackInput() {
    if(lPressed)
    {
        lPressed = false;
        playbackItems[0].onSelect();
        togetherItems[0].onSelect();
    }

    if(cPressed)
    {
        cPressed = false;
        timerPaused = !timerPaused;
        if (!timerPaused) { //if switch from paused to playing
            startTime = millis();
            lastTimer += currentTimer;
            currentTimer = 0;
        }
        playbackItems[1].onSelect();
    }

    if(rPressed)
    {
        rPressed = false;
        timerPaused = true;
        togetherMenuItems[currentIndex].onSelect();
    }
}

void UI::handleVolumeInput(){

    if(lPressed)
    {
        lPressed = false;
    }

    if(cPressed)
    {
        cPressed = false;
    }

    if(rPressed)
    {
        rPressed = false;
    }
}

void UI::handleTunerInput(){

    if(lPressed)
    {
        lPressed = false;
    }

    if(cPressed)
    {
        cPressed = false;
    }

    if(rPressed)
    {
        rPressed = false;
    }
}

void UI::handleLightsInput(){

    if(lPressed)
    {
        lPressed = false;
    }

    if(cPressed)
    {
        cPressed = false;
    }

    if(rPressed)
    {
        rPressed = false;
    }
}

void UI::handleConfigureInput(){
    mainMenuItems[currentIndex].onSelect();
    if(lPressed)
    {
        lPressed = false;
    }

    if(cPressed)
    {
        cPressed = false;
    }

    if(rPressed)
    {
        rPressed = false;
    }

}

///////////PUBLIC FUNCTIONS////////////////////////
bool UI::begin(){
    u8g2.begin();
    info();
    return true;
}

void UI::info()
{
    currentScreen = INFO;
    oldScreen = currentScreen;
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_courR18_tf);

    const char* title = "Together";
    u8g2.drawStr((128-u8g2.getUTF8Width(title))/2, 20, title);

    u8g2.setFont(u8g2_font_ciircle13_tr);
    const char *text = "By Nathan";
    int textWidth = u8g2.getUTF8Width(text);
    int textX = (128 - textWidth - 16) / 2;
    int spaceWidth = u8g2.getUTF8Width(" ");

    u8g2.drawStr(textX, 42, text);
    u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
    u8g2.drawGlyph((textX + textWidth + spaceWidth), 42, 183);

    u8g2.sendBuffer();
}

void UI::infoText(const char *text)
{
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 64, 128, 13);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_6x12_tf);

    u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(text)) / 2, 61, text);
    u8g2.sendBuffer();
}

void UI::waitForInput()
{
    while(true)
        {
            C.tick();
            if(cPressed)
            {
                cPressed = false;
                return;
            }
        }
}

//set newScreen to EMPTY to continue without changing screen, time = 0 to wait for user, >0 for delay
bool UI::centerText(String text, const uint8_t *font, Screen newScreen, uint8_t lineSpacing, unsigned long time){
    u8g2.clearBuffer();
    u8g2.setFont(font);
    u8g2.setDrawColor(1);

    if(u8g2.getUTF8Width(text.c_str()) > u8g2.getDisplayWidth()){
        int spaceIndex = text.length() / 2;
        // Search for the closest space to the middle
        int bestSplit = -1;
        for (int i = 0; i < text.length(); i++) {
            if (text[i] == ' ') {
                if (bestSplit == -1 || abs(i - spaceIndex) < abs(bestSplit - spaceIndex)) {
                    bestSplit = i;
                }
            }
        }

        String line1 = text.substring(0, bestSplit);
        String line2 = text.substring(bestSplit + 1);

        if (u8g2.getUTF8Width(line1.c_str()) > u8g2.getDisplayWidth() || u8g2.getUTF8Width(line2.c_str()) > u8g2.getDisplayWidth()) {
        //3 line logic
            int split1 = text.length() / 3;
            bestSplit = -1;
            for (int i = 0; i < text.length() / 2; ++i) {
                if (text[i] == ' ') {
                    if (bestSplit == -1 || abs(i - split1) < abs(bestSplit - split1)) {
                        bestSplit = i;
                    }
                }
            }
            split1 = bestSplit;

            int split2 = text.length() / 3 * 2;
            bestSplit = -1;
            for (int i = text.length() / 2; i < text.length(); ++i) {
                if (text[i] == ' ') {
                    if (bestSplit == -1 || abs(i - split2) < abs(bestSplit - split2)) {
                        bestSplit = i;
                    }
                }
            }
            split2 = bestSplit;

            line1 = text.substring(0, split1);
            line2 = text.substring(split1 + 1, split2);
            String line3 = text.substring(split2 + 1);
            int line2Y = (u8g2.getDisplayHeight() + u8g2.getAscent()) / 2;

            u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line2.c_str())) / 2, line2Y, line2.c_str());
            u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line3.c_str())) / 2, line2Y + lineSpacing + u8g2.getMaxCharHeight(), line3.c_str());
            u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line1.c_str())) / 2, line2Y - lineSpacing - u8g2.getMaxCharHeight(), line1.c_str());
        } else { //2 line logic
            if (lineSpacing == 1) {
                u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line1.c_str())) / 2, u8g2.getDisplayHeight()/2 - 1, line1.c_str());
                u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line2.c_str())) / 2, u8g2.getDisplayHeight()/2 + u8g2.getMaxCharHeight(), line2.c_str());
            } else {
                int offset = lineSpacing / 2;
                u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line1.c_str())) / 2, u8g2.getDisplayHeight()/2 - lineSpacing, line1.c_str());
                u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(line2.c_str())) / 2, u8g2.getDisplayHeight()/2 + lineSpacing + u8g2.getMaxCharHeight(), line2.c_str());
            }
            
        }
    } else {
        u8g2.drawUTF8((u8g2.getDisplayWidth() - u8g2.getUTF8Width(text.c_str())) / 2, 
        (u8g2.getDisplayHeight() + u8g2.getMaxCharHeight())/2, text.c_str());
    }

    if(!time)
    {
        const char *text1 = "C to continue...";
        u8g2.setFont(u8g2_font_6x13_tf);
        u8g2.drawUTF8((u8g2.getDisplayWidth()-u8g2.getUTF8Width(text1))/2, 61, text1);
        u8g2.sendBuffer();
        while(true)
        {
            C.tick();
            if(cPressed)
            {
                cPressed = false;
                if(newScreen != EMPTY) currentScreen = newScreen;
                return true;
            }
        }
    }

    u8g2.sendBuffer();
    if(newScreen != EMPTY) currentScreen = newScreen;
    if(time) delay(time);
    return true;
}

bool UI::mainMenu(){
    currentScreen = MAIN_MENU;
    oldScreen = currentScreen;
    currentIndex = 0;
    encoder.setPosition(0);
    drawGeneralMenu(mainMenuItems, currentIndex);
    return true;
}

void UI::together(String date, String prompt) {
    currentScreen = TOGETHER;
    oldScreen = currentScreen;

    this->date = date;
    this->prompt = prompt;
    
    centerText(prompt.c_str(), u8g2_font_6x12_tr, EMPTY, 0, 1);

    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_ciircle13_tr);
    u8g2.drawUTF8((u8g2.getDisplayWidth()-u8g2.getUTF8Width(date.c_str()))/2, u8g2.getAscent(), date.c_str());
    
    u8g2.sendBuffer();
    infoText("C to continue...");
    

    if (prompt.equals("No prompts available")) {
        waitForInput();
        mainMenu();
    }
}

void UI::togetherMenu() {
    currentScreen = TOGETHER_MENU;
    oldScreen = currentScreen;
    currentIndex = 0;
    encoder.setPosition(0);
    
    drawTogetherMenu(currentIndex);
}

void UI::recording() {
    currentScreen = RECORDING;
    oldScreen = currentScreen;
    lastTimer = 0;
    currentTimer = 0;
    timerPaused = true;

    centerText(prompt.c_str(), u8g2_font_6x12_t_symbols, EMPTY, 0, 1);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_ciircle13_tr);
    u8g2.drawUTF8((u8g2.getDisplayWidth()-u8g2.getUTF8Width(date.c_str()))/2, u8g2.getAscent(), date.c_str());

    u8g2.setFont(u8g2_font_6x12_t_symbols);
    int width = u8g2.getUTF8Width("0:00/1:00");
    // Serial.println(width);
    u8g2.drawUTF8((u8g2.getWidth() - (width)) / 2 + 7, 63, "0:00/1:00");

    u8g2.setFont(u8g2_font_twelvedings_t_all);
    u8g2.drawGlyph(0, 64, 117); //back arrow
    u8g2.drawGlyph((u8g2.getWidth() - (width + 14)) / 2, 64, 68); //play arrow
    u8g2.drawGlyph(u8g2.getWidth() - 14, 64, 115); //save
    u8g2.sendBuffer();
}

void UI::playback(const char* user, unsigned long length) {
    currentScreen = PLAYBACK;
    timerPaused = true;
    lastTimer = 0;
    currentTimer = 0;
    playbackLength = (togetherMenuItems[currentIndex].length);
    // Serial.println(playbackLength);
    centerText(prompt.c_str(), u8g2_font_6x12_tr, EMPTY, 0, 1);

    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_ciircle13_tr);
    u8g2.drawUTF8((u8g2.getDisplayWidth()-u8g2.getUTF8Width(user))/2, u8g2.getAscent(), user);

    u8g2.setFont(u8g2_font_6x12_t_symbols);
    int width = u8g2.getUTF8Width("0:00/1:00");
    unsigned int totalSeconds = length / 1000;
    unsigned int minutes = totalSeconds / 60;
    unsigned int seconds = totalSeconds % 60;

    char timeBuffer[10];
    sprintf(timeBuffer, "0:00/%d:%02d", minutes, seconds);
    u8g2.drawUTF8((u8g2.getDisplayWidth() - width) / 2 + 7, 64, timeBuffer);

    u8g2.setFont(u8g2_font_twelvedings_t_all);
    u8g2.drawGlyph(0, 64, 117); // back arrow
    u8g2.drawGlyph((u8g2.getDisplayWidth() - width) / 2 - 7, 64, 68); //play
    u8g2.drawGlyph(u8g2.getWidth() - 14, 64, 82); //back to beginning

    u8g2.sendBuffer();
}

void UI::configure()
{
    currentScreen = CONFIGURE;
    oldScreen = currentScreen;
    centerText("Config portal launched", u8g2_font_ciircle13_tr, EMPTY, 1, 1);
    infoText("L to quit config");
}

void UI::update(){
    encoder.tick();
    L.tick();
    C.tick();
    R.tick();

    switch (currentScreen)
    {
    case INFO:
        handleInfoInput();
        break;
    case MAIN_MENU:
        handleMainMenuInput();
        break;
    case TOGETHER:
        handleTogetherInput();
        break;
    case TOGETHER_MENU:
        handleTogetherMenuInput();
        break;
    case RECORDING:
        handleRecordingInput();
        if (currentScreen == RECORDING) updateRecording();
        break;
    case PLAYBACK:
        handlePlaybackInput();
        if (currentScreen == PLAYBACK && currentTimer < MAX_RECORDING_LENGTH) updatePlayback();
        break;
    case VOLUME:
        handleVolumeInput();
        break;
    case TUNER:
        handleTunerInput();
        break;
    case LIGHTS:
        handleLightsInput();
        break;
    case CONFIGURE:
        handleConfigureInput();
        break;
    default:
        break;
    }
}

int UI::getCurrentIndex() {
    return currentIndex;
}