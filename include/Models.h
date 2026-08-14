#pragma once
#include <Arduino.h>

struct Recording {
    String username;
    time_t timestamp;
    int length;
};

struct Credentials {
    String user;
    String pass;
    String server;
    String invite;
};
