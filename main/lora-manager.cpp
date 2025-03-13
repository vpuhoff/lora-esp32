#include "lora-manager.h"
#include "statistics.h"
#include <LoRa.h>

LoRaManager* loraManager = nullptr;

LoRaManager::LoRaManager(GyverDB* db) : _db(db) {
    _packetsTotal = 0;
    _packetsSuccess = 0;
    _lastRssi = -120.0;
    _isDataUpdated = false;
}

void LoRaManager::applySettings() {
    _spreading = _db->get(DB_NAMESPACE::lora_spreading).toInt();
    _bandwidth = _db->get(DB_NAMESPACE::lora_bandwidth).toFloat();
    _codingRate = _db->get(DB_NAMESPACE::lora_coding_rate).toInt();
    _maxAttempts = _db->get(DB_NAMESPACE::lora_max_attempts).toInt();
    _txPower = _db->get(DB_NAMESPACE::lora_tx_power).toInt();
    
    logger.add("Применение настроек LoRa...");
    
    if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(5000))) {
        logger.add(String("Spreading:") + _spreading);
        logger.add(String("Bandwidth:") + _bandwidth);
        logger.add(String("CodingRate:") + _codingRate);
        logger.add(String("TxPower:") + _txPower);
        LoRa.setSpreadingFactor(_spreading);
        LoRa.setSignalBandwidth(_bandwidth * 1000);
        LoRa.setCodingRate4(_codingRate);
        LoRa.setTxPower(_txPower);
        xSemaphoreGive(loraMutex);
        logger.add("Настройки LoRa применены");
    } else {
        logger.add("Не удалось захватить мьютекс", LOG_ERROR);
    }
    
}


// Инициализация значений LoRa по умолчанию
void LoRaManager::initDefaults() {
    _db->init(DB_NAMESPACE::lora_spreading, LORA_SPREADING);    // SF12
    _db->init(DB_NAMESPACE::lora_bandwidth, LORA_BANDWIDTH/1000); // 31.25 kHz
    _db->init(DB_NAMESPACE::lora_coding_rate, LORA_CODING_RATE); // 4/8
    _db->init(DB_NAMESPACE::lora_max_attempts, LORA_MAX_ATTEMPTS); // 5 попыток
    _db->init(DB_NAMESPACE::lora_tx_power, LORA_TX_POWER);      // 10 dBm
    _db->init(DB_NAMESPACE::lora_spreading_selected, 0);  
    _db->init(DB_NAMESPACE::lora_bandwidth_selected, 0); 
    _db->init(DB_NAMESPACE::lora_coding_rate_selected, 0); 

    _spreading = _db->get(DB_NAMESPACE::lora_spreading).toInt();
    _bandwidth = _db->get(DB_NAMESPACE::lora_bandwidth).toFloat();
    _codingRate = _db->get(DB_NAMESPACE::lora_coding_rate).toInt();
    _maxAttempts = _db->get(DB_NAMESPACE::lora_max_attempts).toInt();
    _txPower = _db->get(DB_NAMESPACE::lora_tx_power).toInt();
}

// Обновление статистических данных из глобальных переменных статистики
void LoRaManager::updateStats() {
    _packetsTotal = totalSent;
    _packetsSuccess = totalReceived;
    _lastRssi = -100; // Здесь можно использовать реальные данные RSSI из последнего пакета
    _isDataUpdated = true;
}

// Проверка обновления и сброс флага
bool LoRaManager::checkAndResetUpdate() {
    bool result = _isDataUpdated;
    _isDataUpdated = false;
    return result;
}

// Геттеры для доступа к данным
int LoRaManager::getSpreadingFactor() const { 
    return _spreading; 
}

float LoRaManager::getBandwidth() const { 
    return _bandwidth; 
}

int LoRaManager::getCodingRate() const { 
    return _codingRate; 
}

int LoRaManager::getMaxAttempts() const { 
    return _maxAttempts; 
}

int LoRaManager::getTxPower() const { 
    return _txPower; 
}

uint32_t LoRaManager::getPacketsTotal() const { 
    return _packetsTotal; 
}

uint32_t LoRaManager::getPacketsSuccess() const { 
    return _packetsSuccess; 
}

float LoRaManager::getLastRssi() const { 
    return _lastRssi; 
}

// Получение процента успешной доставки
int LoRaManager::getSuccessRate() const {
    if (_packetsTotal == 0) return 0;
    return (_packetsSuccess * 100) / _packetsTotal;
}