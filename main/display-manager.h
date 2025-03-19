#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <GyverDB.h>
#include "config.h"
#include "logging.h"
#include "esp32-config.h"

// Определение страниц дисплея
enum DisplayPage {
    PAGE_LOGO = 0,
    PAGE_LORA_STATUS,
    PAGE_WIFI_STATUS,
    PAGE_SYSTEM_INFO,
    PAGE_LOGS,
    PAGE_COUNT
};

class DisplayManager {
public:
    DisplayManager(GyverDB* db);
    ~DisplayManager();
    
    // Инициализация и настройки
    bool setupDisplay();
    void initDefaults();
    void applySettings();
    
    // Управление дисплеем
    void clear();
    void setBrightness(uint8_t brightness);
    void enableDisplay(bool enable);
    bool isEnabled() const;
    
    // Страницы и отображение
    void nextPage();
    void prevPage();
    void setPage(DisplayPage pageIndex);
    DisplayPage getCurrentPage() const;
    void updateCurrentPage();
    
    // Отображение информации
    void showInfo(String text, int duration = 3000);
    void showError(String text, int duration = 5000);
    void showLogo();
    void showLoRaStatus();
    void showWiFiStatus();
    void showSystemInfo();
    void showLogs();
    
    // Автоматическое обновление
    void tick();
    bool checkAndResetUpdate();

private:
    Adafruit_ST7735* _display;
    GyverDB* _db;
    
    DisplayPage _currentPage;
    bool _enabled;
    uint8_t _brightness;
    bool _autoScroll;
    uint32_t _scrollInterval;
    uint32_t _lastScrollTime;
    uint32_t _lastUpdateTime;
    bool _needUpdate;
    
    // Временный текст для отображения сообщений
    String _tempMessage;
    uint32_t _tempMessageTime;
    uint32_t _tempMessageDuration;
    bool _isError;
    
    // Вспомогательные методы
    void drawStatusBar();
    void drawPageIndicator();
    void handleBacklight();
};

// Глобальный экземпляр менеджера дисплея
extern DisplayManager* displayManager;