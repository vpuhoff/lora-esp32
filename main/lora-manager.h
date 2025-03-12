#pragma once
#include <Arduino.h>
#include <GyverDB.h>
#include "esp32-config.h"
#include "logger.h"

class LoRaManager {
public:
    LoRaManager(GyverDB* db);
    
    // Применение настроек LoRa из базы данных
    void applySettings();
    
    // Инициализация значений LoRa по умолчанию
    void initDefaults();
    
    // Обновление статистических данных
    void updateStats();
    
    // Проверка обновления и сброс флага
    bool checkAndResetUpdate();
    
    // Геттеры для доступа к данным
    int getSpreadingFactor() const;
    float getBandwidth() const;
    int getCodingRate() const;
    int getMaxAttempts() const;
    int getTxPower() const;
    
    uint32_t getPacketsTotal() const;
    uint32_t getPacketsSuccess() const;
    float getLastRssi() const;
    
    // Получение процента успешной доставки
    int getSuccessRate() const;

private:
    GyverDB* _db;
    
    // Текущие настройки
    int _spreading;
    float _bandwidth;
    int _codingRate;
    int _maxAttempts;
    int _txPower;
    
    // Статистика
    uint32_t _packetsTotal;
    uint32_t _packetsSuccess;
    float _lastRssi;
    bool _isDataUpdated;
};

// Глобальный экземпляр менеджера LoRa
extern LoRaManager* loraManager;