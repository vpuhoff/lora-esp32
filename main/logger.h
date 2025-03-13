#pragma once
#include <Arduino.h>
#include "esp32-config.h"

class Logger {
public:
    Logger(size_t maxBufferSize = 10000);
    
    // Добавление записи в лог
    void add(const String& message, int level = LOG_INFO);
    
    // Очистка буфера логов
    void clear();
    
    // Получение содержимого буфера
    const String& getBuffer() const;
    
    // Проверка обновления и сброс флага
    bool checkAndResetUpdate();
    
    // Проверка уровня логирования
    bool shouldLog(int level);
    
    // Установка текущего уровня логирования
    void setLogLevel(int level);
    
    // Получение текущего уровня логирования
    int getLogLevel() const;

private:
    String _buffer;              // Буфер для хранения логов
    size_t _maxBufferSize;       // Максимальный размер буфера
    bool _isUpdated;             // Флаг обновления лога
    int _currentLogLevel;        // Текущий уровень логирования
};

// Глобальный экземпляр логгера
extern Logger logger;