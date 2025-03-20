#include "statistics.h"

// Инициализация глобальных переменных
int totalSent = 0;
int totalReceived = 0;
float successRateSmoothed = 0.0;

// Инициализация переменных для отслеживания пакетов
int packetId = 0;
bool packetStatus[100] = {false}; // Для хранения статуса пакетов

void updateStats(bool success) {
    totalSent++;
    if (success) totalReceived++;

    // Вычисляем текущую успешность доставки в процентах
    float currentSuccessRate = (totalReceived * 100.0) / totalSent;
    
    // Применяем сглаживание с использованием EWMA
    successRateSmoothed = (ALPHA * currentSuccessRate) + ((1 - ALPHA) * successRateSmoothed);

    Serial.printf("Smoothed success rate: %.2f%% | Overall success rate: %.2f%% (%d/%d)\n", 
                  successRateSmoothed, currentSuccessRate, totalReceived, totalSent);
}

void updatePacketStatus(int id, bool success) {
    if (id < 100) { // Проверка границ массива
        packetStatus[id] = success;
        
        // Пересчитываем общую статистику
        int received = 0;
        for (int i = 0; i < packetId; i++) {
            if (packetStatus[i]) received++;
        }
        
        totalReceived = received;
        totalSent = packetId;
        
        // Обновляем сглаженную статистику после пересчета
        float successRate = (totalReceived * 100.0) / totalSent;
        successRateSmoothed = (ALPHA * successRate) + ((1 - ALPHA) * successRateSmoothed);
        
        Serial.printf("Updated overall success rate: %.2f%% (%d/%d)\n", 
                      successRate, totalReceived, totalSent);
    }
}