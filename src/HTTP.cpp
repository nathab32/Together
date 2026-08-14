#include "HTTP.h"

void HTTP::updateCreds() {
    Preferences pref;
    pref.begin("TogetherCreds", true);

    creds.user = pref.getString("username");
    creds.pass = pref.getString("password");
    creds.server = pref.getString("server");
    creds.invite = pref.getString("invite");

    pref.end();

    http.setAuthorization(creds.user.c_str(), creds.pass.c_str());
}

bool HTTP::testConnection() {
    http.begin(creds.server.c_str(), HTTP_PORT, "/verify_auth");
    int httpCode = http.sendRequest("HEAD");
    http.end();
    if(httpCode < 0){
        Serial.println("testConnection: Couldn't connect to server.");
        return false;
    } else {
        Serial.println("testConnection: Connected to server");
        return true;
    }
    
    
}

bool HTTP::registerUser(){
    http.begin(creds.server.c_str(), HTTP_PORT, "/register");
        

    JsonDocument doc;
    doc["username"] = creds.user;
    doc["password"] = creds.pass;
    doc["invite_code"] = creds.invite;

    String jsonBody;
    serializeJson(doc, jsonBody);

    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonBody);
    http.end();
    if (httpCode == 201) return true;
    else if (httpCode < 0) {
        Serial.println("registerUser: Couldn't connect to server.");
        return false;
    }
    else {
        Serial.println("registerUser failed");
        return false;
    }
   
}

bool HTTP::testLogin(){
    http.begin(creds.server.c_str(), HTTP_PORT, "/verify_auth");

    int httpCode = http.sendRequest("HEAD");

    // http.println("HEAD /verify_auth HTTP/1.1");
    // http.println("Host: " + creds.server);
    // http.println(getAuthHeader());
    // http.println("Connection: close");
    // http.println();

    // unsigned long timeout = millis();
    // while(http.begined() && millis() - timeout < 2000){
    //     if(http.available()){
    //         String line = http.readStringUntil('\n');
    //         if(line.startsWith("HTTP/1.1 200")){
    //             http.end();
    //             Serial.println("Login successful");
    //             return true;
    //         } else if (line.startsWith("HTTP/1.1 401")){
    //             http.end();
    //             Serial.println("Login failed: 401 unauthorized");
    //             return false;
    //         }
    //     }
    // }
    http.end();
    if (httpCode == 200) {
        Serial.println("Server login successful");
        return true;
    } else if (httpCode < 0) {
        Serial.println("testLogin(): Couldn't connect to server.");
        return false;    
    } else {
        Serial.println("Server login unsuccessful");
        return false;
    }
    
}

String HTTP::getAuthHeader(){
    return "Authorization: Basic " + base64::encode(creds.user + ":" + creds.pass);
}

//Need to fix
std::vector<Recording> HTTP::fetchRecordings(const char* date) {
    std::vector<Recording> fileList;
    http.begin(creds.server.c_str(), HTTP_PORT, "/list_recordings");

    String jsonResponse = "{}";
    int httpCode = http.GET();
    http.end();

    if(httpCode == 200) {
        jsonResponse = http.getString();
    } else if (httpCode < 0) {
        Serial.println("fetchFileList(): Connection to server failed");
        return fileList;
    }
    
    
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
        // fileList.push_back(name.c_str());
    }
    return fileList;
}

std::vector<Recording> HTTP::fetchTodayRecordings() {
    std::vector<Recording> recordings;
    if (!http.begin(creds.server.c_str(), HTTP_PORT, "/list_today_recordings")){
        Serial.println("submitPrompt(): http initialization failed");
        return recordings;
    }

    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String payload = http.getString();
        // Serial.println(payload);
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("fetchTodayRecordings: ");
            Serial.println(error.c_str());
            return recordings;
        }
        JsonArray arr = doc.as<JsonArray>();

        for (JsonObject obj : arr) {
            Recording rec;
            rec.username = obj["username"].as<String>();
            rec.timestamp = obj["timestamp"].as<long>();
            rec.length = obj["duration"].as<int>();
            
            recordings.push_back(rec);
        }
    }
    http.end();
    return recordings;
}

JsonDocument HTTP::fetchTodayPrompt()
{
    JsonDocument doc;
    http.begin(creds.server.c_str(), HTTP_PORT, "/get_prompt");


    // http.println("GET /get_prompt HTTP/1.1");
    // http.println("Host: " + creds.server);
    // http.println(getAuthHeader());
    // http.println("Connection: close");
    // http.println();

    int httpCode = http.GET();
    String payload = http.getString();
    http.end();

    if (httpCode == 200) {
        deserializeJson(doc, payload);
    } else if (httpCode < 0) {
        Serial.println("fetchFileList(): Connection to server failed");
    }

    return doc;
}

bool HTTP::submitPrompt(String prompt) {
    JsonDocument doc;
    doc["prompt"] = prompt;

    String jsonBody;
    serializeJson(doc, jsonBody);

    if (!http.begin(creds.server.c_str(), HTTP_PORT, "/submit_prompt")){
        Serial.println("submitPrompt(): http initialization failed");
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonBody);
    http.end();

    if (httpCode == 200) {
        Serial.println("Prompt submitted");
        return true;
    } else if (httpCode < 0) {
        Serial.println("submitPrompt(): Connection to server failed");
    }
    return false;
}