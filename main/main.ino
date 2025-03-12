#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>
#include <GyverDBFile.h>
#include <SettingsESPWS.h>

// Подключение всех модулей
#include "config.h"
#include "led.h"
#include "lora_module.h"
#include "statistics.h"
#include "tasks.h"

// Модули веб-интерфейса
#include "esp32-config.h"
#include "logger.h"
#include "wifi-manager.h"    // В этом файле объявлен extern WiFiManager* wifiManager;
#include "lora-manager.h"    // В этом файле объявлен extern LoRaManager* loraManager;
#include "plot-manager.h"
#include "ui-builder.h"

// Объявляем глобальный указатель на UIBuilder
UIBuilder* uiBuilder = nullptr;

// Создаем базу данных для хранения настроек
GyverDBFile db(&LittleFS, "/settings.db");

// Создаем объект настроек с поддержкой WebSocket
SettingsESPWS sett("ESP32 LoRa Control Panel", &db);

// Функция-обработчик построения интерфейса
void buildInterface(sets::Builder& b) {
    uiBuilder->buildInterface(b);
}

// Задача для обработки веб-интерфейса
void taskWebInterface(void *parameter) {
    for (;;) {
        sett.tick();
        
        // Обновление данных для графика каждые 500 мс
        static uint32_t plotTimer = 0;
        if (millis() - plotTimer >= 500) {
            plotTimer = millis();
            plotManager.updateData();
        }
        
        // Периодическое обновление данных LoRa каждые 5000 мс
        static uint32_t loraTimer = 0;
        if (millis() - loraTimer >= 5000) {
            loraTimer = millis();
            loraManager->updateStats();
        }
        
        // Периодическое логирование состояния системы каждые 10000 мс
        static uint32_t logTimer = 0;
        if (millis() - logTimer >= 10000) {
            logTimer = millis();
            if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
                if (WiFi.status() == WL_CONNECTED) {
                    logger.add("WiFi подключен к " + WiFi.SSID() + ", сигнал: " + String(WiFi.RSSI()) + " dBm", LOG_DEBUG);
                }
            }
            logger.add("Свободная память: " + String(ESP.getFreeHeap()) + " байт", LOG_DEBUG);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Небольшая задержка для экономии ресурсов
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    
    // Инициализация LED
    setupLed();
    
    // Инициализация файловой системы
    if (!LittleFS.begin(true)) {
        Serial.println("Ошибка инициализации LittleFS");
    }
    
    // Инициализация базы данных
    db.begin();
    
    // Создание и инициализация менеджеров
    wifiManager = new WiFiManager(&db);
    loraManager = new LoRaManager(&db);
    uiBuilder = new UIBuilder(&db);
    
    // Инициализация значений по умолчанию
    wifiManager->initDefaults();
    loraManager->initDefaults();
    db.init(DB_NAMESPACE::log_level, LOG_INFO);
    
    // Применение настроек WiFi
    wifiManager->applySettings();
    
    // Инициализация LoRa
    if (!setupLoRa()) {
        logger.add("LoRa init failed permanently.", LOG_ERROR);
        Serial.println("LoRa init failed permanently. Blinking indefinitely...");
        while (true) {
            blinkLED(1, 200);
        }
    }
    logger.add("LoRa started successfully!");
    Serial.println("LoRa started successfully!");
    blinkLED(3, 100);
    
    // Инициализация веб-сервера
    sett.begin();
    sett.onBuild(buildInterface);
    
    // Создание мьютекса для LoRa
    loraMutex = xSemaphoreCreateMutex();
    
    // Создание задач
    xTaskCreatePinnedToCore(taskWebInterface, "WebInterface", 8192, NULL, 1, NULL, 0);
    createTasks();
    
    logger.add("Система инициализирована");
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
