#pragma once // Добавляем эту строку

#ifndef LOGGING_H
#define LOGGING_H

#include <SettingsESPWS.h>

// Объявляем внешний объект логгера
extern sets::Logger logger;

// Короткие шаблоны для префиксов логов
inline String info_() { return sets::Logger::info(); }
inline String warn_() { return sets::Logger::warn(); }
inline String error_() { return sets::Logger::error(); }

#endif // LOGGING_H