#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <GyverDB.h>
#include "esp32-config.h"
#include "logger.h"

class WiFiManager {
public:
    WiFiManager(GyverDB* db);
    void applySettings();
    String getStatusText();
    void initDefaults();
private:
    GyverDB* _db;
};

extern WiFiManager* wifiManager;
