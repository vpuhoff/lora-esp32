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

    float newValue = success ? 100.0 : 0.0;
    successRateSmoothed = (ALPHA * newValue) + ((1 - ALPHA) * successRateSmoothed);

    float totalSuccessRate = (totalReceived / (float)totalSent) * 100.0;
    Serial.printf("Smoothed success rate: %.2f%% | Overall success rate: %.2f%% (%d/%d)\n", 
                  successRateSmoothed, totalSuccessRate, totalReceived, totalSent);
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
        
        float successRate = (totalReceived / (float)totalSent) * 100.0;
        Serial.printf("Updated overall success rate: %.2f%% (%d/%d)\n", 
                      successRate, totalReceived, totalSent);
    }
}