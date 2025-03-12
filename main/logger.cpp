#include "logger.h"

// Глобальный экземпляр логгера
Logger logger;

// Конструктор
Logger::Logger(size_t maxBufferSize) 
    : _maxBufferSize(maxBufferSize), _isUpdated(false), _currentLogLevel(LOG_INFO) {
}

// Добавление записи в лог
void Logger::add(const String& message, int level) {
    static const char* levelNames[] = {"INFO", "WARN", "ERROR", "DEBUG"};
    
    // Проверка уровня логирования
    if (!shouldLog(level)) {
        return;
    }
    
    // Ограничиваем размер буфера
    if (_buffer.length() > _maxBufferSize) {
        // Оставляем только последние 50% символов
        _buffer = _buffer.substring(_buffer.length() - (_maxBufferSize / 2));
    }
    
    // Формируем запись лога
    String entry = String("[") + levelNames[constrain(level, 0, 3)] + "] " + 
                  String(millis()/1000) + "s: " + message + "\n";
    
    // Добавляем в буфер
    _buffer += entry;
    
    // Выводим в Serial
    Serial.print(entry);
    
    // Устанавливаем флаг обновления
    _isUpdated = true;
}

// Очистка буфера логов
void Logger::clear() {
    _buffer = "";
    _isUpdated = true;
}

// Получение содержимого буфера
const String& Logger::getBuffer() const {
    return _buffer;
}

// Проверка обновления и сброс флага
bool Logger::checkAndResetUpdate() {
    bool result = _isUpdated;
    _isUpdated = false;
    return result;
}

// Проверка уровня логирования
bool Logger::shouldLog(int level) {
    return level <= _currentLogLevel || level == LOG_ERROR;
}

// Установка текущего уровня логирования
void Logger::setLogLevel(int level) {
    _currentLogLevel = constrain(level, LOG_INFO, LOG_DEBUG);
}

// Получение текущего уровня логирования
int Logger::getLogLevel() const {
    return _currentLogLevel;
}