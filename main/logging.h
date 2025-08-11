#pragma once

#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

// Simple logger that prints messages to the Serial port
class SimpleLogger {
public:
    template <typename T>
    void println(const T& msg) { Serial.println(msg); }

    static String info() { return "[INFO] "; }
    static String warn() { return "[WARN] "; }
    static String error() { return "[ERROR] "; }
};

// Global logger instance declaration
extern SimpleLogger logger;

// Short templates for log prefixes
inline String info_() { return SimpleLogger::info(); }
inline String warn_() { return SimpleLogger::warn(); }
inline String error_() { return SimpleLogger::error(); }

#endif // LOGGING_H