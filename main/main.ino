#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LittleFS.h>
#include <GyverDBFile.h>
#include <WiFiClient.h>

// Blynk credentials (replace with real values)
#define BLYNK_TEMPLATE_ID "TMPLXXXXXX"
#define BLYNK_DEVICE_NAME "ESP32 LoRa Control Panel"
#define BLYNK_AUTH_TOKEN  "YOUR_BLYNK_AUTH_TOKEN"

#include <BlynkSimpleEsp32.h>
#include "esp_task_wdt.h"

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

DisplayManager* displayManager = nullptr;
SystemMonitor* systemMonitor = nullptr;

// Создаем базу данных для хранения настроек
GyverDBFile db(&LittleFS, "/settings.db");


void setup() {
    Serial.begin(115200);
    logger.println();

    // Инициализация watchdog таймера для задач
    esp_task_wdt_init(60, true);

    // Создание мьютекса для SPI
    spi_lock_mutex = xSemaphoreCreateMutex();
    
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
    systemMonitor = new SystemMonitor(); // Создаем системный монитор
    displayManager = new DisplayManager(&db); // Создаем менеджер дисплея
    
    // Инициализация значений по умолчанию
    wifiManager->initDefaults();
    loraManager->initDefaults();
    displayManager->initDefaults(); // Инициализация настроек дисплея
    db.init(DB_NAMESPACE::log_level, LOG_INFO);

    #if DISPLAY_ENABLED
    displayManager = new DisplayManager(&db); // Создаем менеджер дисплея только для ESP32
    // Инициализация дисплея
    displayManager->initDefaults();
    if (displayManager->setupDisplay()) {
        displayManager->showLogo(); // Показываем заставку
        logger.println("Дисплей инициализирован успешно");
    } else {
        logger.println(error_() + "Ошибка инициализации дисплея");
    }
    #endif
    // Применение настроек WiFi
    wifiManager->applySettings();
    
    // Инициализация LoRa
    if (!setupLoRa()) {
        logger.println("LoRa init failed permanently.");
        logger.println("LoRa init failed permanently. Blinking indefinitely...");
        #if DISPLAY_ENABLED
        if (displayManager->isEnabled()) {
            displayManager->showError("LoRa initialization failed!");
        }
        #endif
        while (true) {
            blinkLED(1, 200);
        }
    } else {
      loraManager->applySettings();
    }
    
    logger.println("LoRa started successfully!");
    logger.println("LoRa started successfully!");
    blinkLED(3, 100);

    // Инициализация Blynk
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();

    // Создание задач
    createTasks();
    
    logger.println("Система инициализирована");

    #if DISPLAY_ENABLED
    // Показываем информационное сообщение на дисплее
    if (displayManager->isEnabled()) {
        displayManager->showInfo("System initialized successfully!", 3000);
    }
    #endif
}

void loop() {
    static bool wdt_added = false;
    if (!wdt_added) {
        esp_task_wdt_add(NULL);
        wdt_added = true;
    }

    esp_task_wdt_reset();
    vTaskDelay(500);
}