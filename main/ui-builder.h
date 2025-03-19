#pragma once
#include <Arduino.h>
#include <GyverDB.h>
#include <SettingsESPWS.h>
#include "esp32-config.h"
#include "logging.h"
#include "wifi-manager.h"
#include "lora-manager.h"
#include "plot-manager.h"
#include "system-monitor.h"
#include "display-manager.h"

class UIBuilder {
public:
    UIBuilder(GyverDB* db);
    void buildInterface(sets::Builder& b);
    bool needsRestart() const;
private:
    void buildDashboardTab(sets::Builder& b);
    void buildLogsTab(sets::Builder& b);
    void buildLoRaStatusTab(sets::Builder& b);
    void buildSettingsTab(sets::Builder& b);
    void buildDisplayTab(sets::Builder& b); // Добавляем новый метод
    void buildSystemMonitorTab(sets::Builder& b); // Добавляем метод для мониторинга системы

    GyverDB* _db;
    bool _needRestart;
};
