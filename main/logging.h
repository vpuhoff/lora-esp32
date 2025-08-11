#pragma once // Добавляем эту строку

#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

class SimpleLogger {
public:
    template <typename T>
    void println(const T& msg) { Serial.println(msg); }
};

// Объявляем внешний объект логгера
extern SimpleLogger logger;

// Короткие шаблоны для префиксов логов
inline String info_() { return String("info: "); }
inline String warn_() { return String("warn: "); }
inline String error_() { return String("err: "); }

#endif // LOGGING_H