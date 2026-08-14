#ifndef HTTP_H
#define HTTP_H

#include <Arduino.h>
#include "Models.h"
#include <ArduinoJson.h>
#include <vector>
#include <base64.h>
#include <Preferences.h>
#include <HTTPClient.h>

#define HTTP_PORT 8000
class HTTP{

private:
    HTTPClient http;

    Credentials creds;

    

public:
    void updateCreds();
    bool testConnection();
    bool registerUser();
    bool testLogin();
    String getAuthHeader();
    std::vector<Recording> fetchRecordings(const char* date);
    std::vector<Recording> fetchTodayRecordings();
    JsonDocument fetchTodayPrompt();
    bool submitPrompt(String prompt);
};

#endif //HTTP_H