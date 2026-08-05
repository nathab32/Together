#ifndef HTTP_H
#define HTTP_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <base64.h>
#include <Preferences.h>

class HTTP{

private:
    WiFiClient http;

    struct Credentials {
        String user;
        String pass;
        String server;
        String invite;
    };

    Credentials creds;

public:
    void updateCreds();
    bool registerUser();
    bool testLogin();
    String getAuthHeader();
    std::vector<String> fetchFileList();
};

#endif //HTTP_H