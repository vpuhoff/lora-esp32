#pragma once
#include <Arduino.h>
#include <GyverDB.h> // Для определения DB_KEYS

// Настройки приложения по умолчанию
#define DEFAULT_AP_SSID "ESP32_LoRa"
#define DEFAULT_AP_PASS "12345678"

// Максимальное количество точек на графике
#define MAX_PLOT_POINTS 100

// Идентификаторы для полей базы данных
#define DB_NAMESPACE lora_config

// Определение ключей базы данных
DB_KEYS(
    DB_NAMESPACE,
    // Настройки WiFi
    wifi_mode,        // Режим WiFi (0 - AP, 1 - STA, 2 - AP+STA)
    ap_ssid,          // SSID точки доступа
    ap_pass,          // Пароль точки доступа
    sta_ssid,         // SSID для подключения
    sta_pass,         // Пароль для подключения
    apply_wifi,       // Кнопка применения настроек WiFi
    
    // Настройки логгера
    log_level,        // Уровень логирования
    clear_log,        // Кнопка очистки лога
    
    // Управление устройством
    restart_device,   // Кнопка перезагрузки устройства
    
    // Настройки LoRa
    lora_spreading,   // Spreading Factor (коэффициент расширения)
    lora_bandwidth,   // Bandwidth (полоса пропускания)
    lora_coding_rate, // Coding Rate (скорость кодирования)
    lora_spreading_selected,   // Spreading Factor (коэффициент расширения) (номер выбранного поля)
    lora_bandwidth_selected,   // Bandwidth (полоса пропускания) (номер выбранного поля)
    lora_coding_rate_selected, // Coding Rate (скорость кодирования) (номер выбранного поля)
    lora_max_attempts,// Максимальное количество попыток
    lora_tx_power,    // Мощность передачи
    apply_lora        // Кнопка применения настроек LoRa
);

// Уровни логирования
enum LogLevel {
    LOG_INFO = 0,
    LOG_WARN = 1,
    LOG_ERROR = 2,
    LOG_DEBUG = 3
};

// Режимы WiFi
// Переименованные режимы Wi-Fi (чтобы избежать конфликта с esp_wifi_types.h)
enum MyWiFiMode {
    MY_WIFI_MODE_AP = 0,
    MY_WIFI_MODE_STA = 1,
    MY_WIFI_MODE_AP_STA = 2
};

