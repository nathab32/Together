#include "HTTP.h"

void HTTP::updateCreds(){
    Preferences pref;
    pref.begin("TogetherCreds", true);

    creds.user = pref.getString("username");
    creds.pass = pref.getString("password");
    creds.server = pref.getString("server");
    creds.invite = pref.getString("invite");

    pref.end();
}

bool HTTP::registerUser(){
    if(!http.connect(creds.server.c_str(), 8000)){
        Serial.println("registerUser: Couldn't connect to server.");
        return false;
    }

    JsonDocument doc;
    doc["username"] = creds.user;
    doc["password"] = creds.pass;
    doc["invite_code"] = creds.invite;

    String jsonBody;
    serializeJson(doc, jsonBody);

    http.println("POST /register HTTP/1.1");
    http.println("Host: " + creds.server);
    http.println("Content-Type: application/json");
    http.print("Content-Length: "); http.println(jsonBody.length());
    http.println("Connection: close");
    http.println();
    http.print(jsonBody);

    unsigned long timeout = millis();
    while (http.connected() && millis() - timeout < 5000) {
        if (http.available()) {
            String line = http.readStringUntil('\n');
            if (line.startsWith("HTTP/1.1 201")) { // 201 Created
                http.stop();
                Serial.println("New user registered");
                return true;
            }
        }
    }
    http.stop();
    Serial.println("registerUser failed");
    return false;
}

bool HTTP::testLogin(){
    if(!http.connect(creds.server.c_str(), 8000)){
        Serial.println("testLogin(): Couldn't connect to server.");
        return false;
    }

    http.println("HEAD /verify_auth HTTP/1.1");
    http.println("Host: " + creds.server);
    http.println(getAuthHeader());
    http.println("Connection: close");
    http.println();

    unsigned long timeout = millis();
    while(http.connected() && millis() - timeout < 2000){
        if(http.available()){
            String line = http.readStringUntil('\n');
            if(line.startsWith("HTTP/1.1 200")){
                http.stop();
                Serial.println("Login successful");
                return true;
            } else if (line.startsWith("HTTP/1.1 401")){
                http.stop();
                Serial.println("Login failed: 401 unauthorized");
                return false;
            }
        }
    }
    http.stop();
    return false;
}

String HTTP::getAuthHeader(){
    return "Authorization: Basic " + base64::encode(creds.user + ":" + creds.pass);
}

std::vector<String> HTTP::fetchFileList(){
    std::vector<String> fileList;
    if (!http.connect(creds.server.c_str(), 8000)){
        Serial.println("fetchFileList(): Connection to server failed");
        return fileList;
    }
    http.println("GET /list_recordings HTTP/1.1");
    http.println("Host: " + creds.server);
    http.println(getAuthHeader());
    http.println("Connection: close");
    http.println();

    String jsonResponse = "";
    bool foundStart = false;
    unsigned long timeout = millis();
    while (true) {
        if(http.available())
        {
            char c = http.read();
            if (c == '['){
                jsonResponse += c;
                foundStart = true;
            }

            if (foundStart) {
                jsonResponse += http.readStringUntil(']');
                jsonResponse += ']';
                break;
            }
        }
        if(millis() - timeout > 2000) break;
    }
    http.stop();
    Serial.println("Raw response: " + jsonResponse);
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    

    if (error){
        Serial.print("fetchFileList(): ");
        Serial.println(error.c_str());
        
        return fileList;
    }

    JsonArray arr = doc.as<JsonArray>();
    for (JsonString name : arr) {
        fileList.push_back(name.c_str());
    }
    return fileList;
}