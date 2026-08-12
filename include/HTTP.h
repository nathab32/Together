#ifndef HTTP_H
#define HTTP_H

#include <Arduino.h>
// #include <WiFiClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <base64.h>
#include <Preferences.h>
#include <HTTPClient.h>

#define HTTP_PORT 8000
class HTTP{

private:
    HTTPClient http;

    struct Credentials {
        String user;
        String pass;
        String server;
        String invite;
    };

    Credentials creds;

public:
    void updateCreds();
    bool testConnection();
    bool registerUser();
    bool testLogin();
    String getAuthHeader();
    std::vector<String> fetchFileList();
    JsonDocument fetchTodayPrompt();
    bool submitPrompt(String prompt);
};

#endif //HTTP_H