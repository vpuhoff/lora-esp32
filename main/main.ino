#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LittleFS.h>
#include <GyverDBFile.h>
#include <SettingsESPWS.h>
#include <GTimer.h>

// Подключение всех модулей
#include "config.h"
#include "logging.h" 
#include "led.h"
#include "lora_module.h"
#include "statistics.h"
#include "tasks.h"
#include "display-manager.h"
#include "system-monitor.h"


// Модули веб-интерфейса
#include "esp32-config.h"

#include "wifi-manager.h"    // В этом файле объявлен extern WiFiManager* wifiManager;
#include "lora-manager.h"    // В этом файле объявлен extern LoRaManager* loraManager;
#include "plot-manager.h"
#include "ui-builder.h"

// Объявляем глобальный указатель на UIBuilder
UIBuilder* uiBuilder = nullptr;
DisplayManager* displayManager = nullptr;
SystemMonitor* systemMonitor = nullptr;

// Создаем базу данных для хранения настроек
GyverDBFile db(&LittleFS, "/settings.db");

// Создаем объект настроек с поддержкой WebSocket
SettingsESPWS sett("ESP32 LoRa Control Panel", &db);

// Функция-обработчик построения интерфейса
void buildInterface(sets::Builder& b) {
    uiBuilder->buildInterface(b);
}

void setup() {
    Serial.begin(115200);
    logger.println();
    
    // Инициализация LED
    setupLed();
    
    // Инициализация файловой системы
    if (!LittleFS.begin(true)) {
        logger.println("Ошибка инициализации LittleFS");
    }
    
    // Инициализация базы данных
    db.begin();
    
    // Создание и инициализация менеджеров
    wifiManager = new WiFiManager(&db);
    loraManager = new LoRaManager(&db);
    uiBuilder = new UIBuilder(&db);
    systemMonitor = new SystemMonitor(); // Создаем системный монитор
    displayManager = new DisplayManager(&db); // Создаем менеджер дисплея
    
    // Инициализация значений по умолчанию
    wifiManager->initDefaults();
    loraManager->initDefaults();
    displayManager->initDefaults(); // Инициализация настроек дисплея
    db.init(DB_NAMESPACE::log_level, LOG_INFO);

    // Настройка дисплея
    if (displayManager->setupDisplay()) {
        displayManager->showLogo(); // Показываем заставку
        logger.println("Дисплей инициализирован успешно");
    } else {
        logger.println(error_() + "Ошибка инициализации дисплея");
    }
    
    // Применение настроек WiFi
    wifiManager->applySettings();
    
    // Инициализация LoRa
    if (!setupLoRa()) {
        logger.println("LoRa init failed permanently.");
        logger.println("LoRa init failed permanently. Blinking indefinitely...");
        if (displayManager->isEnabled()) {
            displayManager->showError("LoRa initialization failed!");
        }
        while (true) {
            blinkLED(1, 200);
        }
    }
    logger.println("LoRa started successfully!");
    logger.println("LoRa started successfully!");
    blinkLED(3, 100);
    
    // Инициализация веб-сервера
    sett.begin();
    sett.onBuild(buildInterface);
    
    // Создание мьютекса для LoRa
    loraMutex = xSemaphoreCreateMutex();
    
    // Создание задач
    createTasks();
    
    logger.println("Система инициализирована");

    // Показываем информационное сообщение на дисплее
    if (displayManager->isEnabled()) {
        displayManager->showInfo("System initialized successfully!", 3000);
    }
}

void loop() {
    static GTimer<millis> tmr(1000, true);
    if (tmr) {
        sett.updater()
            .update(H("logger"), logger);
            //.update(H(lbl2), random(100));
    }
    vTaskDelay(50);
}